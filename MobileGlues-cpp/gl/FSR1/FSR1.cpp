// MobileGlues - gl/FSR1/FSR1.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
//   https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header
//
// mg-3backends FSR1: Arm Accuracy Super Resolution (FFXM FSR1), two passes.
//
//   pass 1  EASU : g_renderTexture (render res) -> g_targetTexture (surface res)
//   pass 2  RCAS : g_targetTexture (surface res) -> real framebuffer 0 (surface)
//
// This replaces the old single-pass AMD port, which had three fatal defects:
// its one fragment shader computed EASU and then discarded the result (RCAS
// sampled the raw input texture), its textureGather-based EASU could not be
// transpiled to ESSL 300 at all (SPIRV-Cross rejects OpImageGather below ESSL
// 310, so the raw GLSL 450 source reached the driver and never compiled), and
// its resolution bookkeeping let the surface query overwrite the render size
// every frame, which reduced FSR to a wasted up- and downscale. The Arm
// upgrade is only worth what the plumbing around it is; the plumbing below is
// what makes the presets real.
#include "FSR1.h"
#include <atomic>
#include <cstdio>
#include <mutex>
#include <ska/flat_hash_map.hpp>
#include "FSRShaderSource.h"
#include "../../config/settings.h"

#define DEBUG 0

// Which pieces of GL state a body in this file overwrites. Saving the rest is not
// free: everything the guard cannot answer from this layer's own tracking is a
// driver round trip, and the upscale runs once per presented frame.
enum GLStateBits : unsigned int {
    GUARD_PROGRAM = 1u << 0,
    GUARD_VAO = 1u << 1,
    GUARD_ARRAY_BUFFER = 1u << 2,
    // Unit 0's GL_TEXTURE_2D binding and the active unit together, because the
    // guard makes unit 0 current for its whole lifetime.
    GUARD_TEXTURE = 1u << 3,
    GUARD_FRAMEBUFFER = 1u << 4,
    GUARD_RENDERBUFFER = 1u << 5,
    // Enables the passes need off (they draw one opaque fullscreen quad) plus
    // the color mask, none of which the application is required to have reset.
    GUARD_ENABLES = 1u << 6,
    GUARD_COLOR_MASK = 1u << 7,
};

// Saves the GL state the bodies in this file overwrite and puts it back.
//
// Answered from this layer's own tracking, at no driver cost:
//   - the current program. gl/program.cpp already treats gl_state->current_program
//     as the truth -- it drops a glUseProgram that repeats it -- and program names
//     are not renamed on the way to GLES, so the tracked value is the driver's.
//   - the draw framebuffer. gl/framebuffer.cpp writes gl_state->current_draw_fbo
//     with the name it hands the driver, the redirect of framebuffer 0 to the FSR1
//     render target already resolved, and it is the only file that binds a draw
//     framebuffer through GLES other than this one. It also keeps that field off
//     deleted names, through its own glDeleteFramebuffers -- with one exception,
//     RecreateRenderTargets, which deletes the render FBO behind its back and so
//     has to republish the replacement itself. A saved name has to be live:
//     restoring one GL has deleted is rejected, and the binding then stays wherever
//     the body left it.
//
// Asked of the driver, because nothing in the tree can answer:
//   - the vertex array. What this layer tracks is the application's name, the
//     mapping to the driver's lives in gl/buffer.cpp and is not exported, and
//     the tracked name outlives glDeleteVertexArrays -- restoring from it could
//     hand GLES a name it never generated.
//   - the active unit and unit 0's GL_TEXTURE_2D binding. gl/texture.h's driver
//     shadow declines to answer while FSR1 is enabled, which is exactly when this
//     runs. The active unit has to come from the driver in any case: these guards
//     nest, the moves below go straight to GLES and so never reach that shadow,
//     and an inner guard reading it would restore the outer guard's unit and leave
//     the body running on a unit it never asked for.
//   - the read framebuffer and the renderbuffer binding, which nothing tracks.
//   - the enables and the color mask, tracked by the frontend's virtual enable
//     table only where it opts to answer; the driver read is the honest answer
//     and this runs once per frame.
struct GLStateGuard {
    unsigned int saved;
    GLint prevProgram = 0;
    GLint prevVAO = 0;
    GLint prevArrayBuffer = 0;
    GLint prevActiveTexture = GL_TEXTURE0;
    GLint prevTexture = 0;
    GLint prevReadFBO = 0;
    GLint prevDrawFBO = 0;
    GLint prevRenderbuffer = 0;
    GLboolean prevScissor = GL_FALSE, prevBlend = GL_FALSE, prevDepth = GL_FALSE, prevCull = GL_FALSE;
    GLboolean prevMask[4] = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};

    explicit GLStateGuard(unsigned int bits) : saved(bits) {
        if (saved & GUARD_PROGRAM) prevProgram = static_cast<GLint>(gl_state->current_program);
        if (saved & GUARD_VAO) GLES.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
        if (saved & GUARD_ARRAY_BUFFER) GLES.glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuffer);
        if (saved & GUARD_TEXTURE) {
            GLES.glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTexture);
            GLES.glActiveTexture(GL_TEXTURE0);
            GLES.glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);
        }
        if (saved & GUARD_FRAMEBUFFER) {
            GLES.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
            prevDrawFBO = static_cast<GLint>(gl_state->current_draw_fbo);
        }
        if (saved & GUARD_RENDERBUFFER) GLES.glGetIntegerv(GL_RENDERBUFFER_BINDING, &prevRenderbuffer);
        if (saved & GUARD_ENABLES) {
            prevScissor = GLES.glIsEnabled(GL_SCISSOR_TEST);
            prevBlend = GLES.glIsEnabled(GL_BLEND);
            prevDepth = GLES.glIsEnabled(GL_DEPTH_TEST);
            prevCull = GLES.glIsEnabled(GL_CULL_FACE);
            GLES.glDisable(GL_SCISSOR_TEST);
            GLES.glDisable(GL_BLEND);
            GLES.glDisable(GL_DEPTH_TEST);
            GLES.glDisable(GL_CULL_FACE);
        }
        if (saved & GUARD_COLOR_MASK) {
            GLES.glGetBooleanv(GL_COLOR_WRITEMASK, prevMask);
            GLES.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }

    // Follow a framebuffer this guard saved through a delete-and-recreate.
    //
    // A saved name that the body then deletes cannot be restored: GL rejects it and
    // leaves the binding wherever the body happened to put it. RecreateRenderTargets
    // is the only body here that deletes framebuffers, and the render FBO is the
    // name gl/framebuffer.cpp redirects a bind of framebuffer 0 to -- which is the
    // case this whole path exists for -- so the guard is told where the replacement
    // went instead of being left to restore a dead name.
    void framebuffer_recreated(GLuint from, GLuint to) {
        if (!(saved & GUARD_FRAMEBUFFER) || from == 0 || from == to) return;
        if (prevReadFBO == static_cast<GLint>(from)) prevReadFBO = static_cast<GLint>(to);
        if (prevDrawFBO == static_cast<GLint>(from)) prevDrawFBO = static_cast<GLint>(to);
    }

    ~GLStateGuard() {
        if (saved & GUARD_COLOR_MASK) GLES.glColorMask(prevMask[0], prevMask[1], prevMask[2], prevMask[3]);
        if (saved & GUARD_ENABLES) {
            // glIsEnabled told us what to put back; the enables are restored in the
            // order they were taken so nesting guards compose predictably.
            if (prevCull) GLES.glEnable(GL_CULL_FACE); else GLES.glDisable(GL_CULL_FACE);
            if (prevDepth) GLES.glEnable(GL_DEPTH_TEST); else GLES.glDisable(GL_DEPTH_TEST);
            if (prevBlend) GLES.glEnable(GL_BLEND); else GLES.glDisable(GL_BLEND);
            if (prevScissor) GLES.glEnable(GL_SCISSOR_TEST); else GLES.glDisable(GL_SCISSOR_TEST);
        }
        if (saved & GUARD_PROGRAM) GLES.glUseProgram(prevProgram);
        if (saved & GUARD_VAO) GLES.glBindVertexArray(prevVAO);
        if (saved & GUARD_ARRAY_BUFFER) GLES.glBindBuffer(GL_ARRAY_BUFFER, prevArrayBuffer);
        if (saved & GUARD_TEXTURE) {
            // Unit 0 is current for the guard's lifetime, but say so anyway: a body
            // is free to move the active unit as long as this line puts it back.
            GLES.glActiveTexture(GL_TEXTURE0);
            GLES.glBindTexture(GL_TEXTURE_2D, prevTexture);
            GLES.glActiveTexture(prevActiveTexture);
        }
        if (saved & GUARD_RENDERBUFFER) GLES.glBindRenderbuffer(GL_RENDERBUFFER, prevRenderbuffer);
        if (saved & GUARD_FRAMEBUFFER) {
            GLES.glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFBO);
            GLES.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
        }
    }
};

