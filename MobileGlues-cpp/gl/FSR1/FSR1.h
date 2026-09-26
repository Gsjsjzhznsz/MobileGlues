// MobileGlues - gl/FSR1/FSR1.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
//   https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header
#pragma once

#include <cstdlib>
#include <cstring>
#include <vector>

#ifndef __APPLE__
#include <malloc.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "../../gles/gles.h"
#include "../../gles/loader.h"
#include "../../includes.h"
#include "../framebuffer.h"
#include "../glsl/glsl_for_es.h"
#include "../log.h"
#include "../mg.h"
#include <GL/gl.h>

// mg-3backends FSR1: Arm Accuracy Super Resolution (FFXM FSR1) on a two-pass
// pipeline. The GPU shaders live in FSRShaderSource.h; these are the Arm CPU
// headers, vendored so the pass constants are computed with the very same
// ffxFsrPopulateEasuConstants()/FsrRcasCon() code the shader headers ship,
// bit for bit. GPU sections of these headers are compiled out (FFXM_GPU undefined).
namespace FFXM_CPU_NS {
// The Arm headers expect the C runtime but do not include it themselves.
#include <cstdint>
#include <cmath>
#define FFXM_CPU 1
#include "../../include/ffxm/ffxm_common_types.h"
#include "../../include/ffxm/ffxm_core_cpu.h"
#include "../../include/ffxm/fsr1/ffxm_fsr1.h"
#undef FFXM_CPU
} // namespace FFXM_CPU_NS

namespace FSR1_Context {
    // The owned default-framebuffer redirect: what framebuffer 0 becomes while
    // FSR1 is on. Sized to the RENDER resolution -- the app renders low, the two
    // passes below bring the frame back up to the surface.
    extern GLuint g_renderFBO;
    extern GLuint g_renderTexture;
    extern GLuint g_depthStencilRBO;

    // The EASU intermediate, sized to the SURFACE resolution: pass 1 upscales
    // render -> intermediate, pass 2 sharpens intermediate -> real surface.
    extern GLuint g_targetFBO;
    extern GLuint g_targetTexture;

    extern GLuint g_quadVAO;
    extern GLuint g_quadVBO;

    extern GLuint g_easuProgram;
    extern GLuint g_rcasProgram;

    // Uniform locations of the two programs, resolved when they are linked and
    // valid for as long as they live. -1 for a name the linker dropped, which
    // glUniform* ignores.
    extern GLint g_easuTexLoc;     // "uInputTex" (EASU)
    extern GLint g_easuConLoc[4];  // "uEasuCon0".."uEasuCon3"
    extern GLint g_rcasTexLoc;     // "uInputTex" (RCAS)
    extern GLint g_rcasConLoc;     // "uRcasCon"

    extern GLsizei g_targetWidth;   // surface resolution the passes produce at
    extern GLsizei g_targetHeight;
    extern GLsizei g_renderWidth;   // resolution the app renders at
    extern GLsizei g_renderHeight;

    // The application's own window units on the redirect. A full-bleed viewport
    // (or a full-bleed blit dst) issued on framebuffer 0 is the app's notion of
    // its window, and it need not match the EGL surface: a launcher is free to
    // size the surface differently from the game's window (Minecraft via
    // Zalith: window 2360x1080, surface 1920x1080). The scissor and blit
    // rewrites scale game-unit rectangles into render pixels, so their
    // denominator has to be this, not the surface size. 0 = nothing captured
    // yet; the rewrites then fall back to the surface sizes (the old
    // behavior). Grown-only while a size is known; reset when the surface
    // resolution changes, so a rotation re-learns the new window instead of
    // keeping the stale one.
    extern GLsizei g_viewWidth;
    extern GLsizei g_viewHeight;

    extern bool g_dirty;

    extern bool g_resolutionChanged;      // a new size is waiting to be applied
    extern GLsizei g_pendingWidth;        // pending SURFACE size
    extern GLsizei g_pendingHeight;
} // namespace FSR1_Context

extern bool fsrInitialized;

// Swap the FSR1 objects when the current context changes.
//
// Every name above is a GL object owned by the context that created it, and
// gl/framebuffer.cpp redirects framebuffer 0 to g_renderFBO -- in a second
// context that name refers to nothing, or to somebody else's object. The values
// are saved and reloaded rather than reached through a pointer because they are
// declared extern and read from several translation units.
void mg_fsr1_bind_context(unsigned long long ctx_id);
void ApplyFSR();
void InitFSRResources();
void CheckResolutionChange(EGLDisplay display, EGLSurface surface);
void OnResize(int width, int height);

extern "C"
{
    GLAPI void glViewport(GLint x, GLint y, GLsizei w, GLsizei h);
    GLAPI void glScissor(GLint x, GLint y, GLsizei w, GLsizei h);
}
