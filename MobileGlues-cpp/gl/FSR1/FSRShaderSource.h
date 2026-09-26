// MobileGlues - gl/FSR1/FSRShaderSource.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
//   https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header
//
// mg-3backends: built-in FSR1 replaced with Arm(R) Accuracy Super Resolution(TM)
// (Arm's mobile-optimized FidelityFX FSR 1.0.2, "FFXM"). Sources flattened from
// https://github.com/arm/accuracy-super-resolution-generic-library (MIT license,
// (c) 2023 Advanced Micro Devices, Inc. / (c) 2024-2025 Arm Limited); the MIT
// grant below covers every flattened block. The shader body differs from the
// old single-pass AMD port in one structural way: EASU and RCAS now run as two
// separate fragment passes (the old port computed EASU and then discarded it,
// sampling the raw input in RCAS), and all pass constants are computed on the
// CPU instead of per fragment.
#pragma once
#include <string>

#define FSR_ASR_MIT_LICENSE R"fsr_license(
Copyright  (c) 2023 Advanced Micro Devices, Inc.
Copyright  (c) 2024-2025 Arm Limited.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
)fsr_license"

// Fullscreen triangle-strip quad: position passthrough, 0..1 UV.
const char* FSR_VSSource = R"fsr_glsl(#version 450

layout(location = 0) in vec2 aPosition;
out vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aPosition * 0.5 + 0.5;
})fsr_glsl";

// EASU pass: 12-tap edge-adaptive spatial upscale (fp32).
const char* FSR_EASU_FSSource = R"fsr_glsl(#version 450

// Arm Accuracy Super Resolution (FFXM FSR1), MIT license -
// full notice in FSR_ASR_MIT_LICENSE at the top of this file.
// (c) 2023 Advanced Micro Devices, Inc. / (c) 2024-2025 Arm Limited
#define FFXM_GPU 1
#define FFXM_GLSL 1
// FFXM_HALF=0: the fp16 paths need GL_EXT_shader_explicit_arithmetic_types
// storage, which GLES3 drivers do not offer; the fp32 paths below are the
// ones Arm ships for such targets. FFXM_WAVE/FFXM_LONG stay undefined.
#define FFXM_HALF 0
#define FFXM_FSR_EASU_FLOAT 1
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin ffxm_common_types.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef FFXM_COMMON_TYPES_H
#define FFXM_COMMON_TYPES_H

#if defined(FFXM_CPU)
#define FFXM_PARAMETER_IN
#define FFXM_PARAMETER_OUT
#define FFXM_PARAMETER_INOUT
#define FFXM_PARAMETER_UNIFORM
#elif defined(FFXM_HLSL)
#define FFXM_PARAMETER_IN        in
#define FFXM_PARAMETER_OUT       out
#define FFXM_PARAMETER_INOUT     inout
#define FFXM_PARAMETER_UNIFORM uniform
#elif defined(FFXM_GLSL)
#define FFXM_PARAMETER_IN        in
#define FFXM_PARAMETER_OUT       out
#define FFXM_PARAMETER_INOUT     inout
#define FFXM_PARAMETER_UNIFORM const //[cacao_placeholder] until a better fit is found!
#endif // #if defined(FFXM_CPU)

#if defined(FFXM_CPU)
/// A typedef for a boolean value.
///
/// @ingroup CPUTypes
typedef bool FfxBoolean;

/// A typedef for a unsigned 8bit integer.
///
/// @ingroup CPUTypes
typedef uint8_t FfxUInt8;

/// A typedef for a unsigned 16bit integer.
///
/// @ingroup CPUTypes
typedef uint16_t FfxUInt16;

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32;

/// A typedef for a unsigned 64bit integer.
///
/// @ingroup CPUTypes
typedef uint64_t FfxUInt64;

/// A typedef for a signed 8bit integer.
///
/// @ingroup CPUTypes
typedef int8_t FfxInt8;

/// A typedef for a signed 16bit integer.
///
/// @ingroup CPUTypes
typedef int16_t FfxInt16;

/// A typedef for a signed 32bit integer.
///
/// @ingroup CPUTypes
typedef int32_t FfxInt32;

/// A typedef for a signed 64bit integer.
///
/// @ingroup CPUTypes
typedef int64_t FfxInt64;

/// A typedef for a floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32;

/// A typedef for a 2-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x2[2];

/// A typedef for a 3-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x3[3];

/// A typedef for a 4-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x4[4];

/// A typedef for a 2-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x2[2];

/// A typedef for a 3-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x3[3];

/// A typedef for a 4-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x4[4];
#endif // #if defined(FFXM_CPU)

#if defined(FFXM_HLSL)

// Unless defined, go for the conservative option.
#if !defined(FFXM_HLSL_6_2)
#define FFXM_HLSL_6_2 (0)
#endif

#define FfxFloat32Mat4 matrix <float, 4, 4>
#define FfxFloat32Mat3 matrix <float, 3, 3>

/// A typedef for a boolean value.
///
/// @ingroup HLSLTypes
typedef bool FfxBoolean;

#if FFXM_HLSL_6_2

/// @defgroup HLSL62Types HLSL 6.2 And Above Types
/// HLSL 6.2 and above type defines for all commonly used variables
///
/// @ingroup HLSLTypes

/// A typedef for a floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t   FfxFloat32;

/// A typedef for a 2-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t2  FfxFloat32x2;

/// A typedef for a 3-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t3  FfxFloat32x3;

/// A typedef for a 4-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t4  FfxFloat32x4;

/// A [cacao_placeholder] typedef for matrix type until confirmed.
typedef float4x4 FfxFloat32x4x4;
typedef float3x3 FfxFloat32x3x3;
typedef float2x2 FfxFloat32x2x2;

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup HLSL62Types
typedef uint32_t    FfxUInt32;

/// A typedef for a 2-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t2   FfxUInt32x2;

/// A typedef for a 3-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t3   FfxUInt32x3;

/// A typedef for a 4-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t4   FfxUInt32x4;

/// A typedef for a signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t     FfxInt32;

/// A typedef for a 2-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t2    FfxInt32x2;

/// A typedef for a 3-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t3    FfxInt32x3;

/// A typedef for a 4-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t4    FfxInt32x4;

#else // #if defined(FFXM_HLSL_6_2)

/// @defgroup HLSLBaseTypes HLSL 6.1 And Below Types
/// HLSL 6.1 and below type defines for all commonly used variables
///
/// @ingroup HLSLTypes

#define FfxFloat32   float
#define FfxFloat32x2 float2
#define FfxFloat32x3 float3
#define FfxFloat32x4 float4

/// A [cacao_placeholder] typedef for matrix type until confirmed.
#define FfxFloat32x4x4 float4x4
#define FfxFloat32x3x3 float3x3
#define FfxFloat32x2x2 float2x2

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup GPU
typedef uint        FfxUInt32;
typedef uint2       FfxUInt32x2;
typedef uint3       FfxUInt32x3;
typedef uint4       FfxUInt32x4;

typedef int         FfxInt32;
typedef int2        FfxInt32x2;
typedef int3        FfxInt32x3;
typedef int4        FfxInt32x4;

#endif // #if defined(FFXM_HLSL_6_2)

// Arm ASR relies in efficient FP16 arithmetic.
#if !defined(FFXM_HALF)
#define FFXM_HALF (1)
#endif

#if FFXM_HALF

#if FFXM_HLSL_6_2

typedef float16_t   FfxFloat16;
typedef float16_t2  FfxFloat16x2;
typedef float16_t3  FfxFloat16x3;
typedef float16_t4  FfxFloat16x4;

/// A typedef for an unsigned 16bit integer.
///
/// @ingroup HLSLTypes
typedef uint16_t    FfxUInt16;
typedef uint16_t2   FfxUInt16x2;
typedef uint16_t3   FfxUInt16x3;
typedef uint16_t4   FfxUInt16x4;

/// A typedef for a signed 16bit integer.
///
/// @ingroup HLSLTypes
typedef int16_t     FfxInt16;
typedef int16_t2    FfxInt16x2;
typedef int16_t3    FfxInt16x3;
typedef int16_t4    FfxInt16x4;
#else // #if FFXM_HLSL_6_2
typedef min16float  FfxFloat16;
typedef min16float2 FfxFloat16x2;
typedef min16float3 FfxFloat16x3;
typedef min16float4 FfxFloat16x4;

/// A typedef for an unsigned 16bit integer.
///
/// @ingroup HLSLTypes
typedef min16uint   FfxUInt16;
typedef min16uint2  FfxUInt16x2;
typedef min16uint3  FfxUInt16x3;
typedef min16uint4  FfxUInt16x4;

/// A typedef for a signed 16bit integer.
///
/// @ingroup HLSLTypes
typedef min16int    FfxInt16;
typedef min16int2   FfxInt16x2;
typedef min16int3   FfxInt16x3;
typedef min16int4   FfxInt16x4;
#endif  // #if FFXM_HLSL_6_2

#endif // FFXM_HALF

#endif // #if defined(FFXM_HLSL)

#if defined(FFXM_GLSL)

#define FfxFloat32Mat4 mat4
#define FfxFloat32Mat3 mat3

/// A typedef for a boolean value.
///
/// @ingroup GLSLTypes
#define FfxBoolean   bool
#define FfxFloat32   float
#define FfxFloat32x2 vec2
#define FfxFloat32x3 vec3
#define FfxFloat32x4 vec4
#define FfxUInt32    uint
#define FfxUInt32x2  uvec2
#define FfxUInt32x3  uvec3
#define FfxUInt32x4  uvec4
#define FfxInt32     int
#define FfxInt32x2   ivec2
#define FfxInt32x3   ivec3
#define FfxInt32x4   ivec4

/// A [cacao_placeholder] typedef for matrix type until confirmed.
#define FfxFloat32x4x4 mat4
#define FfxFloat32x3x3 mat3
#define FfxFloat32x2x2 mat2

#if FFXM_HALF
#define FfxFloat16   float16_t
#define FfxFloat16x2 f16vec2
#define FfxFloat16x3 f16vec3
#define FfxFloat16x4 f16vec4
#define FfxUInt16    uint16_t
#define FfxUInt16x2  u16vec2
#define FfxUInt16x3  u16vec3
#define FfxUInt16x4  u16vec4
#define FfxInt16     int16_t
#define FfxInt16x2   i16vec2
#define FfxInt16x3   i16vec3
#define FfxInt16x4   i16vec4
#endif // FFXM_HALF
#endif // #if defined(FFXM_GLSL)


#if FFXM_HALF

#if FFXM_HLSL_6_2

#define FFXM_MIN16_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType##16_t TypeName;
#define FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType##16_t, COL> TypeName;
#define FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType##16_t, ROW, COL> TypeName;

#define FFXM_16BIT_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType##16_t TypeName;
#define FFXM_16BIT_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType##16_t, COL> TypeName;
#define FFXM_16BIT_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType##16_t, ROW, COL> TypeName;

#else //FFXM_HLSL_6_2

#define FFXM_MIN16_SCALAR( TypeName, BaseComponentType )           typedef min16##BaseComponentType TypeName;
#define FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<min16##BaseComponentType, COL> TypeName;
#define FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<min16##BaseComponentType, ROW, COL> TypeName;

#define FFXM_16BIT_SCALAR( TypeName, BaseComponentType )           FFXM_MIN16_SCALAR( TypeName, BaseComponentType );
#define FFXM_16BIT_VECTOR( TypeName, BaseComponentType, COL )      FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL );
#define FFXM_16BIT_MATRIX( TypeName, BaseComponentType, ROW, COL ) FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL );

#endif //FFXM_HLSL_6_2

#else //FFXM_HALF

#define FFXM_MIN16_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType TypeName;
#define FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType, COL> TypeName;
#define FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType, ROW, COL> TypeName;

#define FFXM_16BIT_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType TypeName;
#define FFXM_16BIT_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType, COL> TypeName;
#define FFXM_16BIT_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType, ROW, COL> TypeName;

#endif //FFXM_HALF

#if defined(FFXM_GPU)
// Common typedefs:
#if defined(FFXM_HLSL)
FFXM_MIN16_SCALAR( FFXM_MIN16_F , float );
FFXM_MIN16_VECTOR( FFXM_MIN16_F2, float, 2 );
FFXM_MIN16_VECTOR( FFXM_MIN16_F3, float, 3 );
FFXM_MIN16_VECTOR( FFXM_MIN16_F4, float, 4 );

FFXM_MIN16_SCALAR( FFXM_MIN16_I,  int );
FFXM_MIN16_VECTOR( FFXM_MIN16_I2, int, 2 );
FFXM_MIN16_VECTOR( FFXM_MIN16_I3, int, 3 );
FFXM_MIN16_VECTOR( FFXM_MIN16_I4, int, 4 );

FFXM_MIN16_SCALAR( FFXM_MIN16_U,  uint );
FFXM_MIN16_VECTOR( FFXM_MIN16_U2, uint, 2 );
FFXM_MIN16_VECTOR( FFXM_MIN16_U3, uint, 3 );
FFXM_MIN16_VECTOR( FFXM_MIN16_U4, uint, 4 );

FFXM_16BIT_SCALAR( FFXM_F16_t , float );
FFXM_16BIT_VECTOR( FFXM_F16_t2, float, 2 );
FFXM_16BIT_VECTOR( FFXM_F16_t3, float, 3 );
FFXM_16BIT_VECTOR( FFXM_F16_t4, float, 4 );

FFXM_16BIT_SCALAR( FFXM_I16_t,  int );
FFXM_16BIT_VECTOR( FFXM_I16_t2, int, 2 );
FFXM_16BIT_VECTOR( FFXM_I16_t3, int, 3 );
FFXM_16BIT_VECTOR( FFXM_I16_t4, int, 4 );

FFXM_16BIT_SCALAR( FFXM_U16_t,  uint );
FFXM_16BIT_VECTOR( FFXM_U16_t2, uint, 2 );
FFXM_16BIT_VECTOR( FFXM_U16_t3, uint, 3 );
FFXM_16BIT_VECTOR( FFXM_U16_t4, uint, 4 );

#define TYPEDEF_MIN16_TYPES(Prefix)           \
typedef FFXM_MIN16_F     Prefix##_F;           \
typedef FFXM_MIN16_F2    Prefix##_F2;          \
typedef FFXM_MIN16_F3    Prefix##_F3;          \
typedef FFXM_MIN16_F4    Prefix##_F4;          \
typedef FFXM_MIN16_I     Prefix##_I;           \
typedef FFXM_MIN16_I2    Prefix##_I2;          \
typedef FFXM_MIN16_I3    Prefix##_I3;          \
typedef FFXM_MIN16_I4    Prefix##_I4;          \
typedef FFXM_MIN16_U     Prefix##_U;           \
typedef FFXM_MIN16_U2    Prefix##_U2;          \
typedef FFXM_MIN16_U3    Prefix##_U3;          \
typedef FFXM_MIN16_U4    Prefix##_U4;

#define TYPEDEF_16BIT_TYPES(Prefix)           \
typedef FFXM_16BIT_F     Prefix##_F;           \
typedef FFXM_16BIT_F2    Prefix##_F2;          \
typedef FFXM_16BIT_F3    Prefix##_F3;          \
typedef FFXM_16BIT_F4    Prefix##_F4;          \
typedef FFXM_16BIT_I     Prefix##_I;           \
typedef FFXM_16BIT_I2    Prefix##_I2;          \
typedef FFXM_16BIT_I3    Prefix##_I3;          \
typedef FFXM_16BIT_I4    Prefix##_I4;          \
typedef FFXM_16BIT_U     Prefix##_U;           \
typedef FFXM_16BIT_U2    Prefix##_U2;          \
typedef FFXM_16BIT_U3    Prefix##_U3;          \
typedef FFXM_16BIT_U4    Prefix##_U4;

#define TYPEDEF_FULL_PRECISION_TYPES(Prefix)  \
typedef FfxFloat32      Prefix##_F;           \
typedef FfxFloat32x2    Prefix##_F2;          \
typedef FfxFloat32x3    Prefix##_F3;          \
typedef FfxFloat32x4    Prefix##_F4;          \
typedef FfxInt32        Prefix##_I;           \
typedef FfxInt32x2      Prefix##_I2;          \
typedef FfxInt32x3      Prefix##_I3;          \
typedef FfxInt32x4      Prefix##_I4;          \
typedef FfxUInt32       Prefix##_U;           \
typedef FfxUInt32x2     Prefix##_U2;          \
typedef FfxUInt32x3     Prefix##_U3;          \
typedef FfxUInt32x4     Prefix##_U4;
#endif // #if defined(FFXM_HLSL)

#if defined(FFXM_GLSL)

#if FFXM_HALF

#define  FFXM_MIN16_F  float16_t
#define  FFXM_MIN16_F2 f16vec2
#define  FFXM_MIN16_F3 f16vec3
#define  FFXM_MIN16_F4 f16vec4

#define  FFXM_MIN16_I  int16_t
#define  FFXM_MIN16_I2 i16vec2
#define  FFXM_MIN16_I3 i16vec3
#define  FFXM_MIN16_I4 i16vec4

#define  FFXM_MIN16_U  uint16_t
#define  FFXM_MIN16_U2 u16vec2
#define  FFXM_MIN16_U3 u16vec3
#define  FFXM_MIN16_U4 u16vec4

#define FFXM_16BIT_F  float16_t
#define FFXM_16BIT_F2 f16vec2
#define FFXM_16BIT_F3 f16vec3
#define FFXM_16BIT_F4 f16vec4

#define FFXM_16BIT_I  int16_t
#define FFXM_16BIT_I2 i16vec2
#define FFXM_16BIT_I3 i16vec3
#define FFXM_16BIT_I4 i16vec4

#define FFXM_16BIT_U  uint16_t
#define FFXM_16BIT_U2 u16vec2
#define FFXM_16BIT_U3 u16vec3
#define FFXM_16BIT_U4 u16vec4

#else // FFXM_HALF

#define  FFXM_MIN16_F  float
#define  FFXM_MIN16_F2 vec2
#define  FFXM_MIN16_F3 vec3
#define  FFXM_MIN16_F4 vec4

#define  FFXM_MIN16_I  int
#define  FFXM_MIN16_I2 ivec2
#define  FFXM_MIN16_I3 ivec3
#define  FFXM_MIN16_I4 ivec4

#define  FFXM_MIN16_U  uint
#define  FFXM_MIN16_U2 uvec2
#define  FFXM_MIN16_U3 uvec3
#define  FFXM_MIN16_U4 uvec4

#define FFXM_16BIT_F  float
#define FFXM_16BIT_F2 vec2
#define FFXM_16BIT_F3 vec3
#define FFXM_16BIT_F4 vec4

#define FFXM_16BIT_I  int
#define FFXM_16BIT_I2 ivec2
#define FFXM_16BIT_I3 ivec3
#define FFXM_16BIT_I4 ivec4

#define FFXM_16BIT_U  uint
#define FFXM_16BIT_U2 uvec2
#define FFXM_16BIT_U3 uvec3
#define FFXM_16BIT_U4 uvec4

#endif // FFXM_HALF

#endif // #if defined(FFXM_GLSL)

#endif // #if defined(FFXM_GPU)
#endif // #ifndef FFXM_COMMON_TYPES_H
// --- end ffxm_common_types.h ---
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin ffxm_core_glsl.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// @defgroup GLSLCore GLSL Core
/// GLSL core defines and functions
///
/// @ingroup FfxGLSL

/// A define for abstracting shared memory between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_GROUPSHARED shared

/// A define for abstracting compute memory barriers between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_GROUP_MEMORY_BARRIER() groupMemoryBarrier(); barrier()

/// A define for abstracting compute atomic additions between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_ATOMIC_ADD(x, y) atomicAdd(x, y)

/// A define added to accept static markup on functions to aid CPU/GPU portability of code.
///
/// @ingroup GLSLCore
#define FFXM_STATIC

/// A define for abstracting loop unrolling between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_UNROLL

/// A define for abstracting a 'greater than' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_GREATER_THAN(x, y) greaterThan(x, y)

/// A define for abstracting a 'greater than or equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_GREATER_THAN_EQUAL(x, y) greaterThanEqual(x, y)

/// A define for abstracting a 'less than' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_LESS_THAN(x, y) lessThan(x, y)

/// A define for abstracting a 'less than or equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_LESS_THAN_EQUAL(x, y) lessThanEqual(x, y)

/// A define for abstracting an 'equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_EQUAL(x, y) equal(x, y)

/// A define for abstracting a 'not equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_NOT_EQUAL(x, y) notEqual(x, y)

/// A define for abstracting matrix multiply operations between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_MATRIX_MULTIPLY(a, b) (a * b)

/// A define for abstracting vector transformations between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_TRANSFORM_VECTOR(a, b) (a * b)

/// A define for abstracting modulo operations between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_MODULO(a, b) (mod(a, b))

/// Broadcast a scalar value to a 1-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32(x)   FfxFloat32(x)

/// Broadcast a scalar value to a 2-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32X2(x) FfxFloat32x2(FfxFloat32(x))

/// Broadcast a scalar value to a 3-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32X3(x) FfxFloat32x3(FfxFloat32(x))

/// Broadcast a scalar value to a 4-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32X4(x) FfxFloat32x4(FfxFloat32(x))

/// Broadcast a scalar value to a 1-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32(x)   FfxUInt32(x)

/// Broadcast a scalar value to a 2-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32X2(x) FfxUInt32x2(FfxUInt32(x))

/// Broadcast a scalar value to a 3-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32X3(x) FfxUInt32x3(FfxUInt32(x))

/// Broadcast a scalar value to a 4-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32X4(x) FfxUInt32x4(FfxUInt32(x))

/// Broadcast a scalar value to a 1-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32(x)   FfxInt32(x)

/// Broadcast a scalar value to a 2-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32X2(x) FfxInt32x2(FfxInt32(x))

/// Broadcast a scalar value to a 3-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32X3(x) FfxInt32x3(FfxInt32(x))

/// Broadcast a scalar value to a 4-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32X4(x) FfxInt32x4(FfxInt32(x))

/// Broadcast a scalar value to a 1-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16(x)   FFXM_MIN16_F(x)

/// Broadcast a scalar value to a 2-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16X2(x) FFXM_MIN16_F2(FFXM_MIN16_F(x))

/// Broadcast a scalar value to a 3-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16X3(x) FFXM_MIN16_F3(FFXM_MIN16_F(x))

/// Broadcast a scalar value to a 4-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16X4(x) FFXM_MIN16_F4(FFXM_MIN16_F(x))

/// Broadcast a scalar value to a 1-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16(x)   FFXM_MIN16_U(x)

/// Broadcast a scalar value to a 2-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16X2(x) FFXM_MIN16_U2(FFXM_MIN16_U(x))

/// Broadcast a scalar value to a 3-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16X3(x) FFXM_MIN16_U3(FFXM_MIN16_U(x))

/// Broadcast a scalar value to a 4-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16X4(x) FFXM_MIN16_U4(FFXM_MIN16_U(x))

/// Broadcast a scalar value to a 1-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16(x)   FFXM_MIN16_I(x)

/// Broadcast a scalar value to a 2-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16X2(x) FFXM_MIN16_I2(FFXM_MIN16_I(x))

/// Broadcast a scalar value to a 3-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16X3(x) FFXM_MIN16_I3(FFXM_MIN16_I(x))

/// Broadcast a scalar value to a 4-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16X4(x) FFXM_MIN16_I4(FFXM_MIN16_I(x))

    #extension GL_EXT_shader_explicit_arithmetic_types : require
#if !defined(FFXM_SKIP_EXT)
#if FFXM_HALF
    #extension GL_EXT_shader_16bit_storage : require
#endif // FFXM_HALF

#if defined(FFXM_LONG)
    #extension GL_ARB_gpu_shader_int64 : require
    #extension GL_NV_shader_atomic_int64 : require
#endif // #if defined(FFXM_LONG)

#if defined(FFXM_WAVE)
    #extension GL_KHR_shader_subgroup_arithmetic : require
    #extension GL_KHR_shader_subgroup_ballot : require
    #extension GL_KHR_shader_subgroup_quad : require
    #extension GL_KHR_shader_subgroup_shuffle : require
#endif // #if defined(FFXM_WAVE)
#endif // #if !defined(FFXM_SKIP_EXT)

// Forward declarations
FfxFloat32   ffxSqrt(FfxFloat32 x);
FfxFloat32x2 ffxSqrt(FfxFloat32x2 x);
FfxFloat32x3 ffxSqrt(FfxFloat32x3 x);
FfxFloat32x4 ffxSqrt(FfxFloat32x4 x);

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32 ffxAsFloat(FfxUInt32 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxAsFloat(FfxUInt32x2 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxAsFloat(FfxUInt32x3 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxAsFloat(FfxUInt32x4 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32 ffxAsUInt32(FfxFloat32 x)
{
    return floatBitsToUint(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxAsUInt32(FfxFloat32x2 x)
{
    return floatBitsToUint(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxAsUInt32(FfxFloat32x3 x)
{
    return floatBitsToUint(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxAsUInt32(FfxFloat32x4 x)
{
    return floatBitsToUint(x);
}

/// Convert a 32bit IEEE 754 floating point value to its nearest 16bit equivalent.
///
/// @param [in] value               The value to convert.
///
/// @returns
/// The nearest 16bit equivalent of <c><i>value</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32 f32tof16(FfxFloat32 value)
{
    return packHalf2x16(FfxFloat32x2(value, 0.0));
}

/// Broadcast a scalar value to a 2-dimensional floating point vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 2-dimensional floating point vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxBroadcast2(FfxFloat32 value)
{
    return FfxFloat32x2(value, value);
}

/// Broadcast a scalar value to a 3-dimensional floating point vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 3-dimensional floating point vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxBroadcast3(FfxFloat32 value)
{
    return FfxFloat32x3(value, value, value);
}

/// Broadcast a scalar value to a 4-dimensional floating point vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 4-dimensional floating point vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxBroadcast4(FfxFloat32 value)
{
    return FfxFloat32x4(value, value, value, value);
}

/// Broadcast a scalar value to a 2-dimensional signed integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 2-dimensional signed integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxInt32x2 ffxBroadcast2(FfxInt32 value)
{
    return FfxInt32x2(value, value);
}

/// Broadcast a scalar value to a 3-dimensional signed integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 3-dimensional signed integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxInt32x3 ffxBroadcast3(FfxInt32 value)
{
    return FfxInt32x3(value, value, value);
}

/// Broadcast a scalar value to a 4-dimensional signed integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 4-dimensional signed integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxInt32x4 ffxBroadcast4(FfxInt32 value)
{
    return FfxInt32x4(value, value, value, value);
}

/// Broadcast a scalar value to a 2-dimensional unsigned integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 2-dimensional unsigned integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxBroadcast2(FfxUInt32 value)
{
    return FfxUInt32x2(value, value);
}

/// Broadcast a scalar value to a 3-dimensional unsigned integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 3-dimensional unsigned integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxBroadcast3(FfxUInt32 value)
{
    return FfxUInt32x3(value, value, value);
}

/// Broadcast a scalar value to a 4-dimensional unsigned integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 4-dimensional unsigned integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxBroadcast4(FfxUInt32 value)
{
    return FfxUInt32x4(value, value, value, value);
}

///
///
/// @ingroup GLSLCore
FfxUInt32 bitfieldExtract(FfxUInt32 src, FfxUInt32 off, FfxUInt32 bits)
{
    return bitfieldExtract(src, FfxInt32(off), FfxInt32(bits));
}

///
///
/// @ingroup GLSLCore
FfxUInt32 bitfieldInsert(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 mask)
{
    return (ins & mask) | (src & (~mask));
}

// Proxy for V_BFI_B32 where the 'mask' is set as 'bits', 'mask=(1<<bits)-1', and 'bits' needs to be an immediate.
///
///
/// @ingroup GLSLCore
FfxUInt32 bitfieldInsertMask(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 bits)
{
    return bitfieldInsert(src, ins, 0, FfxInt32(bits));
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxLerp(FfxFloat32 x, FfxFloat32 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxLerp(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxLerp(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxLerp(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxLerp(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxLerp(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxLerp(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 t)
{
    return mix(x, y, t);
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single V_MAX3_F32 operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxMax3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxMax3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxMax3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxMax3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32 ffxMax3(FfxUInt32 x, FfxUInt32 y, FfxUInt32 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxMax3(FfxUInt32x2 x, FfxUInt32x2 y, FfxUInt32x2 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxMax3(FfxUInt32x3 x, FfxUInt32x3 y, FfxUInt32x3 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxMax3(FfxUInt32x4 x, FfxUInt32x4 y, FfxUInt32x4 z)
{
    return max(x, max(y, z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxMed3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxMed3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxMed3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxMed3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32 ffxMed3(FfxInt32 x, FfxInt32 y, FfxInt32 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32x2 ffxMed3(FfxInt32x2 x, FfxInt32x2 y, FfxInt32x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32x3 ffxMed3(FfxInt32x3 x, FfxInt32x3 y, FfxInt32x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32x4 ffxMed3(FfxInt32x4 x, FfxInt32x4 y, FfxInt32x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}


/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MIN3_F32</i></c> operation on
/// GCN and RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxMin3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxMin3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxMin3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxMin3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32 ffxMin3(FfxUInt32 x, FfxUInt32 y, FfxUInt32 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxMin3(FfxUInt32x2 x, FfxUInt32x2 y, FfxUInt32x2 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxMin3(FfxUInt32x3 x, FfxUInt32x3 y, FfxUInt32x3 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxMin3(FfxUInt32x4 x, FfxUInt32x4 y, FfxUInt32x4 z)
{
    return min(x, min(y, z));
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 rcp(FfxFloat32 x)
{
    return FfxFloat32(1.0) / x;
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 rcp(FfxFloat32x2 x)
{
    return ffxBroadcast2(1.0) / x;
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 rcp(FfxFloat32x3 x)
{
    return ffxBroadcast3(1.0) / x;
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 rcp(FfxFloat32x4 x)
{
    return ffxBroadcast4(1.0) / x;
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 rsqrt(FfxFloat32 x)
{
    return FfxFloat32(1.0) / ffxSqrt(x);
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 rsqrt(FfxFloat32x2 x)
{
    return ffxBroadcast2(1.0) / ffxSqrt(x);
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 rsqrt(FfxFloat32x3 x)
{
    return ffxBroadcast3(1.0) / ffxSqrt(x);
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 rsqrt(FfxFloat32x4 x)
{
    return ffxBroadcast4(1.0) / ffxSqrt(x);
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxSaturate(FfxFloat32 x)
{
    return clamp(x, FfxFloat32(0.0), FfxFloat32(1.0));
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxSaturate(FfxFloat32x2 x)
{
    return clamp(x, ffxBroadcast2(0.0), ffxBroadcast2(1.0));
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxSaturate(FfxFloat32x3 x)
{
    return clamp(x, ffxBroadcast3(0.0), ffxBroadcast3(1.0));
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxSaturate(FfxFloat32x4 x)
{
    return clamp(x, ffxBroadcast4(0.0), ffxBroadcast4(1.0));
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxFract(FfxFloat32 x)
{
    return fract(x);
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxFract(FfxFloat32x2 x)
{
    return fract(x);
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxFract(FfxFloat32x3 x)
{
    return fract(x);
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxFract(FfxFloat32x4 x)
{
    return fract(x);
}

FfxUInt32 AShrSU1(FfxUInt32 a, FfxUInt32 b)
{
    return FfxUInt32(FfxInt32(a) >> FfxInt32(b));
}

FfxUInt32 ffxPackF32(FfxFloat32x2 v){
    return packHalf2x16(v);
}

FfxFloat32x2 ffxUnpackF32(FfxUInt32 u){
    return unpackHalf2x16(u);
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32 ffxInvertSafe(FfxFloat32 v){
    FfxFloat32 s = sign(v);
    FfxFloat32 s2 = s*s;
    return s2/(v + s2 - 1.0);
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32x2 ffxInvertSafe(FfxFloat32x2 v){
    FfxFloat32x2 s = sign(v);
    FfxFloat32x2 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x2(1.0, 1.0));
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32x3 ffxInvertSafe(FfxFloat32x3 v){
    FfxFloat32x3 s = sign(v);
    FfxFloat32x3 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x3(1.0, 1.0, 1.0));
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32x4 ffxInvertSafe(FfxFloat32x4 v){
    FfxFloat32x4 s = sign(v);
    FfxFloat32x4 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x4(1.0, 1.0, 1.0, 1.0));
}
#if FFXM_HALF
#define FFXM_UINT32_TO_FLOAT16X2(x) unpackFloat2x16(FfxUInt32(x))

FfxUInt32 ffxPackF16(FfxFloat16x2 v){
    return packHalf2x16(v);
}

FfxFloat16x2 ffxUnpackF16(FfxUInt32 u){
    return FfxFloat16x2(unpackHalf2x16(u));
}

FfxFloat16x4 ffxUint32x2ToFloat16x4(FfxUInt32x2 x)
{
    return FfxFloat16x4(unpackFloat2x16(x.x), unpackFloat2x16(x.y));
}
#define FFXM_UINT32X2_TO_FLOAT16X4(x) ffxUint32x2ToFloat16x4(FfxUInt32x2(x))
#define FFXM_UINT32_TO_UINT16X2(x) unpackUint2x16(FfxUInt32(x))
#define FFXM_UINT32X2_TO_UINT16X4(x) unpackUint4x16(pack64(FfxUInt32x2(x)))
//------------------------------------------------------------------------------------------------------------------------------
#define FFXM_FLOAT16X2_TO_UINT32(x) packFloat2x16(FfxFloat16x2(x))
FfxUInt32x2 ffxFloat16x4ToUint32x2(FfxFloat16x4 x)
{
    return FfxUInt32x2(packFloat2x16(x.xy), packFloat2x16(x.zw));
}
#define FFXM_FLOAT16X4_TO_UINT32X2(x) ffxFloat16x4ToUint32x2(FfxFloat16x4(x))
#define FFXM_UINT16X2_TO_UINT32(x) packUint2x16(FfxUInt16x2(x))
#define FFXM_UINT16X4_TO_UINT32X2(x) unpack32(packUint4x16(FfxUInt16x4(x)))
//==============================================================================================================================
#define FFXM_TO_UINT16(x) halfBitsToUint16(FfxFloat16(x))
#define FFXM_TO_UINT16X2(x) halfBitsToUint16(FfxFloat16x2(x))
#define FFXM_TO_UINT16X3(x) halfBitsToUint16(FfxFloat16x3(x))
#define FFXM_TO_UINT16X4(x) halfBitsToUint16(FfxFloat16x4(x))
//------------------------------------------------------------------------------------------------------------------------------
#define FFXM_TO_FLOAT16(x) uint16BitsToHalf(FfxUInt16(x))
#define FFXM_TO_FLOAT16X2(x) uint16BitsToHalf(FfxUInt16x2(x))
#define FFXM_TO_FLOAT16X3(x) uint16BitsToHalf(FfxUInt16x3(x))
#define FFXM_TO_FLOAT16X4(x) uint16BitsToHalf(FfxUInt16x4(x))
//==============================================================================================================================
FfxFloat16 ffxBroadcastFloat16(FfxFloat16 a)
{
    return FfxFloat16(a);
}
FfxFloat16x2 ffxBroadcastFloat16x2(FfxFloat16 a)
{
    return FfxFloat16x2(a, a);
}
FfxFloat16x3 ffxBroadcastFloat16x3(FfxFloat16 a)
{
    return FfxFloat16x3(a, a, a);
}
FfxFloat16x4 ffxBroadcastFloat16x4(FfxFloat16 a)
{
    return FfxFloat16x4(a, a, a, a);
}
#define FFXM_BROADCAST_FLOAT16(a)   FfxFloat16(a)
#define FFXM_BROADCAST_FLOAT16X2(a) FfxFloat16x2(FfxFloat16(a))
#define FFXM_BROADCAST_FLOAT16X3(a) FfxFloat16x3(FfxFloat16(a))
#define FFXM_BROADCAST_FLOAT16X4(a) FfxFloat16x4(FfxFloat16(a))
//------------------------------------------------------------------------------------------------------------------------------
FfxInt16 ffxBroadcastInt16(FfxInt16 a)
{
    return FfxInt16(a);
}
FfxInt16x2 ffxBroadcastInt16x2(FfxInt16 a)
{
    return FfxInt16x2(a, a);
}
FfxInt16x3 ffxBroadcastInt16x3(FfxInt16 a)
{
    return FfxInt16x3(a, a, a);
}
FfxInt16x4 ffxBroadcastInt16x4(FfxInt16 a)
{
    return FfxInt16x4(a, a, a, a);
}
#define FFXM_BROADCAST_INT16(a)   FfxInt16(a)
#define FFXM_BROADCAST_INT16X2(a) FfxInt16x2(FfxInt16(a))
#define FFXM_BROADCAST_INT16X3(a) FfxInt16x3(FfxInt16(a))
#define FFXM_BROADCAST_INT16X4(a) FfxInt16x4(FfxInt16(a))
//------------------------------------------------------------------------------------------------------------------------------
FfxUInt16 ffxBroadcastUInt16(FfxUInt16 a)
{
    return FfxUInt16(a);
}
FfxUInt16x2 ffxBroadcastUInt16x2(FfxUInt16 a)
{
    return FfxUInt16x2(a, a);
}
FfxUInt16x3 ffxBroadcastUInt16x3(FfxUInt16 a)
{
    return FfxUInt16x3(a, a, a);
}
FfxUInt16x4 ffxBroadcastUInt16x4(FfxUInt16 a)
{
    return FfxUInt16x4(a, a, a, a);
}
#define FFXM_BROADCAST_UINT16(a)   FfxUInt16(a)
#define FFXM_BROADCAST_UINT16X2(a) FfxUInt16x2(FfxUInt16(a))
#define FFXM_BROADCAST_UINT16X3(a) FfxUInt16x3(FfxUInt16(a))
#define FFXM_BROADCAST_UINT16X4(a) FfxUInt16x4(FfxUInt16(a))
//==============================================================================================================================
FfxUInt16 ffxAbsHalf(FfxUInt16 a)
{
    return FfxUInt16(abs(FfxInt16(a)));
}
FfxUInt16x2 ffxAbsHalf(FfxUInt16x2 a)
{
    return FfxUInt16x2(abs(FfxInt16x2(a)));
}
FfxUInt16x3 ffxAbsHalf(FfxUInt16x3 a)
{
    return FfxUInt16x3(abs(FfxInt16x3(a)));
}
FfxUInt16x4 ffxAbsHalf(FfxUInt16x4 a)
{
    return FfxUInt16x4(abs(FfxInt16x4(a)));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxClampHalf(FfxFloat16 x, FfxFloat16 n, FfxFloat16 m)
{
    return clamp(x, n, m);
}
FfxFloat16x2 ffxClampHalf(FfxFloat16x2 x, FfxFloat16x2 n, FfxFloat16x2 m)
{
    return clamp(x, n, m);
}
FfxFloat16x3 ffxClampHalf(FfxFloat16x3 x, FfxFloat16x3 n, FfxFloat16x3 m)
{
    return clamp(x, n, m);
}
FfxFloat16x4 ffxClampHalf(FfxFloat16x4 x, FfxFloat16x4 n, FfxFloat16x4 m)
{
    return clamp(x, n, m);
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxFract(FfxFloat16 x)
{
    return fract(x);
}
FfxFloat16x2 ffxFract(FfxFloat16x2 x)
{
    return fract(x);
}
FfxFloat16x3 ffxFract(FfxFloat16x3 x)
{
    return fract(x);
}
FfxFloat16x4 ffxFract(FfxFloat16x4 x)
{
    return fract(x);
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxLerp(FfxFloat16 x, FfxFloat16 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x2 ffxLerp(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x2 ffxLerp(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 a)
{
    return mix(x, y, a);
}
FfxFloat16x3 ffxLerp(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 a)
{
    return mix(x, y, a);
}
FfxFloat16x3 ffxLerp(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x4 ffxLerp(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x4 ffxLerp(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 a)
{
    return mix(x, y, a);
}
//------------------------------------------------------------------------------------------------------------------------------
// No packed version of ffxMax3.
FfxFloat16 ffxMax3Half(FfxFloat16 x, FfxFloat16 y, FfxFloat16 z)
{
    return max(x, max(y, z));
}
FfxFloat16x2 ffxMax3Half(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 z)
{
    return max(x, max(y, z));
}
FfxFloat16x3 ffxMax3Half(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 z)
{
    return max(x, max(y, z));
}
FfxFloat16x4 ffxMax3Half(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 z)
{
    return max(x, max(y, z));
}
//------------------------------------------------------------------------------------------------------------------------------
// No packed version of ffxMin3.
FfxFloat16 ffxMin3Half(FfxFloat16 x, FfxFloat16 y, FfxFloat16 z)
{
    return min(x, min(y, z));
}
FfxFloat16x2 ffxMin3Half(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 z)
{
    return min(x, min(y, z));
}
FfxFloat16x3 ffxMin3Half(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 z)
{
    return min(x, min(y, z));
}
FfxFloat16x4 ffxMin3Half(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 z)
{
    return min(x, min(y, z));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxMed3Half(FfxFloat16 x, FfxFloat16 y, FfxFloat16 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FfxFloat16x2 ffxMed3Half(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FfxFloat16x3 ffxMed3Half(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FfxFloat16x4 ffxMed3Half(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxReciprocalHalf(FfxFloat16 x)
{
    return FFXM_BROADCAST_FLOAT16(1.0) / x;
}
FfxFloat16x2 ffxReciprocalHalf(FfxFloat16x2 x)
{
    return FFXM_BROADCAST_FLOAT16X2(1.0) / x;
}
FfxFloat16x3 ffxReciprocalHalf(FfxFloat16x3 x)
{
    return FFXM_BROADCAST_FLOAT16X3(1.0) / x;
}
FfxFloat16x4 ffxReciprocalHalf(FfxFloat16x4 x)
{
    return FFXM_BROADCAST_FLOAT16X4(1.0) / x;
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxReciprocalSquareRootHalf(FfxFloat16 x)
{
    return FFXM_BROADCAST_FLOAT16(1.0) / sqrt(x);
}
FfxFloat16x2 ffxReciprocalSquareRootHalf(FfxFloat16x2 x)
{
    return FFXM_BROADCAST_FLOAT16X2(1.0) / sqrt(x);
}
FfxFloat16x3 ffxReciprocalSquareRootHalf(FfxFloat16x3 x)
{
    return FFXM_BROADCAST_FLOAT16X3(1.0) / sqrt(x);
}
FfxFloat16x4 ffxReciprocalSquareRootHalf(FfxFloat16x4 x)
{
    return FFXM_BROADCAST_FLOAT16X4(1.0) / sqrt(x);
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxSaturate(FfxFloat16 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16(0.0), FFXM_BROADCAST_FLOAT16(1.0));
}
FfxFloat16x2 ffxSaturate(FfxFloat16x2 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16X2(0.0), FFXM_BROADCAST_FLOAT16X2(1.0));
}
FfxFloat16x3 ffxSaturate(FfxFloat16x3 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16X3(0.0), FFXM_BROADCAST_FLOAT16X3(1.0));
}
FfxFloat16x4 ffxSaturate(FfxFloat16x4 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16X4(0.0), FFXM_BROADCAST_FLOAT16X4(1.0));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxUInt16 ffxBitShiftRightHalf(FfxUInt16 a, FfxUInt16 b)
{
    return FfxUInt16(FfxInt16(a) >> FfxInt16(b));
}
FfxUInt16x2 ffxBitShiftRightHalf(FfxUInt16x2 a, FfxUInt16x2 b)
{
    return FfxUInt16x2(FfxInt16x2(a) >> FfxInt16x2(b));
}
FfxUInt16x3 ffxBitShiftRightHalf(FfxUInt16x3 a, FfxUInt16x3 b)
{
    return FfxUInt16x3(FfxInt16x3(a) >> FfxInt16x3(b));
}
FfxUInt16x4 ffxBitShiftRightHalf(FfxUInt16x4 a, FfxUInt16x4 b)
{
    return FfxUInt16x4(FfxInt16x4(a) >> FfxInt16x4(b));
}
#endif // FFXM_HALF

#if defined(FFXM_WAVE)
// Where 'x' must be a compile time literal.
FfxFloat32 AWaveXorF1(FfxFloat32 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxFloat32x2 AWaveXorF2(FfxFloat32x2 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxFloat32x3 AWaveXorF3(FfxFloat32x3 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxFloat32x4 AWaveXorF4(FfxFloat32x4 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32 AWaveXorU1(FfxUInt32 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32x2 AWaveXorU2(FfxUInt32x2 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32x3 AWaveXorU3(FfxUInt32x3 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32x4 AWaveXorU4(FfxUInt32x4 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxBoolean AWaveIsFirstLane()
{
    return subgroupElect();
}
FfxUInt32 AWaveLaneIndex()
{
    return gl_SubgroupInvocationID;
}
FfxBoolean AWaveReadAtLaneIndexB1(FfxBoolean v, FfxUInt32 x )
{
    return subgroupShuffle(v, x);
}
FfxUInt32 AWavePrefixCountBits(FfxBoolean v)
{
    return subgroupBallotExclusiveBitCount(subgroupBallot(v));
}
FfxUInt32 AWaveActiveCountBits(FfxBoolean v)
{
    return subgroupBallotBitCount(subgroupBallot(v));
}
FfxUInt32 AWaveReadLaneFirstU1(FfxUInt32 v)
{
    return subgroupBroadcastFirst(v);
}
FfxUInt32 WaveOr(FfxUInt32 a)
{
    return subgroupOr(a);
}
FfxFloat32 WaveMin(FfxFloat32 a)
{
    return subgroupMin(a);
}
FfxFloat32 WaveMax(FfxFloat32 a)
{
    return subgroupMax(a);
}
FfxUInt32 WaveLaneCount()
{
    return gl_SubgroupSize;
}
#if defined(FFXM_WAVE_ALL_TRUE)
FfxBoolean WaveAllTrue(FfxBoolean v)
{
    return subgroupAll(v);
}
#endif
FfxFloat32 QuadReadX(FfxFloat32 v)
{
    return subgroupQuadSwapHorizontal(v);
}
FfxFloat32x2 QuadReadX(FfxFloat32x2 v)
{
    return subgroupQuadSwapHorizontal(v);
}
FfxFloat32 QuadReadY(FfxFloat32 v)
{
    return subgroupQuadSwapVertical(v);
}
FfxFloat32x2 QuadReadY(FfxFloat32x2 v)
{
    return subgroupQuadSwapVertical(v);
}

//------------------------------------------------------------------------------------------------------------------------------
#if FFXM_HALF
FfxFloat16x2 ffxWaveXorFloat16x2(FfxFloat16x2 v, FfxUInt32 x)
{
    return FFXM_UINT32_TO_FLOAT16X2(subgroupShuffleXor(FFXM_FLOAT16X2_TO_UINT32(v), x));
}
FfxFloat16x4 ffxWaveXorFloat16x4(FfxFloat16x4 v, FfxUInt32 x)
{
    return FFXM_UINT32X2_TO_FLOAT16X4(subgroupShuffleXor(FFXM_FLOAT16X4_TO_UINT32X2(v), x));
}
FfxUInt16x2 ffxWaveXorUint16x2(FfxUInt16x2 v, FfxUInt32 x)
{
    return FFXM_UINT32_TO_UINT16X2(subgroupShuffleXor(FFXM_UINT16X2_TO_UINT32(v), x));
}
FfxUInt16x4 ffxWaveXorUint16x4(FfxUInt16x4 v, FfxUInt32 x)
{
    return FFXM_UINT32X2_TO_UINT16X4(subgroupShuffleXor(FFXM_UINT16X4_TO_UINT32X2(v), x));
}
#endif // FFXM_HALF
#endif // #if defined(FFXM_WAVE)
// --- end ffxm_core_glsl.h ---
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin ffxm_core_gpu_common.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// Shader quality related defines
///
/// FFXM_FSR2_OPTION_SHADER_OPT_BALANCED. If defined, optimizations related to the balanced preset will be enabled.
/// FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE. If defined, optimizations related to the performance preset will be enabled.
/// FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE. If defined, optimizations related to the ultra performance preset will be enabled.
#ifndef FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
#define FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE 0
#endif
#ifndef FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE
#define FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE 0
#endif
#ifndef FFXM_FSR2_OPTION_SHADER_OPT_BALANCED
#define FFXM_FSR2_OPTION_SHADER_OPT_BALANCED 0
#endif
/// FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE. Helper to identify if any of these profiles is used.
#define FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE (FFXM_FSR2_OPTION_SHADER_OPT_BALANCED || FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE)

/// Both Balanced/Performance. Keep the temporal reactive as a separate RT to improve bandwidth of color history buffer.
#define FFXM_SHADER_QUALITY_OPT_SEPARATE_TEMPORAL_REACTIVE FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Both Balanced/Performance. Disable deringing when doing the color reprojection with the history
#define FFXM_SHADER_QUALITY_OPT_DISABLE_DERINGING FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Both Balanced/Performance. Disable the Luma stability factor
#define FFXM_SHADER_QUALITY_OPT_DISABLE_LUMA_INSTABILITY FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Both Balanced/Performance. Use a 5-tap Lanczos kernel instead of the 9-tap used for `Quality` for upsampling
#define FFXM_SHADER_QUALITY_OPT_UPSCALING_LANCZOS_5TAP FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Balanced. Use Catmull-Rom (9 samples) for history reprojection
#define FFXM_SHADER_QUALITY_OPT_REPROJECT_CATMULL_9TAP FFXM_FSR2_OPTION_SHADER_OPT_BALANCED
/// Performance. PreparedInputColor is now stored as R8G8B8A8_Unorm tonemapped data. Rectification don't use `YCoCg` anymore
#define FFXM_SHADER_QUALITY_OPT_TONEMAPPED_RGB_PREPARED_INPUT_COLOR FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE
/// Performance. Use Catmull-Rom (5 samples) for history reprojection
#define FFXM_SHADER_QUALITY_OPT_REPROJECT_CATMULL_5TAP FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE

#if !defined(FFXM_SHADER_PLATFORM_GLES_3_2)
#define FFXM_SHADER_PLATFORM_GLES_3_2 (0)
#endif

/// A define for a true value in a boolean expression.
///
/// @ingroup GPUCore
#define FFXM_TRUE (true)

/// A define for a false value in a boolean expression.
///
/// @ingroup GPUCore
#define FFXM_FALSE (false)

/// A define value for positive infinity.
///
/// @ingroup GPUCore
#define FFXM_POSITIVE_INFINITY_FLOAT ffxAsFloat(0x7f800000u)

/// A define value for negative infinity.
///
/// @ingroup GPUCore
#define FFXM_NEGATIVE_INFINITY_FLOAT ffxAsFloat(0xff800000u)

/// A define value for PI.
///
/// @ingroup GPUCore
#define FFXM_PI  (3.14159)


/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxReciprocal(FfxFloat32 value)
{
    return rcp(value);
}

/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxReciprocal(FfxFloat32x2 value)
{
    return rcp(value);
}

/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxReciprocal(FfxFloat32x3 value)
{
    return rcp(value);
}

/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxReciprocal(FfxFloat32x4 value)
{
    return rcp(value);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32 ffxMin(FfxFloat32 x, FfxFloat32 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxMin(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxMin(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxMin(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32 ffxMin(FfxInt32 x, FfxInt32 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x2 ffxMin(FfxInt32x2 x, FfxInt32x2 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x3 ffxMin(FfxInt32x3 x, FfxInt32x3 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x4 ffxMin(FfxInt32x4 x, FfxInt32x4 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32 ffxMin(FfxUInt32 x, FfxUInt32 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxMin(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxMin(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxMin(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return min(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32 ffxMax(FfxFloat32 x, FfxFloat32 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxMax(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxMax(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxMax(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32 ffxMax(FfxInt32 x, FfxInt32 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x2 ffxMax(FfxInt32x2 x, FfxInt32x2 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x3 ffxMax(FfxInt32x3 x, FfxInt32x3 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x4 ffxMax(FfxInt32x4 x, FfxInt32x4 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32 ffxMax(FfxUInt32 x, FfxUInt32 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxMax(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxMax(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxMax(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return max(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32 ffxPow(FfxFloat32 x, FfxFloat32 y)
{
    return pow(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxPow(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return pow(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxPow(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return pow(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxPow(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return pow(x, y);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxSqrt(FfxFloat32 x)
{
    return sqrt(x);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxSqrt(FfxFloat32x2 x)
{
    return sqrt(x);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxSqrt(FfxFloat32x3 x)
{
    return sqrt(x);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxSqrt(FfxFloat32x4 x)
{
    return sqrt(x);
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxCopySignBit(FfxFloat32 d, FfxFloat32 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & FfxUInt32(0x80000000u)));
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxCopySignBit(FfxFloat32x2 d, FfxFloat32x2 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & ffxBroadcast2(0x80000000u)));
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxCopySignBit(FfxFloat32x3 d, FfxFloat32x3 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & ffxBroadcast3(0x80000000u)));
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxCopySignBit(FfxFloat32x4 d, FfxFloat32x4 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & ffxBroadcast4(0x80000000u)));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against 0.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or position.
///
/// @ingroup GPUCore
FfxFloat32 ffxIsSigned(FfxFloat32 m)
{
    return ffxSaturate(m * FfxFloat32(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against 0.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or position.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxIsSigned(FfxFloat32x2 m)
{
    return ffxSaturate(m * ffxBroadcast2(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against 0.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or position.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxIsSigned(FfxFloat32x3 m)
{
    return ffxSaturate(m * ffxBroadcast3(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against for have the sign set.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or positive.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxIsSigned(FfxFloat32x4 m)
{
    return ffxSaturate(m * ffxBroadcast4(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32 ffxIsGreaterThanZero(FfxFloat32 m)
{
    return ffxSaturate(m * FfxFloat32(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxIsGreaterThanZero(FfxFloat32x2 m)
{
    return ffxSaturate(m * ffxBroadcast2(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxIsGreaterThanZero(FfxFloat32x3 m)
{
    return ffxSaturate(m * ffxBroadcast3(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxIsGreaterThanZero(FfxFloat32x4 m)
{
    return ffxSaturate(m * ffxBroadcast4(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Convert a 32bit floating point value to sortable integer.
///
///  - If sign bit=0, flip the sign bit (positives).
///  - If sign bit=1, flip all bits     (negatives).
///
/// The function has the side effects that:
///  - Larger integers are more positive values.
///  - Float zero is mapped to center of integers (so clear to integer zero is a nice default for atomic max usage).
///
/// @param [in] value                       The floating point value to make sortable.
///
/// @returns
/// The sortable integer value.
///
/// @ingroup GPUCore
FfxUInt32 ffxFloatToSortableInteger(FfxUInt32 value)
{
    return value ^ ((AShrSU1(value, FfxUInt32(31))) | FfxUInt32(0x80000000));
}

/// Convert a sortable integer to a 32bit floating point value.
///
/// The function has the side effects that:
///  - If sign bit=1, flip the sign bit (positives).
///  - If sign bit=0, flip all bits     (negatives).
///
/// @param [in] value                       The floating point value to make sortable.
///
/// @returns
/// The sortable integer value.
///
/// @ingroup GPUCore
FfxUInt32 ffxSortableIntegerToFloat(FfxUInt32 value)
{
    return value ^ ((~AShrSU1(value, FfxUInt32(31))) | FfxUInt32(0x80000000));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateSqrt(FfxFloat32 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> FfxUInt32(1)) + FfxUInt32(0x1fbc4639));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateReciprocal(FfxFloat32 value)
{
    return ffxAsFloat(FfxUInt32(0x7ef07ebb) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateReciprocalMedium(FfxFloat32 value)
{
    FfxFloat32 b = ffxAsFloat(FfxUInt32(0x7ef19fff) - ffxAsUInt32(value));
    return b * (-b * value + FfxFloat32(2.0));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal square root for.
///
/// @returns
/// An approximation of the reciprocal square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateReciprocalSquareRoot(FfxFloat32 value)
{
    return ffxAsFloat(FfxUInt32(0x5f347d74) - (ffxAsUInt32(value) >> FfxUInt32(1)));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateSqrt(FfxFloat32x2 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> ffxBroadcast2(1u)) + ffxBroadcast2(0x1fbc4639u));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateReciprocal(FfxFloat32x2 value)
{
    return ffxAsFloat(ffxBroadcast2(0x7ef07ebbu) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateReciprocalMedium(FfxFloat32x2 value)
{
    FfxFloat32x2 b = ffxAsFloat(ffxBroadcast2(0x7ef19fffu) - ffxAsUInt32(value));
    return b * (-b * value + ffxBroadcast2(2.0f));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateReciprocalSquareRoot(FfxFloat32x2 value)
{
    return ffxAsFloat(ffxBroadcast2(0x5f347d74u) - (ffxAsUInt32(value) >> ffxBroadcast2(1u)));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateSqrt(FfxFloat32x3 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> ffxBroadcast3(1u)) + ffxBroadcast3(0x1fbc4639u));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateReciprocal(FfxFloat32x3 value)
{
    return ffxAsFloat(ffxBroadcast3(0x7ef07ebbu) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateReciprocalMedium(FfxFloat32x3 value)
{
    FfxFloat32x3 b = ffxAsFloat(ffxBroadcast3(0x7ef19fffu) - ffxAsUInt32(value));
    return b * (-b * value + ffxBroadcast3(2.0f));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateReciprocalSquareRoot(FfxFloat32x3 value)
{
    return ffxAsFloat(ffxBroadcast3(0x5f347d74u) - (ffxAsUInt32(value) >> ffxBroadcast3(1u)));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateSqrt(FfxFloat32x4 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> ffxBroadcast4(1u)) + ffxBroadcast4(0x1fbc4639u));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateReciprocal(FfxFloat32x4 value)
{
    return ffxAsFloat(ffxBroadcast4(0x7ef07ebbu) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateReciprocalMedium(FfxFloat32x4 value)
{
    FfxFloat32x4 b = ffxAsFloat(ffxBroadcast4(0x7ef19fffu) - ffxAsUInt32(value));
    return b * (-b * value + ffxBroadcast4(2.0f));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateReciprocalSquareRoot(FfxFloat32x4 value)
{
    return ffxAsFloat(ffxBroadcast4(0x5f347d74u) - (ffxAsUInt32(value) >> ffxBroadcast4(1u)));
}

/// Calculate dot product of 'a' and 'b'.
///
/// @param [in] a                   First vector input.
/// @param [in] b                   Second vector input.
///
/// @returns
/// The value of <c><i>a</i></c> dot <c><i>b</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxDot2(FfxFloat32x2 a, FfxFloat32x2 b)
{
    return dot(a, b);
}

/// Calculate dot product of 'a' and 'b'.
///
/// @param [in] a                   First vector input.
/// @param [in] b                   Second vector input.
///
/// @returns
/// The value of <c><i>a</i></c> dot <c><i>b</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxDot3(FfxFloat32x3 a, FfxFloat32x3 b)
{
    return dot(a, b);
}

/// Calculate dot product of 'a' and 'b'.
///
/// @param [in] a                   First vector input.
/// @param [in] b                   Second vector input.
///
/// @returns
/// The value of <c><i>a</i></c> dot <c><i>b</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxDot4(FfxFloat32x4 a, FfxFloat32x4 b)
{
    return dot(a, b);
}


/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximatePQToGamma2Medium(FfxFloat32 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximatePQToLinear(FfxFloat32 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateGamma2ToPQ(FfxFloat32 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(2)) + FfxUInt32(0x2F9A4E46));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateGamma2ToPQMedium(FfxFloat32 a)
{
    FfxFloat32 b  = ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(2)) + FfxUInt32(0x2F9A4E46));
    FfxFloat32 b4 = b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateGamma2ToPQHigh(FfxFloat32 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateLinearToPQ(FfxFloat32 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(3)) + FfxUInt32(0x378D8723));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateLinearToPQMedium(FfxFloat32 a)
{
    FfxFloat32 b  = ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(3)) + FfxUInt32(0x378D8723));
    FfxFloat32 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateLinearToPQHigh(FfxFloat32 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximatePQToGamma2Medium(FfxFloat32x2 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximatePQToLinear(FfxFloat32x2 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateGamma2ToPQ(FfxFloat32x2 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(2u)) + ffxBroadcast2(0x2F9A4E46u));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateGamma2ToPQMedium(FfxFloat32x2 a)
{
    FfxFloat32x2 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(2u)) + ffxBroadcast2(0x2F9A4E46u));
    FfxFloat32x2 b4 = b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateGamma2ToPQHigh(FfxFloat32x2 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateLinearToPQ(FfxFloat32x2 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(3u)) + ffxBroadcast2(0x378D8723u));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateLinearToPQMedium(FfxFloat32x2 a)
{
    FfxFloat32x2 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(3u)) + ffxBroadcast2(0x378D8723u));
    FfxFloat32x2 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateLinearToPQHigh(FfxFloat32x2 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximatePQToGamma2Medium(FfxFloat32x3 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximatePQToLinear(FfxFloat32x3 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateGamma2ToPQ(FfxFloat32x3 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(2u)) + ffxBroadcast3(0x2F9A4E46u));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateGamma2ToPQMedium(FfxFloat32x3 a)
{
    FfxFloat32x3 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(2u)) + ffxBroadcast3(0x2F9A4E46u));
    FfxFloat32x3 b4 = b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateGamma2ToPQHigh(FfxFloat32x3 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateLinearToPQ(FfxFloat32x3 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(3u)) + ffxBroadcast3(0x378D8723u));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateLinearToPQMedium(FfxFloat32x3 a)
{
    FfxFloat32x3 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(3u)) + ffxBroadcast3(0x378D8723u));
    FfxFloat32x3 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateLinearToPQHigh(FfxFloat32x3 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximatePQToGamma2Medium(FfxFloat32x4 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximatePQToLinear(FfxFloat32x4 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateGamma2ToPQ(FfxFloat32x4 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(2u)) + ffxBroadcast4(0x2F9A4E46u));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateGamma2ToPQMedium(FfxFloat32x4 a)
{
    FfxFloat32x4 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(2u)) + ffxBroadcast4(0x2F9A4E46u));
    FfxFloat32x4 b4 = b * b * b * b * b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateGamma2ToPQHigh(FfxFloat32x4 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateLinearToPQ(FfxFloat32x4 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(3u)) + ffxBroadcast4(0x378D8723u));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateLinearToPQMedium(FfxFloat32x4 a)
{
    FfxFloat32x4 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(3u)) + ffxBroadcast4(0x378D8723u));
    FfxFloat32x4 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateLinearToPQHigh(FfxFloat32x4 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

// An approximation of sine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate sine for.
//
// @returns
// The approximate sine of <c><i>value</i></c>.
FfxFloat32 ffxParabolicSin(FfxFloat32 value)
{
    return value * abs(value) - value;
}

// An approximation of sine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate sine for.
//
// @returns
// The approximate sine of <c><i>value</i></c>.
FfxFloat32x2 ffxParabolicSin(FfxFloat32x2 x)
{
    return x * abs(x) - x;
}

// An approximation of cosine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate cosine for.
//
// @returns
// The approximate cosine of <c><i>value</i></c>.
FfxFloat32 ffxParabolicCos(FfxFloat32 x)
{
    x = ffxFract(x * FfxFloat32(0.5) + FfxFloat32(0.75));
    x = x * FfxFloat32(2.0) - FfxFloat32(1.0);
    return ffxParabolicSin(x);
}

// An approximation of cosine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate cosine for.
//
// @returns
// The approximate cosine of <c><i>value</i></c>.
FfxFloat32x2 ffxParabolicCos(FfxFloat32x2 x)
{
    x = ffxFract(x * ffxBroadcast2(0.5f) + ffxBroadcast2(0.75f));
    x = x * ffxBroadcast2(2.0f) - ffxBroadcast2(1.0f);
    return ffxParabolicSin(x);
}

// An approximation of both sine and cosine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate cosine for.
//
// @returns
// A <c><i>FfxFloat32x2</i></c> containing approximations of both sine and cosine of <c><i>value</i></c>.
FfxFloat32x2 ffxParabolicSinCos(FfxFloat32 x)
{
    FfxFloat32 y = ffxFract(x * FfxFloat32(0.5) + FfxFloat32(0.75));
    y = y * FfxFloat32(2.0) - FfxFloat32(1.0);
    return ffxParabolicSin(FfxFloat32x2(x, y));
}

/// Conditional free logic AND operation using values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneAnd(FfxUInt32 x, FfxUInt32 y)
{
    return min(x, y);
}

/// Conditional free logic AND operation using two values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneAnd(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return min(x, y);
}

/// Conditional free logic AND operation using two values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneAnd(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return min(x, y);
}

/// Conditional free logic AND operation using two values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneAnd(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return min(x, y);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneAnd(FfxUInt32 x)
{
    return x ^ FfxUInt32(1);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneAnd(FfxUInt32x2 x)
{
    return x ^ ffxBroadcast2(1u);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneAnd(FfxUInt32x3 x)
{
    return x ^ ffxBroadcast3(1u);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneAnd(FfxUInt32x4 x)
{
    return x ^ ffxBroadcast4(1u);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneOr(FfxUInt32 x, FfxUInt32 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneOr(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneOr(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneOr(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return max(x, y);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneAndToU1(FfxFloat32 x)
{
    return FfxUInt32(FfxFloat32(1.0) - x);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneAndToU2(FfxFloat32x2 x)
{
    return FfxUInt32x2(ffxBroadcast2(1.0) - x);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneAndToU3(FfxFloat32x3 x)
{
    return FfxUInt32x3(ffxBroadcast3(1.0) - x);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneAndToU4(FfxFloat32x4 x)
{
    return FfxUInt32x4(ffxBroadcast4(1.0) - x);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneAndOr(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return ffxSaturate(x * y + z);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneAndOr(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return ffxSaturate(x * y + z);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneAndOr(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return ffxSaturate(x * y + z);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneAndOr(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return ffxSaturate(x * y + z);
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneIsGreaterThanZero(FfxFloat32 x)
{
    return ffxSaturate(x * FfxFloat32(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneIsGreaterThanZero(FfxFloat32x2 x)
{
    return ffxSaturate(x * ffxBroadcast2(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneIsGreaterThanZero(FfxFloat32x3 x)
{
    return ffxSaturate(x * ffxBroadcast3(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneIsGreaterThanZero(FfxFloat32x4 x)
{
    return ffxSaturate(x * ffxBroadcast4(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneAnd(FfxFloat32 x)
{
    return FfxFloat32(1.0) - x;
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneAnd(FfxFloat32x2 x)
{
    return ffxBroadcast2(1.0) - x;
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneAnd(FfxFloat32x3 x)
{
    return ffxBroadcast3(1.0) - x;
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneAnd(FfxFloat32x4 x)
{
    return ffxBroadcast4(1.0) - x;
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneOr(FfxFloat32 x, FfxFloat32 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneOr(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneOr(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneOr(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return max(x, y);
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneSelect(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    FfxFloat32 r = (-x) * z + z;
    return x * y + r;
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneSelect(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    FfxFloat32x2 r = (-x) * z + z;
    return x * y + r;
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneSelect(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    FfxFloat32x3 r = (-x) * z + z;
    return x * y + r;
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneSelect(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    FfxFloat32x4 r = (-x) * z + z;
    return x * y + r;
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneIsSigned(FfxFloat32 x)
{
    return ffxSaturate(x * FfxFloat32(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneIsSigned(FfxFloat32x2 x)
{
    return ffxSaturate(x * ffxBroadcast2(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneIsSigned(FfxFloat32x3 x)
{
    return ffxSaturate(x * ffxBroadcast3(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneIsSigned(FfxFloat32x4 x)
{
    return ffxSaturate(x * ffxBroadcast4(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Compute a Rec.709 color space.
///
/// Rec.709 is used for some HDTVs.
///
/// Both Rec.709 and sRGB have a linear segment which as spec'ed would intersect the curved segment 2 times.
///  (a.) For 8-bit sRGB, steps {0 to 10.3} are in the linear region (4% of the encoding range).
///  (b.) For 8-bit  709, steps {0 to 20.7} are in the linear region (8% of the encoding range).
///
/// @param [in] color           The color to convert to Rec. 709.
///
/// @returns
/// The <c><i>color</i></c> in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxRec709FromLinear(FfxFloat32 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.018 * 4.5, 4.5, 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.099, -0.099);
    return clamp(j.x, color * j.y, pow(color, j.z) * k.x + k.y);
}

/// Compute a Rec.709 color space.
///
/// Rec.709 is used for some HDTVs.
///
/// Both Rec.709 and sRGB have a linear segment which as spec'ed would intersect the curved segment 2 times.
///  (a.) For 8-bit sRGB, steps {0 to 10.3} are in the linear region (4% of the encoding range).
///  (b.) For 8-bit  709, steps {0 to 20.7} are in the linear region (8% of the encoding range).
///
/// @param [in] color           The color to convert to Rec. 709.
///
/// @returns
/// The <c><i>color</i></c> in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxRec709FromLinear(FfxFloat32x2 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.018 * 4.5, 4.5, 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.099, -0.099);
    return clamp(j.xx, color * j.yy, pow(color, j.zz) * k.xx + k.yy);
}

/// Compute a Rec.709 color space.
///
/// Rec.709 is used for some HDTVs.
///
/// Both Rec.709 and sRGB have a linear segment which as spec'ed would intersect the curved segment 2 times.
///  (a.) For 8-bit sRGB, steps {0 to 10.3} are in the linear region (4% of the encoding range).
///  (b.) For 8-bit  709, steps {0 to 20.7} are in the linear region (8% of the encoding range).
///
/// @param [in] color           The color to convert to Rec. 709.
///
/// @returns
/// The <c><i>color</i></c> in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxRec709FromLinear(FfxFloat32x3 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.018 * 4.5, 4.5, 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.099, -0.099);
    return clamp(j.xxx, color * j.yyy, pow(color, j.zzz) * k.xxx + k.yyy);
}

/// Compute a gamma value from a linear value.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// Note: 'rcpX' is '1/x', where the 'x' is what would be used in <c><i>ffxLinearFromGamma</i></c>.
///
/// @param [in] value           The value to convert to gamma space from linear.
/// @param [in] power           The reciprocal of power value used for the gamma curve.
///
/// @returns
/// A value in gamma space.
///
/// @ingroup GPUCore
FfxFloat32 ffxGammaFromLinear(FfxFloat32 value, FfxFloat32 power)
{
    return pow(value, FfxFloat32(power));
}

/// Compute a gamma value from a linear value.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// Note: 'rcpX' is '1/x', where the 'x' is what would be used in <c><i>ffxLinearFromGamma</i></c>.
///
/// @param [in] value           The value to convert to gamma space from linear.
/// @param [in] power           The reciprocal of power value used for the gamma curve.
///
/// @returns
/// A value in gamma space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxGammaFromLinear(FfxFloat32x2 value, FfxFloat32 power)
{
    return pow(value, ffxBroadcast2(power));
}

/// Compute a gamma value from a linear value.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// Note: 'rcpX' is '1/x', where the 'x' is what would be used in <c><i>ffxLinearFromGamma</i></c>.
///
/// @param [in] value           The value to convert to gamma space from linear.
/// @param [in] power           The reciprocal of power value used for the gamma curve.
///
/// @returns
/// A value in gamma space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxGammaFromLinear(FfxFloat32x3 value, FfxFloat32 power)
{
    return pow(value, ffxBroadcast3(power));
}

/// Compute a PQ value from a linear value.
///
/// @param [in] value           The value to convert to PQ from linear.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxPQToLinear(FfxFloat32 value)
{
    FfxFloat32 p = pow(value, FfxFloat32(0.159302));
    return pow((FfxFloat32(0.835938) + FfxFloat32(18.8516) * p) / (FfxFloat32(1.0) + FfxFloat32(18.6875) * p), FfxFloat32(78.8438));
}

/// Compute a PQ value from a linear value.
///
/// @param [in] value           The value to convert to PQ from linear.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxPQToLinear(FfxFloat32x2 value)
{
    FfxFloat32x2 p = pow(value, ffxBroadcast2(0.159302));
    return pow((ffxBroadcast2(0.835938) + ffxBroadcast2(18.8516) * p) / (ffxBroadcast2(1.0) + ffxBroadcast2(18.6875) * p), ffxBroadcast2(78.8438));
}

/// Compute a PQ value from a linear value.
///
/// @param [in] value           The value to convert to PQ from linear.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxPQToLinear(FfxFloat32x3 value)
{
    FfxFloat32x3 p = pow(value, ffxBroadcast3(0.159302));
    return pow((ffxBroadcast3(0.835938) + ffxBroadcast3(18.8516) * p) / (ffxBroadcast3(1.0) + ffxBroadcast3(18.6875) * p), ffxBroadcast3(78.8438));
}

/// Compute a linear value from a SRGB value.
///
/// @param [in] value           The value to convert to linear from SRGB.
///
/// @returns
/// A value in SRGB space.
///
/// @ingroup GPUCore
FfxFloat32 ffxSrgbToLinear(FfxFloat32 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.055, -0.055);
    return clamp(j.x, value * j.y, pow(value, j.z) * k.x + k.y);
}

/// Compute a linear value from a SRGB value.
///
/// @param [in] value           The value to convert to linear from SRGB.
///
/// @returns
/// A value in SRGB space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxSrgbToLinear(FfxFloat32x2 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.055, -0.055);
    return clamp(j.xx, value * j.yy, pow(value, j.zz) * k.xx + k.yy);
}

/// Compute a linear value from a SRGB value.
///
/// @param [in] value           The value to convert to linear from SRGB.
///
/// @returns
/// A value in SRGB space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxSrgbToLinear(FfxFloat32x3 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.055, -0.055);
    return clamp(j.xxx, value * j.yyy, pow(value, j.zzz) * k.xxx + k.yyy);
}

/// Compute a linear value from a REC.709 value.
///
/// @param [in] color           The value to convert to linear from REC.709.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromRec709(FfxFloat32 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.081 / 4.5, 1.0 / 4.5, 1.0 / 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.099, 0.099 / 1.099);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(color - j.x), color * j.y, pow(color * k.x + k.y, j.z));
}

/// Compute a linear value from a REC.709 value.
///
/// @param [in] color           The value to convert to linear from REC.709.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromRec709(FfxFloat32x2 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.081 / 4.5, 1.0 / 4.5, 1.0 / 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.099, 0.099 / 1.099);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(color - j.xx), color * j.yy, pow(color * k.xx + k.yy, j.zz));
}

/// Compute a linear value from a REC.709 value.
///
/// @param [in] color           The value to convert to linear from REC.709.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromRec709(FfxFloat32x3 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.081 / 4.5, 1.0 / 4.5, 1.0 / 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.099, 0.099 / 1.099);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(color - j.xxx), color * j.yyy, pow(color * k.xxx + k.yyy, j.zzz));
}

/// Compute a linear value from a value in a gamma space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] color           The value to convert to linear in gamma space.
/// @param [in] power           The power value used for the gamma curve.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromGamma(FfxFloat32 color, FfxFloat32 power)
{
    return pow(color, FfxFloat32(power));
}

/// Compute a linear value from a value in a gamma space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] color           The value to convert to linear in gamma space.
/// @param [in] power           The power value used for the gamma curve.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromGamma(FfxFloat32x2 color, FfxFloat32 power)
{
    return pow(color, ffxBroadcast2(power));
}

/// Compute a linear value from a value in a gamma space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] color           The value to convert to linear in gamma space.
/// @param [in] power           The power value used for the gamma curve.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromGamma(FfxFloat32x3 color, FfxFloat32 power)
{
    return pow(color, ffxBroadcast3(power));
}

/// Compute a linear value from a value in a PQ space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in PQ space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromPQ(FfxFloat32 value)
{
    FfxFloat32 p = pow(value, FfxFloat32(0.0126833));
    return pow(ffxSaturate(p - FfxFloat32(0.835938)) / (FfxFloat32(18.8516) - FfxFloat32(18.6875) * p), FfxFloat32(6.27739));
}

/// Compute a linear value from a value in a PQ space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in PQ space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromPQ(FfxFloat32x2 value)
{
    FfxFloat32x2 p = pow(value, ffxBroadcast2(0.0126833));
    return pow(ffxSaturate(p - ffxBroadcast2(0.835938)) / (ffxBroadcast2(18.8516) - ffxBroadcast2(18.6875) * p), ffxBroadcast2(6.27739));
}

/// Compute a linear value from a value in a PQ space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in PQ space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromPQ(FfxFloat32x3 value)
{
    FfxFloat32x3 p = pow(value, ffxBroadcast3(0.0126833));
    return pow(ffxSaturate(p - ffxBroadcast3(0.835938)) / (ffxBroadcast3(18.8516) - ffxBroadcast3(18.6875) * p), ffxBroadcast3(6.27739));
}

/// Compute a linear value from a value in a SRGB space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in SRGB space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromSrgb(FfxFloat32 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.04045 / 12.92, 1.0 / 12.92, 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.055, 0.055 / 1.055);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(value - j.x), value * j.y, pow(value * k.x + k.y, j.z));
}

/// Compute a linear value from a value in a SRGB space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in SRGB space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromSrgb(FfxFloat32x2 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.04045 / 12.92, 1.0 / 12.92, 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.055, 0.055 / 1.055);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(value - j.xx), value * j.yy, pow(value * k.xx + k.yy, j.zz));
}

/// Compute a linear value from a value in a SRGB space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in SRGB space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromSrgb(FfxFloat32x3 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.04045 / 12.92, 1.0 / 12.92, 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.055, 0.055 / 1.055);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(value - j.xxx), value * j.yyy, pow(value * k.xxx + k.yyy, j.zzz));
}

/// A remapping of 64x1 to 8x8 imposing rotated 2x2 pixel quads in quad linear.
///
///  543210
///  ======
///  ..xxx.
///  yy...y
///
/// @param [in] a       The input 1D coordinates to remap.
///
/// @returns
/// The remapped 2D coordinates.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxRemapForQuad(FfxUInt32 a)
{
    return FfxUInt32x2(bitfieldExtract(a, 1u, 3u), bitfieldInsertMask(bitfieldExtract(a, 3u, 3u), a, 1u));
}

/// A helper function performing a remap 64x1 to 8x8 remapping which is necessary for 2D wave reductions.
///
/// The 64-wide lane indices to 8x8 remapping is performed as follows:
///
///     00 01 08 09 10 11 18 19
///     02 03 0a 0b 12 13 1a 1b
///     04 05 0c 0d 14 15 1c 1d
///     06 07 0e 0f 16 17 1e 1f
///     20 21 28 29 30 31 38 39
///     22 23 2a 2b 32 33 3a 3b
///     24 25 2c 2d 34 35 3c 3d
///     26 27 2e 2f 36 37 3e 3f
///
/// @param [in] a       The input 1D coordinate to remap.
///
/// @returns
/// The remapped 2D coordinates.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxRemapForWaveReduction(FfxUInt32 a)
{
    return FfxUInt32x2(bitfieldInsertMask(bitfieldExtract(a, 2u, 3u), a, 1u), bitfieldInsertMask(bitfieldExtract(a, 3u, 3u), bitfieldExtract(a, 1u, 2u), 2u));
}
// --- end ffxm_core_gpu_common.h ---
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin fsr1/ffxm_fsr1.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// @defgroup FfxGPUFsr1 FidelityFX FSR1
/// FidelityFX Super Resolution 1 GPU documentation
///
/// @ingroup FfxGPUEffects

/// Setup required constant values for EASU (works on CPU or GPU).
///
/// @param [out] con0
/// @param [out] con1
/// @param [out] con2
/// @param [out] con3
/// @param [in] inputViewportInPixelsX                  The rendered image resolution being upscaled in X dimension.
/// @param [in] inputViewportInPixelsY                  The rendered image resolution being upscaled in Y dimension.
/// @param [in] inputSizeInPixelsX                      The resolution of the resource containing the input image (useful for dynamic resolution) in X dimension.
/// @param [in] inputSizeInPixelsY                      The resolution of the resource containing the input image (useful for dynamic resolution) in Y dimension.
/// @param [in] outputSizeInPixelsX                     The display resolution which the input image gets upscaled to in X dimension.
/// @param [in] outputSizeInPixelsY                     The display resolution which the input image gets upscaled to in Y dimension.
///
/// @ingroup FfxGPUFsr1
FFXM_STATIC void ffxFsrPopulateEasuConstants(
    FFXM_PARAMETER_INOUT FfxUInt32x4 con0,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con1,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con2,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con3,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsY)
{
    // Output integer position to a pixel position in viewport.
    con0[0] = ffxAsUInt32(inputViewportInPixelsX * ffxReciprocal(outputSizeInPixelsX));
    con0[1] = ffxAsUInt32(inputViewportInPixelsY * ffxReciprocal(outputSizeInPixelsY));
    con0[2] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsX * ffxReciprocal(outputSizeInPixelsX) - FfxFloat32(0.5));
    con0[3] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsY * ffxReciprocal(outputSizeInPixelsY) - FfxFloat32(0.5));

    // Viewport pixel position to normalized image space.
    // This is used to get upper-left of 'F' tap.
    con1[0] = ffxAsUInt32(ffxReciprocal(inputSizeInPixelsX));
    con1[1] = ffxAsUInt32(ffxReciprocal(inputSizeInPixelsY));

    // Centers of gather4, first offset from upper-left of 'F'.
    //      +---+---+
    //      |   |   |
    //      +--(0)--+
    //      | b | c |
    //  +---F---+---+---+
    //  | e | f | g | h |
    //  +--(1)--+--(2)--+
    //  | i | j | k | l |
    //  +---+---+---+---+
    //      | n | o |
    //      +--(3)--+
    //      |   |   |
    //      +---+---+
    con1[2] = ffxAsUInt32(FfxFloat32(1.0) * ffxReciprocal(inputSizeInPixelsX));
    con1[3] = ffxAsUInt32(FfxFloat32(-1.0) * ffxReciprocal(inputSizeInPixelsY));

    // These are from (0) instead of 'F'.
    con2[0] = ffxAsUInt32(FfxFloat32(-1.0) * ffxReciprocal(inputSizeInPixelsX));
    con2[1] = ffxAsUInt32(FfxFloat32(2.0) * ffxReciprocal(inputSizeInPixelsY));
    con2[2] = ffxAsUInt32(FfxFloat32(1.0) * ffxReciprocal(inputSizeInPixelsX));
    con2[3] = ffxAsUInt32(FfxFloat32(2.0) * ffxReciprocal(inputSizeInPixelsY));
    con3[0] = ffxAsUInt32(FfxFloat32(0.0) * ffxReciprocal(inputSizeInPixelsX));
    con3[1] = ffxAsUInt32(FfxFloat32(4.0) * ffxReciprocal(inputSizeInPixelsY));
    con3[2] = con3[3] = 0;
}

/// Setup required constant values for EASU (works on CPU or GPU).
///
/// @param [out] con0
/// @param [out] con1
/// @param [out] con2
/// @param [out] con3
/// @param [in] inputViewportInPixelsX              The resolution of the input in the X dimension.
/// @param [in] inputViewportInPixelsY              The resolution of the input in the Y dimension.
/// @param [in] inputSizeInPixelsX                  The input size in pixels in the X dimension.
/// @param [in] inputSizeInPixelsY                  The input size in pixels in the Y dimension.
/// @param [in] outputSizeInPixelsX                 The output size in pixels in the X dimension.
/// @param [in] outputSizeInPixelsY                 The output size in pixels in the Y dimension.
/// @param [in] inputOffsetInPixelsX                The input image offset in the X dimension into the resource containing it (useful for dynamic resolution).
/// @param [in] inputOffsetInPixelsY                The input image offset in the Y dimension into the resource containing it (useful for dynamic resolution).
///
/// @ingroup FfxGPUFsr1
FFXM_STATIC void ffxFsrPopulateEasuConstantsOffset(
    FFXM_PARAMETER_INOUT FfxUInt32x4 con0,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con1,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con2,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con3,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 inputOffsetInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputOffsetInPixelsY)
{
    ffxFsrPopulateEasuConstants(
        con0,
        con1,
        con2,
        con3,
        inputViewportInPixelsX,
        inputViewportInPixelsY,
        inputSizeInPixelsX,
        inputSizeInPixelsY,
        outputSizeInPixelsX,
        outputSizeInPixelsY);

    // override
    con0[2] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsX * ffxReciprocal(outputSizeInPixelsX) - FfxFloat32(0.5) + inputOffsetInPixelsX);
    con0[3] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsY * ffxReciprocal(outputSizeInPixelsY) - FfxFloat32(0.5) + inputOffsetInPixelsY);
}

#if defined(FFXM_GPU) && defined(FFXM_FSR_EASU_FLOAT)
// Input callback prototypes, need to be implemented by calling shader
FfxFloat32x4 FsrEasuRF(FfxFloat32x2 p);
FfxFloat32x4 FsrEasuGF(FfxFloat32x2 p);
FfxFloat32x4 FsrEasuBF(FfxFloat32x2 p);

// Filtering for a given tap for the scalar.
void fsrEasuTapFloat(
    FFXM_PARAMETER_INOUT FfxFloat32x3 accumulatedColor,   // Accumulated color, with negative lobe.
    FFXM_PARAMETER_INOUT FfxFloat32 accumulatedWeight,    // Accumulated weight.
    FFXM_PARAMETER_IN FfxFloat32x2 pixelOffset,           // Pixel offset from resolve position to tap.
    FFXM_PARAMETER_IN FfxFloat32x2 gradientDirection,     // Gradient direction.
    FFXM_PARAMETER_IN FfxFloat32x2 length,                // Length.
    FFXM_PARAMETER_IN FfxFloat32 negativeLobeStrength,    // Negative lobe strength.
    FFXM_PARAMETER_IN FfxFloat32 clippingPoint,           // Clipping point.
    FFXM_PARAMETER_IN FfxFloat32x3 color)                 // Tap color.
{
    // Rotate offset by direction.
    FfxFloat32x2 rotatedOffset;
    rotatedOffset.x = (pixelOffset.x * (gradientDirection.x)) + (pixelOffset.y * gradientDirection.y);
    rotatedOffset.y = (pixelOffset.x * (-gradientDirection.y)) + (pixelOffset.y * gradientDirection.x);

    // Anisotropy.
    rotatedOffset *= length;

    // Compute distance^2.
    FfxFloat32 distanceSquared = rotatedOffset.x * rotatedOffset.x + rotatedOffset.y * rotatedOffset.y;

    // Limit to the window as at corner, 2 taps can easily be outside.
    distanceSquared = ffxMin(distanceSquared, clippingPoint);

    // Approximation of lancos2 without sin() or rcp(), or sqrt() to get x.
    //  (25/16 * (2/5 * x^2 - 1)^2 - (25/16 - 1)) * (1/4 * x^2 - 1)^2
    //  |_______________________________________|   |_______________|
    //                   base                             window
    // The general form of the 'base' is,
    //  (a*(b*x^2-1)^2-(a-1))
    // Where 'a=1/(2*b-b^2)' and 'b' moves around the negative lobe.
    FfxFloat32 weightB = FfxFloat32(2.0 / 5.0) * distanceSquared + FfxFloat32(-1.0);
    FfxFloat32 weightA = negativeLobeStrength * distanceSquared + FfxFloat32(-1.0);
    weightB *= weightB;
    weightA *= weightA;
    weightB = FfxFloat32(25.0 / 16.0) * weightB + FfxFloat32(-(25.0 / 16.0 - 1.0));
    FfxFloat32 weight = weightB * weightA;

    // Do weighted average.
    accumulatedColor += color * weight;
    accumulatedWeight += weight;
}

// Accumulate direction and length.
void fsrEasuSetFloat(
    FFXM_PARAMETER_INOUT FfxFloat32x2 direction,
    FFXM_PARAMETER_INOUT FfxFloat32 length,
    FFXM_PARAMETER_IN FfxFloat32x2 pp,
    FFXM_PARAMETER_IN FfxBoolean biS,
    FFXM_PARAMETER_IN FfxBoolean biT,
    FFXM_PARAMETER_IN FfxBoolean biU,
    FFXM_PARAMETER_IN FfxBoolean biV,
    FFXM_PARAMETER_IN FfxFloat32 lA,
    FFXM_PARAMETER_IN FfxFloat32 lB,
    FFXM_PARAMETER_IN FfxFloat32 lC,
    FFXM_PARAMETER_IN FfxFloat32 lD,
    FFXM_PARAMETER_IN FfxFloat32 lE)
{
    // Compute bilinear weight, branches factor out as predicates are compiler time immediates.
    //  s t
    //  u v
    FfxFloat32 weight = FfxFloat32(0.0);
    if (biS)
        weight = (FfxFloat32(1.0) - pp.x) * (FfxFloat32(1.0) - pp.y);
    if (biT)
        weight = pp.x * (FfxFloat32(1.0) - pp.y);
    if (biU)
        weight = (FfxFloat32(1.0) - pp.x) * pp.y;
    if (biV)
        weight = pp.x * pp.y;

    // Direction is the '+' diff.
    //    a
    //  b c d
    //    e
    // Then takes magnitude from abs average of both sides of 'c'.
    // Length converts gradient reversal to 0, smoothly to non-reversal at 1, shaped, then adding horz and vert terms.
    FfxFloat32 dc = lD - lC;
    FfxFloat32 cb = lC - lB;
    FfxFloat32 lengthX = max(abs(dc), abs(cb));
    lengthX = ffxApproximateReciprocal(lengthX);
    FfxFloat32 directionX = lD - lB;
    direction.x += directionX * weight;
    lengthX = ffxSaturate(abs(directionX) * lengthX);
    lengthX *= lengthX;
    length += lengthX * weight;

    // Repeat for the y axis.
    FfxFloat32 ec = lE - lC;
    FfxFloat32 ca = lC - lA;
    FfxFloat32 lengthY = max(abs(ec), abs(ca));
    lengthY = ffxApproximateReciprocal(lengthY);
    FfxFloat32 directionY = lE - lA;
    direction.y += directionY * weight;
    lengthY = ffxSaturate(abs(directionY) * lengthY);
    lengthY *= lengthY;
    length += lengthY * weight;
}

/// Apply edge-aware spatial upsampling using 32bit floating point precision calculations.
///
/// @param [out] outPixel               The computed color of a pixel.
/// @param [in]  integerPosition        Integer pixel position within the output.
/// @param [in]  con0                   The first constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
/// @param [in]  con1                   The second constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
/// @param [in]  con2                   The third constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
/// @param [in]  con3                   The fourth constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
///
/// @ingroup FSR
void ffxFsrEasuFloat(
    FFXM_PARAMETER_OUT FfxFloat32x3 pix,
    FFXM_PARAMETER_IN FfxUInt32x2 ip,
    FFXM_PARAMETER_IN FfxUInt32x4 con0,
    FFXM_PARAMETER_IN FfxUInt32x4 con1,
    FFXM_PARAMETER_IN FfxUInt32x4 con2,
    FFXM_PARAMETER_IN FfxUInt32x4 con3)
{
    // Get position of 'f'.
    FfxFloat32x2 pp = FfxFloat32x2(ip) * ffxAsFloat(con0.xy) + ffxAsFloat(con0.zw);
    FfxFloat32x2 fp = floor(pp);
    pp -= fp;

    // 12-tap kernel.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    // Gather 4 ordering.
    //  a b
    //  r g
    // For packed FP16, need either {rg} or {ab} so using the following setup for gather in all versions,
    //    a b    <- unused (z)
    //    r g
    //  a b a b
    //  r g r g
    //    a b
    //    r g    <- unused (z)
    // Allowing dead-code removal to remove the 'z's.
    FfxFloat32x2 p0 = fp * ffxAsFloat(con1.xy) + ffxAsFloat(con1.zw);

    // These are from p0 to avoid pulling two constants on pre-Navi hardware.
    FfxFloat32x2 p1    = p0 + ffxAsFloat(con2.xy);
    FfxFloat32x2 p2    = p0 + ffxAsFloat(con2.zw);
    FfxFloat32x2 p3    = p0 + ffxAsFloat(con3.xy);
    FfxFloat32x4 bczzR = FsrEasuRF(p0);
    FfxFloat32x4 bczzG = FsrEasuGF(p0);
    FfxFloat32x4 bczzB = FsrEasuBF(p0);
    FfxFloat32x4 ijfeR = FsrEasuRF(p1);
    FfxFloat32x4 ijfeG = FsrEasuGF(p1);
    FfxFloat32x4 ijfeB = FsrEasuBF(p1);
    FfxFloat32x4 klhgR = FsrEasuRF(p2);
    FfxFloat32x4 klhgG = FsrEasuGF(p2);
    FfxFloat32x4 klhgB = FsrEasuBF(p2);
    FfxFloat32x4 zzonR = FsrEasuRF(p3);
    FfxFloat32x4 zzonG = FsrEasuGF(p3);
    FfxFloat32x4 zzonB = FsrEasuBF(p3);

    // Simplest multi-channel approximate luma possible (luma times 2, in 2 FMA/MAD).
    FfxFloat32x4 bczzL = bczzB * ffxBroadcast4(0.5) + (bczzR * ffxBroadcast4(0.5) + bczzG);
    FfxFloat32x4 ijfeL = ijfeB * ffxBroadcast4(0.5) + (ijfeR * ffxBroadcast4(0.5) + ijfeG);
    FfxFloat32x4 klhgL = klhgB * ffxBroadcast4(0.5) + (klhgR * ffxBroadcast4(0.5) + klhgG);
    FfxFloat32x4 zzonL = zzonB * ffxBroadcast4(0.5) + (zzonR * ffxBroadcast4(0.5) + zzonG);

    // Rename.
    FfxFloat32 bL = bczzL.x;
    FfxFloat32 cL = bczzL.y;
    FfxFloat32 iL = ijfeL.x;
    FfxFloat32 jL = ijfeL.y;
    FfxFloat32 fL = ijfeL.z;
    FfxFloat32 eL = ijfeL.w;
    FfxFloat32 kL = klhgL.x;
    FfxFloat32 lL = klhgL.y;
    FfxFloat32 hL = klhgL.z;
    FfxFloat32 gL = klhgL.w;
    FfxFloat32 oL = zzonL.z;
    FfxFloat32 nL = zzonL.w;

    // Accumulate for bilinear interpolation.
    FfxFloat32x2 dir = ffxBroadcast2(0.0);
    FfxFloat32  len = FfxFloat32(0.0);
    fsrEasuSetFloat(dir, len, pp, FFXM_TRUE,  FFXM_FALSE, FFXM_FALSE, FFXM_FALSE, bL, eL, fL, gL, jL);
    fsrEasuSetFloat(dir, len, pp, FFXM_FALSE, FFXM_TRUE,  FFXM_FALSE, FFXM_FALSE, cL, fL, gL, hL, kL);
    fsrEasuSetFloat(dir, len, pp, FFXM_FALSE, FFXM_FALSE, FFXM_TRUE,  FFXM_FALSE, fL, iL, jL, kL, nL);
    fsrEasuSetFloat(dir, len, pp, FFXM_FALSE, FFXM_FALSE, FFXM_FALSE, FFXM_TRUE,  gL, jL, kL, lL, oL);

    // Normalize with approximation, and cleanup close to zero.
    FfxFloat32x2 dir2 = dir * dir;
    FfxFloat32 dirR = dir2.x + dir2.y;
    FfxBoolean zro  = dirR < FfxFloat32(1.0 / 32768.0);
    dirR = ffxApproximateReciprocalSquareRoot(dirR);
    dirR = zro ? FfxFloat32(1.0) : dirR;
    dir.x = zro ? FfxFloat32(1.0) : dir.x;
    dir *= ffxBroadcast2(dirR);

    // Transform from {0 to 2} to {0 to 1} range, and shape with square.
    len = len * FfxFloat32(0.5);
    len *= len;

    // Stretch kernel {1.0 vert|horz, to sqrt(2.0) on diagonal}.
    FfxFloat32 stretch = (dir.x * dir.x + dir.y * dir.y) * ffxApproximateReciprocal(max(abs(dir.x), abs(dir.y)));

    // Anisotropic length after rotation,
    //  x := 1.0 lerp to 'stretch' on edges
    //  y := 1.0 lerp to 2x on edges
    FfxFloat32x2 len2 = FfxFloat32x2(FfxFloat32(1.0) + (stretch - FfxFloat32(1.0)) * len, FfxFloat32(1.0) + FfxFloat32(-0.5) * len);

    // Based on the amount of 'edge',
    // the window shifts from +/-{sqrt(2.0) to slightly beyond 2.0}.
    FfxFloat32 lob = FfxFloat32(0.5) + FfxFloat32((1.0 / 4.0 - 0.04) - 0.5) * len;

    // Set distance^2 clipping point to the end of the adjustable window.
    FfxFloat32 clp = ffxApproximateReciprocal(lob);

    // Accumulation mixed with min/max of 4 nearest.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    FfxFloat32x3 min4 =
        ffxMin(ffxMin3(FfxFloat32x3(ijfeR.z, ijfeG.z, ijfeB.z), FfxFloat32x3(klhgR.w, klhgG.w, klhgB.w), FfxFloat32x3(ijfeR.y, ijfeG.y, ijfeB.y)),
               FfxFloat32x3(klhgR.x, klhgG.x, klhgB.x));
    FfxFloat32x3 max4 =
        max(ffxMax3(FfxFloat32x3(ijfeR.z, ijfeG.z, ijfeB.z), FfxFloat32x3(klhgR.w, klhgG.w, klhgB.w), FfxFloat32x3(ijfeR.y, ijfeG.y, ijfeB.y)), FfxFloat32x3(klhgR.x, klhgG.x, klhgB.x));

    // Accumulation.
    FfxFloat32x3 aC = ffxBroadcast3(0.0);
    FfxFloat32  aW = FfxFloat32(0.0);
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, -1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(bczzR.x, bczzG.x, bczzB.x));  // b
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, -1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(bczzR.y, bczzG.y, bczzB.y));  // c
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(-1.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.x, ijfeG.x, ijfeB.x));  // i
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.y, ijfeG.y, ijfeB.y));   // j
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.z, ijfeG.z, ijfeB.z));   // f
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(-1.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.w, ijfeG.w, ijfeB.w));  // e
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.x, klhgG.x, klhgB.x));   // k
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(2.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.y, klhgG.y, klhgB.y));   // l
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(2.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.z, klhgG.z, klhgB.z));   // h
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.w, klhgG.w, klhgB.w));   // g
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, 2.0) - pp, dir, len2, lob, clp, FfxFloat32x3(zzonR.z, zzonG.z, zzonB.z));   // o
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, 2.0) - pp, dir, len2, lob, clp, FfxFloat32x3(zzonR.w, zzonG.w, zzonB.w));   // n

    // Normalize and dering.
    pix = ffxMin(max4, max(min4, aC * ffxBroadcast3(rcp(aW))));
}
#endif // #if defined(FFXM_GPU) && defined(FFXM_FSR_EASU_FLOAT)

#if defined(FFXM_GPU) && FFXM_HALF == 1 && defined(FFXM_FSR_EASU_HALF)
// Input callback prototypes, need to be implemented by calling shader
FfxFloat16x4 FsrEasuRH(FfxFloat32x2 p);
FfxFloat16x4 FsrEasuGH(FfxFloat32x2 p);
FfxFloat16x4 FsrEasuBH(FfxFloat32x2 p);

// This runs 2 taps in parallel.
void FsrEasuTapH(
    FFXM_PARAMETER_INOUT FfxFloat16x2 aCR,
    FFXM_PARAMETER_INOUT FfxFloat16x2 aCG,
    FFXM_PARAMETER_INOUT FfxFloat16x2 aCB,
    FFXM_PARAMETER_INOUT FfxFloat16x2 aW,
    FFXM_PARAMETER_IN FfxFloat16x2 offX,
    FFXM_PARAMETER_IN FfxFloat16x2 offY,
    FFXM_PARAMETER_IN FfxFloat16x2 dir,
    FFXM_PARAMETER_IN FfxFloat16x2 len,
    FFXM_PARAMETER_IN FfxFloat16 lob,
    FFXM_PARAMETER_IN FfxFloat16 clp,
    FFXM_PARAMETER_IN FfxFloat16x2 cR,
    FFXM_PARAMETER_IN FfxFloat16x2 cG,
    FFXM_PARAMETER_IN FfxFloat16x2 cB)
{
    FfxFloat16x2 vX, vY;
    vX = offX * dir.xx + offY * dir.yy;
    vY = offX * (-dir.yy) + offY * dir.xx;
    vX *= len.x;
    vY *= len.y;
    FfxFloat16x2 d2 = vX * vX + vY * vY;
    d2              = min(d2, FFXM_BROADCAST_FLOAT16X2(clp));
    FfxFloat16x2 wB = FFXM_BROADCAST_FLOAT16X2(2.0 / 5.0) * d2 + FFXM_BROADCAST_FLOAT16X2(-1.0);
    FfxFloat16x2 wA = FFXM_BROADCAST_FLOAT16X2(lob) * d2 + FFXM_BROADCAST_FLOAT16X2(-1.0);
    wB *= wB;
    wA *= wA;
    wB             = FFXM_BROADCAST_FLOAT16X2(25.0 / 16.0) * wB + FFXM_BROADCAST_FLOAT16X2(-(25.0 / 16.0 - 1.0));
    FfxFloat16x2 w = wB * wA;
    aCR += cR * w;
    aCG += cG * w;
    aCB += cB * w;
    aW += w;
}

// This runs 2 taps in parallel.
void FsrEasuSetH(
    FFXM_PARAMETER_INOUT FfxFloat16x2 dirPX,
    FFXM_PARAMETER_INOUT FfxFloat16x2  dirPY,
    FFXM_PARAMETER_INOUT FfxFloat16x2 lenP,
    FFXM_PARAMETER_IN FfxFloat16x2 pp,
    FFXM_PARAMETER_IN FfxBoolean biST,
    FFXM_PARAMETER_IN FfxBoolean biUV,
    FFXM_PARAMETER_IN FfxFloat16x2 lA,
    FFXM_PARAMETER_IN FfxFloat16x2 lB,
    FFXM_PARAMETER_IN FfxFloat16x2 lC,
    FFXM_PARAMETER_IN FfxFloat16x2 lD,
    FFXM_PARAMETER_IN FfxFloat16x2 lE)
{
    FfxFloat16x2 w = FFXM_BROADCAST_FLOAT16X2(0.0);

    if (biST)
        w = (FfxFloat16x2(1.0, 0.0) + FfxFloat16x2(-pp.x, pp.x)) * FFXM_BROADCAST_FLOAT16X2(FFXM_BROADCAST_FLOAT16(1.0) - pp.y);

    if (biUV)
        w = (FfxFloat16x2(1.0, 0.0) + FfxFloat16x2(-pp.x, pp.x)) * FFXM_BROADCAST_FLOAT16X2(pp.y);

    // ABS is not free in the packed FP16 path.
    FfxFloat16x2 dc   = lD - lC;
    FfxFloat16x2 cb   = lC - lB;
    FfxFloat16x2 lenX = max(abs(dc), abs(cb));
    lenX              = ffxReciprocalHalf(lenX);

    FfxFloat16x2 dirX = lD - lB;
    dirPX += dirX * w;
    lenX = ffxSaturate(abs(dirX) * lenX);
    lenX *= lenX;
    lenP += lenX * w;
    FfxFloat16x2 ec   = lE - lC;
    FfxFloat16x2 ca   = lC - lA;
    FfxFloat16x2 lenY = max(abs(ec), abs(ca));
    lenY              = ffxReciprocalHalf(lenY);
    FfxFloat16x2 dirY = lE - lA;
    dirPY += dirY * w;
    lenY = ffxSaturate(abs(dirY) * lenY);
    lenY *= lenY;
    lenP += lenY * w;
}

void FsrEasuH(
    FFXM_PARAMETER_OUT FfxFloat16x3 pix,
    FFXM_PARAMETER_IN FfxUInt32x2 ip,
    FFXM_PARAMETER_IN FfxUInt32x4 con0,
    FFXM_PARAMETER_IN FfxUInt32x4 con1,
    FFXM_PARAMETER_IN FfxUInt32x4 con2,
    FFXM_PARAMETER_IN FfxUInt32x4 con3)
{
    FfxFloat32x2 pp = FfxFloat32x2(ip) * ffxAsFloat(con0.xy) + ffxAsFloat(con0.zw);
    FfxFloat32x2 fp = floor(pp);
    pp -= fp;
    FfxFloat16x2 ppp = FfxFloat16x2(pp);

    FfxFloat32x2 p0    = fp * ffxAsFloat(con1.xy) + ffxAsFloat(con1.zw);
    FfxFloat32x2 p1    = p0 + ffxAsFloat(con2.xy);
    FfxFloat32x2 p2    = p0 + ffxAsFloat(con2.zw);
    FfxFloat32x2 p3    = p0 + ffxAsFloat(con3.xy);
    FfxFloat16x4 bczzR = FsrEasuRH(p0);
    FfxFloat16x4 bczzG = FsrEasuGH(p0);
    FfxFloat16x4 bczzB = FsrEasuBH(p0);
    FfxFloat16x4 ijfeR = FsrEasuRH(p1);
    FfxFloat16x4 ijfeG = FsrEasuGH(p1);
    FfxFloat16x4 ijfeB = FsrEasuBH(p1);
    FfxFloat16x4 klhgR = FsrEasuRH(p2);
    FfxFloat16x4 klhgG = FsrEasuGH(p2);
    FfxFloat16x4 klhgB = FsrEasuBH(p2);
    FfxFloat16x4 zzonR = FsrEasuRH(p3);
    FfxFloat16x4 zzonG = FsrEasuGH(p3);
    FfxFloat16x4 zzonB = FsrEasuBH(p3);

    FfxFloat16x4 bczzL = bczzB * FFXM_BROADCAST_FLOAT16X4(0.5) + (bczzR * FFXM_BROADCAST_FLOAT16X4(0.5) + bczzG);
    FfxFloat16x4 ijfeL = ijfeB * FFXM_BROADCAST_FLOAT16X4(0.5) + (ijfeR * FFXM_BROADCAST_FLOAT16X4(0.5) + ijfeG);
    FfxFloat16x4 klhgL = klhgB * FFXM_BROADCAST_FLOAT16X4(0.5) + (klhgR * FFXM_BROADCAST_FLOAT16X4(0.5) + klhgG);
    FfxFloat16x4 zzonL = zzonB * FFXM_BROADCAST_FLOAT16X4(0.5) + (zzonR * FFXM_BROADCAST_FLOAT16X4(0.5) + zzonG);
    FfxFloat16   bL    = bczzL.x;
    FfxFloat16   cL    = bczzL.y;
    FfxFloat16   iL    = ijfeL.x;
    FfxFloat16   jL    = ijfeL.y;
    FfxFloat16   fL    = ijfeL.z;
    FfxFloat16   eL    = ijfeL.w;
    FfxFloat16   kL    = klhgL.x;
    FfxFloat16   lL    = klhgL.y;
    FfxFloat16   hL    = klhgL.z;
    FfxFloat16   gL    = klhgL.w;
    FfxFloat16   oL    = zzonL.z;
    FfxFloat16   nL    = zzonL.w;

    // This part is different, accumulating 2 taps in parallel.
    FfxFloat16x2 dirPX = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 dirPY = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 lenP  = FFXM_BROADCAST_FLOAT16X2(0.0);
    FsrEasuSetH(dirPX,
                dirPY,
                lenP,
                ppp,
                FfxBoolean(true),
                FfxBoolean(false),
                FfxFloat16x2(bL, cL),
                FfxFloat16x2(eL, fL),
                FfxFloat16x2(fL, gL),
                FfxFloat16x2(gL, hL),
                FfxFloat16x2(jL, kL));
    FsrEasuSetH(dirPX,
                dirPY,
                lenP,
                ppp,
                FfxBoolean(false),
                FfxBoolean(true),
                FfxFloat16x2(fL, gL),
                FfxFloat16x2(iL, jL),
                FfxFloat16x2(jL, kL),
                FfxFloat16x2(kL, lL),
                FfxFloat16x2(nL, oL));
    FfxFloat16x2 dir = FfxFloat16x2(dirPX.r + dirPX.g, dirPY.r + dirPY.g);
    FfxFloat16   len = lenP.r + lenP.g;

    FfxFloat16x2 dir2 = dir * dir;
    FfxFloat16   dirR = dir2.x + dir2.y;
    FfxUInt32    zro  = FfxUInt32(dirR < FFXM_BROADCAST_FLOAT16(1.0 / 32768.0));
    dirR              = ffxApproximateReciprocalSquareRootHalf(dirR);
    dirR              = (zro > 0) ? FFXM_BROADCAST_FLOAT16(1.0) : dirR;
    dir.x             = (zro > 0) ? FFXM_BROADCAST_FLOAT16(1.0) : dir.x;
    dir *= FFXM_BROADCAST_FLOAT16X2(dirR);
    len = len * FFXM_BROADCAST_FLOAT16(0.5);
    len *= len;
    FfxFloat16   stretch = (dir.x * dir.x + dir.y * dir.y) * ffxApproximateReciprocalHalf(max(abs(dir.x), abs(dir.y)));
    FfxFloat16x2 len2 =
        FfxFloat16x2(FFXM_BROADCAST_FLOAT16(1.0) + (stretch - FFXM_BROADCAST_FLOAT16(1.0)) * len, FFXM_BROADCAST_FLOAT16(1.0) + FFXM_BROADCAST_FLOAT16(-0.5) * len);
    FfxFloat16 lob = FFXM_BROADCAST_FLOAT16(0.5) + FFXM_BROADCAST_FLOAT16((1.0 / 4.0 - 0.04) - 0.5) * len;
    FfxFloat16 clp = ffxApproximateReciprocalHalf(lob);

    // FP16 is different, using packed trick to do min and max in same operation.
    FfxFloat16x2 bothR =
        max(max(FfxFloat16x2(-ijfeR.z, ijfeR.z), FfxFloat16x2(-klhgR.w, klhgR.w)), max(FfxFloat16x2(-ijfeR.y, ijfeR.y), FfxFloat16x2(-klhgR.x, klhgR.x)));
    FfxFloat16x2 bothG =
        max(max(FfxFloat16x2(-ijfeG.z, ijfeG.z), FfxFloat16x2(-klhgG.w, klhgG.w)), max(FfxFloat16x2(-ijfeG.y, ijfeG.y), FfxFloat16x2(-klhgG.x, klhgG.x)));
    FfxFloat16x2 bothB =
        max(max(FfxFloat16x2(-ijfeB.z, ijfeB.z), FfxFloat16x2(-klhgB.w, klhgB.w)), max(FfxFloat16x2(-ijfeB.y, ijfeB.y), FfxFloat16x2(-klhgB.x, klhgB.x)));

    // This part is different for FP16, working pairs of taps at a time.
    FfxFloat16x2 pR = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 pG = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 pB = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 pW = FFXM_BROADCAST_FLOAT16X2(0.0);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(0.0, 1.0) - ppp.xx, FfxFloat16x2(-1.0, -1.0) - ppp.yy, dir, len2, lob, clp, bczzR.xy, bczzG.xy, bczzB.xy);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(-1.0, 0.0) - ppp.xx, FfxFloat16x2(1.0, 1.0) - ppp.yy, dir, len2, lob, clp, ijfeR.xy, ijfeG.xy, ijfeB.xy);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(0.0, -1.0) - ppp.xx, FfxFloat16x2(0.0, 0.0) - ppp.yy, dir, len2, lob, clp, ijfeR.zw, ijfeG.zw, ijfeB.zw);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(1.0, 2.0) - ppp.xx, FfxFloat16x2(1.0, 1.0) - ppp.yy, dir, len2, lob, clp, klhgR.xy, klhgG.xy, klhgB.xy);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(2.0, 1.0) - ppp.xx, FfxFloat16x2(0.0, 0.0) - ppp.yy, dir, len2, lob, clp, klhgR.zw, klhgG.zw, klhgB.zw);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(1.0, 0.0) - ppp.xx, FfxFloat16x2(2.0, 2.0) - ppp.yy, dir, len2, lob, clp, zzonR.zw, zzonG.zw, zzonB.zw);
    FfxFloat16x3 aC = FfxFloat16x3(pR.x + pR.y, pG.x + pG.y, pB.x + pB.y);
    FfxFloat16   aW = pW.x + pW.y;

    // Slightly different for FP16 version due to combined min and max.
    pix = min(FfxFloat16x3(bothR.y, bothG.y, bothB.y), max(-FfxFloat16x3(bothR.x, bothG.x, bothB.x), aC * FFXM_BROADCAST_FLOAT16X3(ffxReciprocalHalf(aW))));
}
#endif // #if defined(FFXM_GPU) && defined(FFXM_HALF) && defined(FFXM_FSR_EASU_HALF)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                      FSR - [RCAS] ROBUST CONTRAST ADAPTIVE SHARPENING
//
//------------------------------------------------------------------------------------------------------------------------------
// CAS uses a simplified mechanism to convert local contrast into a variable amount of sharpness.
// RCAS uses a more exact mechanism, solving for the maximum local sharpness possible before clipping.
// RCAS also has a built in process to limit sharpening of what it detects as possible noise.
// RCAS sharper does not support scaling, as it should be applied after EASU scaling.
// Pass EASU output straight into RCAS, no color conversions necessary.
//------------------------------------------------------------------------------------------------------------------------------
// RCAS is based on the following logic.
// RCAS uses a 5 tap filter in a cross pattern (same as CAS),
//    w                n
//  w 1 w  for taps  w m e
//    w                s
// Where 'w' is the negative lobe weight.
//  output = (w*(n+e+w+s)+m)/(4*w+1)
// RCAS solves for 'w' by seeing where the signal might clip out of the {0 to 1} input range,
//  0 == (w*(n+e+w+s)+m)/(4*w+1) -> w = -m/(n+e+w+s)
//  1 == (w*(n+e+w+s)+m)/(4*w+1) -> w = (1-m)/(n+e+w+s-4*1)
// Then chooses the 'w' which results in no clipping, limits 'w', and multiplies by the 'sharp' amount.
// This solution above has issues with MSAA input as the steps along the gradient cause edge detection issues.
// So RCAS uses 4x the maximum and 4x the minimum (depending on equation)in place of the individual taps.
// As well as switching from 'm' to either the minimum or maximum (depending on side), to help in energy conservation.
// This stabilizes RCAS.
// RCAS does a simple highpass which is normalized against the local contrast then shaped,
//       0.25
//  0.25  -1  0.25
//       0.25
// This is used as a noise detection filter, to reduce the effect of RCAS on grain, and focus on real edges.
//
//  GLSL example for the required callbacks :
//
//  FfxFloat16x4 FsrRcasLoadH(FfxInt16x2 p){return FfxFloat16x4(imageLoad(imgSrc,FfxInt32x2(p)));}
//  void FsrRcasInputH(inout FfxFloat16 r,inout FfxFloat16 g,inout FfxFloat16 b)
//  {
//    //do any simple input color conversions here or leave empty if none needed
//  }
//
//  FsrRcasCon need to be called from the CPU or GPU to set up constants.
//  Including a GPU example here, the 'con' value would be stored out to a constant buffer.
//
//  FfxUInt32x4 con;
//  FsrRcasCon(con,
//   0.0); // The scale is {0.0 := maximum sharpness, to N>0, where N is the number of stops (halving) of the reduction of sharpness}.
// ---------------
// RCAS sharpening supports a CAS-like pass-through alpha via,
//  #define FSR_RCAS_PASSTHROUGH_ALPHA 1
// RCAS also supports a define to enable a more expensive path to avoid some sharpening of noise.
// Would suggest it is better to apply film grain after RCAS sharpening (and after scaling) instead of using this define,
//  #define FSR_RCAS_DENOISE 1
//==============================================================================================================================
// This is set at the limit of providing unnatural results for sharpening.
#define FSR_RCAS_LIMIT (0.25-(1.0/16.0))
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                      CONSTANT SETUP
//==============================================================================================================================
// Call to setup required constant values (works on CPU or GPU).
 FFXM_STATIC void FsrRcasCon(FfxUInt32x4 con,
                            // The scale is {0.0 := maximum, to N>0, where N is the number of stops (halving) of the reduction of sharpness}.
                            FfxFloat32 sharpness)
 {
     // Transform from stops to linear value.
     sharpness = exp2(-sharpness);
     FfxFloat32x2 hSharp  = {sharpness, sharpness};
     con[0] = ffxAsUInt32(sharpness);
     con[1] = packHalf2x16(hSharp);
     con[2] = 0;
     con[3] = 0;
 }
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                   NON-PACKED 32-BIT VERSION
//==============================================================================================================================
#if defined(FFXM_GPU)&&defined(FSR_RCAS_F)
 // Input callback prototypes that need to be implemented by calling shader
 FfxFloat32x4 FsrRcasLoadF(FfxInt32x2 p);
 void FsrRcasInputF(inout FfxFloat32 r,inout FfxFloat32 g,inout FfxFloat32 b);
//------------------------------------------------------------------------------------------------------------------------------
 void FsrRcasF(out FfxFloat32 pixR,  // Output values, non-vector so port between RcasFilter() and RcasFilterH() is easy.
               out FfxFloat32 pixG,
               out FfxFloat32 pixB,
#ifdef FSR_RCAS_PASSTHROUGH_ALPHA
               out FfxFloat32 pixA,
#endif
               FfxUInt32x2 ip,  // Integer pixel position in output.
               FfxUInt32x4 con)
 {  // Constant generated by RcasSetup().
     // Algorithm uses minimal 3x3 pixel neighborhood.
     //    b
     //  d e f
     //    h
     FfxInt32x2   sp = FfxInt32x2(ip);
     FfxFloat32x3 b  = FsrRcasLoadF(sp + FfxInt32x2(0, -1)).rgb;
     FfxFloat32x3 d  = FsrRcasLoadF(sp + FfxInt32x2(-1, 0)).rgb;
#ifdef FSR_RCAS_PASSTHROUGH_ALPHA
     FfxFloat32x4 ee = FsrRcasLoadF(sp);
     FfxFloat32x3 e  = ee.rgb;
     pixA            = ee.a;
#else
     FfxFloat32x3 e = FsrRcasLoadF(sp).rgb;
#endif
     FfxFloat32x3 f = FsrRcasLoadF(sp + FfxInt32x2(1, 0)).rgb;
     FfxFloat32x3 h = FsrRcasLoadF(sp + FfxInt32x2(0, 1)).rgb;
     // Rename (32-bit) or regroup (16-bit).
     FfxFloat32 bR = b.r;
     FfxFloat32 bG = b.g;
     FfxFloat32 bB = b.b;
     FfxFloat32 dR = d.r;
     FfxFloat32 dG = d.g;
     FfxFloat32 dB = d.b;
     FfxFloat32 eR = e.r;
     FfxFloat32 eG = e.g;
     FfxFloat32 eB = e.b;
     FfxFloat32 fR = f.r;
     FfxFloat32 fG = f.g;
     FfxFloat32 fB = f.b;
     FfxFloat32 hR = h.r;
     FfxFloat32 hG = h.g;
     FfxFloat32 hB = h.b;
     // Run optional input transform.
     FsrRcasInputF(bR, bG, bB);
     FsrRcasInputF(dR, dG, dB);
     FsrRcasInputF(eR, eG, eB);
     FsrRcasInputF(fR, fG, fB);
     FsrRcasInputF(hR, hG, hB);
     // Luma times 2.
     FfxFloat32 bL = bB * FfxFloat32(0.5) + (bR * FfxFloat32(0.5) + bG);
     FfxFloat32 dL = dB * FfxFloat32(0.5) + (dR * FfxFloat32(0.5) + dG);
     FfxFloat32 eL = eB * FfxFloat32(0.5) + (eR * FfxFloat32(0.5) + eG);
     FfxFloat32 fL = fB * FfxFloat32(0.5) + (fR * FfxFloat32(0.5) + fG);
     FfxFloat32 hL = hB * FfxFloat32(0.5) + (hR * FfxFloat32(0.5) + hG);
     // Noise detection.
     FfxFloat32 nz = FfxFloat32(0.25) * bL + FfxFloat32(0.25) * dL + FfxFloat32(0.25) * fL + FfxFloat32(0.25) * hL - eL;
     nz            = ffxSaturate(abs(nz) * ffxApproximateReciprocalMedium(ffxMax3(ffxMax3(bL, dL, eL), fL, hL) - ffxMin3(ffxMin3(bL, dL, eL), fL, hL)));
     nz            = FfxFloat32(-0.5) * nz + FfxFloat32(1.0);
     // Min and max of ring.
     FfxFloat32 mn4R = ffxMin(ffxMin3(bR, dR, fR), hR);
     FfxFloat32 mn4G = ffxMin(ffxMin3(bG, dG, fG), hG);
     FfxFloat32 mn4B = ffxMin(ffxMin3(bB, dB, fB), hB);
     FfxFloat32 mx4R = max(ffxMax3(bR, dR, fR), hR);
     FfxFloat32 mx4G = max(ffxMax3(bG, dG, fG), hG);
     FfxFloat32 mx4B = max(ffxMax3(bB, dB, fB), hB);
     // Immediate constants for peak range.
     FfxFloat32x2 peakC = FfxFloat32x2(1.0, -1.0 * 4.0);
     // Limiters, these need to be high precision RCPs.
     FfxFloat32 hitMinR = mn4R * rcp(FfxFloat32(4.0) * mx4R);
     FfxFloat32 hitMinG = mn4G * rcp(FfxFloat32(4.0) * mx4G);
     FfxFloat32 hitMinB = mn4B * rcp(FfxFloat32(4.0) * mx4B);
     FfxFloat32 hitMaxR = (peakC.x - mx4R) * rcp(FfxFloat32(4.0) * mn4R + peakC.y);
     FfxFloat32 hitMaxG = (peakC.x - mx4G) * rcp(FfxFloat32(4.0) * mn4G + peakC.y);
     FfxFloat32 hitMaxB = (peakC.x - mx4B) * rcp(FfxFloat32(4.0) * mn4B + peakC.y);
     FfxFloat32 lobeR   = max(-hitMinR, hitMaxR);
     FfxFloat32 lobeG   = max(-hitMinG, hitMaxG);
     FfxFloat32 lobeB   = max(-hitMinB, hitMaxB);
     FfxFloat32 lobe    = max(FfxFloat32(-FSR_RCAS_LIMIT), ffxMin(ffxMax3(lobeR, lobeG, lobeB), FfxFloat32(0.0))) * ffxAsFloat
     (con.x);
 // Apply noise removal.
#ifdef FSR_RCAS_DENOISE
     lobe *= nz;
#endif
     // Resolve, which needs the medium precision rcp approximation to avoid visible tonality changes.
     FfxFloat32 rcpL = ffxApproximateReciprocalMedium(FfxFloat32(4.0) * lobe + FfxFloat32(1.0));
     pixR            = (lobe * bR + lobe * dR + lobe * hR + lobe * fR + eR) * rcpL;
     pixG            = (lobe * bG + lobe * dG + lobe * hG + lobe * fG + eG) * rcpL;
     pixB            = (lobe * bB + lobe * dB + lobe * hB + lobe * fB + eB) * rcpL;
     return;
 }
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                  NON-PACKED 16-BIT VERSION
//==============================================================================================================================
#if defined(FFXM_GPU) && FFXM_HALF == 1 && defined(FSR_RCAS_H)
 // Input callback prototypes that need to be implemented by calling shader
 FfxFloat16x4 FsrRcasLoadH(FfxInt16x2 p);
 void FsrRcasInputH(inout FfxFloat16 r,inout FfxFloat16 g,inout FfxFloat16 b);
//------------------------------------------------------------------------------------------------------------------------------
 void FsrRcasH(
 out FfxFloat16 pixR, // Output values, non-vector so port between RcasFilter() and RcasFilterH() is easy.
 out FfxFloat16 pixG,
 out FfxFloat16 pixB,
 #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
  out FfxFloat16 pixA,
 #endif
 FfxUInt32x2 ip, // Integer pixel position in output.
 FfxUInt32x4 con){ // Constant generated by RcasSetup().
  // Sharpening algorithm uses minimal 3x3 pixel neighborhood.
  //    b
  //  d e f
  //    h
  FfxInt16x2 sp=FfxInt16x2(ip);
  FfxFloat16x3 b=FsrRcasLoadH(sp+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d=FsrRcasLoadH(sp+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee=FsrRcasLoadH(sp);
   FfxFloat16x3 e=ee.rgb;pixA=ee.a;
  #else
   FfxFloat16x3 e=FsrRcasLoadH(sp).rgb;
  #endif
  FfxFloat16x3 f=FsrRcasLoadH(sp+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h=FsrRcasLoadH(sp+FfxInt16x2( 0, 1)).rgb;
  // Rename (32-bit) or regroup (16-bit).
  FfxFloat16 bR=b.r;
  FfxFloat16 bG=b.g;
  FfxFloat16 bB=b.b;
  FfxFloat16 dR=d.r;
  FfxFloat16 dG=d.g;
  FfxFloat16 dB=d.b;
  FfxFloat16 eR=e.r;
  FfxFloat16 eG=e.g;
  FfxFloat16 eB=e.b;
  FfxFloat16 fR=f.r;
  FfxFloat16 fG=f.g;
  FfxFloat16 fB=f.b;
  FfxFloat16 hR=h.r;
  FfxFloat16 hG=h.g;
  FfxFloat16 hB=h.b;
  // Run optional input transform.
  FsrRcasInputH(bR,bG,bB);
  FsrRcasInputH(dR,dG,dB);
  FsrRcasInputH(eR,eG,eB);
  FsrRcasInputH(fR,fG,fB);
  FsrRcasInputH(hR,hG,hB);
  // Luma times 2.
  FfxFloat16 bL=bB*FFXM_BROADCAST_FLOAT16(0.5)+(bR*FFXM_BROADCAST_FLOAT16(0.5)+bG);
  FfxFloat16 dL=dB*FFXM_BROADCAST_FLOAT16(0.5)+(dR*FFXM_BROADCAST_FLOAT16(0.5)+dG);
  FfxFloat16 eL=eB*FFXM_BROADCAST_FLOAT16(0.5)+(eR*FFXM_BROADCAST_FLOAT16(0.5)+eG);
  FfxFloat16 fL=fB*FFXM_BROADCAST_FLOAT16(0.5)+(fR*FFXM_BROADCAST_FLOAT16(0.5)+fG);
  FfxFloat16 hL=hB*FFXM_BROADCAST_FLOAT16(0.5)+(hR*FFXM_BROADCAST_FLOAT16(0.5)+hG);
  // Noise detection.
  FfxFloat16 nz=FFXM_BROADCAST_FLOAT16(0.25)*bL+FFXM_BROADCAST_FLOAT16(0.25)*dL+FFXM_BROADCAST_FLOAT16(0.25)*fL+FFXM_BROADCAST_FLOAT16(0.25)*hL-eL;
  nz=ffxSaturate(abs(nz)*ffxApproximateReciprocalMediumHalf(ffxMax3Half(ffxMax3Half(bL,dL,eL),fL,hL)-ffxMin3Half(ffxMin3Half(bL,dL,eL),fL,hL)));
  nz=FFXM_BROADCAST_FLOAT16(-0.5)*nz+FFXM_BROADCAST_FLOAT16(1.0);
  // Min and max of ring.
  FfxFloat16 mn4R=min(ffxMin3Half(bR,dR,fR),hR);
  FfxFloat16 mn4G=min(ffxMin3Half(bG,dG,fG),hG);
  FfxFloat16 mn4B=min(ffxMin3Half(bB,dB,fB),hB);
  FfxFloat16 mx4R=max(ffxMax3Half(bR,dR,fR),hR);
  FfxFloat16 mx4G=max(ffxMax3Half(bG,dG,fG),hG);
  FfxFloat16 mx4B=max(ffxMax3Half(bB,dB,fB),hB);
  // Immediate constants for peak range.
  FfxFloat16x2 peakC=FfxFloat16x2(1.0,-1.0*4.0);
  // Limiters, these need to be high precision RCPs.
  FfxFloat16 hitMinR=mn4R*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4R);
  FfxFloat16 hitMinG=mn4G*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4G);
  FfxFloat16 hitMinB=mn4B*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4B);
  FfxFloat16 hitMaxR=(peakC.x-mx4R)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4R+peakC.y);
  FfxFloat16 hitMaxG=(peakC.x-mx4G)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4G+peakC.y);
  FfxFloat16 hitMaxB=(peakC.x-mx4B)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4B+peakC.y);
  FfxFloat16 lobeR=max(-hitMinR,hitMaxR);
  FfxFloat16 lobeG=max(-hitMinG,hitMaxG);
  FfxFloat16 lobeB=max(-hitMinB,hitMaxB);
  FfxFloat16 lobe=max(FFXM_BROADCAST_FLOAT16(-FSR_RCAS_LIMIT),min(ffxMax3Half(lobeR,lobeG,lobeB),FFXM_BROADCAST_FLOAT16(0.0)))*FFXM_UINT32_TO_FLOAT16X2(con.y).x;
  // Apply noise removal.
  #ifdef FSR_RCAS_DENOISE
   lobe*=nz;
  #endif
  // Resolve, which needs the medium precision rcp approximation to avoid visible tonality changes.
  FfxFloat16 rcpL=ffxApproximateReciprocalMediumHalf(FFXM_BROADCAST_FLOAT16(4.0)*lobe+FFXM_BROADCAST_FLOAT16(1.0));
  pixR=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;
  pixG=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;
  pixB=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                     PACKED 16-BIT VERSION
//==============================================================================================================================
#if defined(FFXM_GPU)&& FFXM_HALF == 1 && defined(FSR_RCAS_HX2)
 // Input callback prototypes that need to be implemented by the calling shader
 FfxFloat16x4 FsrRcasLoadHx2(FfxInt16x2 p);
 void FsrRcasInputHx2(inout FfxFloat16x2 r,inout FfxFloat16x2 g,inout FfxFloat16x2 b);
//------------------------------------------------------------------------------------------------------------------------------
 // Can be used to convert from packed Structures of Arrays to Arrays of Structures for store.
 void FsrRcasDepackHx2(out FfxFloat16x4 pix0,out FfxFloat16x4 pix1,FfxFloat16x2 pixR,FfxFloat16x2 pixG,FfxFloat16x2 pixB){
  #ifdef FFXM_HLSL
   // Invoke a slower path for DX only, since it won't allow uninitialized values.
   pix0.a=pix1.a=0.0;
  #endif
  pix0.rgb=FfxFloat16x3(pixR.x,pixG.x,pixB.x);
  pix1.rgb=FfxFloat16x3(pixR.y,pixG.y,pixB.y);}
//------------------------------------------------------------------------------------------------------------------------------
 void FsrRcasHx2(
 // Output values are for 2 8x8 tiles in a 16x8 region.
 //  pix<R,G,B>.x =  left 8x8 tile
 //  pix<R,G,B>.y = right 8x8 tile
 // This enables later processing to easily be packed as well.
 out FfxFloat16x2 pixR,
 out FfxFloat16x2 pixG,
 out FfxFloat16x2 pixB,
 #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
  out FfxFloat16x2 pixA,
 #endif
 FfxUInt32x2 ip, // Integer pixel position in output.
 FfxUInt32x4 con){ // Constant generated by RcasSetup().
  // No scaling algorithm uses minimal 3x3 pixel neighborhood.
  FfxInt16x2 sp0=FfxInt16x2(ip);
  FfxFloat16x3 b0=FsrRcasLoadHx2(sp0+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d0=FsrRcasLoadHx2(sp0+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee0=FsrRcasLoadHx2(sp0);
   FfxFloat16x3 e0=ee0.rgb;pixA.r=ee0.a;
  #else
   FfxFloat16x3 e0=FsrRcasLoadHx2(sp0).rgb;
  #endif
  FfxFloat16x3 f0=FsrRcasLoadHx2(sp0+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h0=FsrRcasLoadHx2(sp0+FfxInt16x2( 0, 1)).rgb;
  FfxInt16x2 sp1=sp0+FfxInt16x2(8,0);
  FfxFloat16x3 b1=FsrRcasLoadHx2(sp1+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d1=FsrRcasLoadHx2(sp1+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee1=FsrRcasLoadHx2(sp1);
   FfxFloat16x3 e1=ee1.rgb;pixA.g=ee1.a;
  #else
   FfxFloat16x3 e1=FsrRcasLoadHx2(sp1).rgb;
  #endif
  FfxFloat16x3 f1=FsrRcasLoadHx2(sp1+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h1=FsrRcasLoadHx2(sp1+FfxInt16x2( 0, 1)).rgb;
  // Arrays of Structures to Structures of Arrays conversion.
  FfxFloat16x2 bR=FfxFloat16x2(b0.r,b1.r);
  FfxFloat16x2 bG=FfxFloat16x2(b0.g,b1.g);
  FfxFloat16x2 bB=FfxFloat16x2(b0.b,b1.b);
  FfxFloat16x2 dR=FfxFloat16x2(d0.r,d1.r);
  FfxFloat16x2 dG=FfxFloat16x2(d0.g,d1.g);
  FfxFloat16x2 dB=FfxFloat16x2(d0.b,d1.b);
  FfxFloat16x2 eR=FfxFloat16x2(e0.r,e1.r);
  FfxFloat16x2 eG=FfxFloat16x2(e0.g,e1.g);
  FfxFloat16x2 eB=FfxFloat16x2(e0.b,e1.b);
  FfxFloat16x2 fR=FfxFloat16x2(f0.r,f1.r);
  FfxFloat16x2 fG=FfxFloat16x2(f0.g,f1.g);
  FfxFloat16x2 fB=FfxFloat16x2(f0.b,f1.b);
  FfxFloat16x2 hR=FfxFloat16x2(h0.r,h1.r);
  FfxFloat16x2 hG=FfxFloat16x2(h0.g,h1.g);
  FfxFloat16x2 hB=FfxFloat16x2(h0.b,h1.b);
  // Run optional input transform.
  FsrRcasInputHx2(bR,bG,bB);
  FsrRcasInputHx2(dR,dG,dB);
  FsrRcasInputHx2(eR,eG,eB);
  FsrRcasInputHx2(fR,fG,fB);
  FsrRcasInputHx2(hR,hG,hB);
  // Luma times 2.
  FfxFloat16x2 bL=bB*FFXM_BROADCAST_FLOAT16X2(0.5)+(bR*FFXM_BROADCAST_FLOAT16X2(0.5)+bG);
  FfxFloat16x2 dL=dB*FFXM_BROADCAST_FLOAT16X2(0.5)+(dR*FFXM_BROADCAST_FLOAT16X2(0.5)+dG);
  FfxFloat16x2 eL=eB*FFXM_BROADCAST_FLOAT16X2(0.5)+(eR*FFXM_BROADCAST_FLOAT16X2(0.5)+eG);
  FfxFloat16x2 fL=fB*FFXM_BROADCAST_FLOAT16X2(0.5)+(fR*FFXM_BROADCAST_FLOAT16X2(0.5)+fG);
  FfxFloat16x2 hL=hB*FFXM_BROADCAST_FLOAT16X2(0.5)+(hR*FFXM_BROADCAST_FLOAT16X2(0.5)+hG);
  // Noise detection.
  FfxFloat16x2 nz=FFXM_BROADCAST_FLOAT16X2(0.25)*bL+FFXM_BROADCAST_FLOAT16X2(0.25)*dL+FFXM_BROADCAST_FLOAT16X2(0.25)*fL+FFXM_BROADCAST_FLOAT16X2(0.25)*hL-eL;
  nz=ffxSaturate(abs(nz)*ffxApproximateReciprocalMediumHalf(ffxMax3Half(ffxMax3Half(bL,dL,eL),fL,hL)-ffxMin3Half(ffxMin3Half(bL,dL,eL),fL,hL)));
  nz=FFXM_BROADCAST_FLOAT16X2(-0.5)*nz+FFXM_BROADCAST_FLOAT16X2(1.0);
  // Min and max of ring.
  FfxFloat16x2 mn4R=min(ffxMin3Half(bR,dR,fR),hR);
  FfxFloat16x2 mn4G=min(ffxMin3Half(bG,dG,fG),hG);
  FfxFloat16x2 mn4B=min(ffxMin3Half(bB,dB,fB),hB);
  FfxFloat16x2 mx4R=max(ffxMax3Half(bR,dR,fR),hR);
  FfxFloat16x2 mx4G=max(ffxMax3Half(bG,dG,fG),hG);
  FfxFloat16x2 mx4B=max(ffxMax3Half(bB,dB,fB),hB);
  // Immediate constants for peak range.
  FfxFloat16x2 peakC=FfxFloat16x2(1.0,-1.0*4.0);
  // Limiters, these need to be high precision RCPs.
  FfxFloat16x2 hitMinR=mn4R*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4R);
  FfxFloat16x2 hitMinG=mn4G*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4G);
  FfxFloat16x2 hitMinB=mn4B*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4B);
  FfxFloat16x2 hitMaxR=(peakC.x-mx4R)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4R+peakC.y);
  FfxFloat16x2 hitMaxG=(peakC.x-mx4G)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4G+peakC.y);
  FfxFloat16x2 hitMaxB=(peakC.x-mx4B)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4B+peakC.y);
  FfxFloat16x2 lobeR=max(-hitMinR,hitMaxR);
  FfxFloat16x2 lobeG=max(-hitMinG,hitMaxG);
  FfxFloat16x2 lobeB=max(-hitMinB,hitMaxB);
  FfxFloat16x2 lobe=max(FFXM_BROADCAST_FLOAT16X2(-FSR_RCAS_LIMIT),min(ffxMax3Half(lobeR,lobeG,lobeB),FFXM_BROADCAST_FLOAT16X2(0.0)))*FFXM_BROADCAST_FLOAT16X2(FFXM_UINT32_TO_FLOAT16X2(con.y).x);
  // Apply noise removal.
  #ifdef FSR_RCAS_DENOISE
   lobe*=nz;
  #endif
  // Resolve, which needs the medium precision rcp approximation to avoid visible tonality changes.
  FfxFloat16x2 rcpL=ffxApproximateReciprocalMediumHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*lobe+FFXM_BROADCAST_FLOAT16X2(1.0));
  pixR=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;
  pixG=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;
  pixB=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                          FSR - [LFGA] LINEAR FILM GRAIN APPLICATOR
//
//------------------------------------------------------------------------------------------------------------------------------
// Adding output-resolution film grain after scaling is a good way to mask both rendering and scaling artifacts.
// Suggest using tiled blue noise as film grain input, with peak noise frequency set for a specific look and feel.
// The 'Lfga*()' functions provide a convenient way to introduce grain.
// These functions limit grain based on distance to signal limits.
// This is done so that the grain is temporally energy preserving, and thus won't modify image tonality.
// Grain application should be done in a linear colorspace.
// The grain should be temporally changing, but have a temporal sum per pixel that adds to zero (non-biased).
//------------------------------------------------------------------------------------------------------------------------------
// Usage,
//   FsrLfga*(
//    color, // In/out linear colorspace color {0 to 1} ranged.
//    grain, // Per pixel grain texture value {-0.5 to 0.5} ranged, input is 3-channel to support colored grain.
//    amount); // Amount of grain (0 to 1} ranged.
//------------------------------------------------------------------------------------------------------------------------------
// Example if grain texture is monochrome: 'FsrLfgaF(color,ffxBroadcast3(grain),amount)'
//==============================================================================================================================
#if defined(FFXM_GPU)
 // Maximum grain is the minimum distance to the signal limit.
 void FsrLfgaF(inout FfxFloat32x3 c, FfxFloat32x3 t, FfxFloat32 a)
 {
     c += (t * ffxBroadcast3(a)) * ffxMin(ffxBroadcast3(1.0) - c, c);
 }
#endif
//==============================================================================================================================
#if defined(FFXM_GPU)&& FFXM_HALF == 1
 // Half precision version (slower).
 void FsrLfgaH(inout FfxFloat16x3 c, FfxFloat16x3 t, FfxFloat16 a)
 {
     c += (t * FFXM_BROADCAST_FLOAT16X3(a)) * min(FFXM_BROADCAST_FLOAT16X3(1.0) - c, c);
 }
 //------------------------------------------------------------------------------------------------------------------------------
 // Packed half precision version (faster).
 void FsrLfgaHx2(inout FfxFloat16x2 cR,inout FfxFloat16x2 cG,inout FfxFloat16x2 cB,FfxFloat16x2 tR,FfxFloat16x2 tG,FfxFloat16x2 tB,FfxFloat16 a){
  cR+=(tR*FFXM_BROADCAST_FLOAT16X2(a))*min(FFXM_BROADCAST_FLOAT16X2(1.0)-cR,cR);cG+=(tG*FFXM_BROADCAST_FLOAT16X2(a))*min(FFXM_BROADCAST_FLOAT16X2(1.0)-cG,cG);cB+=(tB*FFXM_BROADCAST_FLOAT16X2(a))*min(FFXM_BROADCAST_FLOAT16X2(1.0)-cB,cB);}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                          FSR - [SRTM] SIMPLE REVERSIBLE TONE-MAPPER
//
//------------------------------------------------------------------------------------------------------------------------------
// This provides a way to take linear HDR color {0 to FP16_MAX} and convert it into a temporary {0 to 1} ranged post-tonemapped linear.
// The tonemapper preserves RGB ratio, which helps maintain HDR color bleed during filtering.
//------------------------------------------------------------------------------------------------------------------------------
// Reversible tonemapper usage,
//  FsrSrtm*(color); // {0 to FP16_MAX} converted to {0 to 1}.
//  FsrSrtmInv*(color); // {0 to 1} converted into {0 to 32768, output peak safe for FP16}.
//==============================================================================================================================
#if defined(FFXM_GPU)
 void FsrSrtmF(inout FfxFloat32x3 c)
 {
     c *= ffxBroadcast3(rcp(ffxMax3(c.r, c.g, c.b) + FfxFloat32(1.0)));
 }
 // The extra max solves the c=1.0 case (which is a /0).
 void FsrSrtmInvF(inout FfxFloat32x3 c){c*=ffxBroadcast3(rcp(max(FfxFloat32(1.0/32768.0),FfxFloat32(1.0)-ffxMax3(c.r,c.g,c.b))));}
#endif
//==============================================================================================================================
#if defined(FFXM_GPU )&& FFXM_HALF == 1
 void FsrSrtmH(inout FfxFloat16x3 c)
 {
     c *= FFXM_BROADCAST_FLOAT16X3(ffxReciprocalHalf(ffxMax3Half(c.r, c.g, c.b) + FFXM_BROADCAST_FLOAT16(1.0)));
 }
 void FsrSrtmInvH(inout FfxFloat16x3 c)
 {
     c *= FFXM_BROADCAST_FLOAT16X3(ffxReciprocalHalf(max(FFXM_BROADCAST_FLOAT16(1.0 / 32768.0), FFXM_BROADCAST_FLOAT16(1.0) - ffxMax3Half(c.r, c.g, c.b))));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrSrtmHx2(inout FfxFloat16x2 cR, inout FfxFloat16x2 cG, inout FfxFloat16x2 cB)
 {
     FfxFloat16x2 rcp = ffxReciprocalHalf(ffxMax3Half(cR, cG, cB) + FFXM_BROADCAST_FLOAT16X2(1.0));
     cR *= rcp;
     cG *= rcp;
     cB *= rcp;
 }
 void FsrSrtmInvHx2(inout FfxFloat16x2 cR,inout FfxFloat16x2 cG,inout FfxFloat16x2 cB)
 {
     FfxFloat16x2 rcp=ffxReciprocalHalf(max(FFXM_BROADCAST_FLOAT16X2(1.0/32768.0),FFXM_BROADCAST_FLOAT16X2(1.0)-ffxMax3Half(cR,cG,cB)));
     cR*=rcp;
     cG*=rcp;
     cB*=rcp;
 }
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                       FSR - [TEPD] TEMPORAL ENERGY PRESERVING DITHER
//
//------------------------------------------------------------------------------------------------------------------------------
// Temporally energy preserving dithered {0 to 1} linear to gamma 2.0 conversion.
// Gamma 2.0 is used so that the conversion back to linear is just to square the color.
// The conversion comes in 8-bit and 10-bit modes, designed for output to 8-bit UNORM or 10:10:10:2 respectively.
// Given good non-biased temporal blue noise as dither input,
// the output dither will temporally conserve energy.
// This is done by choosing the linear nearest step point instead of perceptual nearest.
// See code below for details.
//------------------------------------------------------------------------------------------------------------------------------
// DX SPEC RULES FOR FLOAT->UNORM 8-BIT CONVERSION
// ===============================================
// - Output is 'FfxUInt32(floor(saturate(n)*255.0+0.5))'.
// - Thus rounding is to nearest.
// - NaN gets converted to zero.
// - INF is clamped to {0.0 to 1.0}.
//==============================================================================================================================
#if defined(FFXM_GPU)
 // Hand tuned integer position to dither value, with more values than simple checkerboard.
 // Only 32-bit has enough precision for this compddation.
 // Output is {0 to <1}.
 FfxFloat32 FsrTepdDitF(FfxUInt32x2 p, FfxUInt32 f)
 {
     FfxFloat32 x = FfxFloat32(p.x + f);
     FfxFloat32 y = FfxFloat32(p.y);
     // The 1.61803 golden ratio.
     FfxFloat32 a = FfxFloat32((1.0 + ffxSqrt(5.0f)) / 2.0);
     // Number designed to provide a good visual pattern.
     FfxFloat32 b = FfxFloat32(1.0 / 3.69);
     x            = x * a + (y * b);
     return ffxFract(x);
 }
  //------------------------------------------------------------------------------------------------------------------------------
 // This version is 8-bit gamma 2.0.
 // The 'c' input is {0 to 1}.
 // Output is {0 to 1} ready for image store.
 void FsrTepdC8F(inout FfxFloat32x3 c, FfxFloat32 dit)
 {
     FfxFloat32x3 n = ffxSqrt(c);
     n              = floor(n * ffxBroadcast3(255.0)) * ffxBroadcast3(1.0 / 255.0);
     FfxFloat32x3 a = n * n;
     FfxFloat32x3 b = n + ffxBroadcast3(1.0 / 255.0);
     b              = b * b;
     // Ratio of 'a' to 'b' required to produce 'c'.
     // ffxApproximateReciprocal() won't work here (at least for very high dynamic ranges).
     // ffxApproximateReciprocalMedium() is an IADD,FMA,MUL.
     FfxFloat32x3 r = (c - b) * ffxApproximateReciprocalMedium(a - b);
     // Use the ratio as a cutoff to choose 'a' or 'b'.
     // ffxIsGreaterThanZero() is a MUL.
     c = ffxSaturate(n + ffxIsGreaterThanZero(ffxBroadcast3(dit) - r) * ffxBroadcast3(1.0 / 255.0));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 // This version is 10-bit gamma 2.0.
 // The 'c' input is {0 to 1}.
 // Output is {0 to 1} ready for image store.
 void FsrTepdC10F(inout FfxFloat32x3 c, FfxFloat32 dit)
 {
     FfxFloat32x3 n = ffxSqrt(c);
     n              = floor(n * ffxBroadcast3(1023.0)) * ffxBroadcast3(1.0 / 1023.0);
     FfxFloat32x3 a = n * n;
     FfxFloat32x3 b = n + ffxBroadcast3(1.0 / 1023.0);
     b              = b * b;
     FfxFloat32x3 r = (c - b) * ffxApproximateReciprocalMedium(a - b);
     c              = ffxSaturate(n + ffxIsGreaterThanZero(ffxBroadcast3(dit) - r) * ffxBroadcast3(1.0 / 1023.0));
 }
#endif
//==============================================================================================================================
#if defined(FFXM_GPU)&& FFXM_HALF == 1
 FfxFloat16 FsrTepdDitH(FfxUInt32x2 p, FfxUInt32 f)
 {
     FfxFloat32 x = FfxFloat32(p.x + f);
     FfxFloat32 y = FfxFloat32(p.y);
     FfxFloat32 a = FfxFloat32((1.0 + ffxSqrt(5.0f)) / 2.0);
     FfxFloat32 b = FfxFloat32(1.0 / 3.69);
     x       = x * a + (y * b);
     return FfxFloat16(ffxFract(x));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC8H(inout FfxFloat16x3 c, FfxFloat16 dit)
 {
     FfxFloat16x3 n = sqrt(c);
     n     = floor(n * FFXM_BROADCAST_FLOAT16X3(255.0)) * FFXM_BROADCAST_FLOAT16X3(1.0 / 255.0);
     FfxFloat16x3 a = n * n;
     FfxFloat16x3 b = n + FFXM_BROADCAST_FLOAT16X3(1.0 / 255.0);
     b     = b * b;
     FfxFloat16x3 r = (c - b) * ffxApproximateReciprocalMediumHalf(a - b);
     c     = ffxSaturate(n + ffxIsGreaterThanZeroHalf(FFXM_BROADCAST_FLOAT16X3(dit) - r) * FFXM_BROADCAST_FLOAT16X3(1.0 / 255.0));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC10H(inout FfxFloat16x3 c, FfxFloat16 dit)
 {
     FfxFloat16x3 n = sqrt(c);
     n     = floor(n * FFXM_BROADCAST_FLOAT16X3(1023.0)) * FFXM_BROADCAST_FLOAT16X3(1.0 / 1023.0);
     FfxFloat16x3 a = n * n;
     FfxFloat16x3 b = n + FFXM_BROADCAST_FLOAT16X3(1.0 / 1023.0);
     b     = b * b;
     FfxFloat16x3 r = (c - b) * ffxApproximateReciprocalMediumHalf(a - b);
     c     = ffxSaturate(n + ffxIsGreaterThanZeroHalf(FFXM_BROADCAST_FLOAT16X3(dit) - r) * FFXM_BROADCAST_FLOAT16X3(1.0 / 1023.0));
 }
 //==============================================================================================================================
 // This computes dither for positions 'p' and 'p+{8,0}'.
 FfxFloat16x2 FsrTepdDitHx2(FfxUInt32x2 p, FfxUInt32 f)
 {
     FfxFloat32x2 x;
     x.x     = FfxFloat32(p.x + f);
     x.y     = x.x + FfxFloat32(8.0);
     FfxFloat32 y = FfxFloat32(p.y);
     FfxFloat32 a = FfxFloat32((1.0 + ffxSqrt(5.0f)) / 2.0);
     FfxFloat32 b = FfxFloat32(1.0 / 3.69);
     x       = x * ffxBroadcast2(a) + ffxBroadcast2(y * b);
     return FfxFloat16x2(ffxFract(x));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC8Hx2(inout FfxFloat16x2 cR, inout FfxFloat16x2 cG, inout FfxFloat16x2 cB, FfxFloat16x2 dit)
 {
     FfxFloat16x2 nR = sqrt(cR);
     FfxFloat16x2 nG = sqrt(cG);
     FfxFloat16x2 nB = sqrt(cB);
     nR     = floor(nR * FFXM_BROADCAST_FLOAT16X2(255.0)) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     nG     = floor(nG * FFXM_BROADCAST_FLOAT16X2(255.0)) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     nB     = floor(nB * FFXM_BROADCAST_FLOAT16X2(255.0)) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     FfxFloat16x2 aR = nR * nR;
     FfxFloat16x2 aG = nG * nG;
     FfxFloat16x2 aB = nB * nB;
     FfxFloat16x2 bR = nR + FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     bR     = bR * bR;
     FfxFloat16x2 bG = nG + FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     bG     = bG * bG;
     FfxFloat16x2 bB = nB + FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     bB     = bB * bB;
     FfxFloat16x2 rR = (cR - bR) * ffxApproximateReciprocalMediumHalf(aR - bR);
     FfxFloat16x2 rG = (cG - bG) * ffxApproximateReciprocalMediumHalf(aG - bG);
     FfxFloat16x2 rB = (cB - bB) * ffxApproximateReciprocalMediumHalf(aB - bB);
     cR     = ffxSaturate(nR + ffxIsGreaterThanZeroHalf(dit - rR) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0));
     cG     = ffxSaturate(nG + ffxIsGreaterThanZeroHalf(dit - rG) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0));
     cB     = ffxSaturate(nB + ffxIsGreaterThanZeroHalf(dit - rB) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC10Hx2(inout FfxFloat16x2 cR,inout FfxFloat16x2 cG,inout FfxFloat16x2 cB,FfxFloat16x2 dit){
  FfxFloat16x2 nR=sqrt(cR);
  FfxFloat16x2 nG=sqrt(cG);
  FfxFloat16x2 nB=sqrt(cB);
  nR=floor(nR*FFXM_BROADCAST_FLOAT16X2(1023.0))*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);
  nG=floor(nG*FFXM_BROADCAST_FLOAT16X2(1023.0))*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);
  nB=floor(nB*FFXM_BROADCAST_FLOAT16X2(1023.0))*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);
  FfxFloat16x2 aR=nR*nR;
  FfxFloat16x2 aG=nG*nG;
  FfxFloat16x2 aB=nB*nB;
  FfxFloat16x2 bR=nR+FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);bR=bR*bR;
  FfxFloat16x2 bG=nG+FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);bG=bG*bG;
  FfxFloat16x2 bB=nB+FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);bB=bB*bB;
  FfxFloat16x2 rR=(cR-bR)*ffxApproximateReciprocalMediumHalf(aR-bR);
  FfxFloat16x2 rG=(cG-bG)*ffxApproximateReciprocalMediumHalf(aG-bG);
  FfxFloat16x2 rB=(cB-bB)*ffxApproximateReciprocalMediumHalf(aB-bB);
  cR=ffxSaturate(nR+ffxIsGreaterThanZeroHalf(dit-rR)*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0));
  cG=ffxSaturate(nG+ffxIsGreaterThanZeroHalf(dit-rG)*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0));
  cB                                                       = ffxSaturate(nB + ffxIsGreaterThanZeroHalf(dit - rB) * FFXM_BROADCAST_FLOAT16X2(1.0 / 1023.0));
}
#endif
// --- end fsr1/ffxm_fsr1.h ---
//----------------------------------------------------------------------------------
// MobileGlues EASU pass glue: constants are computed once per resolution on the
// CPU (see FSR1.cpp) and uploaded as uEasuCon0..3; this pass reads the low-res
// render texture and writes the upscaled intermediate at surface size.
//
// textureGather is unavailable in ESSL 300 (SPIRV-Cross rejects OpImageGather
// below ESSL 310, which is how the old AMD port ended up failing its conversion
// and feeding raw GLSL 450 to the driver). The gather is therefore emulated
// with texelFetch exactly per the OpenGL ES textureGather definition:
//   i0 = floor(p * size - 0.5); i1 = i0 + 1; j0/j1 likewise;
//   result = (T(i0,j1)[c], T(i1,j1)[c], T(i0,j0)[c], T(i1,j0)[c])
// with every coordinate clamped to the edge, which is what the default
// CLAMP_TO_EDGE sampler state does for the hardware gather.
//----------------------------------------------------------------------------------
in vec2 vTexCoord;
out vec4 oFragColor;

uniform sampler2D uInputTex;
uniform FfxUInt32x4 uEasuCon0;
uniform FfxUInt32x4 uEasuCon1;
uniform FfxUInt32x4 uEasuCon2;
uniform FfxUInt32x4 uEasuCon3;

FfxFloat32x4 asrGatherR(FfxFloat32x2 p) {
    FfxInt32x2 ts = FfxInt32x2(textureSize(uInputTex, 0));
    FfxInt32x2 t0 = FfxInt32x2(floor(p * FfxFloat32x2(ts) - FfxFloat32(0.5)));
    FfxInt32x2 lo = FfxInt32x2(0, 0);
    FfxInt32x2 hi = ts - FfxInt32x2(1, 1);
    return FfxFloat32x4(
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(0, 1), lo, hi), 0).r,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(1, 1), lo, hi), 0).r,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(0, 0), lo, hi), 0).r,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(1, 0), lo, hi), 0).r);
}

// One per channel instead of a comp parameter: the component then is a constant
// at every texelFetch, so no dynamic vector component indexing survives to the
// driver (Mali's ESSL compiler is picky about dynamic indexing).
FfxFloat32x4 FsrEasuRF(FfxFloat32x2 p) { return asrGatherR(p); }

FfxFloat32x4 FsrEasuGF(FfxFloat32x2 p) {
    FfxInt32x2 ts = FfxInt32x2(textureSize(uInputTex, 0));
    FfxInt32x2 t0 = FfxInt32x2(floor(p * FfxFloat32x2(ts) - FfxFloat32(0.5)));
    FfxInt32x2 lo = FfxInt32x2(0, 0);
    FfxInt32x2 hi = ts - FfxInt32x2(1, 1);
    return FfxFloat32x4(
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(0, 1), lo, hi), 0).g,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(1, 1), lo, hi), 0).g,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(0, 0), lo, hi), 0).g,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(1, 0), lo, hi), 0).g);
}

FfxFloat32x4 FsrEasuBF(FfxFloat32x2 p) {
    FfxInt32x2 ts = FfxInt32x2(textureSize(uInputTex, 0));
    FfxInt32x2 t0 = FfxInt32x2(floor(p * FfxFloat32x2(ts) - FfxFloat32(0.5)));
    FfxInt32x2 lo = FfxInt32x2(0, 0);
    FfxInt32x2 hi = ts - FfxInt32x2(1, 1);
    return FfxFloat32x4(
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(0, 1), lo, hi), 0).b,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(1, 1), lo, hi), 0).b,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(0, 0), lo, hi), 0).b,
        texelFetch(uInputTex, clamp(t0 + FfxInt32x2(1, 0), lo, hi), 0).b);
}

void main() {
    FfxFloat32x3 color;
    ffxFsrEasuFloat(color, FfxUInt32x2(gl_FragCoord.xy),
                    uEasuCon0, uEasuCon1, uEasuCon2, uEasuCon3);
    oFragColor = FfxFloat32x4(color, FfxFloat32(1.0));
}
)fsr_glsl";

// RCAS pass: robust contrast-adaptive sharpening (fp32).
const char* FSR_RCAS_FSSource = R"fsr_glsl(#version 450

// Arm Accuracy Super Resolution (FFXM FSR1), MIT license -
// full notice in FSR_ASR_MIT_LICENSE at the top of this file.
// (c) 2023 Advanced Micro Devices, Inc. / (c) 2024-2025 Arm Limited
#define FFXM_GPU 1
#define FFXM_GLSL 1
// FFXM_HALF=0: the fp16 paths need GL_EXT_shader_explicit_arithmetic_types
// storage, which GLES3 drivers do not offer; the fp32 paths below are the
// ones Arm ships for such targets. FFXM_WAVE/FFXM_LONG stay undefined.
#define FFXM_HALF 0
#define FSR_RCAS_F 1
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin ffxm_common_types.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef FFXM_COMMON_TYPES_H
#define FFXM_COMMON_TYPES_H

#if defined(FFXM_CPU)
#define FFXM_PARAMETER_IN
#define FFXM_PARAMETER_OUT
#define FFXM_PARAMETER_INOUT
#define FFXM_PARAMETER_UNIFORM
#elif defined(FFXM_HLSL)
#define FFXM_PARAMETER_IN        in
#define FFXM_PARAMETER_OUT       out
#define FFXM_PARAMETER_INOUT     inout
#define FFXM_PARAMETER_UNIFORM uniform
#elif defined(FFXM_GLSL)
#define FFXM_PARAMETER_IN        in
#define FFXM_PARAMETER_OUT       out
#define FFXM_PARAMETER_INOUT     inout
#define FFXM_PARAMETER_UNIFORM const //[cacao_placeholder] until a better fit is found!
#endif // #if defined(FFXM_CPU)

#if defined(FFXM_CPU)
/// A typedef for a boolean value.
///
/// @ingroup CPUTypes
typedef bool FfxBoolean;

/// A typedef for a unsigned 8bit integer.
///
/// @ingroup CPUTypes
typedef uint8_t FfxUInt8;

/// A typedef for a unsigned 16bit integer.
///
/// @ingroup CPUTypes
typedef uint16_t FfxUInt16;

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32;

/// A typedef for a unsigned 64bit integer.
///
/// @ingroup CPUTypes
typedef uint64_t FfxUInt64;

/// A typedef for a signed 8bit integer.
///
/// @ingroup CPUTypes
typedef int8_t FfxInt8;

/// A typedef for a signed 16bit integer.
///
/// @ingroup CPUTypes
typedef int16_t FfxInt16;

/// A typedef for a signed 32bit integer.
///
/// @ingroup CPUTypes
typedef int32_t FfxInt32;

/// A typedef for a signed 64bit integer.
///
/// @ingroup CPUTypes
typedef int64_t FfxInt64;

/// A typedef for a floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32;

/// A typedef for a 2-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x2[2];

/// A typedef for a 3-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x3[3];

/// A typedef for a 4-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x4[4];

/// A typedef for a 2-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x2[2];

/// A typedef for a 3-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x3[3];

/// A typedef for a 4-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x4[4];
#endif // #if defined(FFXM_CPU)

#if defined(FFXM_HLSL)

// Unless defined, go for the conservative option.
#if !defined(FFXM_HLSL_6_2)
#define FFXM_HLSL_6_2 (0)
#endif

#define FfxFloat32Mat4 matrix <float, 4, 4>
#define FfxFloat32Mat3 matrix <float, 3, 3>

/// A typedef for a boolean value.
///
/// @ingroup HLSLTypes
typedef bool FfxBoolean;

#if FFXM_HLSL_6_2

/// @defgroup HLSL62Types HLSL 6.2 And Above Types
/// HLSL 6.2 and above type defines for all commonly used variables
///
/// @ingroup HLSLTypes

/// A typedef for a floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t   FfxFloat32;

/// A typedef for a 2-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t2  FfxFloat32x2;

/// A typedef for a 3-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t3  FfxFloat32x3;

/// A typedef for a 4-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t4  FfxFloat32x4;

/// A [cacao_placeholder] typedef for matrix type until confirmed.
typedef float4x4 FfxFloat32x4x4;
typedef float3x3 FfxFloat32x3x3;
typedef float2x2 FfxFloat32x2x2;

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup HLSL62Types
typedef uint32_t    FfxUInt32;

/// A typedef for a 2-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t2   FfxUInt32x2;

/// A typedef for a 3-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t3   FfxUInt32x3;

/// A typedef for a 4-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t4   FfxUInt32x4;

/// A typedef for a signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t     FfxInt32;

/// A typedef for a 2-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t2    FfxInt32x2;

/// A typedef for a 3-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t3    FfxInt32x3;

/// A typedef for a 4-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t4    FfxInt32x4;

#else // #if defined(FFXM_HLSL_6_2)

/// @defgroup HLSLBaseTypes HLSL 6.1 And Below Types
/// HLSL 6.1 and below type defines for all commonly used variables
///
/// @ingroup HLSLTypes

#define FfxFloat32   float
#define FfxFloat32x2 float2
#define FfxFloat32x3 float3
#define FfxFloat32x4 float4

/// A [cacao_placeholder] typedef for matrix type until confirmed.
#define FfxFloat32x4x4 float4x4
#define FfxFloat32x3x3 float3x3
#define FfxFloat32x2x2 float2x2

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup GPU
typedef uint        FfxUInt32;
typedef uint2       FfxUInt32x2;
typedef uint3       FfxUInt32x3;
typedef uint4       FfxUInt32x4;

typedef int         FfxInt32;
typedef int2        FfxInt32x2;
typedef int3        FfxInt32x3;
typedef int4        FfxInt32x4;

#endif // #if defined(FFXM_HLSL_6_2)

// Arm ASR relies in efficient FP16 arithmetic.
#if !defined(FFXM_HALF)
#define FFXM_HALF (1)
#endif

#if FFXM_HALF

#if FFXM_HLSL_6_2

typedef float16_t   FfxFloat16;
typedef float16_t2  FfxFloat16x2;
typedef float16_t3  FfxFloat16x3;
typedef float16_t4  FfxFloat16x4;

/// A typedef for an unsigned 16bit integer.
///
/// @ingroup HLSLTypes
typedef uint16_t    FfxUInt16;
typedef uint16_t2   FfxUInt16x2;
typedef uint16_t3   FfxUInt16x3;
typedef uint16_t4   FfxUInt16x4;

/// A typedef for a signed 16bit integer.
///
/// @ingroup HLSLTypes
typedef int16_t     FfxInt16;
typedef int16_t2    FfxInt16x2;
typedef int16_t3    FfxInt16x3;
typedef int16_t4    FfxInt16x4;
#else // #if FFXM_HLSL_6_2
typedef min16float  FfxFloat16;
typedef min16float2 FfxFloat16x2;
typedef min16float3 FfxFloat16x3;
typedef min16float4 FfxFloat16x4;

/// A typedef for an unsigned 16bit integer.
///
/// @ingroup HLSLTypes
typedef min16uint   FfxUInt16;
typedef min16uint2  FfxUInt16x2;
typedef min16uint3  FfxUInt16x3;
typedef min16uint4  FfxUInt16x4;

/// A typedef for a signed 16bit integer.
///
/// @ingroup HLSLTypes
typedef min16int    FfxInt16;
typedef min16int2   FfxInt16x2;
typedef min16int3   FfxInt16x3;
typedef min16int4   FfxInt16x4;
#endif  // #if FFXM_HLSL_6_2

#endif // FFXM_HALF

#endif // #if defined(FFXM_HLSL)

#if defined(FFXM_GLSL)

#define FfxFloat32Mat4 mat4
#define FfxFloat32Mat3 mat3

/// A typedef for a boolean value.
///
/// @ingroup GLSLTypes
#define FfxBoolean   bool
#define FfxFloat32   float
#define FfxFloat32x2 vec2
#define FfxFloat32x3 vec3
#define FfxFloat32x4 vec4
#define FfxUInt32    uint
#define FfxUInt32x2  uvec2
#define FfxUInt32x3  uvec3
#define FfxUInt32x4  uvec4
#define FfxInt32     int
#define FfxInt32x2   ivec2
#define FfxInt32x3   ivec3
#define FfxInt32x4   ivec4

/// A [cacao_placeholder] typedef for matrix type until confirmed.
#define FfxFloat32x4x4 mat4
#define FfxFloat32x3x3 mat3
#define FfxFloat32x2x2 mat2

#if FFXM_HALF
#define FfxFloat16   float16_t
#define FfxFloat16x2 f16vec2
#define FfxFloat16x3 f16vec3
#define FfxFloat16x4 f16vec4
#define FfxUInt16    uint16_t
#define FfxUInt16x2  u16vec2
#define FfxUInt16x3  u16vec3
#define FfxUInt16x4  u16vec4
#define FfxInt16     int16_t
#define FfxInt16x2   i16vec2
#define FfxInt16x3   i16vec3
#define FfxInt16x4   i16vec4
#endif // FFXM_HALF
#endif // #if defined(FFXM_GLSL)


#if FFXM_HALF

#if FFXM_HLSL_6_2

#define FFXM_MIN16_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType##16_t TypeName;
#define FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType##16_t, COL> TypeName;
#define FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType##16_t, ROW, COL> TypeName;

#define FFXM_16BIT_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType##16_t TypeName;
#define FFXM_16BIT_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType##16_t, COL> TypeName;
#define FFXM_16BIT_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType##16_t, ROW, COL> TypeName;

#else //FFXM_HLSL_6_2

#define FFXM_MIN16_SCALAR( TypeName, BaseComponentType )           typedef min16##BaseComponentType TypeName;
#define FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<min16##BaseComponentType, COL> TypeName;
#define FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<min16##BaseComponentType, ROW, COL> TypeName;

#define FFXM_16BIT_SCALAR( TypeName, BaseComponentType )           FFXM_MIN16_SCALAR( TypeName, BaseComponentType );
#define FFXM_16BIT_VECTOR( TypeName, BaseComponentType, COL )      FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL );
#define FFXM_16BIT_MATRIX( TypeName, BaseComponentType, ROW, COL ) FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL );

#endif //FFXM_HLSL_6_2

#else //FFXM_HALF

#define FFXM_MIN16_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType TypeName;
#define FFXM_MIN16_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType, COL> TypeName;
#define FFXM_MIN16_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType, ROW, COL> TypeName;

#define FFXM_16BIT_SCALAR( TypeName, BaseComponentType )           typedef BaseComponentType TypeName;
#define FFXM_16BIT_VECTOR( TypeName, BaseComponentType, COL )      typedef vector<BaseComponentType, COL> TypeName;
#define FFXM_16BIT_MATRIX( TypeName, BaseComponentType, ROW, COL ) typedef matrix<BaseComponentType, ROW, COL> TypeName;

#endif //FFXM_HALF

#if defined(FFXM_GPU)
// Common typedefs:
#if defined(FFXM_HLSL)
FFXM_MIN16_SCALAR( FFXM_MIN16_F , float );
FFXM_MIN16_VECTOR( FFXM_MIN16_F2, float, 2 );
FFXM_MIN16_VECTOR( FFXM_MIN16_F3, float, 3 );
FFXM_MIN16_VECTOR( FFXM_MIN16_F4, float, 4 );

FFXM_MIN16_SCALAR( FFXM_MIN16_I,  int );
FFXM_MIN16_VECTOR( FFXM_MIN16_I2, int, 2 );
FFXM_MIN16_VECTOR( FFXM_MIN16_I3, int, 3 );
FFXM_MIN16_VECTOR( FFXM_MIN16_I4, int, 4 );

FFXM_MIN16_SCALAR( FFXM_MIN16_U,  uint );
FFXM_MIN16_VECTOR( FFXM_MIN16_U2, uint, 2 );
FFXM_MIN16_VECTOR( FFXM_MIN16_U3, uint, 3 );
FFXM_MIN16_VECTOR( FFXM_MIN16_U4, uint, 4 );

FFXM_16BIT_SCALAR( FFXM_F16_t , float );
FFXM_16BIT_VECTOR( FFXM_F16_t2, float, 2 );
FFXM_16BIT_VECTOR( FFXM_F16_t3, float, 3 );
FFXM_16BIT_VECTOR( FFXM_F16_t4, float, 4 );

FFXM_16BIT_SCALAR( FFXM_I16_t,  int );
FFXM_16BIT_VECTOR( FFXM_I16_t2, int, 2 );
FFXM_16BIT_VECTOR( FFXM_I16_t3, int, 3 );
FFXM_16BIT_VECTOR( FFXM_I16_t4, int, 4 );

FFXM_16BIT_SCALAR( FFXM_U16_t,  uint );
FFXM_16BIT_VECTOR( FFXM_U16_t2, uint, 2 );
FFXM_16BIT_VECTOR( FFXM_U16_t3, uint, 3 );
FFXM_16BIT_VECTOR( FFXM_U16_t4, uint, 4 );

#define TYPEDEF_MIN16_TYPES(Prefix)           \
typedef FFXM_MIN16_F     Prefix##_F;           \
typedef FFXM_MIN16_F2    Prefix##_F2;          \
typedef FFXM_MIN16_F3    Prefix##_F3;          \
typedef FFXM_MIN16_F4    Prefix##_F4;          \
typedef FFXM_MIN16_I     Prefix##_I;           \
typedef FFXM_MIN16_I2    Prefix##_I2;          \
typedef FFXM_MIN16_I3    Prefix##_I3;          \
typedef FFXM_MIN16_I4    Prefix##_I4;          \
typedef FFXM_MIN16_U     Prefix##_U;           \
typedef FFXM_MIN16_U2    Prefix##_U2;          \
typedef FFXM_MIN16_U3    Prefix##_U3;          \
typedef FFXM_MIN16_U4    Prefix##_U4;

#define TYPEDEF_16BIT_TYPES(Prefix)           \
typedef FFXM_16BIT_F     Prefix##_F;           \
typedef FFXM_16BIT_F2    Prefix##_F2;          \
typedef FFXM_16BIT_F3    Prefix##_F3;          \
typedef FFXM_16BIT_F4    Prefix##_F4;          \
typedef FFXM_16BIT_I     Prefix##_I;           \
typedef FFXM_16BIT_I2    Prefix##_I2;          \
typedef FFXM_16BIT_I3    Prefix##_I3;          \
typedef FFXM_16BIT_I4    Prefix##_I4;          \
typedef FFXM_16BIT_U     Prefix##_U;           \
typedef FFXM_16BIT_U2    Prefix##_U2;          \
typedef FFXM_16BIT_U3    Prefix##_U3;          \
typedef FFXM_16BIT_U4    Prefix##_U4;

#define TYPEDEF_FULL_PRECISION_TYPES(Prefix)  \
typedef FfxFloat32      Prefix##_F;           \
typedef FfxFloat32x2    Prefix##_F2;          \
typedef FfxFloat32x3    Prefix##_F3;          \
typedef FfxFloat32x4    Prefix##_F4;          \
typedef FfxInt32        Prefix##_I;           \
typedef FfxInt32x2      Prefix##_I2;          \
typedef FfxInt32x3      Prefix##_I3;          \
typedef FfxInt32x4      Prefix##_I4;          \
typedef FfxUInt32       Prefix##_U;           \
typedef FfxUInt32x2     Prefix##_U2;          \
typedef FfxUInt32x3     Prefix##_U3;          \
typedef FfxUInt32x4     Prefix##_U4;
#endif // #if defined(FFXM_HLSL)

#if defined(FFXM_GLSL)

#if FFXM_HALF

#define  FFXM_MIN16_F  float16_t
#define  FFXM_MIN16_F2 f16vec2
#define  FFXM_MIN16_F3 f16vec3
#define  FFXM_MIN16_F4 f16vec4

#define  FFXM_MIN16_I  int16_t
#define  FFXM_MIN16_I2 i16vec2
#define  FFXM_MIN16_I3 i16vec3
#define  FFXM_MIN16_I4 i16vec4

#define  FFXM_MIN16_U  uint16_t
#define  FFXM_MIN16_U2 u16vec2
#define  FFXM_MIN16_U3 u16vec3
#define  FFXM_MIN16_U4 u16vec4

#define FFXM_16BIT_F  float16_t
#define FFXM_16BIT_F2 f16vec2
#define FFXM_16BIT_F3 f16vec3
#define FFXM_16BIT_F4 f16vec4

#define FFXM_16BIT_I  int16_t
#define FFXM_16BIT_I2 i16vec2
#define FFXM_16BIT_I3 i16vec3
#define FFXM_16BIT_I4 i16vec4

#define FFXM_16BIT_U  uint16_t
#define FFXM_16BIT_U2 u16vec2
#define FFXM_16BIT_U3 u16vec3
#define FFXM_16BIT_U4 u16vec4

#else // FFXM_HALF

#define  FFXM_MIN16_F  float
#define  FFXM_MIN16_F2 vec2
#define  FFXM_MIN16_F3 vec3
#define  FFXM_MIN16_F4 vec4

#define  FFXM_MIN16_I  int
#define  FFXM_MIN16_I2 ivec2
#define  FFXM_MIN16_I3 ivec3
#define  FFXM_MIN16_I4 ivec4

#define  FFXM_MIN16_U  uint
#define  FFXM_MIN16_U2 uvec2
#define  FFXM_MIN16_U3 uvec3
#define  FFXM_MIN16_U4 uvec4

#define FFXM_16BIT_F  float
#define FFXM_16BIT_F2 vec2
#define FFXM_16BIT_F3 vec3
#define FFXM_16BIT_F4 vec4

#define FFXM_16BIT_I  int
#define FFXM_16BIT_I2 ivec2
#define FFXM_16BIT_I3 ivec3
#define FFXM_16BIT_I4 ivec4

#define FFXM_16BIT_U  uint
#define FFXM_16BIT_U2 uvec2
#define FFXM_16BIT_U3 uvec3
#define FFXM_16BIT_U4 uvec4

#endif // FFXM_HALF

#endif // #if defined(FFXM_GLSL)

#endif // #if defined(FFXM_GPU)
#endif // #ifndef FFXM_COMMON_TYPES_H
// --- end ffxm_common_types.h ---
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin ffxm_core_glsl.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// @defgroup GLSLCore GLSL Core
/// GLSL core defines and functions
///
/// @ingroup FfxGLSL

/// A define for abstracting shared memory between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_GROUPSHARED shared

/// A define for abstracting compute memory barriers between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_GROUP_MEMORY_BARRIER() groupMemoryBarrier(); barrier()

/// A define for abstracting compute atomic additions between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_ATOMIC_ADD(x, y) atomicAdd(x, y)

/// A define added to accept static markup on functions to aid CPU/GPU portability of code.
///
/// @ingroup GLSLCore
#define FFXM_STATIC

/// A define for abstracting loop unrolling between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_UNROLL

/// A define for abstracting a 'greater than' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_GREATER_THAN(x, y) greaterThan(x, y)

/// A define for abstracting a 'greater than or equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_GREATER_THAN_EQUAL(x, y) greaterThanEqual(x, y)

/// A define for abstracting a 'less than' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_LESS_THAN(x, y) lessThan(x, y)

/// A define for abstracting a 'less than or equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_LESS_THAN_EQUAL(x, y) lessThanEqual(x, y)

/// A define for abstracting an 'equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_EQUAL(x, y) equal(x, y)

/// A define for abstracting a 'not equal' comparison operator between two types.
///
/// @ingroup GLSLCore
#define FFXM_NOT_EQUAL(x, y) notEqual(x, y)

/// A define for abstracting matrix multiply operations between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_MATRIX_MULTIPLY(a, b) (a * b)

/// A define for abstracting vector transformations between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_TRANSFORM_VECTOR(a, b) (a * b)

/// A define for abstracting modulo operations between shading languages.
///
/// @ingroup GLSLCore
#define FFXM_MODULO(a, b) (mod(a, b))

/// Broadcast a scalar value to a 1-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32(x)   FfxFloat32(x)

/// Broadcast a scalar value to a 2-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32X2(x) FfxFloat32x2(FfxFloat32(x))

/// Broadcast a scalar value to a 3-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32X3(x) FfxFloat32x3(FfxFloat32(x))

/// Broadcast a scalar value to a 4-dimensional floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_FLOAT32X4(x) FfxFloat32x4(FfxFloat32(x))

/// Broadcast a scalar value to a 1-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32(x)   FfxUInt32(x)

/// Broadcast a scalar value to a 2-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32X2(x) FfxUInt32x2(FfxUInt32(x))

/// Broadcast a scalar value to a 3-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32X3(x) FfxUInt32x3(FfxUInt32(x))

/// Broadcast a scalar value to a 4-dimensional unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_UINT32X4(x) FfxUInt32x4(FfxUInt32(x))

/// Broadcast a scalar value to a 1-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32(x)   FfxInt32(x)

/// Broadcast a scalar value to a 2-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32X2(x) FfxInt32x2(FfxInt32(x))

/// Broadcast a scalar value to a 3-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32X3(x) FfxInt32x3(FfxInt32(x))

/// Broadcast a scalar value to a 4-dimensional signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_INT32X4(x) FfxInt32x4(FfxInt32(x))

/// Broadcast a scalar value to a 1-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16(x)   FFXM_MIN16_F(x)

/// Broadcast a scalar value to a 2-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16X2(x) FFXM_MIN16_F2(FFXM_MIN16_F(x))

/// Broadcast a scalar value to a 3-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16X3(x) FFXM_MIN16_F3(FFXM_MIN16_F(x))

/// Broadcast a scalar value to a 4-dimensional half-precision floating point vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_FLOAT16X4(x) FFXM_MIN16_F4(FFXM_MIN16_F(x))

/// Broadcast a scalar value to a 1-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16(x)   FFXM_MIN16_U(x)

/// Broadcast a scalar value to a 2-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16X2(x) FFXM_MIN16_U2(FFXM_MIN16_U(x))

/// Broadcast a scalar value to a 3-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16X3(x) FFXM_MIN16_U3(FFXM_MIN16_U(x))

/// Broadcast a scalar value to a 4-dimensional half-precision unsigned integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_UINT16X4(x) FFXM_MIN16_U4(FFXM_MIN16_U(x))

/// Broadcast a scalar value to a 1-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16(x)   FFXM_MIN16_I(x)

/// Broadcast a scalar value to a 2-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16X2(x) FFXM_MIN16_I2(FFXM_MIN16_I(x))

/// Broadcast a scalar value to a 3-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16X3(x) FFXM_MIN16_I3(FFXM_MIN16_I(x))

/// Broadcast a scalar value to a 4-dimensional half-precision signed integer vector.
///
/// @ingroup GLSLCore
#define FFXM_BROADCAST_MIN_INT16X4(x) FFXM_MIN16_I4(FFXM_MIN16_I(x))

    #extension GL_EXT_shader_explicit_arithmetic_types : require
#if !defined(FFXM_SKIP_EXT)
#if FFXM_HALF
    #extension GL_EXT_shader_16bit_storage : require
#endif // FFXM_HALF

#if defined(FFXM_LONG)
    #extension GL_ARB_gpu_shader_int64 : require
    #extension GL_NV_shader_atomic_int64 : require
#endif // #if defined(FFXM_LONG)

#if defined(FFXM_WAVE)
    #extension GL_KHR_shader_subgroup_arithmetic : require
    #extension GL_KHR_shader_subgroup_ballot : require
    #extension GL_KHR_shader_subgroup_quad : require
    #extension GL_KHR_shader_subgroup_shuffle : require
#endif // #if defined(FFXM_WAVE)
#endif // #if !defined(FFXM_SKIP_EXT)

// Forward declarations
FfxFloat32   ffxSqrt(FfxFloat32 x);
FfxFloat32x2 ffxSqrt(FfxFloat32x2 x);
FfxFloat32x3 ffxSqrt(FfxFloat32x3 x);
FfxFloat32x4 ffxSqrt(FfxFloat32x4 x);

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32 ffxAsFloat(FfxUInt32 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxAsFloat(FfxUInt32x2 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxAsFloat(FfxUInt32x3 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as a floating-point number.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as a floating-point number.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxAsFloat(FfxUInt32x4 x)
{
    return uintBitsToFloat(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32 ffxAsUInt32(FfxFloat32 x)
{
    return floatBitsToUint(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxAsUInt32(FfxFloat32x2 x)
{
    return floatBitsToUint(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxAsUInt32(FfxFloat32x3 x)
{
    return floatBitsToUint(x);
}

/// Interprets the bit pattern of x as an unsigned integer.
///
/// @param [in] x               The input value.
///
/// @returns
/// The input interpreted as an unsigned integer.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxAsUInt32(FfxFloat32x4 x)
{
    return floatBitsToUint(x);
}

/// Convert a 32bit IEEE 754 floating point value to its nearest 16bit equivalent.
///
/// @param [in] value               The value to convert.
///
/// @returns
/// The nearest 16bit equivalent of <c><i>value</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32 f32tof16(FfxFloat32 value)
{
    return packHalf2x16(FfxFloat32x2(value, 0.0));
}

/// Broadcast a scalar value to a 2-dimensional floating point vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 2-dimensional floating point vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxBroadcast2(FfxFloat32 value)
{
    return FfxFloat32x2(value, value);
}

/// Broadcast a scalar value to a 3-dimensional floating point vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 3-dimensional floating point vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxBroadcast3(FfxFloat32 value)
{
    return FfxFloat32x3(value, value, value);
}

/// Broadcast a scalar value to a 4-dimensional floating point vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 4-dimensional floating point vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxBroadcast4(FfxFloat32 value)
{
    return FfxFloat32x4(value, value, value, value);
}

/// Broadcast a scalar value to a 2-dimensional signed integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 2-dimensional signed integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxInt32x2 ffxBroadcast2(FfxInt32 value)
{
    return FfxInt32x2(value, value);
}

/// Broadcast a scalar value to a 3-dimensional signed integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 3-dimensional signed integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxInt32x3 ffxBroadcast3(FfxInt32 value)
{
    return FfxInt32x3(value, value, value);
}

/// Broadcast a scalar value to a 4-dimensional signed integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 4-dimensional signed integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxInt32x4 ffxBroadcast4(FfxInt32 value)
{
    return FfxInt32x4(value, value, value, value);
}

/// Broadcast a scalar value to a 2-dimensional unsigned integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 2-dimensional unsigned integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxBroadcast2(FfxUInt32 value)
{
    return FfxUInt32x2(value, value);
}

/// Broadcast a scalar value to a 3-dimensional unsigned integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 3-dimensional unsigned integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxBroadcast3(FfxUInt32 value)
{
    return FfxUInt32x3(value, value, value);
}

/// Broadcast a scalar value to a 4-dimensional unsigned integer vector.
///
/// @param [in] value               The value to to broadcast.
///
/// @returns
/// A 4-dimensional unsigned integer vector with <c><i>value</i></c> in each component.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxBroadcast4(FfxUInt32 value)
{
    return FfxUInt32x4(value, value, value, value);
}

///
///
/// @ingroup GLSLCore
FfxUInt32 bitfieldExtract(FfxUInt32 src, FfxUInt32 off, FfxUInt32 bits)
{
    return bitfieldExtract(src, FfxInt32(off), FfxInt32(bits));
}

///
///
/// @ingroup GLSLCore
FfxUInt32 bitfieldInsert(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 mask)
{
    return (ins & mask) | (src & (~mask));
}

// Proxy for V_BFI_B32 where the 'mask' is set as 'bits', 'mask=(1<<bits)-1', and 'bits' needs to be an immediate.
///
///
/// @ingroup GLSLCore
FfxUInt32 bitfieldInsertMask(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 bits)
{
    return bitfieldInsert(src, ins, 0, FfxInt32(bits));
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxLerp(FfxFloat32 x, FfxFloat32 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxLerp(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxLerp(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxLerp(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxLerp(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxLerp(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32 t)
{
    return mix(x, y, t);
}

/// Compute the linear interopation between two values.
///
/// Implemented by calling the GLSL <c><i>mix</i></c> instrinsic function. Implements the
/// following math:
///
///     (1 - t) * x + t * y
///
/// @param [in] x               The first value to lerp between.
/// @param [in] y               The second value to lerp between.
/// @param [in] t               The value to determine how much of <c><i>x</i></c> and how much of <c><i>y</i></c>.
///
/// @returns
/// A linearly interpolated value between <c><i>x</i></c> and <c><i>y</i></c> according to <c><i>t</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxLerp(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 t)
{
    return mix(x, y, t);
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single V_MAX3_F32 operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxMax3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxMax3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxMax3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxMax3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32 ffxMax3(FfxUInt32 x, FfxUInt32 y, FfxUInt32 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN or RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxMax3(FfxUInt32x2 x, FfxUInt32x2 y, FfxUInt32x2 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxMax3(FfxUInt32x3 x, FfxUInt32x3 y, FfxUInt32x3 z)
{
    return max(x, max(y, z));
}

/// Compute the maximum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the max calculation.
/// @param [in] y               The second value to include in the max calcuation.
/// @param [in] z               The third value to include in the max calcuation.
///
/// @returns
/// The maximum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxMax3(FfxUInt32x4 x, FfxUInt32x4 y, FfxUInt32x4 z)
{
    return max(x, max(y, z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxMed3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxMed3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxMed3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_F32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxMed3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32 ffxMed3(FfxInt32 x, FfxInt32 y, FfxInt32 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32x2 ffxMed3(FfxInt32x2 x, FfxInt32x2 y, FfxInt32x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32x3 ffxMed3(FfxInt32x3 x, FfxInt32x3 y, FfxInt32x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}

/// Compute the median of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MED3_I32</i></c> operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the median calculation.
/// @param [in] y               The second value to include in the median calcuation.
/// @param [in] z               The third value to include in the median calcuation.
///
/// @returns
/// The median value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxInt32x4 ffxMed3(FfxInt32x4 x, FfxInt32x4 y, FfxInt32x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}


/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single <c><i>V_MIN3_F32</i></c> operation on
/// GCN and RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxMin3(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxMin3(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxMin3(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxMin3(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32 ffxMin3(FfxUInt32 x, FfxUInt32 y, FfxUInt32 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x2 ffxMin3(FfxUInt32x2 x, FfxUInt32x2 y, FfxUInt32x2 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x3 ffxMin3(FfxUInt32x3 x, FfxUInt32x3 y, FfxUInt32x3 z)
{
    return min(x, min(y, z));
}

/// Compute the minimum of three values.
///
/// NOTE: This function should compile down to a single V_MIN3_F32 operation on
/// GCN/RDNA hardware.
///
/// @param [in] x               The first value to include in the min calculation.
/// @param [in] y               The second value to include in the min calcuation.
/// @param [in] z               The third value to include in the min calcuation.
///
/// @returns
/// The minimum value of <c><i>x</i></c>, <c><i>y</i></c>, and <c><i>z</i></c>.
///
/// @ingroup GLSLCore
FfxUInt32x4 ffxMin3(FfxUInt32x4 x, FfxUInt32x4 y, FfxUInt32x4 z)
{
    return min(x, min(y, z));
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 rcp(FfxFloat32 x)
{
    return FfxFloat32(1.0) / x;
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 rcp(FfxFloat32x2 x)
{
    return ffxBroadcast2(1.0) / x;
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 rcp(FfxFloat32x3 x)
{
    return ffxBroadcast3(1.0) / x;
}

/// Compute the reciprocal of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rcp</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 rcp(FfxFloat32x4 x)
{
    return ffxBroadcast4(1.0) / x;
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 rsqrt(FfxFloat32 x)
{
    return FfxFloat32(1.0) / ffxSqrt(x);
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 rsqrt(FfxFloat32x2 x)
{
    return ffxBroadcast2(1.0) / ffxSqrt(x);
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 rsqrt(FfxFloat32x3 x)
{
    return ffxBroadcast3(1.0) / ffxSqrt(x);
}

/// Compute the reciprocal square root of a value.
///
/// NOTE: This function is only provided for GLSL. In HLSL the intrinsic function <c><i>rsqrt</i></c> can be used.
///
/// @param [in] x               The value to compute the reciprocal for.
///
/// @returns
/// The reciprocal square root value of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 rsqrt(FfxFloat32x4 x)
{
    return ffxBroadcast4(1.0) / ffxSqrt(x);
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxSaturate(FfxFloat32 x)
{
    return clamp(x, FfxFloat32(0.0), FfxFloat32(1.0));
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxSaturate(FfxFloat32x2 x)
{
    return clamp(x, ffxBroadcast2(0.0), ffxBroadcast2(1.0));
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxSaturate(FfxFloat32x3 x)
{
    return clamp(x, ffxBroadcast3(0.0), ffxBroadcast3(1.0));
}

/// Clamp a value to a [0..1] range.
///
/// @param [in] x               The value to clamp to [0..1] range.
///
/// @returns
/// The clamped version of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxSaturate(FfxFloat32x4 x)
{
    return clamp(x, ffxBroadcast4(0.0), ffxBroadcast4(1.0));
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32 ffxFract(FfxFloat32 x)
{
    return fract(x);
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x2 ffxFract(FfxFloat32x2 x)
{
    return fract(x);
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x3 ffxFract(FfxFloat32x3 x)
{
    return fract(x);
}

/// Compute the factional part of a decimal value.
///
/// This function calculates <c><i>x - floor(x)</i></c>. Where <c><i>floor</i></c> is the intrinsic HLSL function.
///
/// NOTE: This function should compile down to a single <c><i>V_MAX3_F32</i></c> operation on GCN/RDNA hardware. It is
/// worth further noting that this function is intentionally distinct from the HLSL <c><i>frac</i></c> intrinsic
/// function.
///
/// @param [in] x               The value to compute the fractional part from.
///
/// @returns
/// The fractional part of <c><i>x</i></c>.
///
/// @ingroup GLSLCore
FfxFloat32x4 ffxFract(FfxFloat32x4 x)
{
    return fract(x);
}

FfxUInt32 AShrSU1(FfxUInt32 a, FfxUInt32 b)
{
    return FfxUInt32(FfxInt32(a) >> FfxInt32(b));
}

FfxUInt32 ffxPackF32(FfxFloat32x2 v){
    return packHalf2x16(v);
}

FfxFloat32x2 ffxUnpackF32(FfxUInt32 u){
    return unpackHalf2x16(u);
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32 ffxInvertSafe(FfxFloat32 v){
    FfxFloat32 s = sign(v);
    FfxFloat32 s2 = s*s;
    return s2/(v + s2 - 1.0);
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32x2 ffxInvertSafe(FfxFloat32x2 v){
    FfxFloat32x2 s = sign(v);
    FfxFloat32x2 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x2(1.0, 1.0));
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32x3 ffxInvertSafe(FfxFloat32x3 v){
    FfxFloat32x3 s = sign(v);
    FfxFloat32x3 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x3(1.0, 1.0, 1.0));
}

/// @brief Inverts the value while avoiding division by zero. If the value is zero, zero is returned.
/// @param v Value to invert.
/// @return If v = 0 returns 0. If v != 0 returns 1/v.
FfxFloat32x4 ffxInvertSafe(FfxFloat32x4 v){
    FfxFloat32x4 s = sign(v);
    FfxFloat32x4 s2 = s*s;
    return s2/(v + s2 - FfxFloat32x4(1.0, 1.0, 1.0, 1.0));
}
#if FFXM_HALF
#define FFXM_UINT32_TO_FLOAT16X2(x) unpackFloat2x16(FfxUInt32(x))

FfxUInt32 ffxPackF16(FfxFloat16x2 v){
    return packHalf2x16(v);
}

FfxFloat16x2 ffxUnpackF16(FfxUInt32 u){
    return FfxFloat16x2(unpackHalf2x16(u));
}

FfxFloat16x4 ffxUint32x2ToFloat16x4(FfxUInt32x2 x)
{
    return FfxFloat16x4(unpackFloat2x16(x.x), unpackFloat2x16(x.y));
}
#define FFXM_UINT32X2_TO_FLOAT16X4(x) ffxUint32x2ToFloat16x4(FfxUInt32x2(x))
#define FFXM_UINT32_TO_UINT16X2(x) unpackUint2x16(FfxUInt32(x))
#define FFXM_UINT32X2_TO_UINT16X4(x) unpackUint4x16(pack64(FfxUInt32x2(x)))
//------------------------------------------------------------------------------------------------------------------------------
#define FFXM_FLOAT16X2_TO_UINT32(x) packFloat2x16(FfxFloat16x2(x))
FfxUInt32x2 ffxFloat16x4ToUint32x2(FfxFloat16x4 x)
{
    return FfxUInt32x2(packFloat2x16(x.xy), packFloat2x16(x.zw));
}
#define FFXM_FLOAT16X4_TO_UINT32X2(x) ffxFloat16x4ToUint32x2(FfxFloat16x4(x))
#define FFXM_UINT16X2_TO_UINT32(x) packUint2x16(FfxUInt16x2(x))
#define FFXM_UINT16X4_TO_UINT32X2(x) unpack32(packUint4x16(FfxUInt16x4(x)))
//==============================================================================================================================
#define FFXM_TO_UINT16(x) halfBitsToUint16(FfxFloat16(x))
#define FFXM_TO_UINT16X2(x) halfBitsToUint16(FfxFloat16x2(x))
#define FFXM_TO_UINT16X3(x) halfBitsToUint16(FfxFloat16x3(x))
#define FFXM_TO_UINT16X4(x) halfBitsToUint16(FfxFloat16x4(x))
//------------------------------------------------------------------------------------------------------------------------------
#define FFXM_TO_FLOAT16(x) uint16BitsToHalf(FfxUInt16(x))
#define FFXM_TO_FLOAT16X2(x) uint16BitsToHalf(FfxUInt16x2(x))
#define FFXM_TO_FLOAT16X3(x) uint16BitsToHalf(FfxUInt16x3(x))
#define FFXM_TO_FLOAT16X4(x) uint16BitsToHalf(FfxUInt16x4(x))
//==============================================================================================================================
FfxFloat16 ffxBroadcastFloat16(FfxFloat16 a)
{
    return FfxFloat16(a);
}
FfxFloat16x2 ffxBroadcastFloat16x2(FfxFloat16 a)
{
    return FfxFloat16x2(a, a);
}
FfxFloat16x3 ffxBroadcastFloat16x3(FfxFloat16 a)
{
    return FfxFloat16x3(a, a, a);
}
FfxFloat16x4 ffxBroadcastFloat16x4(FfxFloat16 a)
{
    return FfxFloat16x4(a, a, a, a);
}
#define FFXM_BROADCAST_FLOAT16(a)   FfxFloat16(a)
#define FFXM_BROADCAST_FLOAT16X2(a) FfxFloat16x2(FfxFloat16(a))
#define FFXM_BROADCAST_FLOAT16X3(a) FfxFloat16x3(FfxFloat16(a))
#define FFXM_BROADCAST_FLOAT16X4(a) FfxFloat16x4(FfxFloat16(a))
//------------------------------------------------------------------------------------------------------------------------------
FfxInt16 ffxBroadcastInt16(FfxInt16 a)
{
    return FfxInt16(a);
}
FfxInt16x2 ffxBroadcastInt16x2(FfxInt16 a)
{
    return FfxInt16x2(a, a);
}
FfxInt16x3 ffxBroadcastInt16x3(FfxInt16 a)
{
    return FfxInt16x3(a, a, a);
}
FfxInt16x4 ffxBroadcastInt16x4(FfxInt16 a)
{
    return FfxInt16x4(a, a, a, a);
}
#define FFXM_BROADCAST_INT16(a)   FfxInt16(a)
#define FFXM_BROADCAST_INT16X2(a) FfxInt16x2(FfxInt16(a))
#define FFXM_BROADCAST_INT16X3(a) FfxInt16x3(FfxInt16(a))
#define FFXM_BROADCAST_INT16X4(a) FfxInt16x4(FfxInt16(a))
//------------------------------------------------------------------------------------------------------------------------------
FfxUInt16 ffxBroadcastUInt16(FfxUInt16 a)
{
    return FfxUInt16(a);
}
FfxUInt16x2 ffxBroadcastUInt16x2(FfxUInt16 a)
{
    return FfxUInt16x2(a, a);
}
FfxUInt16x3 ffxBroadcastUInt16x3(FfxUInt16 a)
{
    return FfxUInt16x3(a, a, a);
}
FfxUInt16x4 ffxBroadcastUInt16x4(FfxUInt16 a)
{
    return FfxUInt16x4(a, a, a, a);
}
#define FFXM_BROADCAST_UINT16(a)   FfxUInt16(a)
#define FFXM_BROADCAST_UINT16X2(a) FfxUInt16x2(FfxUInt16(a))
#define FFXM_BROADCAST_UINT16X3(a) FfxUInt16x3(FfxUInt16(a))
#define FFXM_BROADCAST_UINT16X4(a) FfxUInt16x4(FfxUInt16(a))
//==============================================================================================================================
FfxUInt16 ffxAbsHalf(FfxUInt16 a)
{
    return FfxUInt16(abs(FfxInt16(a)));
}
FfxUInt16x2 ffxAbsHalf(FfxUInt16x2 a)
{
    return FfxUInt16x2(abs(FfxInt16x2(a)));
}
FfxUInt16x3 ffxAbsHalf(FfxUInt16x3 a)
{
    return FfxUInt16x3(abs(FfxInt16x3(a)));
}
FfxUInt16x4 ffxAbsHalf(FfxUInt16x4 a)
{
    return FfxUInt16x4(abs(FfxInt16x4(a)));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxClampHalf(FfxFloat16 x, FfxFloat16 n, FfxFloat16 m)
{
    return clamp(x, n, m);
}
FfxFloat16x2 ffxClampHalf(FfxFloat16x2 x, FfxFloat16x2 n, FfxFloat16x2 m)
{
    return clamp(x, n, m);
}
FfxFloat16x3 ffxClampHalf(FfxFloat16x3 x, FfxFloat16x3 n, FfxFloat16x3 m)
{
    return clamp(x, n, m);
}
FfxFloat16x4 ffxClampHalf(FfxFloat16x4 x, FfxFloat16x4 n, FfxFloat16x4 m)
{
    return clamp(x, n, m);
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxFract(FfxFloat16 x)
{
    return fract(x);
}
FfxFloat16x2 ffxFract(FfxFloat16x2 x)
{
    return fract(x);
}
FfxFloat16x3 ffxFract(FfxFloat16x3 x)
{
    return fract(x);
}
FfxFloat16x4 ffxFract(FfxFloat16x4 x)
{
    return fract(x);
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxLerp(FfxFloat16 x, FfxFloat16 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x2 ffxLerp(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x2 ffxLerp(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 a)
{
    return mix(x, y, a);
}
FfxFloat16x3 ffxLerp(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 a)
{
    return mix(x, y, a);
}
FfxFloat16x3 ffxLerp(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x4 ffxLerp(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16 a)
{
    return mix(x, y, a);
}
FfxFloat16x4 ffxLerp(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 a)
{
    return mix(x, y, a);
}
//------------------------------------------------------------------------------------------------------------------------------
// No packed version of ffxMax3.
FfxFloat16 ffxMax3Half(FfxFloat16 x, FfxFloat16 y, FfxFloat16 z)
{
    return max(x, max(y, z));
}
FfxFloat16x2 ffxMax3Half(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 z)
{
    return max(x, max(y, z));
}
FfxFloat16x3 ffxMax3Half(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 z)
{
    return max(x, max(y, z));
}
FfxFloat16x4 ffxMax3Half(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 z)
{
    return max(x, max(y, z));
}
//------------------------------------------------------------------------------------------------------------------------------
// No packed version of ffxMin3.
FfxFloat16 ffxMin3Half(FfxFloat16 x, FfxFloat16 y, FfxFloat16 z)
{
    return min(x, min(y, z));
}
FfxFloat16x2 ffxMin3Half(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 z)
{
    return min(x, min(y, z));
}
FfxFloat16x3 ffxMin3Half(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 z)
{
    return min(x, min(y, z));
}
FfxFloat16x4 ffxMin3Half(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 z)
{
    return min(x, min(y, z));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxMed3Half(FfxFloat16 x, FfxFloat16 y, FfxFloat16 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FfxFloat16x2 ffxMed3Half(FfxFloat16x2 x, FfxFloat16x2 y, FfxFloat16x2 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FfxFloat16x3 ffxMed3Half(FfxFloat16x3 x, FfxFloat16x3 y, FfxFloat16x3 z)
{
    return max(min(x, y), min(max(x, y), z));
}
FfxFloat16x4 ffxMed3Half(FfxFloat16x4 x, FfxFloat16x4 y, FfxFloat16x4 z)
{
    return max(min(x, y), min(max(x, y), z));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxReciprocalHalf(FfxFloat16 x)
{
    return FFXM_BROADCAST_FLOAT16(1.0) / x;
}
FfxFloat16x2 ffxReciprocalHalf(FfxFloat16x2 x)
{
    return FFXM_BROADCAST_FLOAT16X2(1.0) / x;
}
FfxFloat16x3 ffxReciprocalHalf(FfxFloat16x3 x)
{
    return FFXM_BROADCAST_FLOAT16X3(1.0) / x;
}
FfxFloat16x4 ffxReciprocalHalf(FfxFloat16x4 x)
{
    return FFXM_BROADCAST_FLOAT16X4(1.0) / x;
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxReciprocalSquareRootHalf(FfxFloat16 x)
{
    return FFXM_BROADCAST_FLOAT16(1.0) / sqrt(x);
}
FfxFloat16x2 ffxReciprocalSquareRootHalf(FfxFloat16x2 x)
{
    return FFXM_BROADCAST_FLOAT16X2(1.0) / sqrt(x);
}
FfxFloat16x3 ffxReciprocalSquareRootHalf(FfxFloat16x3 x)
{
    return FFXM_BROADCAST_FLOAT16X3(1.0) / sqrt(x);
}
FfxFloat16x4 ffxReciprocalSquareRootHalf(FfxFloat16x4 x)
{
    return FFXM_BROADCAST_FLOAT16X4(1.0) / sqrt(x);
}
//------------------------------------------------------------------------------------------------------------------------------
FfxFloat16 ffxSaturate(FfxFloat16 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16(0.0), FFXM_BROADCAST_FLOAT16(1.0));
}
FfxFloat16x2 ffxSaturate(FfxFloat16x2 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16X2(0.0), FFXM_BROADCAST_FLOAT16X2(1.0));
}
FfxFloat16x3 ffxSaturate(FfxFloat16x3 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16X3(0.0), FFXM_BROADCAST_FLOAT16X3(1.0));
}
FfxFloat16x4 ffxSaturate(FfxFloat16x4 x)
{
    return clamp(x, FFXM_BROADCAST_FLOAT16X4(0.0), FFXM_BROADCAST_FLOAT16X4(1.0));
}
//------------------------------------------------------------------------------------------------------------------------------
FfxUInt16 ffxBitShiftRightHalf(FfxUInt16 a, FfxUInt16 b)
{
    return FfxUInt16(FfxInt16(a) >> FfxInt16(b));
}
FfxUInt16x2 ffxBitShiftRightHalf(FfxUInt16x2 a, FfxUInt16x2 b)
{
    return FfxUInt16x2(FfxInt16x2(a) >> FfxInt16x2(b));
}
FfxUInt16x3 ffxBitShiftRightHalf(FfxUInt16x3 a, FfxUInt16x3 b)
{
    return FfxUInt16x3(FfxInt16x3(a) >> FfxInt16x3(b));
}
FfxUInt16x4 ffxBitShiftRightHalf(FfxUInt16x4 a, FfxUInt16x4 b)
{
    return FfxUInt16x4(FfxInt16x4(a) >> FfxInt16x4(b));
}
#endif // FFXM_HALF

#if defined(FFXM_WAVE)
// Where 'x' must be a compile time literal.
FfxFloat32 AWaveXorF1(FfxFloat32 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxFloat32x2 AWaveXorF2(FfxFloat32x2 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxFloat32x3 AWaveXorF3(FfxFloat32x3 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxFloat32x4 AWaveXorF4(FfxFloat32x4 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32 AWaveXorU1(FfxUInt32 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32x2 AWaveXorU2(FfxUInt32x2 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32x3 AWaveXorU3(FfxUInt32x3 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxUInt32x4 AWaveXorU4(FfxUInt32x4 v, FfxUInt32 x)
{
    return subgroupShuffleXor(v, x);
}
FfxBoolean AWaveIsFirstLane()
{
    return subgroupElect();
}
FfxUInt32 AWaveLaneIndex()
{
    return gl_SubgroupInvocationID;
}
FfxBoolean AWaveReadAtLaneIndexB1(FfxBoolean v, FfxUInt32 x )
{
    return subgroupShuffle(v, x);
}
FfxUInt32 AWavePrefixCountBits(FfxBoolean v)
{
    return subgroupBallotExclusiveBitCount(subgroupBallot(v));
}
FfxUInt32 AWaveActiveCountBits(FfxBoolean v)
{
    return subgroupBallotBitCount(subgroupBallot(v));
}
FfxUInt32 AWaveReadLaneFirstU1(FfxUInt32 v)
{
    return subgroupBroadcastFirst(v);
}
FfxUInt32 WaveOr(FfxUInt32 a)
{
    return subgroupOr(a);
}
FfxFloat32 WaveMin(FfxFloat32 a)
{
    return subgroupMin(a);
}
FfxFloat32 WaveMax(FfxFloat32 a)
{
    return subgroupMax(a);
}
FfxUInt32 WaveLaneCount()
{
    return gl_SubgroupSize;
}
#if defined(FFXM_WAVE_ALL_TRUE)
FfxBoolean WaveAllTrue(FfxBoolean v)
{
    return subgroupAll(v);
}
#endif
FfxFloat32 QuadReadX(FfxFloat32 v)
{
    return subgroupQuadSwapHorizontal(v);
}
FfxFloat32x2 QuadReadX(FfxFloat32x2 v)
{
    return subgroupQuadSwapHorizontal(v);
}
FfxFloat32 QuadReadY(FfxFloat32 v)
{
    return subgroupQuadSwapVertical(v);
}
FfxFloat32x2 QuadReadY(FfxFloat32x2 v)
{
    return subgroupQuadSwapVertical(v);
}

//------------------------------------------------------------------------------------------------------------------------------
#if FFXM_HALF
FfxFloat16x2 ffxWaveXorFloat16x2(FfxFloat16x2 v, FfxUInt32 x)
{
    return FFXM_UINT32_TO_FLOAT16X2(subgroupShuffleXor(FFXM_FLOAT16X2_TO_UINT32(v), x));
}
FfxFloat16x4 ffxWaveXorFloat16x4(FfxFloat16x4 v, FfxUInt32 x)
{
    return FFXM_UINT32X2_TO_FLOAT16X4(subgroupShuffleXor(FFXM_FLOAT16X4_TO_UINT32X2(v), x));
}
FfxUInt16x2 ffxWaveXorUint16x2(FfxUInt16x2 v, FfxUInt32 x)
{
    return FFXM_UINT32_TO_UINT16X2(subgroupShuffleXor(FFXM_UINT16X2_TO_UINT32(v), x));
}
FfxUInt16x4 ffxWaveXorUint16x4(FfxUInt16x4 v, FfxUInt32 x)
{
    return FFXM_UINT32X2_TO_UINT16X4(subgroupShuffleXor(FFXM_UINT16X4_TO_UINT32X2(v), x));
}
#endif // FFXM_HALF
#endif // #if defined(FFXM_WAVE)
// --- end ffxm_core_glsl.h ---
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin ffxm_core_gpu_common.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// Shader quality related defines
///
/// FFXM_FSR2_OPTION_SHADER_OPT_BALANCED. If defined, optimizations related to the balanced preset will be enabled.
/// FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE. If defined, optimizations related to the performance preset will be enabled.
/// FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE. If defined, optimizations related to the ultra performance preset will be enabled.
#ifndef FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE
#define FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE 0
#endif
#ifndef FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE
#define FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE 0
#endif
#ifndef FFXM_FSR2_OPTION_SHADER_OPT_BALANCED
#define FFXM_FSR2_OPTION_SHADER_OPT_BALANCED 0
#endif
/// FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE. Helper to identify if any of these profiles is used.
#define FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE (FFXM_FSR2_OPTION_SHADER_OPT_BALANCED || FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE)

/// Both Balanced/Performance. Keep the temporal reactive as a separate RT to improve bandwidth of color history buffer.
#define FFXM_SHADER_QUALITY_OPT_SEPARATE_TEMPORAL_REACTIVE FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Both Balanced/Performance. Disable deringing when doing the color reprojection with the history
#define FFXM_SHADER_QUALITY_OPT_DISABLE_DERINGING FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Both Balanced/Performance. Disable the Luma stability factor
#define FFXM_SHADER_QUALITY_OPT_DISABLE_LUMA_INSTABILITY FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Both Balanced/Performance. Use a 5-tap Lanczos kernel instead of the 9-tap used for `Quality` for upsampling
#define FFXM_SHADER_QUALITY_OPT_UPSCALING_LANCZOS_5TAP FFXM_SHADER_QUALITY_BALANCED_OR_PERFORMANCE
/// Balanced. Use Catmull-Rom (9 samples) for history reprojection
#define FFXM_SHADER_QUALITY_OPT_REPROJECT_CATMULL_9TAP FFXM_FSR2_OPTION_SHADER_OPT_BALANCED
/// Performance. PreparedInputColor is now stored as R8G8B8A8_Unorm tonemapped data. Rectification don't use `YCoCg` anymore
#define FFXM_SHADER_QUALITY_OPT_TONEMAPPED_RGB_PREPARED_INPUT_COLOR FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE
/// Performance. Use Catmull-Rom (5 samples) for history reprojection
#define FFXM_SHADER_QUALITY_OPT_REPROJECT_CATMULL_5TAP FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE

#if !defined(FFXM_SHADER_PLATFORM_GLES_3_2)
#define FFXM_SHADER_PLATFORM_GLES_3_2 (0)
#endif

/// A define for a true value in a boolean expression.
///
/// @ingroup GPUCore
#define FFXM_TRUE (true)

/// A define for a false value in a boolean expression.
///
/// @ingroup GPUCore
#define FFXM_FALSE (false)

/// A define value for positive infinity.
///
/// @ingroup GPUCore
#define FFXM_POSITIVE_INFINITY_FLOAT ffxAsFloat(0x7f800000u)

/// A define value for negative infinity.
///
/// @ingroup GPUCore
#define FFXM_NEGATIVE_INFINITY_FLOAT ffxAsFloat(0xff800000u)

/// A define value for PI.
///
/// @ingroup GPUCore
#define FFXM_PI  (3.14159)


/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxReciprocal(FfxFloat32 value)
{
    return rcp(value);
}

/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxReciprocal(FfxFloat32x2 value)
{
    return rcp(value);
}

/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxReciprocal(FfxFloat32x3 value)
{
    return rcp(value);
}

/// Compute the reciprocal of <c><i>value</i></c>.
///
/// @param [in] value               The value to compute the reciprocal of.
///
/// @returns
/// The 1 / <c><i>value</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxReciprocal(FfxFloat32x4 value)
{
    return rcp(value);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32 ffxMin(FfxFloat32 x, FfxFloat32 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxMin(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxMin(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxMin(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32 ffxMin(FfxInt32 x, FfxInt32 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x2 ffxMin(FfxInt32x2 x, FfxInt32x2 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x3 ffxMin(FfxInt32x3 x, FfxInt32x3 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x4 ffxMin(FfxInt32x4 x, FfxInt32x4 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32 ffxMin(FfxUInt32 x, FfxUInt32 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxMin(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxMin(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return min(x, y);
}

/// Compute the min of two values.
///
/// @param [in] x                   The first value to compute the min of.
/// @param [in] y                   The second value to compute the min of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxMin(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return min(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32 ffxMax(FfxFloat32 x, FfxFloat32 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxMax(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxMax(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxMax(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32 ffxMax(FfxInt32 x, FfxInt32 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x2 ffxMax(FfxInt32x2 x, FfxInt32x2 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x3 ffxMax(FfxInt32x3 x, FfxInt32x3 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxInt32x4 ffxMax(FfxInt32x4 x, FfxInt32x4 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32 ffxMax(FfxUInt32 x, FfxUInt32 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxMax(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxMax(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return max(x, y);
}

/// Compute the max of two values.
///
/// @param [in] x                   The first value to compute the max of.
/// @param [in] y                   The second value to compute the max of.
///
/// @returns
/// The the lowest of two values.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxMax(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return max(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32 ffxPow(FfxFloat32 x, FfxFloat32 y)
{
    return pow(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxPow(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return pow(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxPow(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return pow(x, y);
}

/// Compute the value of the first parameter raised to the power of the second.
///
/// @param [in] x                   The value to raise to the power y.
/// @param [in] y                   The power to which to raise x.
///
/// @returns
/// The value of the first parameter raised to the power of the second.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxPow(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return pow(x, y);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxSqrt(FfxFloat32 x)
{
    return sqrt(x);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxSqrt(FfxFloat32x2 x)
{
    return sqrt(x);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxSqrt(FfxFloat32x3 x)
{
    return sqrt(x);
}

/// Compute the square root of a value.
///
/// @param [in] x                   The first value to compute the min of.
///
/// @returns
/// The the square root of <c><i>x</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxSqrt(FfxFloat32x4 x)
{
    return sqrt(x);
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxCopySignBit(FfxFloat32 d, FfxFloat32 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & FfxUInt32(0x80000000u)));
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxCopySignBit(FfxFloat32x2 d, FfxFloat32x2 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & ffxBroadcast2(0x80000000u)));
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxCopySignBit(FfxFloat32x3 d, FfxFloat32x3 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & ffxBroadcast3(0x80000000u)));
}

/// Copy the sign bit from 's' to positive 'd'.
///
/// @param [in] d                   The value to copy the sign bit into.
/// @param [in] s                   The value to copy the sign bit from.
///
/// @returns
/// The value of <c><i>d</i></c> with the sign bit from <c><i>s</i></c>.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxCopySignBit(FfxFloat32x4 d, FfxFloat32x4 s)
{
    return ffxAsFloat(ffxAsUInt32(d) | (ffxAsUInt32(s) & ffxBroadcast4(0x80000000u)));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against 0.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or position.
///
/// @ingroup GPUCore
FfxFloat32 ffxIsSigned(FfxFloat32 m)
{
    return ffxSaturate(m * FfxFloat32(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against 0.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or position.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxIsSigned(FfxFloat32x2 m)
{
    return ffxSaturate(m * ffxBroadcast2(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against 0.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or position.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxIsSigned(FfxFloat32x3 m)
{
    return ffxSaturate(m * ffxBroadcast3(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 0
///     m >= 0  := 0
///     m < 0   := 1
///
/// Uses the following useful floating point logic,
///     saturate(+a*(-INF)==-INF) := 0
///     saturate( 0*(-INF)== NaN) := 0
///     saturate(-a*(-INF)==+INF) := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against for have the sign set.
///
/// @returns
/// 1.0 when the value is negative, or 0.0 when the value is 0 or positive.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxIsSigned(FfxFloat32x4 m)
{
    return ffxSaturate(m * ffxBroadcast4(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32 ffxIsGreaterThanZero(FfxFloat32 m)
{
    return ffxSaturate(m * FfxFloat32(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxIsGreaterThanZero(FfxFloat32x2 m)
{
    return ffxSaturate(m * ffxBroadcast2(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxIsGreaterThanZero(FfxFloat32x3 m)
{
    return ffxSaturate(m * ffxBroadcast3(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// A single operation to return the following:
///     m = NaN := 1
///     m > 0   := 0
///     m <= 0  := 1
///
/// This function is useful when creating masks for branch-free logic.
///
/// @param [in] m                       The value to test against zero.
///
/// @returns
/// 1.0 when the value is position, or 0.0 when the value is 0 or negative.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxIsGreaterThanZero(FfxFloat32x4 m)
{
    return ffxSaturate(m * ffxBroadcast4(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Convert a 32bit floating point value to sortable integer.
///
///  - If sign bit=0, flip the sign bit (positives).
///  - If sign bit=1, flip all bits     (negatives).
///
/// The function has the side effects that:
///  - Larger integers are more positive values.
///  - Float zero is mapped to center of integers (so clear to integer zero is a nice default for atomic max usage).
///
/// @param [in] value                       The floating point value to make sortable.
///
/// @returns
/// The sortable integer value.
///
/// @ingroup GPUCore
FfxUInt32 ffxFloatToSortableInteger(FfxUInt32 value)
{
    return value ^ ((AShrSU1(value, FfxUInt32(31))) | FfxUInt32(0x80000000));
}

/// Convert a sortable integer to a 32bit floating point value.
///
/// The function has the side effects that:
///  - If sign bit=1, flip the sign bit (positives).
///  - If sign bit=0, flip all bits     (negatives).
///
/// @param [in] value                       The floating point value to make sortable.
///
/// @returns
/// The sortable integer value.
///
/// @ingroup GPUCore
FfxUInt32 ffxSortableIntegerToFloat(FfxUInt32 value)
{
    return value ^ ((~AShrSU1(value, FfxUInt32(31))) | FfxUInt32(0x80000000));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateSqrt(FfxFloat32 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> FfxUInt32(1)) + FfxUInt32(0x1fbc4639));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateReciprocal(FfxFloat32 value)
{
    return ffxAsFloat(FfxUInt32(0x7ef07ebb) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateReciprocalMedium(FfxFloat32 value)
{
    FfxFloat32 b = ffxAsFloat(FfxUInt32(0x7ef19fff) - ffxAsUInt32(value));
    return b * (-b * value + FfxFloat32(2.0));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal square root for.
///
/// @returns
/// An approximation of the reciprocal square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateReciprocalSquareRoot(FfxFloat32 value)
{
    return ffxAsFloat(FfxUInt32(0x5f347d74) - (ffxAsUInt32(value) >> FfxUInt32(1)));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateSqrt(FfxFloat32x2 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> ffxBroadcast2(1u)) + ffxBroadcast2(0x1fbc4639u));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateReciprocal(FfxFloat32x2 value)
{
    return ffxAsFloat(ffxBroadcast2(0x7ef07ebbu) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateReciprocalMedium(FfxFloat32x2 value)
{
    FfxFloat32x2 b = ffxAsFloat(ffxBroadcast2(0x7ef19fffu) - ffxAsUInt32(value));
    return b * (-b * value + ffxBroadcast2(2.0f));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateReciprocalSquareRoot(FfxFloat32x2 value)
{
    return ffxAsFloat(ffxBroadcast2(0x5f347d74u) - (ffxAsUInt32(value) >> ffxBroadcast2(1u)));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateSqrt(FfxFloat32x3 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> ffxBroadcast3(1u)) + ffxBroadcast3(0x1fbc4639u));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateReciprocal(FfxFloat32x3 value)
{
    return ffxAsFloat(ffxBroadcast3(0x7ef07ebbu) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateReciprocalMedium(FfxFloat32x3 value)
{
    FfxFloat32x3 b = ffxAsFloat(ffxBroadcast3(0x7ef19fffu) - ffxAsUInt32(value));
    return b * (-b * value + ffxBroadcast3(2.0f));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateReciprocalSquareRoot(FfxFloat32x3 value)
{
    return ffxAsFloat(ffxBroadcast3(0x5f347d74u) - (ffxAsUInt32(value) >> ffxBroadcast3(1u)));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateSqrt(FfxFloat32x4 value)
{
    return ffxAsFloat((ffxAsUInt32(value) >> ffxBroadcast4(1u)) + ffxBroadcast4(0x1fbc4639u));
}

/// Calculate a low-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateReciprocal(FfxFloat32x4 value)
{
    return ffxAsFloat(ffxBroadcast4(0x7ef07ebbu) - ffxAsUInt32(value));
}

/// Calculate a medium-quality approximation for the reciprocal of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the reciprocal for.
///
/// @returns
/// An approximation of the reciprocal, estimated to medium quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateReciprocalMedium(FfxFloat32x4 value)
{
    FfxFloat32x4 b = ffxAsFloat(ffxBroadcast4(0x7ef19fffu) - ffxAsUInt32(value));
    return b * (-b * value + ffxBroadcast4(2.0f));
}

/// Calculate a low-quality approximation for the square root of a value.
///
/// For additional information on the approximation family of functions, you can refer to Michal Drobot's excellent
/// presentation materials:
///
///  - https://michaldrobot.files.wordpress.com/2014/05/gcn_alu_opt_digitaldragons2014.pdf
///  - https://github.com/michaldrobot/ShaderFastLibs/blob/master/ShaderFastMathLib.h
///
/// @param [in] value           The value to calculate an approximate to the square root for.
///
/// @returns
/// An approximation of the square root, estimated to low quality.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateReciprocalSquareRoot(FfxFloat32x4 value)
{
    return ffxAsFloat(ffxBroadcast4(0x5f347d74u) - (ffxAsUInt32(value) >> ffxBroadcast4(1u)));
}

/// Calculate dot product of 'a' and 'b'.
///
/// @param [in] a                   First vector input.
/// @param [in] b                   Second vector input.
///
/// @returns
/// The value of <c><i>a</i></c> dot <c><i>b</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxDot2(FfxFloat32x2 a, FfxFloat32x2 b)
{
    return dot(a, b);
}

/// Calculate dot product of 'a' and 'b'.
///
/// @param [in] a                   First vector input.
/// @param [in] b                   Second vector input.
///
/// @returns
/// The value of <c><i>a</i></c> dot <c><i>b</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxDot3(FfxFloat32x3 a, FfxFloat32x3 b)
{
    return dot(a, b);
}

/// Calculate dot product of 'a' and 'b'.
///
/// @param [in] a                   First vector input.
/// @param [in] b                   Second vector input.
///
/// @returns
/// The value of <c><i>a</i></c> dot <c><i>b</i></c>.
///
/// @ingroup GPUCore
FfxFloat32 ffxDot4(FfxFloat32x4 a, FfxFloat32x4 b)
{
    return dot(a, b);
}


/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximatePQToGamma2Medium(FfxFloat32 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximatePQToLinear(FfxFloat32 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateGamma2ToPQ(FfxFloat32 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(2)) + FfxUInt32(0x2F9A4E46));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateGamma2ToPQMedium(FfxFloat32 a)
{
    FfxFloat32 b  = ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(2)) + FfxUInt32(0x2F9A4E46));
    FfxFloat32 b4 = b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateGamma2ToPQHigh(FfxFloat32 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateLinearToPQ(FfxFloat32 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(3)) + FfxUInt32(0x378D8723));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateLinearToPQMedium(FfxFloat32 a)
{
    FfxFloat32 b  = ffxAsFloat((ffxAsUInt32(a) >> FfxUInt32(3)) + FfxUInt32(0x378D8723));
    FfxFloat32 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32 ffxApproximateLinearToPQHigh(FfxFloat32 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximatePQToGamma2Medium(FfxFloat32x2 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximatePQToLinear(FfxFloat32x2 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateGamma2ToPQ(FfxFloat32x2 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(2u)) + ffxBroadcast2(0x2F9A4E46u));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateGamma2ToPQMedium(FfxFloat32x2 a)
{
    FfxFloat32x2 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(2u)) + ffxBroadcast2(0x2F9A4E46u));
    FfxFloat32x2 b4 = b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateGamma2ToPQHigh(FfxFloat32x2 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateLinearToPQ(FfxFloat32x2 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(3u)) + ffxBroadcast2(0x378D8723u));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateLinearToPQMedium(FfxFloat32x2 a)
{
    FfxFloat32x2 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast2(3u)) + ffxBroadcast2(0x378D8723u));
    FfxFloat32x2 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxApproximateLinearToPQHigh(FfxFloat32x2 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximatePQToGamma2Medium(FfxFloat32x3 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximatePQToLinear(FfxFloat32x3 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateGamma2ToPQ(FfxFloat32x3 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(2u)) + ffxBroadcast3(0x2F9A4E46u));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateGamma2ToPQMedium(FfxFloat32x3 a)
{
    FfxFloat32x3 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(2u)) + ffxBroadcast3(0x2F9A4E46u));
    FfxFloat32x3 b4 = b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateGamma2ToPQHigh(FfxFloat32x3 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateLinearToPQ(FfxFloat32x3 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(3u)) + ffxBroadcast3(0x378D8723u));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateLinearToPQMedium(FfxFloat32x3 a)
{
    FfxFloat32x3 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast3(3u)) + ffxBroadcast3(0x378D8723u));
    FfxFloat32x3 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxApproximateLinearToPQHigh(FfxFloat32x3 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

/// Compute an approximate conversion from PQ to Gamma2 space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and Gamma2.
///
/// @returns
/// The value <c><i>a</i></c> converted into Gamma2.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximatePQToGamma2Medium(FfxFloat32x4 a)
{
    return a * a * a * a;
}

/// Compute an approximate conversion from PQ to linear space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between PQ and linear.
///
/// @returns
/// The value <c><i>a</i></c> converted into linear.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximatePQToLinear(FfxFloat32x4 a)
{
    return a * a * a * a * a * a * a * a;
}

/// Compute an approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateGamma2ToPQ(FfxFloat32x4 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(2u)) + ffxBroadcast4(0x2F9A4E46u));
}

/// Compute a more accurate approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateGamma2ToPQMedium(FfxFloat32x4 a)
{
    FfxFloat32x4 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(2u)) + ffxBroadcast4(0x2F9A4E46u));
    FfxFloat32x4 b4 = b * b * b * b * b * b * b * b;
    return b - b * (b4 - a) / (FfxFloat32(4.0) * b4);
}

/// Compute a high accuracy approximate conversion from gamma2 to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between gamma2 and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateGamma2ToPQHigh(FfxFloat32x4 a)
{
    return ffxSqrt(ffxSqrt(a));
}

/// Compute an approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateLinearToPQ(FfxFloat32x4 a)
{
    return ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(3u)) + ffxBroadcast4(0x378D8723u));
}

/// Compute a more accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateLinearToPQMedium(FfxFloat32x4 a)
{
    FfxFloat32x4 b  = ffxAsFloat((ffxAsUInt32(a) >> ffxBroadcast4(3u)) + ffxBroadcast4(0x378D8723u));
    FfxFloat32x4 b8 = b * b * b * b * b * b * b * b;
    return b - b * (b8 - a) / (FfxFloat32(8.0) * b8);
}

/// Compute a very accurate approximate conversion from linear to PQ space.
///
/// PQ is very close to x^(1/8). The functions below Use the fast FfxFloat32 approximation method to do
/// PQ conversions to and from Gamma2 (4th power and fast 4th root), and PQ to and from Linear
/// (8th power and fast 8th root). The maximum error is approximately 0.2%.
///
/// @param a                    The value to convert between linear and PQ.
///
/// @returns
/// The value <c><i>a</i></c> converted into PQ.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxApproximateLinearToPQHigh(FfxFloat32x4 a)
{
    return ffxSqrt(ffxSqrt(ffxSqrt(a)));
}

// An approximation of sine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate sine for.
//
// @returns
// The approximate sine of <c><i>value</i></c>.
FfxFloat32 ffxParabolicSin(FfxFloat32 value)
{
    return value * abs(value) - value;
}

// An approximation of sine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate sine for.
//
// @returns
// The approximate sine of <c><i>value</i></c>.
FfxFloat32x2 ffxParabolicSin(FfxFloat32x2 x)
{
    return x * abs(x) - x;
}

// An approximation of cosine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate cosine for.
//
// @returns
// The approximate cosine of <c><i>value</i></c>.
FfxFloat32 ffxParabolicCos(FfxFloat32 x)
{
    x = ffxFract(x * FfxFloat32(0.5) + FfxFloat32(0.75));
    x = x * FfxFloat32(2.0) - FfxFloat32(1.0);
    return ffxParabolicSin(x);
}

// An approximation of cosine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate cosine for.
//
// @returns
// The approximate cosine of <c><i>value</i></c>.
FfxFloat32x2 ffxParabolicCos(FfxFloat32x2 x)
{
    x = ffxFract(x * ffxBroadcast2(0.5f) + ffxBroadcast2(0.75f));
    x = x * ffxBroadcast2(2.0f) - ffxBroadcast2(1.0f);
    return ffxParabolicSin(x);
}

// An approximation of both sine and cosine.
//
// Valid input range is {-1 to 1} representing {0 to 2 pi}, and the output range
// is {-1/4 to 1/4} representing {-1 to 1}.
//
// @param [in] value            The value to calculate approximate cosine for.
//
// @returns
// A <c><i>FfxFloat32x2</i></c> containing approximations of both sine and cosine of <c><i>value</i></c>.
FfxFloat32x2 ffxParabolicSinCos(FfxFloat32 x)
{
    FfxFloat32 y = ffxFract(x * FfxFloat32(0.5) + FfxFloat32(0.75));
    y = y * FfxFloat32(2.0) - FfxFloat32(1.0);
    return ffxParabolicSin(FfxFloat32x2(x, y));
}

/// Conditional free logic AND operation using values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneAnd(FfxUInt32 x, FfxUInt32 y)
{
    return min(x, y);
}

/// Conditional free logic AND operation using two values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneAnd(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return min(x, y);
}

/// Conditional free logic AND operation using two values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneAnd(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return min(x, y);
}

/// Conditional free logic AND operation using two values.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
///
/// @returns
/// Result of the AND operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneAnd(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return min(x, y);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneAnd(FfxUInt32 x)
{
    return x ^ FfxUInt32(1);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneAnd(FfxUInt32x2 x)
{
    return x ^ ffxBroadcast2(1u);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneAnd(FfxUInt32x3 x)
{
    return x ^ ffxBroadcast3(1u);
}

/// Conditional free logic NOT operation using two values.
///
/// @param [in] x           The first value to be fed into the NOT operator.
///
/// @returns
/// Result of the NOT operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneAnd(FfxUInt32x4 x)
{
    return x ^ ffxBroadcast4(1u);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneOr(FfxUInt32 x, FfxUInt32 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneOr(FfxUInt32x2 x, FfxUInt32x2 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneOr(FfxUInt32x3 x, FfxUInt32x3 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneOr(FfxUInt32x4 x, FfxUInt32x4 y)
{
    return max(x, y);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32 ffxZeroOneAndToU1(FfxFloat32 x)
{
    return FfxUInt32(FfxFloat32(1.0) - x);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxZeroOneAndToU2(FfxFloat32x2 x)
{
    return FfxUInt32x2(ffxBroadcast2(1.0) - x);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32x3 ffxZeroOneAndToU3(FfxFloat32x3 x)
{
    return FfxUInt32x3(ffxBroadcast3(1.0) - x);
}

/// Conditional free logic signed NOT operation using two half-precision FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxUInt32x4 ffxZeroOneAndToU4(FfxFloat32x4 x)
{
    return FfxUInt32x4(ffxBroadcast4(1.0) - x);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneAndOr(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    return ffxSaturate(x * y + z);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneAndOr(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    return ffxSaturate(x * y + z);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneAndOr(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    return ffxSaturate(x * y + z);
}

/// Conditional free logic AND operation using two values followed by a NOT operation
/// using the resulting value and a third value.
///
/// @param [in] x           The first value to be fed into the AND operator.
/// @param [in] y           The second value to be fed into the AND operator.
/// @param [in] z           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneAndOr(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    return ffxSaturate(x * y + z);
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneIsGreaterThanZero(FfxFloat32 x)
{
    return ffxSaturate(x * FfxFloat32(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneIsGreaterThanZero(FfxFloat32x2 x)
{
    return ffxSaturate(x * ffxBroadcast2(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneIsGreaterThanZero(FfxFloat32x3 x)
{
    return ffxSaturate(x * ffxBroadcast3(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if greater than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the greater than zero comparison.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneIsGreaterThanZero(FfxFloat32x4 x)
{
    return ffxSaturate(x * ffxBroadcast4(FFXM_POSITIVE_INFINITY_FLOAT));
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneAnd(FfxFloat32 x)
{
    return FfxFloat32(1.0) - x;
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneAnd(FfxFloat32x2 x)
{
    return ffxBroadcast2(1.0) - x;
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneAnd(FfxFloat32x3 x)
{
    return ffxBroadcast3(1.0) - x;
}

/// Conditional free logic signed NOT operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the AND OR operator.
///
/// @returns
/// Result of the AND OR operation.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneAnd(FfxFloat32x4 x)
{
    return ffxBroadcast4(1.0) - x;
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneOr(FfxFloat32 x, FfxFloat32 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneOr(FfxFloat32x2 x, FfxFloat32x2 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneOr(FfxFloat32x3 x, FfxFloat32x3 y)
{
    return max(x, y);
}

/// Conditional free logic OR operation using two FfxFloat32 values.
///
/// @param [in] x           The first value to be fed into the OR operator.
/// @param [in] y           The second value to be fed into the OR operator.
///
/// @returns
/// Result of the OR operation.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneOr(FfxFloat32x4 x, FfxFloat32x4 y)
{
    return max(x, y);
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneSelect(FfxFloat32 x, FfxFloat32 y, FfxFloat32 z)
{
    FfxFloat32 r = (-x) * z + z;
    return x * y + r;
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneSelect(FfxFloat32x2 x, FfxFloat32x2 y, FfxFloat32x2 z)
{
    FfxFloat32x2 r = (-x) * z + z;
    return x * y + r;
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneSelect(FfxFloat32x3 x, FfxFloat32x3 y, FfxFloat32x3 z)
{
    FfxFloat32x3 r = (-x) * z + z;
    return x * y + r;
}

/// Choose between two FfxFloat32 values if the first paramter is greater than zero.
///
/// @param [in] x           The value to compare against zero.
/// @param [in] y           The value to return if the comparision is greater than zero.
/// @param [in] z           The value to return if the comparision is less than or equal to zero.
///
/// @returns
/// The selected value.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneSelect(FfxFloat32x4 x, FfxFloat32x4 y, FfxFloat32x4 z)
{
    FfxFloat32x4 r = (-x) * z + z;
    return x * y + r;
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32 ffxZeroOneIsSigned(FfxFloat32 x)
{
    return ffxSaturate(x * FfxFloat32(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxZeroOneIsSigned(FfxFloat32x2 x)
{
    return ffxSaturate(x * ffxBroadcast2(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxZeroOneIsSigned(FfxFloat32x3 x)
{
    return ffxSaturate(x * ffxBroadcast3(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Given a value, returns 1.0 if less than zero and 0.0 if not.
///
/// @param [in] x           The value to be compared.
///
/// @returns
/// Result of the sign value.
///
/// @ingroup GPUCore
FfxFloat32x4 ffxZeroOneIsSigned(FfxFloat32x4 x)
{
    return ffxSaturate(x * ffxBroadcast4(FFXM_NEGATIVE_INFINITY_FLOAT));
}

/// Compute a Rec.709 color space.
///
/// Rec.709 is used for some HDTVs.
///
/// Both Rec.709 and sRGB have a linear segment which as spec'ed would intersect the curved segment 2 times.
///  (a.) For 8-bit sRGB, steps {0 to 10.3} are in the linear region (4% of the encoding range).
///  (b.) For 8-bit  709, steps {0 to 20.7} are in the linear region (8% of the encoding range).
///
/// @param [in] color           The color to convert to Rec. 709.
///
/// @returns
/// The <c><i>color</i></c> in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxRec709FromLinear(FfxFloat32 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.018 * 4.5, 4.5, 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.099, -0.099);
    return clamp(j.x, color * j.y, pow(color, j.z) * k.x + k.y);
}

/// Compute a Rec.709 color space.
///
/// Rec.709 is used for some HDTVs.
///
/// Both Rec.709 and sRGB have a linear segment which as spec'ed would intersect the curved segment 2 times.
///  (a.) For 8-bit sRGB, steps {0 to 10.3} are in the linear region (4% of the encoding range).
///  (b.) For 8-bit  709, steps {0 to 20.7} are in the linear region (8% of the encoding range).
///
/// @param [in] color           The color to convert to Rec. 709.
///
/// @returns
/// The <c><i>color</i></c> in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxRec709FromLinear(FfxFloat32x2 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.018 * 4.5, 4.5, 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.099, -0.099);
    return clamp(j.xx, color * j.yy, pow(color, j.zz) * k.xx + k.yy);
}

/// Compute a Rec.709 color space.
///
/// Rec.709 is used for some HDTVs.
///
/// Both Rec.709 and sRGB have a linear segment which as spec'ed would intersect the curved segment 2 times.
///  (a.) For 8-bit sRGB, steps {0 to 10.3} are in the linear region (4% of the encoding range).
///  (b.) For 8-bit  709, steps {0 to 20.7} are in the linear region (8% of the encoding range).
///
/// @param [in] color           The color to convert to Rec. 709.
///
/// @returns
/// The <c><i>color</i></c> in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxRec709FromLinear(FfxFloat32x3 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.018 * 4.5, 4.5, 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.099, -0.099);
    return clamp(j.xxx, color * j.yyy, pow(color, j.zzz) * k.xxx + k.yyy);
}

/// Compute a gamma value from a linear value.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// Note: 'rcpX' is '1/x', where the 'x' is what would be used in <c><i>ffxLinearFromGamma</i></c>.
///
/// @param [in] value           The value to convert to gamma space from linear.
/// @param [in] power           The reciprocal of power value used for the gamma curve.
///
/// @returns
/// A value in gamma space.
///
/// @ingroup GPUCore
FfxFloat32 ffxGammaFromLinear(FfxFloat32 value, FfxFloat32 power)
{
    return pow(value, FfxFloat32(power));
}

/// Compute a gamma value from a linear value.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// Note: 'rcpX' is '1/x', where the 'x' is what would be used in <c><i>ffxLinearFromGamma</i></c>.
///
/// @param [in] value           The value to convert to gamma space from linear.
/// @param [in] power           The reciprocal of power value used for the gamma curve.
///
/// @returns
/// A value in gamma space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxGammaFromLinear(FfxFloat32x2 value, FfxFloat32 power)
{
    return pow(value, ffxBroadcast2(power));
}

/// Compute a gamma value from a linear value.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// Note: 'rcpX' is '1/x', where the 'x' is what would be used in <c><i>ffxLinearFromGamma</i></c>.
///
/// @param [in] value           The value to convert to gamma space from linear.
/// @param [in] power           The reciprocal of power value used for the gamma curve.
///
/// @returns
/// A value in gamma space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxGammaFromLinear(FfxFloat32x3 value, FfxFloat32 power)
{
    return pow(value, ffxBroadcast3(power));
}

/// Compute a PQ value from a linear value.
///
/// @param [in] value           The value to convert to PQ from linear.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxPQToLinear(FfxFloat32 value)
{
    FfxFloat32 p = pow(value, FfxFloat32(0.159302));
    return pow((FfxFloat32(0.835938) + FfxFloat32(18.8516) * p) / (FfxFloat32(1.0) + FfxFloat32(18.6875) * p), FfxFloat32(78.8438));
}

/// Compute a PQ value from a linear value.
///
/// @param [in] value           The value to convert to PQ from linear.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxPQToLinear(FfxFloat32x2 value)
{
    FfxFloat32x2 p = pow(value, ffxBroadcast2(0.159302));
    return pow((ffxBroadcast2(0.835938) + ffxBroadcast2(18.8516) * p) / (ffxBroadcast2(1.0) + ffxBroadcast2(18.6875) * p), ffxBroadcast2(78.8438));
}

/// Compute a PQ value from a linear value.
///
/// @param [in] value           The value to convert to PQ from linear.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxPQToLinear(FfxFloat32x3 value)
{
    FfxFloat32x3 p = pow(value, ffxBroadcast3(0.159302));
    return pow((ffxBroadcast3(0.835938) + ffxBroadcast3(18.8516) * p) / (ffxBroadcast3(1.0) + ffxBroadcast3(18.6875) * p), ffxBroadcast3(78.8438));
}

/// Compute a linear value from a SRGB value.
///
/// @param [in] value           The value to convert to linear from SRGB.
///
/// @returns
/// A value in SRGB space.
///
/// @ingroup GPUCore
FfxFloat32 ffxSrgbToLinear(FfxFloat32 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.055, -0.055);
    return clamp(j.x, value * j.y, pow(value, j.z) * k.x + k.y);
}

/// Compute a linear value from a SRGB value.
///
/// @param [in] value           The value to convert to linear from SRGB.
///
/// @returns
/// A value in SRGB space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxSrgbToLinear(FfxFloat32x2 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.055, -0.055);
    return clamp(j.xx, value * j.yy, pow(value, j.zz) * k.xx + k.yy);
}

/// Compute a linear value from a SRGB value.
///
/// @param [in] value           The value to convert to linear from SRGB.
///
/// @returns
/// A value in SRGB space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxSrgbToLinear(FfxFloat32x3 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.055, -0.055);
    return clamp(j.xxx, value * j.yyy, pow(value, j.zzz) * k.xxx + k.yyy);
}

/// Compute a linear value from a REC.709 value.
///
/// @param [in] color           The value to convert to linear from REC.709.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromRec709(FfxFloat32 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.081 / 4.5, 1.0 / 4.5, 1.0 / 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.099, 0.099 / 1.099);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(color - j.x), color * j.y, pow(color * k.x + k.y, j.z));
}

/// Compute a linear value from a REC.709 value.
///
/// @param [in] color           The value to convert to linear from REC.709.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromRec709(FfxFloat32x2 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.081 / 4.5, 1.0 / 4.5, 1.0 / 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.099, 0.099 / 1.099);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(color - j.xx), color * j.yy, pow(color * k.xx + k.yy, j.zz));
}

/// Compute a linear value from a REC.709 value.
///
/// @param [in] color           The value to convert to linear from REC.709.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromRec709(FfxFloat32x3 color)
{
    FfxFloat32x3 j = FfxFloat32x3(0.081 / 4.5, 1.0 / 4.5, 1.0 / 0.45);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.099, 0.099 / 1.099);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(color - j.xxx), color * j.yyy, pow(color * k.xxx + k.yyy, j.zzz));
}

/// Compute a linear value from a value in a gamma space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] color           The value to convert to linear in gamma space.
/// @param [in] power           The power value used for the gamma curve.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromGamma(FfxFloat32 color, FfxFloat32 power)
{
    return pow(color, FfxFloat32(power));
}

/// Compute a linear value from a value in a gamma space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] color           The value to convert to linear in gamma space.
/// @param [in] power           The power value used for the gamma curve.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromGamma(FfxFloat32x2 color, FfxFloat32 power)
{
    return pow(color, ffxBroadcast2(power));
}

/// Compute a linear value from a value in a gamma space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] color           The value to convert to linear in gamma space.
/// @param [in] power           The power value used for the gamma curve.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromGamma(FfxFloat32x3 color, FfxFloat32 power)
{
    return pow(color, ffxBroadcast3(power));
}

/// Compute a linear value from a value in a PQ space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in PQ space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromPQ(FfxFloat32 value)
{
    FfxFloat32 p = pow(value, FfxFloat32(0.0126833));
    return pow(ffxSaturate(p - FfxFloat32(0.835938)) / (FfxFloat32(18.8516) - FfxFloat32(18.6875) * p), FfxFloat32(6.27739));
}

/// Compute a linear value from a value in a PQ space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in PQ space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromPQ(FfxFloat32x2 value)
{
    FfxFloat32x2 p = pow(value, ffxBroadcast2(0.0126833));
    return pow(ffxSaturate(p - ffxBroadcast2(0.835938)) / (ffxBroadcast2(18.8516) - ffxBroadcast2(18.6875) * p), ffxBroadcast2(6.27739));
}

/// Compute a linear value from a value in a PQ space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in PQ space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromPQ(FfxFloat32x3 value)
{
    FfxFloat32x3 p = pow(value, ffxBroadcast3(0.0126833));
    return pow(ffxSaturate(p - ffxBroadcast3(0.835938)) / (ffxBroadcast3(18.8516) - ffxBroadcast3(18.6875) * p), ffxBroadcast3(6.27739));
}

/// Compute a linear value from a value in a SRGB space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in SRGB space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32 ffxLinearFromSrgb(FfxFloat32 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.04045 / 12.92, 1.0 / 12.92, 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.055, 0.055 / 1.055);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(value - j.x), value * j.y, pow(value * k.x + k.y, j.z));
}

/// Compute a linear value from a value in a SRGB space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in SRGB space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x2 ffxLinearFromSrgb(FfxFloat32x2 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.04045 / 12.92, 1.0 / 12.92, 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.055, 0.055 / 1.055);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(value - j.xx), value * j.yy, pow(value * k.xx + k.yy, j.zz));
}

/// Compute a linear value from a value in a SRGB space.
///
/// Typically 2.2 for some PC displays, or 2.4-2.5 for CRTs, or 2.2 FreeSync2 native.
///
/// @param [in] value           The value to convert to linear in SRGB space.
///
/// @returns
/// A value in linear space.
///
/// @ingroup GPUCore
FfxFloat32x3 ffxLinearFromSrgb(FfxFloat32x3 value)
{
    FfxFloat32x3 j = FfxFloat32x3(0.04045 / 12.92, 1.0 / 12.92, 2.4);
    FfxFloat32x2 k = FfxFloat32x2(1.0 / 1.055, 0.055 / 1.055);
    return ffxZeroOneSelect(ffxZeroOneIsSigned(value - j.xxx), value * j.yyy, pow(value * k.xxx + k.yyy, j.zzz));
}

/// A remapping of 64x1 to 8x8 imposing rotated 2x2 pixel quads in quad linear.
///
///  543210
///  ======
///  ..xxx.
///  yy...y
///
/// @param [in] a       The input 1D coordinates to remap.
///
/// @returns
/// The remapped 2D coordinates.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxRemapForQuad(FfxUInt32 a)
{
    return FfxUInt32x2(bitfieldExtract(a, 1u, 3u), bitfieldInsertMask(bitfieldExtract(a, 3u, 3u), a, 1u));
}

/// A helper function performing a remap 64x1 to 8x8 remapping which is necessary for 2D wave reductions.
///
/// The 64-wide lane indices to 8x8 remapping is performed as follows:
///
///     00 01 08 09 10 11 18 19
///     02 03 0a 0b 12 13 1a 1b
///     04 05 0c 0d 14 15 1c 1d
///     06 07 0e 0f 16 17 1e 1f
///     20 21 28 29 30 31 38 39
///     22 23 2a 2b 32 33 3a 3b
///     24 25 2c 2d 34 35 3c 3d
///     26 27 2e 2f 36 37 3e 3f
///
/// @param [in] a       The input 1D coordinate to remap.
///
/// @returns
/// The remapped 2D coordinates.
///
/// @ingroup GPUCore
FfxUInt32x2 ffxRemapForWaveReduction(FfxUInt32 a)
{
    return FfxUInt32x2(bitfieldInsertMask(bitfieldExtract(a, 2u, 3u), a, 1u), bitfieldInsertMask(bitfieldExtract(a, 3u, 3u), bitfieldExtract(a, 1u, 2u), 2u));
}
// --- end ffxm_core_gpu_common.h ---
//----------------------------------------------------------------------------------
// Arm Accuracy Super Resolution (FFXM FSR1) - flattened from
// accuracy-super-resolution-generic-library. MIT license, see FSR_ASR_MIT_LICENSE.
//----------------------------------------------------------------------------------
// --- begin fsr1/ffxm_fsr1.h ---
// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// @defgroup FfxGPUFsr1 FidelityFX FSR1
/// FidelityFX Super Resolution 1 GPU documentation
///
/// @ingroup FfxGPUEffects

/// Setup required constant values for EASU (works on CPU or GPU).
///
/// @param [out] con0
/// @param [out] con1
/// @param [out] con2
/// @param [out] con3
/// @param [in] inputViewportInPixelsX                  The rendered image resolution being upscaled in X dimension.
/// @param [in] inputViewportInPixelsY                  The rendered image resolution being upscaled in Y dimension.
/// @param [in] inputSizeInPixelsX                      The resolution of the resource containing the input image (useful for dynamic resolution) in X dimension.
/// @param [in] inputSizeInPixelsY                      The resolution of the resource containing the input image (useful for dynamic resolution) in Y dimension.
/// @param [in] outputSizeInPixelsX                     The display resolution which the input image gets upscaled to in X dimension.
/// @param [in] outputSizeInPixelsY                     The display resolution which the input image gets upscaled to in Y dimension.
///
/// @ingroup FfxGPUFsr1
FFXM_STATIC void ffxFsrPopulateEasuConstants(
    FFXM_PARAMETER_INOUT FfxUInt32x4 con0,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con1,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con2,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con3,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsY)
{
    // Output integer position to a pixel position in viewport.
    con0[0] = ffxAsUInt32(inputViewportInPixelsX * ffxReciprocal(outputSizeInPixelsX));
    con0[1] = ffxAsUInt32(inputViewportInPixelsY * ffxReciprocal(outputSizeInPixelsY));
    con0[2] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsX * ffxReciprocal(outputSizeInPixelsX) - FfxFloat32(0.5));
    con0[3] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsY * ffxReciprocal(outputSizeInPixelsY) - FfxFloat32(0.5));

    // Viewport pixel position to normalized image space.
    // This is used to get upper-left of 'F' tap.
    con1[0] = ffxAsUInt32(ffxReciprocal(inputSizeInPixelsX));
    con1[1] = ffxAsUInt32(ffxReciprocal(inputSizeInPixelsY));

    // Centers of gather4, first offset from upper-left of 'F'.
    //      +---+---+
    //      |   |   |
    //      +--(0)--+
    //      | b | c |
    //  +---F---+---+---+
    //  | e | f | g | h |
    //  +--(1)--+--(2)--+
    //  | i | j | k | l |
    //  +---+---+---+---+
    //      | n | o |
    //      +--(3)--+
    //      |   |   |
    //      +---+---+
    con1[2] = ffxAsUInt32(FfxFloat32(1.0) * ffxReciprocal(inputSizeInPixelsX));
    con1[3] = ffxAsUInt32(FfxFloat32(-1.0) * ffxReciprocal(inputSizeInPixelsY));

    // These are from (0) instead of 'F'.
    con2[0] = ffxAsUInt32(FfxFloat32(-1.0) * ffxReciprocal(inputSizeInPixelsX));
    con2[1] = ffxAsUInt32(FfxFloat32(2.0) * ffxReciprocal(inputSizeInPixelsY));
    con2[2] = ffxAsUInt32(FfxFloat32(1.0) * ffxReciprocal(inputSizeInPixelsX));
    con2[3] = ffxAsUInt32(FfxFloat32(2.0) * ffxReciprocal(inputSizeInPixelsY));
    con3[0] = ffxAsUInt32(FfxFloat32(0.0) * ffxReciprocal(inputSizeInPixelsX));
    con3[1] = ffxAsUInt32(FfxFloat32(4.0) * ffxReciprocal(inputSizeInPixelsY));
    con3[2] = con3[3] = 0;
}

/// Setup required constant values for EASU (works on CPU or GPU).
///
/// @param [out] con0
/// @param [out] con1
/// @param [out] con2
/// @param [out] con3
/// @param [in] inputViewportInPixelsX              The resolution of the input in the X dimension.
/// @param [in] inputViewportInPixelsY              The resolution of the input in the Y dimension.
/// @param [in] inputSizeInPixelsX                  The input size in pixels in the X dimension.
/// @param [in] inputSizeInPixelsY                  The input size in pixels in the Y dimension.
/// @param [in] outputSizeInPixelsX                 The output size in pixels in the X dimension.
/// @param [in] outputSizeInPixelsY                 The output size in pixels in the Y dimension.
/// @param [in] inputOffsetInPixelsX                The input image offset in the X dimension into the resource containing it (useful for dynamic resolution).
/// @param [in] inputOffsetInPixelsY                The input image offset in the Y dimension into the resource containing it (useful for dynamic resolution).
///
/// @ingroup FfxGPUFsr1
FFXM_STATIC void ffxFsrPopulateEasuConstantsOffset(
    FFXM_PARAMETER_INOUT FfxUInt32x4 con0,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con1,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con2,
    FFXM_PARAMETER_INOUT FfxUInt32x4 con3,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputViewportInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputSizeInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 outputSizeInPixelsY,
    FFXM_PARAMETER_IN FfxFloat32 inputOffsetInPixelsX,
    FFXM_PARAMETER_IN FfxFloat32 inputOffsetInPixelsY)
{
    ffxFsrPopulateEasuConstants(
        con0,
        con1,
        con2,
        con3,
        inputViewportInPixelsX,
        inputViewportInPixelsY,
        inputSizeInPixelsX,
        inputSizeInPixelsY,
        outputSizeInPixelsX,
        outputSizeInPixelsY);

    // override
    con0[2] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsX * ffxReciprocal(outputSizeInPixelsX) - FfxFloat32(0.5) + inputOffsetInPixelsX);
    con0[3] = ffxAsUInt32(FfxFloat32(0.5) * inputViewportInPixelsY * ffxReciprocal(outputSizeInPixelsY) - FfxFloat32(0.5) + inputOffsetInPixelsY);
}

#if defined(FFXM_GPU) && defined(FFXM_FSR_EASU_FLOAT)
// Input callback prototypes, need to be implemented by calling shader
FfxFloat32x4 FsrEasuRF(FfxFloat32x2 p);
FfxFloat32x4 FsrEasuGF(FfxFloat32x2 p);
FfxFloat32x4 FsrEasuBF(FfxFloat32x2 p);

// Filtering for a given tap for the scalar.
void fsrEasuTapFloat(
    FFXM_PARAMETER_INOUT FfxFloat32x3 accumulatedColor,   // Accumulated color, with negative lobe.
    FFXM_PARAMETER_INOUT FfxFloat32 accumulatedWeight,    // Accumulated weight.
    FFXM_PARAMETER_IN FfxFloat32x2 pixelOffset,           // Pixel offset from resolve position to tap.
    FFXM_PARAMETER_IN FfxFloat32x2 gradientDirection,     // Gradient direction.
    FFXM_PARAMETER_IN FfxFloat32x2 length,                // Length.
    FFXM_PARAMETER_IN FfxFloat32 negativeLobeStrength,    // Negative lobe strength.
    FFXM_PARAMETER_IN FfxFloat32 clippingPoint,           // Clipping point.
    FFXM_PARAMETER_IN FfxFloat32x3 color)                 // Tap color.
{
    // Rotate offset by direction.
    FfxFloat32x2 rotatedOffset;
    rotatedOffset.x = (pixelOffset.x * (gradientDirection.x)) + (pixelOffset.y * gradientDirection.y);
    rotatedOffset.y = (pixelOffset.x * (-gradientDirection.y)) + (pixelOffset.y * gradientDirection.x);

    // Anisotropy.
    rotatedOffset *= length;

    // Compute distance^2.
    FfxFloat32 distanceSquared = rotatedOffset.x * rotatedOffset.x + rotatedOffset.y * rotatedOffset.y;

    // Limit to the window as at corner, 2 taps can easily be outside.
    distanceSquared = ffxMin(distanceSquared, clippingPoint);

    // Approximation of lancos2 without sin() or rcp(), or sqrt() to get x.
    //  (25/16 * (2/5 * x^2 - 1)^2 - (25/16 - 1)) * (1/4 * x^2 - 1)^2
    //  |_______________________________________|   |_______________|
    //                   base                             window
    // The general form of the 'base' is,
    //  (a*(b*x^2-1)^2-(a-1))
    // Where 'a=1/(2*b-b^2)' and 'b' moves around the negative lobe.
    FfxFloat32 weightB = FfxFloat32(2.0 / 5.0) * distanceSquared + FfxFloat32(-1.0);
    FfxFloat32 weightA = negativeLobeStrength * distanceSquared + FfxFloat32(-1.0);
    weightB *= weightB;
    weightA *= weightA;
    weightB = FfxFloat32(25.0 / 16.0) * weightB + FfxFloat32(-(25.0 / 16.0 - 1.0));
    FfxFloat32 weight = weightB * weightA;

    // Do weighted average.
    accumulatedColor += color * weight;
    accumulatedWeight += weight;
}

// Accumulate direction and length.
void fsrEasuSetFloat(
    FFXM_PARAMETER_INOUT FfxFloat32x2 direction,
    FFXM_PARAMETER_INOUT FfxFloat32 length,
    FFXM_PARAMETER_IN FfxFloat32x2 pp,
    FFXM_PARAMETER_IN FfxBoolean biS,
    FFXM_PARAMETER_IN FfxBoolean biT,
    FFXM_PARAMETER_IN FfxBoolean biU,
    FFXM_PARAMETER_IN FfxBoolean biV,
    FFXM_PARAMETER_IN FfxFloat32 lA,
    FFXM_PARAMETER_IN FfxFloat32 lB,
    FFXM_PARAMETER_IN FfxFloat32 lC,
    FFXM_PARAMETER_IN FfxFloat32 lD,
    FFXM_PARAMETER_IN FfxFloat32 lE)
{
    // Compute bilinear weight, branches factor out as predicates are compiler time immediates.
    //  s t
    //  u v
    FfxFloat32 weight = FfxFloat32(0.0);
    if (biS)
        weight = (FfxFloat32(1.0) - pp.x) * (FfxFloat32(1.0) - pp.y);
    if (biT)
        weight = pp.x * (FfxFloat32(1.0) - pp.y);
    if (biU)
        weight = (FfxFloat32(1.0) - pp.x) * pp.y;
    if (biV)
        weight = pp.x * pp.y;

    // Direction is the '+' diff.
    //    a
    //  b c d
    //    e
    // Then takes magnitude from abs average of both sides of 'c'.
    // Length converts gradient reversal to 0, smoothly to non-reversal at 1, shaped, then adding horz and vert terms.
    FfxFloat32 dc = lD - lC;
    FfxFloat32 cb = lC - lB;
    FfxFloat32 lengthX = max(abs(dc), abs(cb));
    lengthX = ffxApproximateReciprocal(lengthX);
    FfxFloat32 directionX = lD - lB;
    direction.x += directionX * weight;
    lengthX = ffxSaturate(abs(directionX) * lengthX);
    lengthX *= lengthX;
    length += lengthX * weight;

    // Repeat for the y axis.
    FfxFloat32 ec = lE - lC;
    FfxFloat32 ca = lC - lA;
    FfxFloat32 lengthY = max(abs(ec), abs(ca));
    lengthY = ffxApproximateReciprocal(lengthY);
    FfxFloat32 directionY = lE - lA;
    direction.y += directionY * weight;
    lengthY = ffxSaturate(abs(directionY) * lengthY);
    lengthY *= lengthY;
    length += lengthY * weight;
}

/// Apply edge-aware spatial upsampling using 32bit floating point precision calculations.
///
/// @param [out] outPixel               The computed color of a pixel.
/// @param [in]  integerPosition        Integer pixel position within the output.
/// @param [in]  con0                   The first constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
/// @param [in]  con1                   The second constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
/// @param [in]  con2                   The third constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
/// @param [in]  con3                   The fourth constant value generated by <c><i>ffxFsrPopulateEasuConstants</i></c>.
///
/// @ingroup FSR
void ffxFsrEasuFloat(
    FFXM_PARAMETER_OUT FfxFloat32x3 pix,
    FFXM_PARAMETER_IN FfxUInt32x2 ip,
    FFXM_PARAMETER_IN FfxUInt32x4 con0,
    FFXM_PARAMETER_IN FfxUInt32x4 con1,
    FFXM_PARAMETER_IN FfxUInt32x4 con2,
    FFXM_PARAMETER_IN FfxUInt32x4 con3)
{
    // Get position of 'f'.
    FfxFloat32x2 pp = FfxFloat32x2(ip) * ffxAsFloat(con0.xy) + ffxAsFloat(con0.zw);
    FfxFloat32x2 fp = floor(pp);
    pp -= fp;

    // 12-tap kernel.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    // Gather 4 ordering.
    //  a b
    //  r g
    // For packed FP16, need either {rg} or {ab} so using the following setup for gather in all versions,
    //    a b    <- unused (z)
    //    r g
    //  a b a b
    //  r g r g
    //    a b
    //    r g    <- unused (z)
    // Allowing dead-code removal to remove the 'z's.
    FfxFloat32x2 p0 = fp * ffxAsFloat(con1.xy) + ffxAsFloat(con1.zw);

    // These are from p0 to avoid pulling two constants on pre-Navi hardware.
    FfxFloat32x2 p1    = p0 + ffxAsFloat(con2.xy);
    FfxFloat32x2 p2    = p0 + ffxAsFloat(con2.zw);
    FfxFloat32x2 p3    = p0 + ffxAsFloat(con3.xy);
    FfxFloat32x4 bczzR = FsrEasuRF(p0);
    FfxFloat32x4 bczzG = FsrEasuGF(p0);
    FfxFloat32x4 bczzB = FsrEasuBF(p0);
    FfxFloat32x4 ijfeR = FsrEasuRF(p1);
    FfxFloat32x4 ijfeG = FsrEasuGF(p1);
    FfxFloat32x4 ijfeB = FsrEasuBF(p1);
    FfxFloat32x4 klhgR = FsrEasuRF(p2);
    FfxFloat32x4 klhgG = FsrEasuGF(p2);
    FfxFloat32x4 klhgB = FsrEasuBF(p2);
    FfxFloat32x4 zzonR = FsrEasuRF(p3);
    FfxFloat32x4 zzonG = FsrEasuGF(p3);
    FfxFloat32x4 zzonB = FsrEasuBF(p3);

    // Simplest multi-channel approximate luma possible (luma times 2, in 2 FMA/MAD).
    FfxFloat32x4 bczzL = bczzB * ffxBroadcast4(0.5) + (bczzR * ffxBroadcast4(0.5) + bczzG);
    FfxFloat32x4 ijfeL = ijfeB * ffxBroadcast4(0.5) + (ijfeR * ffxBroadcast4(0.5) + ijfeG);
    FfxFloat32x4 klhgL = klhgB * ffxBroadcast4(0.5) + (klhgR * ffxBroadcast4(0.5) + klhgG);
    FfxFloat32x4 zzonL = zzonB * ffxBroadcast4(0.5) + (zzonR * ffxBroadcast4(0.5) + zzonG);

    // Rename.
    FfxFloat32 bL = bczzL.x;
    FfxFloat32 cL = bczzL.y;
    FfxFloat32 iL = ijfeL.x;
    FfxFloat32 jL = ijfeL.y;
    FfxFloat32 fL = ijfeL.z;
    FfxFloat32 eL = ijfeL.w;
    FfxFloat32 kL = klhgL.x;
    FfxFloat32 lL = klhgL.y;
    FfxFloat32 hL = klhgL.z;
    FfxFloat32 gL = klhgL.w;
    FfxFloat32 oL = zzonL.z;
    FfxFloat32 nL = zzonL.w;

    // Accumulate for bilinear interpolation.
    FfxFloat32x2 dir = ffxBroadcast2(0.0);
    FfxFloat32  len = FfxFloat32(0.0);
    fsrEasuSetFloat(dir, len, pp, FFXM_TRUE,  FFXM_FALSE, FFXM_FALSE, FFXM_FALSE, bL, eL, fL, gL, jL);
    fsrEasuSetFloat(dir, len, pp, FFXM_FALSE, FFXM_TRUE,  FFXM_FALSE, FFXM_FALSE, cL, fL, gL, hL, kL);
    fsrEasuSetFloat(dir, len, pp, FFXM_FALSE, FFXM_FALSE, FFXM_TRUE,  FFXM_FALSE, fL, iL, jL, kL, nL);
    fsrEasuSetFloat(dir, len, pp, FFXM_FALSE, FFXM_FALSE, FFXM_FALSE, FFXM_TRUE,  gL, jL, kL, lL, oL);

    // Normalize with approximation, and cleanup close to zero.
    FfxFloat32x2 dir2 = dir * dir;
    FfxFloat32 dirR = dir2.x + dir2.y;
    FfxBoolean zro  = dirR < FfxFloat32(1.0 / 32768.0);
    dirR = ffxApproximateReciprocalSquareRoot(dirR);
    dirR = zro ? FfxFloat32(1.0) : dirR;
    dir.x = zro ? FfxFloat32(1.0) : dir.x;
    dir *= ffxBroadcast2(dirR);

    // Transform from {0 to 2} to {0 to 1} range, and shape with square.
    len = len * FfxFloat32(0.5);
    len *= len;

    // Stretch kernel {1.0 vert|horz, to sqrt(2.0) on diagonal}.
    FfxFloat32 stretch = (dir.x * dir.x + dir.y * dir.y) * ffxApproximateReciprocal(max(abs(dir.x), abs(dir.y)));

    // Anisotropic length after rotation,
    //  x := 1.0 lerp to 'stretch' on edges
    //  y := 1.0 lerp to 2x on edges
    FfxFloat32x2 len2 = FfxFloat32x2(FfxFloat32(1.0) + (stretch - FfxFloat32(1.0)) * len, FfxFloat32(1.0) + FfxFloat32(-0.5) * len);

    // Based on the amount of 'edge',
    // the window shifts from +/-{sqrt(2.0) to slightly beyond 2.0}.
    FfxFloat32 lob = FfxFloat32(0.5) + FfxFloat32((1.0 / 4.0 - 0.04) - 0.5) * len;

    // Set distance^2 clipping point to the end of the adjustable window.
    FfxFloat32 clp = ffxApproximateReciprocal(lob);

    // Accumulation mixed with min/max of 4 nearest.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    FfxFloat32x3 min4 =
        ffxMin(ffxMin3(FfxFloat32x3(ijfeR.z, ijfeG.z, ijfeB.z), FfxFloat32x3(klhgR.w, klhgG.w, klhgB.w), FfxFloat32x3(ijfeR.y, ijfeG.y, ijfeB.y)),
               FfxFloat32x3(klhgR.x, klhgG.x, klhgB.x));
    FfxFloat32x3 max4 =
        max(ffxMax3(FfxFloat32x3(ijfeR.z, ijfeG.z, ijfeB.z), FfxFloat32x3(klhgR.w, klhgG.w, klhgB.w), FfxFloat32x3(ijfeR.y, ijfeG.y, ijfeB.y)), FfxFloat32x3(klhgR.x, klhgG.x, klhgB.x));

    // Accumulation.
    FfxFloat32x3 aC = ffxBroadcast3(0.0);
    FfxFloat32  aW = FfxFloat32(0.0);
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, -1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(bczzR.x, bczzG.x, bczzB.x));  // b
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, -1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(bczzR.y, bczzG.y, bczzB.y));  // c
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(-1.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.x, ijfeG.x, ijfeB.x));  // i
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.y, ijfeG.y, ijfeB.y));   // j
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.z, ijfeG.z, ijfeB.z));   // f
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(-1.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(ijfeR.w, ijfeG.w, ijfeB.w));  // e
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.x, klhgG.x, klhgB.x));   // k
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(2.0, 1.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.y, klhgG.y, klhgB.y));   // l
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(2.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.z, klhgG.z, klhgB.z));   // h
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, 0.0) - pp, dir, len2, lob, clp, FfxFloat32x3(klhgR.w, klhgG.w, klhgB.w));   // g
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(1.0, 2.0) - pp, dir, len2, lob, clp, FfxFloat32x3(zzonR.z, zzonG.z, zzonB.z));   // o
    fsrEasuTapFloat(aC, aW, FfxFloat32x2(0.0, 2.0) - pp, dir, len2, lob, clp, FfxFloat32x3(zzonR.w, zzonG.w, zzonB.w));   // n

    // Normalize and dering.
    pix = ffxMin(max4, max(min4, aC * ffxBroadcast3(rcp(aW))));
}
#endif // #if defined(FFXM_GPU) && defined(FFXM_FSR_EASU_FLOAT)

#if defined(FFXM_GPU) && FFXM_HALF == 1 && defined(FFXM_FSR_EASU_HALF)
// Input callback prototypes, need to be implemented by calling shader
FfxFloat16x4 FsrEasuRH(FfxFloat32x2 p);
FfxFloat16x4 FsrEasuGH(FfxFloat32x2 p);
FfxFloat16x4 FsrEasuBH(FfxFloat32x2 p);

// This runs 2 taps in parallel.
void FsrEasuTapH(
    FFXM_PARAMETER_INOUT FfxFloat16x2 aCR,
    FFXM_PARAMETER_INOUT FfxFloat16x2 aCG,
    FFXM_PARAMETER_INOUT FfxFloat16x2 aCB,
    FFXM_PARAMETER_INOUT FfxFloat16x2 aW,
    FFXM_PARAMETER_IN FfxFloat16x2 offX,
    FFXM_PARAMETER_IN FfxFloat16x2 offY,
    FFXM_PARAMETER_IN FfxFloat16x2 dir,
    FFXM_PARAMETER_IN FfxFloat16x2 len,
    FFXM_PARAMETER_IN FfxFloat16 lob,
    FFXM_PARAMETER_IN FfxFloat16 clp,
    FFXM_PARAMETER_IN FfxFloat16x2 cR,
    FFXM_PARAMETER_IN FfxFloat16x2 cG,
    FFXM_PARAMETER_IN FfxFloat16x2 cB)
{
    FfxFloat16x2 vX, vY;
    vX = offX * dir.xx + offY * dir.yy;
    vY = offX * (-dir.yy) + offY * dir.xx;
    vX *= len.x;
    vY *= len.y;
    FfxFloat16x2 d2 = vX * vX + vY * vY;
    d2              = min(d2, FFXM_BROADCAST_FLOAT16X2(clp));
    FfxFloat16x2 wB = FFXM_BROADCAST_FLOAT16X2(2.0 / 5.0) * d2 + FFXM_BROADCAST_FLOAT16X2(-1.0);
    FfxFloat16x2 wA = FFXM_BROADCAST_FLOAT16X2(lob) * d2 + FFXM_BROADCAST_FLOAT16X2(-1.0);
    wB *= wB;
    wA *= wA;
    wB             = FFXM_BROADCAST_FLOAT16X2(25.0 / 16.0) * wB + FFXM_BROADCAST_FLOAT16X2(-(25.0 / 16.0 - 1.0));
    FfxFloat16x2 w = wB * wA;
    aCR += cR * w;
    aCG += cG * w;
    aCB += cB * w;
    aW += w;
}

// This runs 2 taps in parallel.
void FsrEasuSetH(
    FFXM_PARAMETER_INOUT FfxFloat16x2 dirPX,
    FFXM_PARAMETER_INOUT FfxFloat16x2  dirPY,
    FFXM_PARAMETER_INOUT FfxFloat16x2 lenP,
    FFXM_PARAMETER_IN FfxFloat16x2 pp,
    FFXM_PARAMETER_IN FfxBoolean biST,
    FFXM_PARAMETER_IN FfxBoolean biUV,
    FFXM_PARAMETER_IN FfxFloat16x2 lA,
    FFXM_PARAMETER_IN FfxFloat16x2 lB,
    FFXM_PARAMETER_IN FfxFloat16x2 lC,
    FFXM_PARAMETER_IN FfxFloat16x2 lD,
    FFXM_PARAMETER_IN FfxFloat16x2 lE)
{
    FfxFloat16x2 w = FFXM_BROADCAST_FLOAT16X2(0.0);

    if (biST)
        w = (FfxFloat16x2(1.0, 0.0) + FfxFloat16x2(-pp.x, pp.x)) * FFXM_BROADCAST_FLOAT16X2(FFXM_BROADCAST_FLOAT16(1.0) - pp.y);

    if (biUV)
        w = (FfxFloat16x2(1.0, 0.0) + FfxFloat16x2(-pp.x, pp.x)) * FFXM_BROADCAST_FLOAT16X2(pp.y);

    // ABS is not free in the packed FP16 path.
    FfxFloat16x2 dc   = lD - lC;
    FfxFloat16x2 cb   = lC - lB;
    FfxFloat16x2 lenX = max(abs(dc), abs(cb));
    lenX              = ffxReciprocalHalf(lenX);

    FfxFloat16x2 dirX = lD - lB;
    dirPX += dirX * w;
    lenX = ffxSaturate(abs(dirX) * lenX);
    lenX *= lenX;
    lenP += lenX * w;
    FfxFloat16x2 ec   = lE - lC;
    FfxFloat16x2 ca   = lC - lA;
    FfxFloat16x2 lenY = max(abs(ec), abs(ca));
    lenY              = ffxReciprocalHalf(lenY);
    FfxFloat16x2 dirY = lE - lA;
    dirPY += dirY * w;
    lenY = ffxSaturate(abs(dirY) * lenY);
    lenY *= lenY;
    lenP += lenY * w;
}

void FsrEasuH(
    FFXM_PARAMETER_OUT FfxFloat16x3 pix,
    FFXM_PARAMETER_IN FfxUInt32x2 ip,
    FFXM_PARAMETER_IN FfxUInt32x4 con0,
    FFXM_PARAMETER_IN FfxUInt32x4 con1,
    FFXM_PARAMETER_IN FfxUInt32x4 con2,
    FFXM_PARAMETER_IN FfxUInt32x4 con3)
{
    FfxFloat32x2 pp = FfxFloat32x2(ip) * ffxAsFloat(con0.xy) + ffxAsFloat(con0.zw);
    FfxFloat32x2 fp = floor(pp);
    pp -= fp;
    FfxFloat16x2 ppp = FfxFloat16x2(pp);

    FfxFloat32x2 p0    = fp * ffxAsFloat(con1.xy) + ffxAsFloat(con1.zw);
    FfxFloat32x2 p1    = p0 + ffxAsFloat(con2.xy);
    FfxFloat32x2 p2    = p0 + ffxAsFloat(con2.zw);
    FfxFloat32x2 p3    = p0 + ffxAsFloat(con3.xy);
    FfxFloat16x4 bczzR = FsrEasuRH(p0);
    FfxFloat16x4 bczzG = FsrEasuGH(p0);
    FfxFloat16x4 bczzB = FsrEasuBH(p0);
    FfxFloat16x4 ijfeR = FsrEasuRH(p1);
    FfxFloat16x4 ijfeG = FsrEasuGH(p1);
    FfxFloat16x4 ijfeB = FsrEasuBH(p1);
    FfxFloat16x4 klhgR = FsrEasuRH(p2);
    FfxFloat16x4 klhgG = FsrEasuGH(p2);
    FfxFloat16x4 klhgB = FsrEasuBH(p2);
    FfxFloat16x4 zzonR = FsrEasuRH(p3);
    FfxFloat16x4 zzonG = FsrEasuGH(p3);
    FfxFloat16x4 zzonB = FsrEasuBH(p3);

    FfxFloat16x4 bczzL = bczzB * FFXM_BROADCAST_FLOAT16X4(0.5) + (bczzR * FFXM_BROADCAST_FLOAT16X4(0.5) + bczzG);
    FfxFloat16x4 ijfeL = ijfeB * FFXM_BROADCAST_FLOAT16X4(0.5) + (ijfeR * FFXM_BROADCAST_FLOAT16X4(0.5) + ijfeG);
    FfxFloat16x4 klhgL = klhgB * FFXM_BROADCAST_FLOAT16X4(0.5) + (klhgR * FFXM_BROADCAST_FLOAT16X4(0.5) + klhgG);
    FfxFloat16x4 zzonL = zzonB * FFXM_BROADCAST_FLOAT16X4(0.5) + (zzonR * FFXM_BROADCAST_FLOAT16X4(0.5) + zzonG);
    FfxFloat16   bL    = bczzL.x;
    FfxFloat16   cL    = bczzL.y;
    FfxFloat16   iL    = ijfeL.x;
    FfxFloat16   jL    = ijfeL.y;
    FfxFloat16   fL    = ijfeL.z;
    FfxFloat16   eL    = ijfeL.w;
    FfxFloat16   kL    = klhgL.x;
    FfxFloat16   lL    = klhgL.y;
    FfxFloat16   hL    = klhgL.z;
    FfxFloat16   gL    = klhgL.w;
    FfxFloat16   oL    = zzonL.z;
    FfxFloat16   nL    = zzonL.w;

    // This part is different, accumulating 2 taps in parallel.
    FfxFloat16x2 dirPX = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 dirPY = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 lenP  = FFXM_BROADCAST_FLOAT16X2(0.0);
    FsrEasuSetH(dirPX,
                dirPY,
                lenP,
                ppp,
                FfxBoolean(true),
                FfxBoolean(false),
                FfxFloat16x2(bL, cL),
                FfxFloat16x2(eL, fL),
                FfxFloat16x2(fL, gL),
                FfxFloat16x2(gL, hL),
                FfxFloat16x2(jL, kL));
    FsrEasuSetH(dirPX,
                dirPY,
                lenP,
                ppp,
                FfxBoolean(false),
                FfxBoolean(true),
                FfxFloat16x2(fL, gL),
                FfxFloat16x2(iL, jL),
                FfxFloat16x2(jL, kL),
                FfxFloat16x2(kL, lL),
                FfxFloat16x2(nL, oL));
    FfxFloat16x2 dir = FfxFloat16x2(dirPX.r + dirPX.g, dirPY.r + dirPY.g);
    FfxFloat16   len = lenP.r + lenP.g;

    FfxFloat16x2 dir2 = dir * dir;
    FfxFloat16   dirR = dir2.x + dir2.y;
    FfxUInt32    zro  = FfxUInt32(dirR < FFXM_BROADCAST_FLOAT16(1.0 / 32768.0));
    dirR              = ffxApproximateReciprocalSquareRootHalf(dirR);
    dirR              = (zro > 0) ? FFXM_BROADCAST_FLOAT16(1.0) : dirR;
    dir.x             = (zro > 0) ? FFXM_BROADCAST_FLOAT16(1.0) : dir.x;
    dir *= FFXM_BROADCAST_FLOAT16X2(dirR);
    len = len * FFXM_BROADCAST_FLOAT16(0.5);
    len *= len;
    FfxFloat16   stretch = (dir.x * dir.x + dir.y * dir.y) * ffxApproximateReciprocalHalf(max(abs(dir.x), abs(dir.y)));
    FfxFloat16x2 len2 =
        FfxFloat16x2(FFXM_BROADCAST_FLOAT16(1.0) + (stretch - FFXM_BROADCAST_FLOAT16(1.0)) * len, FFXM_BROADCAST_FLOAT16(1.0) + FFXM_BROADCAST_FLOAT16(-0.5) * len);
    FfxFloat16 lob = FFXM_BROADCAST_FLOAT16(0.5) + FFXM_BROADCAST_FLOAT16((1.0 / 4.0 - 0.04) - 0.5) * len;
    FfxFloat16 clp = ffxApproximateReciprocalHalf(lob);

    // FP16 is different, using packed trick to do min and max in same operation.
    FfxFloat16x2 bothR =
        max(max(FfxFloat16x2(-ijfeR.z, ijfeR.z), FfxFloat16x2(-klhgR.w, klhgR.w)), max(FfxFloat16x2(-ijfeR.y, ijfeR.y), FfxFloat16x2(-klhgR.x, klhgR.x)));
    FfxFloat16x2 bothG =
        max(max(FfxFloat16x2(-ijfeG.z, ijfeG.z), FfxFloat16x2(-klhgG.w, klhgG.w)), max(FfxFloat16x2(-ijfeG.y, ijfeG.y), FfxFloat16x2(-klhgG.x, klhgG.x)));
    FfxFloat16x2 bothB =
        max(max(FfxFloat16x2(-ijfeB.z, ijfeB.z), FfxFloat16x2(-klhgB.w, klhgB.w)), max(FfxFloat16x2(-ijfeB.y, ijfeB.y), FfxFloat16x2(-klhgB.x, klhgB.x)));

    // This part is different for FP16, working pairs of taps at a time.
    FfxFloat16x2 pR = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 pG = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 pB = FFXM_BROADCAST_FLOAT16X2(0.0);
    FfxFloat16x2 pW = FFXM_BROADCAST_FLOAT16X2(0.0);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(0.0, 1.0) - ppp.xx, FfxFloat16x2(-1.0, -1.0) - ppp.yy, dir, len2, lob, clp, bczzR.xy, bczzG.xy, bczzB.xy);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(-1.0, 0.0) - ppp.xx, FfxFloat16x2(1.0, 1.0) - ppp.yy, dir, len2, lob, clp, ijfeR.xy, ijfeG.xy, ijfeB.xy);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(0.0, -1.0) - ppp.xx, FfxFloat16x2(0.0, 0.0) - ppp.yy, dir, len2, lob, clp, ijfeR.zw, ijfeG.zw, ijfeB.zw);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(1.0, 2.0) - ppp.xx, FfxFloat16x2(1.0, 1.0) - ppp.yy, dir, len2, lob, clp, klhgR.xy, klhgG.xy, klhgB.xy);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(2.0, 1.0) - ppp.xx, FfxFloat16x2(0.0, 0.0) - ppp.yy, dir, len2, lob, clp, klhgR.zw, klhgG.zw, klhgB.zw);
    FsrEasuTapH(pR, pG, pB, pW, FfxFloat16x2(1.0, 0.0) - ppp.xx, FfxFloat16x2(2.0, 2.0) - ppp.yy, dir, len2, lob, clp, zzonR.zw, zzonG.zw, zzonB.zw);
    FfxFloat16x3 aC = FfxFloat16x3(pR.x + pR.y, pG.x + pG.y, pB.x + pB.y);
    FfxFloat16   aW = pW.x + pW.y;

    // Slightly different for FP16 version due to combined min and max.
    pix = min(FfxFloat16x3(bothR.y, bothG.y, bothB.y), max(-FfxFloat16x3(bothR.x, bothG.x, bothB.x), aC * FFXM_BROADCAST_FLOAT16X3(ffxReciprocalHalf(aW))));
}
#endif // #if defined(FFXM_GPU) && defined(FFXM_HALF) && defined(FFXM_FSR_EASU_HALF)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                      FSR - [RCAS] ROBUST CONTRAST ADAPTIVE SHARPENING
//
//------------------------------------------------------------------------------------------------------------------------------
// CAS uses a simplified mechanism to convert local contrast into a variable amount of sharpness.
// RCAS uses a more exact mechanism, solving for the maximum local sharpness possible before clipping.
// RCAS also has a built in process to limit sharpening of what it detects as possible noise.
// RCAS sharper does not support scaling, as it should be applied after EASU scaling.
// Pass EASU output straight into RCAS, no color conversions necessary.
//------------------------------------------------------------------------------------------------------------------------------
// RCAS is based on the following logic.
// RCAS uses a 5 tap filter in a cross pattern (same as CAS),
//    w                n
//  w 1 w  for taps  w m e
//    w                s
// Where 'w' is the negative lobe weight.
//  output = (w*(n+e+w+s)+m)/(4*w+1)
// RCAS solves for 'w' by seeing where the signal might clip out of the {0 to 1} input range,
//  0 == (w*(n+e+w+s)+m)/(4*w+1) -> w = -m/(n+e+w+s)
//  1 == (w*(n+e+w+s)+m)/(4*w+1) -> w = (1-m)/(n+e+w+s-4*1)
// Then chooses the 'w' which results in no clipping, limits 'w', and multiplies by the 'sharp' amount.
// This solution above has issues with MSAA input as the steps along the gradient cause edge detection issues.
// So RCAS uses 4x the maximum and 4x the minimum (depending on equation)in place of the individual taps.
// As well as switching from 'm' to either the minimum or maximum (depending on side), to help in energy conservation.
// This stabilizes RCAS.
// RCAS does a simple highpass which is normalized against the local contrast then shaped,
//       0.25
//  0.25  -1  0.25
//       0.25
// This is used as a noise detection filter, to reduce the effect of RCAS on grain, and focus on real edges.
//
//  GLSL example for the required callbacks :
//
//  FfxFloat16x4 FsrRcasLoadH(FfxInt16x2 p){return FfxFloat16x4(imageLoad(imgSrc,FfxInt32x2(p)));}
//  void FsrRcasInputH(inout FfxFloat16 r,inout FfxFloat16 g,inout FfxFloat16 b)
//  {
//    //do any simple input color conversions here or leave empty if none needed
//  }
//
//  FsrRcasCon need to be called from the CPU or GPU to set up constants.
//  Including a GPU example here, the 'con' value would be stored out to a constant buffer.
//
//  FfxUInt32x4 con;
//  FsrRcasCon(con,
//   0.0); // The scale is {0.0 := maximum sharpness, to N>0, where N is the number of stops (halving) of the reduction of sharpness}.
// ---------------
// RCAS sharpening supports a CAS-like pass-through alpha via,
//  #define FSR_RCAS_PASSTHROUGH_ALPHA 1
// RCAS also supports a define to enable a more expensive path to avoid some sharpening of noise.
// Would suggest it is better to apply film grain after RCAS sharpening (and after scaling) instead of using this define,
//  #define FSR_RCAS_DENOISE 1
//==============================================================================================================================
// This is set at the limit of providing unnatural results for sharpening.
#define FSR_RCAS_LIMIT (0.25-(1.0/16.0))
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                      CONSTANT SETUP
//==============================================================================================================================
// Call to setup required constant values (works on CPU or GPU).
 FFXM_STATIC void FsrRcasCon(FfxUInt32x4 con,
                            // The scale is {0.0 := maximum, to N>0, where N is the number of stops (halving) of the reduction of sharpness}.
                            FfxFloat32 sharpness)
 {
     // Transform from stops to linear value.
     sharpness = exp2(-sharpness);
     FfxFloat32x2 hSharp  = {sharpness, sharpness};
     con[0] = ffxAsUInt32(sharpness);
     con[1] = packHalf2x16(hSharp);
     con[2] = 0;
     con[3] = 0;
 }
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                   NON-PACKED 32-BIT VERSION
//==============================================================================================================================
#if defined(FFXM_GPU)&&defined(FSR_RCAS_F)
 // Input callback prototypes that need to be implemented by calling shader
 FfxFloat32x4 FsrRcasLoadF(FfxInt32x2 p);
 void FsrRcasInputF(inout FfxFloat32 r,inout FfxFloat32 g,inout FfxFloat32 b);
//------------------------------------------------------------------------------------------------------------------------------
 void FsrRcasF(out FfxFloat32 pixR,  // Output values, non-vector so port between RcasFilter() and RcasFilterH() is easy.
               out FfxFloat32 pixG,
               out FfxFloat32 pixB,
#ifdef FSR_RCAS_PASSTHROUGH_ALPHA
               out FfxFloat32 pixA,
#endif
               FfxUInt32x2 ip,  // Integer pixel position in output.
               FfxUInt32x4 con)
 {  // Constant generated by RcasSetup().
     // Algorithm uses minimal 3x3 pixel neighborhood.
     //    b
     //  d e f
     //    h
     FfxInt32x2   sp = FfxInt32x2(ip);
     FfxFloat32x3 b  = FsrRcasLoadF(sp + FfxInt32x2(0, -1)).rgb;
     FfxFloat32x3 d  = FsrRcasLoadF(sp + FfxInt32x2(-1, 0)).rgb;
#ifdef FSR_RCAS_PASSTHROUGH_ALPHA
     FfxFloat32x4 ee = FsrRcasLoadF(sp);
     FfxFloat32x3 e  = ee.rgb;
     pixA            = ee.a;
#else
     FfxFloat32x3 e = FsrRcasLoadF(sp).rgb;
#endif
     FfxFloat32x3 f = FsrRcasLoadF(sp + FfxInt32x2(1, 0)).rgb;
     FfxFloat32x3 h = FsrRcasLoadF(sp + FfxInt32x2(0, 1)).rgb;
     // Rename (32-bit) or regroup (16-bit).
     FfxFloat32 bR = b.r;
     FfxFloat32 bG = b.g;
     FfxFloat32 bB = b.b;
     FfxFloat32 dR = d.r;
     FfxFloat32 dG = d.g;
     FfxFloat32 dB = d.b;
     FfxFloat32 eR = e.r;
     FfxFloat32 eG = e.g;
     FfxFloat32 eB = e.b;
     FfxFloat32 fR = f.r;
     FfxFloat32 fG = f.g;
     FfxFloat32 fB = f.b;
     FfxFloat32 hR = h.r;
     FfxFloat32 hG = h.g;
     FfxFloat32 hB = h.b;
     // Run optional input transform.
     FsrRcasInputF(bR, bG, bB);
     FsrRcasInputF(dR, dG, dB);
     FsrRcasInputF(eR, eG, eB);
     FsrRcasInputF(fR, fG, fB);
     FsrRcasInputF(hR, hG, hB);
     // Luma times 2.
     FfxFloat32 bL = bB * FfxFloat32(0.5) + (bR * FfxFloat32(0.5) + bG);
     FfxFloat32 dL = dB * FfxFloat32(0.5) + (dR * FfxFloat32(0.5) + dG);
     FfxFloat32 eL = eB * FfxFloat32(0.5) + (eR * FfxFloat32(0.5) + eG);
     FfxFloat32 fL = fB * FfxFloat32(0.5) + (fR * FfxFloat32(0.5) + fG);
     FfxFloat32 hL = hB * FfxFloat32(0.5) + (hR * FfxFloat32(0.5) + hG);
     // Noise detection.
     FfxFloat32 nz = FfxFloat32(0.25) * bL + FfxFloat32(0.25) * dL + FfxFloat32(0.25) * fL + FfxFloat32(0.25) * hL - eL;
     nz            = ffxSaturate(abs(nz) * ffxApproximateReciprocalMedium(ffxMax3(ffxMax3(bL, dL, eL), fL, hL) - ffxMin3(ffxMin3(bL, dL, eL), fL, hL)));
     nz            = FfxFloat32(-0.5) * nz + FfxFloat32(1.0);
     // Min and max of ring.
     FfxFloat32 mn4R = ffxMin(ffxMin3(bR, dR, fR), hR);
     FfxFloat32 mn4G = ffxMin(ffxMin3(bG, dG, fG), hG);
     FfxFloat32 mn4B = ffxMin(ffxMin3(bB, dB, fB), hB);
     FfxFloat32 mx4R = max(ffxMax3(bR, dR, fR), hR);
     FfxFloat32 mx4G = max(ffxMax3(bG, dG, fG), hG);
     FfxFloat32 mx4B = max(ffxMax3(bB, dB, fB), hB);
     // Immediate constants for peak range.
     FfxFloat32x2 peakC = FfxFloat32x2(1.0, -1.0 * 4.0);
     // Limiters, these need to be high precision RCPs.
     FfxFloat32 hitMinR = mn4R * rcp(FfxFloat32(4.0) * mx4R);
     FfxFloat32 hitMinG = mn4G * rcp(FfxFloat32(4.0) * mx4G);
     FfxFloat32 hitMinB = mn4B * rcp(FfxFloat32(4.0) * mx4B);
     FfxFloat32 hitMaxR = (peakC.x - mx4R) * rcp(FfxFloat32(4.0) * mn4R + peakC.y);
     FfxFloat32 hitMaxG = (peakC.x - mx4G) * rcp(FfxFloat32(4.0) * mn4G + peakC.y);
     FfxFloat32 hitMaxB = (peakC.x - mx4B) * rcp(FfxFloat32(4.0) * mn4B + peakC.y);
     FfxFloat32 lobeR   = max(-hitMinR, hitMaxR);
     FfxFloat32 lobeG   = max(-hitMinG, hitMaxG);
     FfxFloat32 lobeB   = max(-hitMinB, hitMaxB);
     FfxFloat32 lobe    = max(FfxFloat32(-FSR_RCAS_LIMIT), ffxMin(ffxMax3(lobeR, lobeG, lobeB), FfxFloat32(0.0))) * ffxAsFloat
     (con.x);
 // Apply noise removal.
#ifdef FSR_RCAS_DENOISE
     lobe *= nz;
#endif
     // Resolve, which needs the medium precision rcp approximation to avoid visible tonality changes.
     FfxFloat32 rcpL = ffxApproximateReciprocalMedium(FfxFloat32(4.0) * lobe + FfxFloat32(1.0));
     pixR            = (lobe * bR + lobe * dR + lobe * hR + lobe * fR + eR) * rcpL;
     pixG            = (lobe * bG + lobe * dG + lobe * hG + lobe * fG + eG) * rcpL;
     pixB            = (lobe * bB + lobe * dB + lobe * hB + lobe * fB + eB) * rcpL;
     return;
 }
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                  NON-PACKED 16-BIT VERSION
//==============================================================================================================================
#if defined(FFXM_GPU) && FFXM_HALF == 1 && defined(FSR_RCAS_H)
 // Input callback prototypes that need to be implemented by calling shader
 FfxFloat16x4 FsrRcasLoadH(FfxInt16x2 p);
 void FsrRcasInputH(inout FfxFloat16 r,inout FfxFloat16 g,inout FfxFloat16 b);
//------------------------------------------------------------------------------------------------------------------------------
 void FsrRcasH(
 out FfxFloat16 pixR, // Output values, non-vector so port between RcasFilter() and RcasFilterH() is easy.
 out FfxFloat16 pixG,
 out FfxFloat16 pixB,
 #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
  out FfxFloat16 pixA,
 #endif
 FfxUInt32x2 ip, // Integer pixel position in output.
 FfxUInt32x4 con){ // Constant generated by RcasSetup().
  // Sharpening algorithm uses minimal 3x3 pixel neighborhood.
  //    b
  //  d e f
  //    h
  FfxInt16x2 sp=FfxInt16x2(ip);
  FfxFloat16x3 b=FsrRcasLoadH(sp+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d=FsrRcasLoadH(sp+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee=FsrRcasLoadH(sp);
   FfxFloat16x3 e=ee.rgb;pixA=ee.a;
  #else
   FfxFloat16x3 e=FsrRcasLoadH(sp).rgb;
  #endif
  FfxFloat16x3 f=FsrRcasLoadH(sp+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h=FsrRcasLoadH(sp+FfxInt16x2( 0, 1)).rgb;
  // Rename (32-bit) or regroup (16-bit).
  FfxFloat16 bR=b.r;
  FfxFloat16 bG=b.g;
  FfxFloat16 bB=b.b;
  FfxFloat16 dR=d.r;
  FfxFloat16 dG=d.g;
  FfxFloat16 dB=d.b;
  FfxFloat16 eR=e.r;
  FfxFloat16 eG=e.g;
  FfxFloat16 eB=e.b;
  FfxFloat16 fR=f.r;
  FfxFloat16 fG=f.g;
  FfxFloat16 fB=f.b;
  FfxFloat16 hR=h.r;
  FfxFloat16 hG=h.g;
  FfxFloat16 hB=h.b;
  // Run optional input transform.
  FsrRcasInputH(bR,bG,bB);
  FsrRcasInputH(dR,dG,dB);
  FsrRcasInputH(eR,eG,eB);
  FsrRcasInputH(fR,fG,fB);
  FsrRcasInputH(hR,hG,hB);
  // Luma times 2.
  FfxFloat16 bL=bB*FFXM_BROADCAST_FLOAT16(0.5)+(bR*FFXM_BROADCAST_FLOAT16(0.5)+bG);
  FfxFloat16 dL=dB*FFXM_BROADCAST_FLOAT16(0.5)+(dR*FFXM_BROADCAST_FLOAT16(0.5)+dG);
  FfxFloat16 eL=eB*FFXM_BROADCAST_FLOAT16(0.5)+(eR*FFXM_BROADCAST_FLOAT16(0.5)+eG);
  FfxFloat16 fL=fB*FFXM_BROADCAST_FLOAT16(0.5)+(fR*FFXM_BROADCAST_FLOAT16(0.5)+fG);
  FfxFloat16 hL=hB*FFXM_BROADCAST_FLOAT16(0.5)+(hR*FFXM_BROADCAST_FLOAT16(0.5)+hG);
  // Noise detection.
  FfxFloat16 nz=FFXM_BROADCAST_FLOAT16(0.25)*bL+FFXM_BROADCAST_FLOAT16(0.25)*dL+FFXM_BROADCAST_FLOAT16(0.25)*fL+FFXM_BROADCAST_FLOAT16(0.25)*hL-eL;
  nz=ffxSaturate(abs(nz)*ffxApproximateReciprocalMediumHalf(ffxMax3Half(ffxMax3Half(bL,dL,eL),fL,hL)-ffxMin3Half(ffxMin3Half(bL,dL,eL),fL,hL)));
  nz=FFXM_BROADCAST_FLOAT16(-0.5)*nz+FFXM_BROADCAST_FLOAT16(1.0);
  // Min and max of ring.
  FfxFloat16 mn4R=min(ffxMin3Half(bR,dR,fR),hR);
  FfxFloat16 mn4G=min(ffxMin3Half(bG,dG,fG),hG);
  FfxFloat16 mn4B=min(ffxMin3Half(bB,dB,fB),hB);
  FfxFloat16 mx4R=max(ffxMax3Half(bR,dR,fR),hR);
  FfxFloat16 mx4G=max(ffxMax3Half(bG,dG,fG),hG);
  FfxFloat16 mx4B=max(ffxMax3Half(bB,dB,fB),hB);
  // Immediate constants for peak range.
  FfxFloat16x2 peakC=FfxFloat16x2(1.0,-1.0*4.0);
  // Limiters, these need to be high precision RCPs.
  FfxFloat16 hitMinR=mn4R*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4R);
  FfxFloat16 hitMinG=mn4G*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4G);
  FfxFloat16 hitMinB=mn4B*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mx4B);
  FfxFloat16 hitMaxR=(peakC.x-mx4R)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4R+peakC.y);
  FfxFloat16 hitMaxG=(peakC.x-mx4G)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4G+peakC.y);
  FfxFloat16 hitMaxB=(peakC.x-mx4B)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16(4.0)*mn4B+peakC.y);
  FfxFloat16 lobeR=max(-hitMinR,hitMaxR);
  FfxFloat16 lobeG=max(-hitMinG,hitMaxG);
  FfxFloat16 lobeB=max(-hitMinB,hitMaxB);
  FfxFloat16 lobe=max(FFXM_BROADCAST_FLOAT16(-FSR_RCAS_LIMIT),min(ffxMax3Half(lobeR,lobeG,lobeB),FFXM_BROADCAST_FLOAT16(0.0)))*FFXM_UINT32_TO_FLOAT16X2(con.y).x;
  // Apply noise removal.
  #ifdef FSR_RCAS_DENOISE
   lobe*=nz;
  #endif
  // Resolve, which needs the medium precision rcp approximation to avoid visible tonality changes.
  FfxFloat16 rcpL=ffxApproximateReciprocalMediumHalf(FFXM_BROADCAST_FLOAT16(4.0)*lobe+FFXM_BROADCAST_FLOAT16(1.0));
  pixR=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;
  pixG=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;
  pixB=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//                                                     PACKED 16-BIT VERSION
//==============================================================================================================================
#if defined(FFXM_GPU)&& FFXM_HALF == 1 && defined(FSR_RCAS_HX2)
 // Input callback prototypes that need to be implemented by the calling shader
 FfxFloat16x4 FsrRcasLoadHx2(FfxInt16x2 p);
 void FsrRcasInputHx2(inout FfxFloat16x2 r,inout FfxFloat16x2 g,inout FfxFloat16x2 b);
//------------------------------------------------------------------------------------------------------------------------------
 // Can be used to convert from packed Structures of Arrays to Arrays of Structures for store.
 void FsrRcasDepackHx2(out FfxFloat16x4 pix0,out FfxFloat16x4 pix1,FfxFloat16x2 pixR,FfxFloat16x2 pixG,FfxFloat16x2 pixB){
  #ifdef FFXM_HLSL
   // Invoke a slower path for DX only, since it won't allow uninitialized values.
   pix0.a=pix1.a=0.0;
  #endif
  pix0.rgb=FfxFloat16x3(pixR.x,pixG.x,pixB.x);
  pix1.rgb=FfxFloat16x3(pixR.y,pixG.y,pixB.y);}
//------------------------------------------------------------------------------------------------------------------------------
 void FsrRcasHx2(
 // Output values are for 2 8x8 tiles in a 16x8 region.
 //  pix<R,G,B>.x =  left 8x8 tile
 //  pix<R,G,B>.y = right 8x8 tile
 // This enables later processing to easily be packed as well.
 out FfxFloat16x2 pixR,
 out FfxFloat16x2 pixG,
 out FfxFloat16x2 pixB,
 #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
  out FfxFloat16x2 pixA,
 #endif
 FfxUInt32x2 ip, // Integer pixel position in output.
 FfxUInt32x4 con){ // Constant generated by RcasSetup().
  // No scaling algorithm uses minimal 3x3 pixel neighborhood.
  FfxInt16x2 sp0=FfxInt16x2(ip);
  FfxFloat16x3 b0=FsrRcasLoadHx2(sp0+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d0=FsrRcasLoadHx2(sp0+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee0=FsrRcasLoadHx2(sp0);
   FfxFloat16x3 e0=ee0.rgb;pixA.r=ee0.a;
  #else
   FfxFloat16x3 e0=FsrRcasLoadHx2(sp0).rgb;
  #endif
  FfxFloat16x3 f0=FsrRcasLoadHx2(sp0+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h0=FsrRcasLoadHx2(sp0+FfxInt16x2( 0, 1)).rgb;
  FfxInt16x2 sp1=sp0+FfxInt16x2(8,0);
  FfxFloat16x3 b1=FsrRcasLoadHx2(sp1+FfxInt16x2( 0,-1)).rgb;
  FfxFloat16x3 d1=FsrRcasLoadHx2(sp1+FfxInt16x2(-1, 0)).rgb;
  #ifdef FSR_RCAS_PASSTHROUGH_ALPHA
   FfxFloat16x4 ee1=FsrRcasLoadHx2(sp1);
   FfxFloat16x3 e1=ee1.rgb;pixA.g=ee1.a;
  #else
   FfxFloat16x3 e1=FsrRcasLoadHx2(sp1).rgb;
  #endif
  FfxFloat16x3 f1=FsrRcasLoadHx2(sp1+FfxInt16x2( 1, 0)).rgb;
  FfxFloat16x3 h1=FsrRcasLoadHx2(sp1+FfxInt16x2( 0, 1)).rgb;
  // Arrays of Structures to Structures of Arrays conversion.
  FfxFloat16x2 bR=FfxFloat16x2(b0.r,b1.r);
  FfxFloat16x2 bG=FfxFloat16x2(b0.g,b1.g);
  FfxFloat16x2 bB=FfxFloat16x2(b0.b,b1.b);
  FfxFloat16x2 dR=FfxFloat16x2(d0.r,d1.r);
  FfxFloat16x2 dG=FfxFloat16x2(d0.g,d1.g);
  FfxFloat16x2 dB=FfxFloat16x2(d0.b,d1.b);
  FfxFloat16x2 eR=FfxFloat16x2(e0.r,e1.r);
  FfxFloat16x2 eG=FfxFloat16x2(e0.g,e1.g);
  FfxFloat16x2 eB=FfxFloat16x2(e0.b,e1.b);
  FfxFloat16x2 fR=FfxFloat16x2(f0.r,f1.r);
  FfxFloat16x2 fG=FfxFloat16x2(f0.g,f1.g);
  FfxFloat16x2 fB=FfxFloat16x2(f0.b,f1.b);
  FfxFloat16x2 hR=FfxFloat16x2(h0.r,h1.r);
  FfxFloat16x2 hG=FfxFloat16x2(h0.g,h1.g);
  FfxFloat16x2 hB=FfxFloat16x2(h0.b,h1.b);
  // Run optional input transform.
  FsrRcasInputHx2(bR,bG,bB);
  FsrRcasInputHx2(dR,dG,dB);
  FsrRcasInputHx2(eR,eG,eB);
  FsrRcasInputHx2(fR,fG,fB);
  FsrRcasInputHx2(hR,hG,hB);
  // Luma times 2.
  FfxFloat16x2 bL=bB*FFXM_BROADCAST_FLOAT16X2(0.5)+(bR*FFXM_BROADCAST_FLOAT16X2(0.5)+bG);
  FfxFloat16x2 dL=dB*FFXM_BROADCAST_FLOAT16X2(0.5)+(dR*FFXM_BROADCAST_FLOAT16X2(0.5)+dG);
  FfxFloat16x2 eL=eB*FFXM_BROADCAST_FLOAT16X2(0.5)+(eR*FFXM_BROADCAST_FLOAT16X2(0.5)+eG);
  FfxFloat16x2 fL=fB*FFXM_BROADCAST_FLOAT16X2(0.5)+(fR*FFXM_BROADCAST_FLOAT16X2(0.5)+fG);
  FfxFloat16x2 hL=hB*FFXM_BROADCAST_FLOAT16X2(0.5)+(hR*FFXM_BROADCAST_FLOAT16X2(0.5)+hG);
  // Noise detection.
  FfxFloat16x2 nz=FFXM_BROADCAST_FLOAT16X2(0.25)*bL+FFXM_BROADCAST_FLOAT16X2(0.25)*dL+FFXM_BROADCAST_FLOAT16X2(0.25)*fL+FFXM_BROADCAST_FLOAT16X2(0.25)*hL-eL;
  nz=ffxSaturate(abs(nz)*ffxApproximateReciprocalMediumHalf(ffxMax3Half(ffxMax3Half(bL,dL,eL),fL,hL)-ffxMin3Half(ffxMin3Half(bL,dL,eL),fL,hL)));
  nz=FFXM_BROADCAST_FLOAT16X2(-0.5)*nz+FFXM_BROADCAST_FLOAT16X2(1.0);
  // Min and max of ring.
  FfxFloat16x2 mn4R=min(ffxMin3Half(bR,dR,fR),hR);
  FfxFloat16x2 mn4G=min(ffxMin3Half(bG,dG,fG),hG);
  FfxFloat16x2 mn4B=min(ffxMin3Half(bB,dB,fB),hB);
  FfxFloat16x2 mx4R=max(ffxMax3Half(bR,dR,fR),hR);
  FfxFloat16x2 mx4G=max(ffxMax3Half(bG,dG,fG),hG);
  FfxFloat16x2 mx4B=max(ffxMax3Half(bB,dB,fB),hB);
  // Immediate constants for peak range.
  FfxFloat16x2 peakC=FfxFloat16x2(1.0,-1.0*4.0);
  // Limiters, these need to be high precision RCPs.
  FfxFloat16x2 hitMinR=mn4R*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4R);
  FfxFloat16x2 hitMinG=mn4G*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4G);
  FfxFloat16x2 hitMinB=mn4B*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mx4B);
  FfxFloat16x2 hitMaxR=(peakC.x-mx4R)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4R+peakC.y);
  FfxFloat16x2 hitMaxG=(peakC.x-mx4G)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4G+peakC.y);
  FfxFloat16x2 hitMaxB=(peakC.x-mx4B)*ffxReciprocalHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*mn4B+peakC.y);
  FfxFloat16x2 lobeR=max(-hitMinR,hitMaxR);
  FfxFloat16x2 lobeG=max(-hitMinG,hitMaxG);
  FfxFloat16x2 lobeB=max(-hitMinB,hitMaxB);
  FfxFloat16x2 lobe=max(FFXM_BROADCAST_FLOAT16X2(-FSR_RCAS_LIMIT),min(ffxMax3Half(lobeR,lobeG,lobeB),FFXM_BROADCAST_FLOAT16X2(0.0)))*FFXM_BROADCAST_FLOAT16X2(FFXM_UINT32_TO_FLOAT16X2(con.y).x);
  // Apply noise removal.
  #ifdef FSR_RCAS_DENOISE
   lobe*=nz;
  #endif
  // Resolve, which needs the medium precision rcp approximation to avoid visible tonality changes.
  FfxFloat16x2 rcpL=ffxApproximateReciprocalMediumHalf(FFXM_BROADCAST_FLOAT16X2(4.0)*lobe+FFXM_BROADCAST_FLOAT16X2(1.0));
  pixR=(lobe*bR+lobe*dR+lobe*hR+lobe*fR+eR)*rcpL;
  pixG=(lobe*bG+lobe*dG+lobe*hG+lobe*fG+eG)*rcpL;
  pixB=(lobe*bB+lobe*dB+lobe*hB+lobe*fB+eB)*rcpL;}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                          FSR - [LFGA] LINEAR FILM GRAIN APPLICATOR
//
//------------------------------------------------------------------------------------------------------------------------------
// Adding output-resolution film grain after scaling is a good way to mask both rendering and scaling artifacts.
// Suggest using tiled blue noise as film grain input, with peak noise frequency set for a specific look and feel.
// The 'Lfga*()' functions provide a convenient way to introduce grain.
// These functions limit grain based on distance to signal limits.
// This is done so that the grain is temporally energy preserving, and thus won't modify image tonality.
// Grain application should be done in a linear colorspace.
// The grain should be temporally changing, but have a temporal sum per pixel that adds to zero (non-biased).
//------------------------------------------------------------------------------------------------------------------------------
// Usage,
//   FsrLfga*(
//    color, // In/out linear colorspace color {0 to 1} ranged.
//    grain, // Per pixel grain texture value {-0.5 to 0.5} ranged, input is 3-channel to support colored grain.
//    amount); // Amount of grain (0 to 1} ranged.
//------------------------------------------------------------------------------------------------------------------------------
// Example if grain texture is monochrome: 'FsrLfgaF(color,ffxBroadcast3(grain),amount)'
//==============================================================================================================================
#if defined(FFXM_GPU)
 // Maximum grain is the minimum distance to the signal limit.
 void FsrLfgaF(inout FfxFloat32x3 c, FfxFloat32x3 t, FfxFloat32 a)
 {
     c += (t * ffxBroadcast3(a)) * ffxMin(ffxBroadcast3(1.0) - c, c);
 }
#endif
//==============================================================================================================================
#if defined(FFXM_GPU)&& FFXM_HALF == 1
 // Half precision version (slower).
 void FsrLfgaH(inout FfxFloat16x3 c, FfxFloat16x3 t, FfxFloat16 a)
 {
     c += (t * FFXM_BROADCAST_FLOAT16X3(a)) * min(FFXM_BROADCAST_FLOAT16X3(1.0) - c, c);
 }
 //------------------------------------------------------------------------------------------------------------------------------
 // Packed half precision version (faster).
 void FsrLfgaHx2(inout FfxFloat16x2 cR,inout FfxFloat16x2 cG,inout FfxFloat16x2 cB,FfxFloat16x2 tR,FfxFloat16x2 tG,FfxFloat16x2 tB,FfxFloat16 a){
  cR+=(tR*FFXM_BROADCAST_FLOAT16X2(a))*min(FFXM_BROADCAST_FLOAT16X2(1.0)-cR,cR);cG+=(tG*FFXM_BROADCAST_FLOAT16X2(a))*min(FFXM_BROADCAST_FLOAT16X2(1.0)-cG,cG);cB+=(tB*FFXM_BROADCAST_FLOAT16X2(a))*min(FFXM_BROADCAST_FLOAT16X2(1.0)-cB,cB);}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                          FSR - [SRTM] SIMPLE REVERSIBLE TONE-MAPPER
//
//------------------------------------------------------------------------------------------------------------------------------
// This provides a way to take linear HDR color {0 to FP16_MAX} and convert it into a temporary {0 to 1} ranged post-tonemapped linear.
// The tonemapper preserves RGB ratio, which helps maintain HDR color bleed during filtering.
//------------------------------------------------------------------------------------------------------------------------------
// Reversible tonemapper usage,
//  FsrSrtm*(color); // {0 to FP16_MAX} converted to {0 to 1}.
//  FsrSrtmInv*(color); // {0 to 1} converted into {0 to 32768, output peak safe for FP16}.
//==============================================================================================================================
#if defined(FFXM_GPU)
 void FsrSrtmF(inout FfxFloat32x3 c)
 {
     c *= ffxBroadcast3(rcp(ffxMax3(c.r, c.g, c.b) + FfxFloat32(1.0)));
 }
 // The extra max solves the c=1.0 case (which is a /0).
 void FsrSrtmInvF(inout FfxFloat32x3 c){c*=ffxBroadcast3(rcp(max(FfxFloat32(1.0/32768.0),FfxFloat32(1.0)-ffxMax3(c.r,c.g,c.b))));}
#endif
//==============================================================================================================================
#if defined(FFXM_GPU )&& FFXM_HALF == 1
 void FsrSrtmH(inout FfxFloat16x3 c)
 {
     c *= FFXM_BROADCAST_FLOAT16X3(ffxReciprocalHalf(ffxMax3Half(c.r, c.g, c.b) + FFXM_BROADCAST_FLOAT16(1.0)));
 }
 void FsrSrtmInvH(inout FfxFloat16x3 c)
 {
     c *= FFXM_BROADCAST_FLOAT16X3(ffxReciprocalHalf(max(FFXM_BROADCAST_FLOAT16(1.0 / 32768.0), FFXM_BROADCAST_FLOAT16(1.0) - ffxMax3Half(c.r, c.g, c.b))));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrSrtmHx2(inout FfxFloat16x2 cR, inout FfxFloat16x2 cG, inout FfxFloat16x2 cB)
 {
     FfxFloat16x2 rcp = ffxReciprocalHalf(ffxMax3Half(cR, cG, cB) + FFXM_BROADCAST_FLOAT16X2(1.0));
     cR *= rcp;
     cG *= rcp;
     cB *= rcp;
 }
 void FsrSrtmInvHx2(inout FfxFloat16x2 cR,inout FfxFloat16x2 cG,inout FfxFloat16x2 cB)
 {
     FfxFloat16x2 rcp=ffxReciprocalHalf(max(FFXM_BROADCAST_FLOAT16X2(1.0/32768.0),FFXM_BROADCAST_FLOAT16X2(1.0)-ffxMax3Half(cR,cG,cB)));
     cR*=rcp;
     cG*=rcp;
     cB*=rcp;
 }
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//_____________________________________________________________/\_______________________________________________________________
//==============================================================================================================================
//
//                                       FSR - [TEPD] TEMPORAL ENERGY PRESERVING DITHER
//
//------------------------------------------------------------------------------------------------------------------------------
// Temporally energy preserving dithered {0 to 1} linear to gamma 2.0 conversion.
// Gamma 2.0 is used so that the conversion back to linear is just to square the color.
// The conversion comes in 8-bit and 10-bit modes, designed for output to 8-bit UNORM or 10:10:10:2 respectively.
// Given good non-biased temporal blue noise as dither input,
// the output dither will temporally conserve energy.
// This is done by choosing the linear nearest step point instead of perceptual nearest.
// See code below for details.
//------------------------------------------------------------------------------------------------------------------------------
// DX SPEC RULES FOR FLOAT->UNORM 8-BIT CONVERSION
// ===============================================
// - Output is 'FfxUInt32(floor(saturate(n)*255.0+0.5))'.
// - Thus rounding is to nearest.
// - NaN gets converted to zero.
// - INF is clamped to {0.0 to 1.0}.
//==============================================================================================================================
#if defined(FFXM_GPU)
 // Hand tuned integer position to dither value, with more values than simple checkerboard.
 // Only 32-bit has enough precision for this compddation.
 // Output is {0 to <1}.
 FfxFloat32 FsrTepdDitF(FfxUInt32x2 p, FfxUInt32 f)
 {
     FfxFloat32 x = FfxFloat32(p.x + f);
     FfxFloat32 y = FfxFloat32(p.y);
     // The 1.61803 golden ratio.
     FfxFloat32 a = FfxFloat32((1.0 + ffxSqrt(5.0f)) / 2.0);
     // Number designed to provide a good visual pattern.
     FfxFloat32 b = FfxFloat32(1.0 / 3.69);
     x            = x * a + (y * b);
     return ffxFract(x);
 }
  //------------------------------------------------------------------------------------------------------------------------------
 // This version is 8-bit gamma 2.0.
 // The 'c' input is {0 to 1}.
 // Output is {0 to 1} ready for image store.
 void FsrTepdC8F(inout FfxFloat32x3 c, FfxFloat32 dit)
 {
     FfxFloat32x3 n = ffxSqrt(c);
     n              = floor(n * ffxBroadcast3(255.0)) * ffxBroadcast3(1.0 / 255.0);
     FfxFloat32x3 a = n * n;
     FfxFloat32x3 b = n + ffxBroadcast3(1.0 / 255.0);
     b              = b * b;
     // Ratio of 'a' to 'b' required to produce 'c'.
     // ffxApproximateReciprocal() won't work here (at least for very high dynamic ranges).
     // ffxApproximateReciprocalMedium() is an IADD,FMA,MUL.
     FfxFloat32x3 r = (c - b) * ffxApproximateReciprocalMedium(a - b);
     // Use the ratio as a cutoff to choose 'a' or 'b'.
     // ffxIsGreaterThanZero() is a MUL.
     c = ffxSaturate(n + ffxIsGreaterThanZero(ffxBroadcast3(dit) - r) * ffxBroadcast3(1.0 / 255.0));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 // This version is 10-bit gamma 2.0.
 // The 'c' input is {0 to 1}.
 // Output is {0 to 1} ready for image store.
 void FsrTepdC10F(inout FfxFloat32x3 c, FfxFloat32 dit)
 {
     FfxFloat32x3 n = ffxSqrt(c);
     n              = floor(n * ffxBroadcast3(1023.0)) * ffxBroadcast3(1.0 / 1023.0);
     FfxFloat32x3 a = n * n;
     FfxFloat32x3 b = n + ffxBroadcast3(1.0 / 1023.0);
     b              = b * b;
     FfxFloat32x3 r = (c - b) * ffxApproximateReciprocalMedium(a - b);
     c              = ffxSaturate(n + ffxIsGreaterThanZero(ffxBroadcast3(dit) - r) * ffxBroadcast3(1.0 / 1023.0));
 }
#endif
//==============================================================================================================================
#if defined(FFXM_GPU)&& FFXM_HALF == 1
 FfxFloat16 FsrTepdDitH(FfxUInt32x2 p, FfxUInt32 f)
 {
     FfxFloat32 x = FfxFloat32(p.x + f);
     FfxFloat32 y = FfxFloat32(p.y);
     FfxFloat32 a = FfxFloat32((1.0 + ffxSqrt(5.0f)) / 2.0);
     FfxFloat32 b = FfxFloat32(1.0 / 3.69);
     x       = x * a + (y * b);
     return FfxFloat16(ffxFract(x));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC8H(inout FfxFloat16x3 c, FfxFloat16 dit)
 {
     FfxFloat16x3 n = sqrt(c);
     n     = floor(n * FFXM_BROADCAST_FLOAT16X3(255.0)) * FFXM_BROADCAST_FLOAT16X3(1.0 / 255.0);
     FfxFloat16x3 a = n * n;
     FfxFloat16x3 b = n + FFXM_BROADCAST_FLOAT16X3(1.0 / 255.0);
     b     = b * b;
     FfxFloat16x3 r = (c - b) * ffxApproximateReciprocalMediumHalf(a - b);
     c     = ffxSaturate(n + ffxIsGreaterThanZeroHalf(FFXM_BROADCAST_FLOAT16X3(dit) - r) * FFXM_BROADCAST_FLOAT16X3(1.0 / 255.0));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC10H(inout FfxFloat16x3 c, FfxFloat16 dit)
 {
     FfxFloat16x3 n = sqrt(c);
     n     = floor(n * FFXM_BROADCAST_FLOAT16X3(1023.0)) * FFXM_BROADCAST_FLOAT16X3(1.0 / 1023.0);
     FfxFloat16x3 a = n * n;
     FfxFloat16x3 b = n + FFXM_BROADCAST_FLOAT16X3(1.0 / 1023.0);
     b     = b * b;
     FfxFloat16x3 r = (c - b) * ffxApproximateReciprocalMediumHalf(a - b);
     c     = ffxSaturate(n + ffxIsGreaterThanZeroHalf(FFXM_BROADCAST_FLOAT16X3(dit) - r) * FFXM_BROADCAST_FLOAT16X3(1.0 / 1023.0));
 }
 //==============================================================================================================================
 // This computes dither for positions 'p' and 'p+{8,0}'.
 FfxFloat16x2 FsrTepdDitHx2(FfxUInt32x2 p, FfxUInt32 f)
 {
     FfxFloat32x2 x;
     x.x     = FfxFloat32(p.x + f);
     x.y     = x.x + FfxFloat32(8.0);
     FfxFloat32 y = FfxFloat32(p.y);
     FfxFloat32 a = FfxFloat32((1.0 + ffxSqrt(5.0f)) / 2.0);
     FfxFloat32 b = FfxFloat32(1.0 / 3.69);
     x       = x * ffxBroadcast2(a) + ffxBroadcast2(y * b);
     return FfxFloat16x2(ffxFract(x));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC8Hx2(inout FfxFloat16x2 cR, inout FfxFloat16x2 cG, inout FfxFloat16x2 cB, FfxFloat16x2 dit)
 {
     FfxFloat16x2 nR = sqrt(cR);
     FfxFloat16x2 nG = sqrt(cG);
     FfxFloat16x2 nB = sqrt(cB);
     nR     = floor(nR * FFXM_BROADCAST_FLOAT16X2(255.0)) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     nG     = floor(nG * FFXM_BROADCAST_FLOAT16X2(255.0)) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     nB     = floor(nB * FFXM_BROADCAST_FLOAT16X2(255.0)) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     FfxFloat16x2 aR = nR * nR;
     FfxFloat16x2 aG = nG * nG;
     FfxFloat16x2 aB = nB * nB;
     FfxFloat16x2 bR = nR + FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     bR     = bR * bR;
     FfxFloat16x2 bG = nG + FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     bG     = bG * bG;
     FfxFloat16x2 bB = nB + FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0);
     bB     = bB * bB;
     FfxFloat16x2 rR = (cR - bR) * ffxApproximateReciprocalMediumHalf(aR - bR);
     FfxFloat16x2 rG = (cG - bG) * ffxApproximateReciprocalMediumHalf(aG - bG);
     FfxFloat16x2 rB = (cB - bB) * ffxApproximateReciprocalMediumHalf(aB - bB);
     cR     = ffxSaturate(nR + ffxIsGreaterThanZeroHalf(dit - rR) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0));
     cG     = ffxSaturate(nG + ffxIsGreaterThanZeroHalf(dit - rG) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0));
     cB     = ffxSaturate(nB + ffxIsGreaterThanZeroHalf(dit - rB) * FFXM_BROADCAST_FLOAT16X2(1.0 / 255.0));
 }
 //------------------------------------------------------------------------------------------------------------------------------
 void FsrTepdC10Hx2(inout FfxFloat16x2 cR,inout FfxFloat16x2 cG,inout FfxFloat16x2 cB,FfxFloat16x2 dit){
  FfxFloat16x2 nR=sqrt(cR);
  FfxFloat16x2 nG=sqrt(cG);
  FfxFloat16x2 nB=sqrt(cB);
  nR=floor(nR*FFXM_BROADCAST_FLOAT16X2(1023.0))*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);
  nG=floor(nG*FFXM_BROADCAST_FLOAT16X2(1023.0))*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);
  nB=floor(nB*FFXM_BROADCAST_FLOAT16X2(1023.0))*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);
  FfxFloat16x2 aR=nR*nR;
  FfxFloat16x2 aG=nG*nG;
  FfxFloat16x2 aB=nB*nB;
  FfxFloat16x2 bR=nR+FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);bR=bR*bR;
  FfxFloat16x2 bG=nG+FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);bG=bG*bG;
  FfxFloat16x2 bB=nB+FFXM_BROADCAST_FLOAT16X2(1.0/1023.0);bB=bB*bB;
  FfxFloat16x2 rR=(cR-bR)*ffxApproximateReciprocalMediumHalf(aR-bR);
  FfxFloat16x2 rG=(cG-bG)*ffxApproximateReciprocalMediumHalf(aG-bG);
  FfxFloat16x2 rB=(cB-bB)*ffxApproximateReciprocalMediumHalf(aB-bB);
  cR=ffxSaturate(nR+ffxIsGreaterThanZeroHalf(dit-rR)*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0));
  cG=ffxSaturate(nG+ffxIsGreaterThanZeroHalf(dit-rG)*FFXM_BROADCAST_FLOAT16X2(1.0/1023.0));
  cB                                                       = ffxSaturate(nB + ffxIsGreaterThanZeroHalf(dit - rB) * FFXM_BROADCAST_FLOAT16X2(1.0 / 1023.0));
}
#endif
// --- end fsr1/ffxm_fsr1.h ---
//----------------------------------------------------------------------------------
// MobileGlues RCAS pass glue: reads the EASU intermediate (same size as the
// output) and writes the sharpened image to the surface's default framebuffer.
//----------------------------------------------------------------------------------
in vec2 vTexCoord;
out vec4 oFragColor;

uniform sampler2D uInputTex;
uniform FfxUInt32x4 uRcasCon;

FfxFloat32x4 FsrRcasLoadF(FfxInt32x2 p) { return texelFetch(uInputTex, p, 0); }
void FsrRcasInputF(inout FfxFloat32 r, inout FfxFloat32 g, inout FfxFloat32 b) {}

void main() {
    FfxFloat32 pixR;
    FfxFloat32 pixG;
    FfxFloat32 pixB;
    FsrRcasF(pixR, pixG, pixB, FfxUInt32x2(gl_FragCoord.xy), uRcasCon);
    oFragColor = FfxFloat32x4(pixR, pixG, pixB, FfxFloat32(1.0));
}
)fsr_glsl";