namespace FSR1_Context {
    GLuint g_renderFBO = 0;
    GLuint g_renderTexture = 0;
    GLuint g_depthStencilRBO = 0;
    GLuint g_targetFBO = 0;
    GLuint g_targetTexture = 0;
    GLuint g_quadVAO = 0;
    GLuint g_quadVBO = 0;

    GLuint g_easuProgram = 0;
    GLuint g_rcasProgram = 0;

    GLint g_easuTexLoc = -1;
    GLint g_easuConLoc[4] = {-1, -1, -1, -1};
    GLint g_rcasTexLoc = -1;
    GLint g_rcasConLoc = -1;

    GLsizei g_targetWidth = 0;
    GLsizei g_targetHeight = 0;
    GLsizei g_renderWidth = 0;
    GLsizei g_renderHeight = 0;
    // The app's window units on the redirect; see FSR1.h for the contract.
    GLsizei g_viewWidth = 0;
    GLsizei g_viewHeight = 0;
    bool g_dirty = false;

    // The swap gate latch, per context (saved/restored by the state swap
    // below). The game's context sets it every frame it renders; a second
    // context that only presents never does, and its swap then skips the
    // upscale instead of painting the surface with its untouched render
    // target -- the empty-frame half of the alternating strobe.
    bool g_presentDirty = false;

    bool g_resolutionChanged = false;
    GLsizei g_pendingWidth = 0;
    GLsizei g_pendingHeight = 0;
    int g_pendingStreak = 0;
} // namespace FSR1_Context

void CalculateRenderResolution(FSR1_Quality_Preset preset, int targetWidth, int targetHeight, int* renderWidth,
                               int* renderHeight) {
    float scale;
    switch (preset) {
    case FSR1_Quality_Preset::UltraQuality:
        scale = 1.3f;
        break;
    case FSR1_Quality_Preset::Quality:
        scale = 1.5f;
        break;
    case FSR1_Quality_Preset::Balanced:
        scale = 1.7f;
        break;
    case FSR1_Quality_Preset::Performance:
    case FSR1_Quality_Preset::Bypass: // diagnostic: same geometry as Performance
        scale = 2.0f;
        break;
    default:
        scale = 1.5f;
    }

    *renderWidth = (int)(targetWidth / scale);
    *renderHeight = (int)(targetHeight / scale);

    *renderWidth = (*renderWidth + 1) & ~1;
    *renderHeight = (*renderHeight + 1) & ~1;
}

// ---------------------------------------------------------------------------
// Pass constants. Computed with the vendored Arm CPU code so the uvec4 uniforms
// are bit-identical to what the shader-side ffxFsrPopulateEasuConstants() would
// produce. Kept as plain arrays: they are handed straight to glUniform4uiv.
// ---------------------------------------------------------------------------

namespace {
GLuint g_cachedEasuCon[4][4];
GLuint g_cachedRcasCon[4];
// The launcher's sharpening slider (config key "fsr1Sharpness") is a 0-100
// percentage, higher = sharper. RCAS wants sharpness stops, where each stop
// halves the effect: the linear map below puts 100% at 0 stops (max
// sharpening) and 0% at 2 stops (barely any). 90% -- the slider's default,
// and what an absent config key resolves to in settings.cpp -- lands on 0.2
// stops, exactly what this file was hardcoded to before the slider existed.
GLfloat RcasSharpnessStops() {
    int percent = global_settings.fsr1_sharpness;
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return (100.0f - static_cast<float>(percent)) / 100.0f * 2.0f;
}
GLsizei g_cachedEasuInputW = 0, g_cachedEasuInputH = 0;
 GLsizei g_cachedEasuOutputW = 0, g_cachedEasuOutputH = 0;
float g_cachedSharpnessStops = -1.0f;

void RefreshFSRConstants() {
    const GLfloat rcasSharpnessStops = RcasSharpnessStops();
    if (FSR1_Context::g_renderWidth == g_cachedEasuInputW && FSR1_Context::g_renderHeight == g_cachedEasuInputH &&
        FSR1_Context::g_targetWidth == g_cachedEasuOutputW && FSR1_Context::g_targetHeight == g_cachedEasuOutputH &&
        g_cachedEasuOutputW != 0 && g_cachedSharpnessStops == rcasSharpnessStops) {
        return;
    }

    FFXM_CPU_NS::FfxUInt32x4 con0 = {0, 0, 0, 0};
    FFXM_CPU_NS::FfxUInt32x4 con1 = {0, 0, 0, 0};
    FFXM_CPU_NS::FfxUInt32x4 con2 = {0, 0, 0, 0};
    FFXM_CPU_NS::FfxUInt32x4 con3 = {0, 0, 0, 0};
    FFXM_CPU_NS::ffxFsrPopulateEasuConstants(
        con0, con1, con2, con3,
        static_cast<float>(FSR1_Context::g_renderWidth), static_cast<float>(FSR1_Context::g_renderHeight),
        static_cast<float>(FSR1_Context::g_renderWidth), static_cast<float>(FSR1_Context::g_renderHeight),
        static_cast<float>(FSR1_Context::g_targetWidth), static_cast<float>(FSR1_Context::g_targetHeight));
    for (int i = 0; i < 4; ++i) {
        g_cachedEasuCon[0][i] = con0[i];
        g_cachedEasuCon[1][i] = con1[i];
        g_cachedEasuCon[2][i] = con2[i];
        g_cachedEasuCon[3][i] = con3[i];
    }

    FFXM_CPU_NS::FfxUInt32x4 rcas = {0, 0, 0, 0};
    FFXM_CPU_NS::FsrRcasCon(rcas, rcasSharpnessStops);
    for (int i = 0; i < 4; ++i) g_cachedRcasCon[i] = rcas[i];

    g_cachedEasuInputW = FSR1_Context::g_renderWidth;
    g_cachedEasuInputH = FSR1_Context::g_renderHeight;
    g_cachedEasuOutputW = FSR1_Context::g_targetWidth;
    g_cachedEasuOutputH = FSR1_Context::g_targetHeight;
    g_cachedSharpnessStops = rcasSharpnessStops;
}
} // namespace

// ---------------------------------------------------------------------------
// Program and resource setup.
// ---------------------------------------------------------------------------

// One VS serves both passes; each pass contributes its own fragment shader.
GLuint CompileFSRProgram(const char* fragmentSource) {
    GLuint program = glCreateProgram();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    LOG_D("Vertex shader source:\n%s", FSR_VSSource);
    glShaderSource(vs, 1, &FSR_VSSource, nullptr);
    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(vs, 512, nullptr, log);
        LOG_F("FSR1 vertex shader error: %s\n", log);
        glDeleteShader(vs);
        return 0;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    LOG_D("Fragment shader source:\n%s", fragmentSource);
    glShaderSource(fs, 1, &fragmentSource, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(fs, 512, nullptr, log);
        LOG_F("FSR1 fragment shader error: %s\n", log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        LOG_F("FSR1 program link error: %s\n", log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void InitFullscreenQuad() {
    GLStateGuard state(GUARD_VAO | GUARD_ARRAY_BUFFER);
    const float quadVertices[] = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                                  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    GLES.glGenVertexArrays(1, &FSR1_Context::g_quadVAO);
    GLES.glGenBuffers(1, &FSR1_Context::g_quadVBO);

    GLES.glBindVertexArray(FSR1_Context::g_quadVAO);
    GLES.glBindBuffer(GL_ARRAY_BUFFER, FSR1_Context::g_quadVBO);

    GLES.glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    GLES.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    GLES.glEnableVertexAttribArray(0);

    GLES.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    GLES.glEnableVertexAttribArray(1);

    GLES.glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLES.glBindVertexArray(0);
}

namespace {

void CreateTexture2D(GLuint* texture, GLsizei width, GLsizei height) {
    GLES.glGenTextures(1, texture);
    GLES.glBindTexture(GL_TEXTURE_2D, *texture);
    GLES.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLES.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLES.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLES.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GLES.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLES.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

// The render set is what gl/framebuffer.cpp's redirect names: color texture,
// depth/stencil and the FBO that binds them. The frontend tracks the FBO name,
// so a recreation has to republish the replacement (see below).
void CreateRenderSet() {
    CreateTexture2D(&FSR1_Context::g_renderTexture, FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight);

    GLES.glGenRenderbuffers(1, &FSR1_Context::g_depthStencilRBO);
    GLES.glBindRenderbuffer(GL_RENDERBUFFER, FSR1_Context::g_depthStencilRBO);
    GLES.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, FSR1_Context::g_renderWidth,
                               FSR1_Context::g_renderHeight);

    GLES.glGenFramebuffers(1, &FSR1_Context::g_renderFBO);
    GLES.glBindFramebuffer(GL_FRAMEBUFFER, FSR1_Context::g_renderFBO);
    GLES.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FSR1_Context::g_renderTexture, 0);
    GLES.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                   FSR1_Context::g_depthStencilRBO);
}

void CreateIntermediate() {
    CreateTexture2D(&FSR1_Context::g_targetTexture, FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight);

    GLES.glGenFramebuffers(1, &FSR1_Context::g_targetFBO);
    GLES.glBindFramebuffer(GL_FRAMEBUFFER, FSR1_Context::g_targetFBO);
    GLES.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FSR1_Context::g_targetTexture, 0);
}

} // namespace

bool fsrInitialized = false;
void InitFSRResources() {
    fsrInitialized = true;
    // No GUARD_VAO or GUARD_ARRAY_BUFFER: the only thing here that binds either is
    // InitFullscreenQuad, which carries its own guard.
    GLStateGuard state(GUARD_PROGRAM | GUARD_TEXTURE | GUARD_FRAMEBUFFER | GUARD_RENDERBUFFER | GUARD_ENABLES |
                       GUARD_COLOR_MASK);

    FSR1_Context::g_easuProgram = CompileFSRProgram(FSR_EASU_FSSource);
    FSR1_Context::g_rcasProgram = CompileFSRProgram(FSR_RCAS_FSSource);

    // A program that failed to compile leaves this file claiming an upscale that
    // cannot draw: the app would render into the redirect while nothing ever
    // presents it, which reads as a black screen. Tear the whole thing back down
    // instead -- no redirect, no FSR, the game renders exactly as it would with
    // FSR1 disabled -- and let the log say why. The self-disable is what makes
    // that "exactly as it would": without it every later glCreateShader retried
    // the whole init (fsrInitialized stayed false) and every swap kept running
    // ApplyFSR on the zeroed state, parking the driver viewport at 0x0 between
    // frames -- a rapidly flickering screen with no upscale, and no hint why,
    // because LOG_F is invisible outside debug builds.
    if (FSR1_Context::g_easuProgram == 0 || FSR1_Context::g_rcasProgram == 0) {
        LOG_W_FORCE("[MG] FSR1 disabled: EASU/RCAS program compilation failed on this context -- FSR1 stays off for "
                    "the whole session (no redirect, no upscale)")
        global_settings.fsr1_setting = FSR1_Quality_Preset::Disabled;
        fsrInitialized = false;
        return;
    }

    FSR1_Context::g_easuTexLoc = glGetUniformLocation(FSR1_Context::g_easuProgram, "uInputTex");
    for (int i = 0; i < 4; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "uEasuCon%d", i);
        FSR1_Context::g_easuConLoc[i] = glGetUniformLocation(FSR1_Context::g_easuProgram, name);
    }
    FSR1_Context::g_rcasTexLoc = glGetUniformLocation(FSR1_Context::g_rcasProgram, "uInputTex");
    FSR1_Context::g_rcasConLoc = glGetUniformLocation(FSR1_Context::g_rcasProgram, "uRcasCon");

    InitFullscreenQuad();

    // Initial sizes. The first presented frame's CheckResolutionChange replaces
    // both with the real surface size before anything but test output has been
    // drawn into them. The render size derives from the preset so the dummy
    // pair is self-consistent (the old hardcoded 960x540 was the 1.3x pair
    // regardless of what the user picked).
    FSR1_Context::g_targetWidth = 1280;
    FSR1_Context::g_targetHeight = 720;
    CalculateRenderResolution(global_settings.fsr1_setting, FSR1_Context::g_targetWidth,
                              FSR1_Context::g_targetHeight,
                              reinterpret_cast<int*>(&FSR1_Context::g_renderWidth),
                              reinterpret_cast<int*>(&FSR1_Context::g_renderHeight));
    FSR1_Context::g_viewWidth = 0;
    FSR1_Context::g_viewHeight = 0;
    CreateRenderSet();
    CreateIntermediate();
    RefreshFSRConstants();

    // GLES.glUseProgram and not this layer's own: the frontend one writes
    // gl_state->current_program, and the guard above restores the driver from that
    // same field. Going through the frontend here would leave the tracked program
    // saying 0 while the driver holds the application's, and gl/program.cpp then
    // drops the application's next glUseProgram(0) as redundant.
    //
    // The samplers and the RCAS constant are set once, here. They are program
    // state, not context state, and these programs are never relinked, so
    // ApplyFSR does not repeat them per frame. The RCAS constant depends on
    // sharpness only, never on a resolution, so one upload lasts forever.
    GLES.glUseProgram(FSR1_Context::g_easuProgram);
    GLES.glUniform1i(FSR1_Context::g_easuTexLoc, 0);
    GLES.glUseProgram(FSR1_Context::g_rcasProgram);
    GLES.glUniform1i(FSR1_Context::g_rcasTexLoc, 0);
    GLES.glUniform4uiv(FSR1_Context::g_rcasConLoc, 1, g_cachedRcasCon);
    GLES.glUseProgram(0);

    // Deliberately NO bind of the render FBO here. This runs from glCreateShader,
    // mid-frame, with the application parked on whatever binding it chose; a raw
    // bind would hijack that while the tracked draw binding still names the old
    // target -- the driver and the tracker would diverge until the app's next
    // glBindFramebuffer, drawing one frame into a target nobody agrees on. The
    // redirect instead takes hold at the application's next bind of framebuffer
    // 0, which is also the first moment the viewport rewrite has a tracked
    // binding to consult.
    //
    // Context identity rides the same line: every context here owns its own
    // target pair, and a second init means a second live redirect -- if two
    // contexts present alternately, their render textures alternate too, and
    // that shows up in a device log as init #2 with no other explanation.
    static std::atomic<int> initCount{0};
    LOAD_EGL(eglGetCurrentContext)
    LOG_W_FORCE("[MG] FSR1 ready: preset %d, sharpening %d%%, surface %dx%d (init #%d, ctx %p)",
                (int)global_settings.fsr1_setting, global_settings.fsr1_sharpness, FSR1_Context::g_targetWidth,
                FSR1_Context::g_targetHeight, ++initCount,
                egl_eglGetCurrentContext ? (void*)egl_eglGetCurrentContext() : nullptr)
}

void RecreateRenderTargets() {
    // Churn telemetry. Every hit is a real size transition; a handful per
    // session is a rotation or a surface rebuild, but a counter racing through
    // these lines is per-frame recreation -- the strobe signature -- and the
    // count says so outright in a log where LOG_D does not exist.
    static std::atomic<int> recreateCount{0};
    const int recreation = ++recreateCount;
    if (recreation <= 6 || recreation % 64 == 0) {
        LOG_W_FORCE("[MG] FSR1 targets recreated #%d: render %dx%d, surface %dx%d", recreation,
                    FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight, FSR1_Context::g_targetWidth,
                    FSR1_Context::g_targetHeight)
    }
    // No GUARD_PROGRAM, GUARD_VAO or GUARD_ARRAY_BUFFER: nothing below binds any of
    // the three. The programs are not recompiled here either, so the uniform
    // locations resolved at link time stay valid across a resolution change.
    GLStateGuard state(GUARD_TEXTURE | GUARD_FRAMEBUFFER | GUARD_RENDERBUFFER);
    // The names about to stop existing. Everything that still refers to either of
    // them once the new pair is up has to be moved over, below.
    const GLuint oldRenderFBO = FSR1_Context::g_renderFBO;
    const GLuint oldTargetFBO = FSR1_Context::g_targetFBO;
    GLES.glDeleteFramebuffers(1, &FSR1_Context::g_renderFBO);
    GLES.glDeleteTextures(1, &FSR1_Context::g_renderTexture);
    GLES.glDeleteRenderbuffers(1, &FSR1_Context::g_depthStencilRBO);

    GLES.glDeleteFramebuffers(1, &FSR1_Context::g_targetFBO);
    GLES.glDeleteTextures(1, &FSR1_Context::g_targetTexture);

    CreateRenderSet();
    CreateIntermediate();

    // The tracked draw binding names the render FBO for as long as the application
    // is drawing to framebuffer 0, because gl/framebuffer.cpp redirects that bind
    // and records the name it handed the driver. That name was deleted above, and
    // nothing in gl/framebuffer.cpp can see it happen -- its glDeleteFramebuffers
    // hook, the one that rebinds 0, is not the path taken here. Left alone, the
    // tracked field would keep naming a dead framebuffer and every GLStateGuard from
    // here on would try to restore it: the restore is rejected, the draw binding
    // stays on framebuffer 0 where ApplyFSR's surface pass leaves it, and the
    // application renders into the surface at render resolution while the upscale
    // keeps reading a render texture nobody writes.
    //
    // Name 0 is excluded, and it is reachable: this runs once a frame from the swap
    // as soon as FSR1 is switched on, while InitFSRResources waits for the first
    // shader. A tracked 0 there is the real surface, not a redirect, and moving it
    // onto the render FBO would diverge from the driver in the other direction --
    // the guard below restores what it saved, which is 0.
    if (oldRenderFBO != 0 && gl_state->current_draw_fbo == oldRenderFBO) {
        set_gl_state_current_draw_fbo(FSR1_Context::g_renderFBO);
    }
    state.framebuffer_recreated(oldRenderFBO, FSR1_Context::g_renderFBO);
    state.framebuffer_recreated(oldTargetFBO, FSR1_Context::g_targetFBO);

    GLES.glBindFramebuffer(GL_FRAMEBUFFER, FSR1_Context::g_renderFBO);
    GLES.glViewport(0, 0, FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight);

    LOG_D("FSR1 resources recreated: render %dx%d, surface %dx%d", FSR1_Context::g_renderWidth,
          FSR1_Context::g_renderHeight, FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight);
}

std::vector<std::pair<GLsizei, GLsizei>> g_viewportStack;

void FSR1_NoteRedirectDraw() { FSR1_Context::g_presentDirty = true; }

bool FSR1_ConsumePresentDirty() {
    const bool dirty = FSR1_Context::g_presentDirty;
    FSR1_Context::g_presentDirty = false;
    return dirty;
}

void ApplyFSR() {
    // No GUARD_ARRAY_BUFFER or GUARD_RENDERBUFFER: nothing below binds either.
    // GL_ARRAY_BUFFER_BINDING is context state and not vertex array object state, so
    // the glBindVertexArray below cannot disturb it.
    GLStateGuard state(GUARD_PROGRAM | GUARD_VAO | GUARD_TEXTURE | GUARD_FRAMEBUFFER | GUARD_ENABLES |
                       GUARD_COLOR_MASK);

    // Diagnostic bypass (preset 5). The redirect, the units latch, the blit
    // rewrites, the swap gate and the surface query all run exactly as with a
    // real preset; the ONLY difference is what lands on the surface at the
    // swap: a plain NEAREST stretch of the render target instead of the
    // EASU+RCAS passes. A device that still strobes here convicts the layers
    // at or below the redirect; a device that goes clean convicts the shader
    // passes. Everything the guard covers (scissor, blend, bindings) is held
    // off for the blit the same as for the passes.
    if (global_settings.fsr1_setting == FSR1_Quality_Preset::Bypass) {
        GLES.glBindFramebuffer(GL_READ_FRAMEBUFFER, FSR1_Context::g_renderFBO);
        GLES.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        GLES.glBlitFramebuffer(0, 0, FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight, 0, 0,
                               FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight, GL_COLOR_BUFFER_BIT,
                               GL_NEAREST);
        GLES.glViewport(0, 0, FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight);
        // Strobe probe (Task 7): the FCL Disabled session went clean, so the
        // redirect machinery is convicted, and both present styles (the RCAS draw
        // and this blit) strobe -- the remaining split is whether the buffer flip
        // races the copy. A full pipeline drain here, in the diagnostic preset
        // only: strobe gone => the fix is a flush discipline; strobe survives =>
        // the layer below the flip owns it. One line marks the probe live.
        static bool s_finishProbeLogged = false;
        if (!s_finishProbeLogged) {
            s_finishProbeLogged = true;
            LOG_W_FORCE("[MG] FSR1 Bypass glFinish probe active (strobe hunt)")
        }
        GLES.glFinish();
        return;
    }

    RefreshFSRConstants();

    // ---- pass 1: EASU, render texture -> intermediate at surface size ----
    GLES.glBindFramebuffer(GL_FRAMEBUFFER, FSR1_Context::g_targetFBO);
    GLES.glViewport(0, 0, FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight);
    GLES.glUseProgram(FSR1_Context::g_easuProgram);

    // Unit 0 is already current -- the guard made it so, and it is the unit
    // uInputTex was pointed at when the program was linked.
    GLES.glBindTexture(GL_TEXTURE_2D, FSR1_Context::g_renderTexture);
    GLES.glUniform4uiv(FSR1_Context::g_easuConLoc[0], 1, g_cachedEasuCon[0]);
    GLES.glUniform4uiv(FSR1_Context::g_easuConLoc[1], 1, g_cachedEasuCon[1]);
    GLES.glUniform4uiv(FSR1_Context::g_easuConLoc[2], 1, g_cachedEasuCon[2]);
    GLES.glUniform4uiv(FSR1_Context::g_easuConLoc[3], 1, g_cachedEasuCon[3]);

    GLES.glBindVertexArray(FSR1_Context::g_quadVAO);
    GLES.glDrawArrays(GL_TRIANGLES, 0, 6);

    // ---- pass 2: RCAS, intermediate -> the real surface (framebuffer 0) ----
    // A plain GLES bind of 0: the frontend's redirect lives a layer above, and
    // this is the one draw of the frame that has to reach the actual surface.
    GLES.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLES.glViewport(0, 0, FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight);
    GLES.glUseProgram(FSR1_Context::g_rcasProgram);
    GLES.glBindTexture(GL_TEXTURE_2D, FSR1_Context::g_targetTexture);
    GLES.glDrawArrays(GL_TRIANGLES, 0, 6);

    // The viewport and nothing else. Neither framebuffer binding is worth setting
    // here: the guard restores both on the next line, and what it restores for the
    // draw binding is the render framebuffer itself whenever the application was
    // drawing to framebuffer 0, which is the case this whole path exists for. The
    // viewport is deliberately outside the guard -- FSR1 owns it between frames, and
    // the next frame has to start at render resolution.
    GLES.glViewport(0, 0, FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight);
}

void CheckResolutionChange(EGLDisplay display, EGLSurface surface) {
    GLsizei width = 0, height = 0;
    LOAD_EGL(eglQuerySurface);
    // Taken from the swap this is hooked into rather than latched into statics on
    // first use. The old code kept the first display and surface it ever saw, so
    // after a rotation or a surface rebuild it queried a destroyed surface every
    // frame and the resolution never changed again.
    if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
        display = eglGetCurrentDisplay();
        surface = eglGetCurrentSurface(EGL_DRAW);
    }
    // Both queries stay, once a frame. EGL has no notification for a surface that
    // changed size, and this query is the file's ONLY size trigger: the
    // glViewport hook now only records the app's own window units (for the
    // scissor/blit rewrites) and deliberately says nothing about the surface,
    // so the query is what catches a surface that shrank as well as one the
    // application never draws full-bleed into. They are also EGL calls,
    // reading attributes the surface record already holds, not GL commands
    // that have to reach the driver's command stream.
    egl_eglQuerySurface(display, surface, EGL_WIDTH, &width);
    egl_eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    OnResize(width, height);

    // Strobe hunt (this round): a size the query hands out on alternating
    // swaps -- two surfaces alive in one session, or one surface mid-rebuild
    // -- is the known strobe source the pendingStreak debounce masks at the
    // target level. The debounce keeps the targets stable but says nothing
    // about WHY the query alternates. Log every identity change of the
    // (surface, size) answer, rate-limited, so a device log shows the
    // alternation, its period and the surface pointers directly.
    {
        static EGLSurface s_lastSurface = EGL_NO_SURFACE;
        static GLsizei s_lastW = 0, s_lastH = 0;
        static int s_identityChanges = 0;
        if (surface != s_lastSurface || width != s_lastW || height != s_lastH) {
            s_lastSurface = surface;
            s_lastW = width;
            s_lastH = height;
            ++s_identityChanges;
            if (s_identityChanges <= 24 || s_identityChanges % 128 == 0) {
                LOG_W_FORCE("[MG] FSR1 surface identity #%d: surface %p now %dx%d", s_identityChanges,
                            (void*)surface, width, height)
            }
        }
    }

    if (FSR1_Context::g_resolutionChanged) {
        FSR1_Context::g_resolutionChanged = false;
        FSR1_Context::g_targetWidth = FSR1_Context::g_pendingWidth;
        FSR1_Context::g_targetHeight = FSR1_Context::g_pendingHeight;

        // The preset decides how far below the surface the app renders. The old
        // pipeline had this backwards: it grew the render size with the surface
        // query and then upscaled past it, buying nothing. Here the surface is
        // the upscale target and the render resolution is derived from it.
        CalculateRenderResolution(global_settings.fsr1_setting, FSR1_Context::g_targetWidth,
                                  FSR1_Context::g_targetHeight, reinterpret_cast<int*>(&FSR1_Context::g_renderWidth),
                                  reinterpret_cast<int*>(&FSR1_Context::g_renderHeight));
        // The window-units capture is tied to the old surface: after a rotation
        // or a surface rebuild it must be re-learned from the app's next
        // full-bleed viewport rather than kept stale.
        FSR1_Context::g_viewWidth = 0;
        FSR1_Context::g_viewHeight = 0;
        RecreateRenderTargets();

        // Once-only visible telemetry: the ready/redirect lines printed at init
        // time carry the hardcoded dummy pair, and every log analysis so far has
        // had to reconstruct the REAL surface size indirectly from blit-rewrite
        // scales. This line, after the derived render size is final, is the first
        // trustworthy size statement in a log.
        static bool mg_fsr_latch_logged = false;
        if (!mg_fsr_latch_logged) {
            mg_fsr_latch_logged = true;
            LOG_W_FORCE("[MG] FSR1 surface latched: %dx%d -> render %dx%d",
                        FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight,
                        FSR1_Context::g_renderWidth, FSR1_Context::g_renderHeight)
        }
    }
    // No glViewport here. This runs immediately after ApplyFSR and the swap, and
    // ApplyFSR ends every frame with exactly this call at exactly this size; on the
    // one frame where the size does change, RecreateRenderTargets ends with it at
    // the new size. It was setting the viewport to the value it already held, once
    // per presented frame.
}

void OnResize(int width, int height) {
    // Debounced. The device log that drove the previous strobing fix (run 26:
    // two size regimes, 1280x720 and 2284x1080, alive in one session) showed a
    // second way to reach per-frame RecreateRenderTargets: the surface query
    // itself handing out two answers on alternating swaps -- two surfaces, or
    // one mid-rebuild -- after the viewport trigger was already gone. A size
    // seen once is not a resolution change; one the query repeats on the next
    // swap is. A flip-flop never reaches two, so the targets freeze at their
    // current size instead of being deleted and rebuilt every frame (which
    // presents as a strobe and garbled blit landings); a real resize repeats
    // by definition and lands one frame later than before.
    if (FSR1_Context::g_targetWidth == width && FSR1_Context::g_targetHeight == height) {
        FSR1_Context::g_pendingStreak = 0;
        return;
    }

    if (FSR1_Context::g_pendingWidth == width && FSR1_Context::g_pendingHeight == height) {
        ++FSR1_Context::g_pendingStreak;
    } else {
        FSR1_Context::g_pendingWidth = width;
        FSR1_Context::g_pendingHeight = height;
        FSR1_Context::g_pendingStreak = 1;
    }
    if (FSR1_Context::g_pendingStreak >= 2) FSR1_Context::g_resolutionChanged = true;
}

// ---------------------------------------------------------------------------
// Viewport and scissor.
//
// Both are the frontend's only definitions of these entry points, so every
// application call goes through here. While the application draws to the
// redirect (its framebuffer 0), its notion of the window is the surface size,
// but the pixels land in the render-sized FBO: a full-bleed viewport of surface
// size would clip the frame to its bottom-left corner. Rewriting the viewport to
// the render size maps the whole frame onto the render target instead, which is
// what makes the preset's resolution savings real. Scissor rectangles are
// framebuffer pixels the application computes at surface size (Minecraft's GUI
// scissors), so they scale by the same ratio.
// ---------------------------------------------------------------------------

// Air Task 82 port. See FSR1.h for the contract; the shape test below is the
// Amethyst fork's latch filter, adapted to this file's denominators (surface =
// g_targetWidth/Height, latch = g_viewWidth/Height). Both latch sites -- the
// glViewport hook below and the glBlitFramebuffer hook in framebuffer.cpp --
// go through here, so one rejection log covers both.
bool FSR1_WindowUnitsCandidate(GLsizei w, GLsizei h) {
    if (w == 0 || h == 0) return false;
    const double candidateAspect = static_cast<double>(w) / static_cast<double>(h);
    auto aspectDrift = [](double a, double b) { return (a > b ? a - b : b - a) / b; };

    // Run 48f4b1c rule, the empty latch. Both call sites only ever hand over a
    // full-bleed candidate (origin 0,0 is checked by the caller), and when the
    // latch is empty there is nothing the shape test could protect: refusing
    // the first candidate left the rewrite denominators at the surface size,
    // which is wrong for the entire FCL family (surface 1280x720 vs window
    // 2360x1080 in that run) -- the log shows the true window REJECTED, the
    // blit rewrite then drawing 1180x540 into the 640x360 render FBO. Seed the
    // latch from the first non-square candidate instead. Square is still
    // refused here: atlas passes are square and fire early, and they are the
    // one shape the empty latch must never learn.
    if (FSR1_Context::g_viewWidth == 0 && FSR1_Context::g_viewHeight == 0 && w != h) {
        static GLsizei s_seededW = -1, s_seededH = -1;
        if (s_seededW != w || s_seededH != h) {
            s_seededW = w;
            s_seededH = h;
            LOG_W_FORCE("[MG] FSR1 window-units latch seeded: %dx%d (empty-latch rule, surface %dx%d; square "
                        "intermediate passes stay refused)",
                        w, h, FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight)
        }
        return true;
    }

    // Rule 1, the window shape: the app's window is a uniform scale of the
    // surface, so a window viewport carries the surface's aspect ratio. The
    // 3% absorbs preset-scale rounding (1814/1262 = 1.4371 vs 2360/1640 =
    // 1.4390, a 0.13% drift in the fork's own numbers).
    if (FSR1_Context::g_targetWidth > 0 && FSR1_Context::g_targetHeight > 0 &&
        aspectDrift(candidateAspect, static_cast<double>(FSR1_Context::g_targetWidth) /
                                            static_cast<double>(FSR1_Context::g_targetHeight)) <= 0.03) {
        return true;
    }
    // Rule 2, continuity: once units are known, only a candidate with the
    // same shape may grow them. A real resize keeps the window's aspect;
    // atlas (square) and shadow-map (square / fixed) passes do not.
    if (FSR1_Context::g_viewWidth > 0 && FSR1_Context::g_viewHeight > 0 &&
        aspectDrift(candidateAspect, static_cast<double>(FSR1_Context::g_viewWidth) /
                                         static_cast<double>(FSR1_Context::g_viewHeight)) <= 0.03) {
        return true;
    }
    // Once per distinct rejected size: the offending pass fires every frame,
    // and the first refusal is the whole story.
    static GLsizei s_rejectedW = -1, s_rejectedH = -1;
    if (s_rejectedW != w || s_rejectedH != h) {
        s_rejectedW = w;
        s_rejectedH = h;
        LOG_W_FORCE("[MG] FSR1 window-units latch rejected (air Task 82): %dx%d is not a window viewport "
                    "(surface %dx%d, latch %dx%d) -- intermediate render pass kept out of the upscale geometry",
                    w, h, FSR1_Context::g_targetWidth, FSR1_Context::g_targetHeight, FSR1_Context::g_viewWidth,
                    FSR1_Context::g_viewHeight)
    }
    return false;
}

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    LOG()
    LOG_D("glViewport: x=%d, y=%d, w=%d, h=%d", x, y, w, h);

    if (fsrInitialized && FSR1_Context::g_renderFBO != 0 &&
        gl_state->current_draw_fbo == FSR1_Context::g_renderFBO) {
        // A viewport issued on the redirect is the app actively producing a
        // frame there -- mark the swap gate, same as the bind-0 redirect and
        // the rewritten blit dst. Without this site a game that binds
        // framebuffer 0 once and renders many frames without rebinding would
        // go gate-dark after the first consumed swap and present stale frames.
        // The viewport the application issues on the redirect is in the app's
        // own window units, not surface pixels: a launcher may size the EGL
        // surface differently from the game's window (Minecraft on Zalith:
        // window 2360x1080, surface 1920x1080). A full-bleed viewport is that
        // window size -- remember the largest one seen, the scissor and blit
        // rewrites need it as their scaling denominator. Scale the rectangle
        // itself by the same ratio, so partial viewports keep their place in
        // the frame (a full-bleed one reduces to the plain render-size rewrite
        // this hook has always done).
        //
        // This is deliberately ALL this hook does about sizes. The old code
        // also read a viewport wider than the surface as "the surface grew"
        // and inflated the pending target -- but the game's window is
        // PERMANENTLY larger than the surface in this pairing, so every frame
        // flagged a resolution change that the next swap's surface query
        // immediately pulled back: the target flip-flopped 1920 <-> 2360,
        // RecreateRenderTargets ran once per frame wiping the render target,
        // and the screen strobed. The surface query at the swap is the only
        // size authority; the app's viewport says nothing about the surface.
        if (x == 0 && y == 0 &&
            (w > FSR1_Context::g_viewWidth || h > FSR1_Context::g_viewHeight) &&
            // Air Task 82: growth alone is not enough -- a square atlas
            // viewport grows the height past the window and poisons every
            // rewrite from that frame on. Window-shaped candidates only.
            FSR1_WindowUnitsCandidate(w, h)) {
            FSR1_Context::g_viewWidth = w;
            FSR1_Context::g_viewHeight = h;
        }
        FSR1_NoteRedirectDraw();
        const GLsizei unitW = FSR1_Context::g_viewWidth ? FSR1_Context::g_viewWidth : FSR1_Context::g_targetWidth;
        const GLsizei unitH = FSR1_Context::g_viewHeight ? FSR1_Context::g_viewHeight : FSR1_Context::g_targetHeight;
        const GLdouble scaleX = static_cast<GLdouble>(FSR1_Context::g_renderWidth) / unitW;
        const GLdouble scaleY = static_cast<GLdouble>(FSR1_Context::g_renderHeight) / unitH;
        GLES.glViewport(x ? static_cast<GLint>(x * scaleX) : 0, y ? static_cast<GLint>(y * scaleY) : 0,
                        static_cast<GLsizei>(w * scaleX), static_cast<GLsizei>(h * scaleY));
        return;
    }

    GLES.glViewport(x, y, w, h);
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    if (fsrInitialized && FSR1_Context::g_renderFBO != 0 &&
        gl_state->current_draw_fbo == FSR1_Context::g_renderFBO &&
        (FSR1_Context::g_renderWidth != FSR1_Context::g_targetWidth ||
         FSR1_Context::g_renderHeight != FSR1_Context::g_targetHeight)) {
        // Game-unit rectangles scale by the app's window size when that is
        // known (it usually is: every frame opens with a full-bleed viewport
        // on the redirect), and by the surface size otherwise. GLdouble
        // because GLsizei products overflow at 4K-plus sizes.
        const GLsizei unitW = FSR1_Context::g_viewWidth ? FSR1_Context::g_viewWidth : FSR1_Context::g_targetWidth;
        const GLsizei unitH = FSR1_Context::g_viewHeight ? FSR1_Context::g_viewHeight : FSR1_Context::g_targetHeight;
        const GLdouble scaleX = static_cast<GLdouble>(FSR1_Context::g_renderWidth) / unitW;
        const GLdouble scaleY = static_cast<GLdouble>(FSR1_Context::g_renderHeight) / unitH;
        const GLint sx = static_cast<GLint>(x * scaleX);
        const GLint sy = static_cast<GLint>(y * scaleY);
        const GLsizei sw = static_cast<GLsizei>(width * scaleX);
        const GLsizei sh = static_cast<GLsizei>(height * scaleY);
        GLES.glScissor(sx, sy, sw, sh);
        return;
    }

    GLES.glScissor(x, y, width, height);
}

// ---------------------------------------------------------------------------

namespace {

struct fsr1_ctx_state_t {
    GLuint renderFBO = 0, renderTexture = 0, depthStencilRBO = 0;
    GLuint targetFBO = 0, targetTexture = 0;
    GLuint quadVAO = 0, quadVBO = 0;
    GLuint easuProgram = 0, rcasProgram = 0;
    // Locations belong to the programs, so they travel with them rather than
    // being re-resolved after a context switch.
    GLint easuTexLoc = -1;
    GLint easuConLoc[4] = {-1, -1, -1, -1};
    GLint rcasTexLoc = -1;
    GLint rcasConLoc = -1;
    GLsizei targetWidth = 0, targetHeight = 0, renderWidth = 0, renderHeight = 0;
    GLsizei viewWidth = 0, viewHeight = 0;
    bool presentDirty = false;
    bool initialised = false;
};

std::mutex g_fsr_mutex;
// Plain value, not a unique_ptr like the other per-context tables: nothing here
// keeps the address of an entry. Both operator[] calls in mg_fsr1_bind_context
// are separate statements, so the first reference is dead before the second one
// can rehash the map.
ska::flat_hash_map<unsigned long long, fsr1_ctx_state_t> g_fsr_states;
fsr1_ctx_state_t g_fsr_default;
thread_local unsigned long long g_fsr_current_id = 0;

void store_into(fsr1_ctx_state_t& d) {
    d.renderFBO = FSR1_Context::g_renderFBO;
    d.renderTexture = FSR1_Context::g_renderTexture;
    d.depthStencilRBO = FSR1_Context::g_depthStencilRBO;
    d.targetFBO = FSR1_Context::g_targetFBO;
    d.targetTexture = FSR1_Context::g_targetTexture;
    d.quadVAO = FSR1_Context::g_quadVAO;
    d.quadVBO = FSR1_Context::g_quadVBO;
    d.easuProgram = FSR1_Context::g_easuProgram;
    d.rcasProgram = FSR1_Context::g_rcasProgram;
    d.easuTexLoc = FSR1_Context::g_easuTexLoc;
    for (int i = 0; i < 4; ++i) d.easuConLoc[i] = FSR1_Context::g_easuConLoc[i];
    d.rcasTexLoc = FSR1_Context::g_rcasTexLoc;
    d.rcasConLoc = FSR1_Context::g_rcasConLoc;
    d.targetWidth = FSR1_Context::g_targetWidth;
    d.targetHeight = FSR1_Context::g_targetHeight;
    d.renderWidth = FSR1_Context::g_renderWidth;
    d.renderHeight = FSR1_Context::g_renderHeight;
    d.viewWidth = FSR1_Context::g_viewWidth;
    d.viewHeight = FSR1_Context::g_viewHeight;
    d.presentDirty = FSR1_Context::g_presentDirty;
    d.initialised = fsrInitialized;
}

void load_from(const fsr1_ctx_state_t& s) {
    FSR1_Context::g_renderFBO = s.renderFBO;
    FSR1_Context::g_renderTexture = s.renderTexture;
    FSR1_Context::g_depthStencilRBO = s.depthStencilRBO;
    FSR1_Context::g_targetFBO = s.targetFBO;
    FSR1_Context::g_targetTexture = s.targetTexture;
    FSR1_Context::g_quadVAO = s.quadVAO;
    FSR1_Context::g_quadVBO = s.quadVBO;
    FSR1_Context::g_easuProgram = s.easuProgram;
    FSR1_Context::g_rcasProgram = s.rcasProgram;
    FSR1_Context::g_easuTexLoc = s.easuTexLoc;
    for (int i = 0; i < 4; ++i) FSR1_Context::g_easuConLoc[i] = s.easuConLoc[i];
    FSR1_Context::g_rcasTexLoc = s.rcasTexLoc;
    FSR1_Context::g_rcasConLoc = s.rcasConLoc;
    FSR1_Context::g_targetWidth = s.targetWidth;
    FSR1_Context::g_targetHeight = s.targetHeight;
    FSR1_Context::g_renderWidth = s.renderWidth;
    FSR1_Context::g_renderHeight = s.renderHeight;
    FSR1_Context::g_viewWidth = s.viewWidth;
    FSR1_Context::g_viewHeight = s.viewHeight;
    FSR1_Context::g_presentDirty = s.presentDirty;
    fsrInitialized = s.initialised;
    // Left alone deliberately: g_dirty, g_resolutionChanged and the pending size
    // describe work queued for the frame in flight, not the context's objects.
}

} // namespace

void mg_fsr1_bind_context(unsigned long long ctx_id) {
    if (ctx_id == g_fsr_current_id) return;
    std::lock_guard<std::mutex> lock(g_fsr_mutex);
    store_into(g_fsr_current_id == 0 ? g_fsr_default : g_fsr_states[g_fsr_current_id]);
    load_from(ctx_id == 0 ? g_fsr_default : g_fsr_states[ctx_id]);
    g_fsr_current_id = ctx_id;
}

void mg_fsr1_forget_context(unsigned long long ctx_id) {
    if (ctx_id == 0) return;
    std::lock_guard<std::mutex> lock(g_fsr_mutex);
    // If this is still the loaded set, the live globals describe a context that is
    // gone. Drop back to the default set rather than storing them into the entry
    // about to be erased.
    if (g_fsr_current_id == ctx_id) {
        load_from(g_fsr_default);
        g_fsr_current_id = 0;
    }
    g_fsr_states.erase(ctx_id);
}
