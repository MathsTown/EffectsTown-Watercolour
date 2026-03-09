/********************************************************************************************************

Authors:		(c) 2023 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************
* 
* Sets up environmental variables
* 
* 
*********************************************************************************************************/
#pragma once

// Canonical x64 check used by all headers.
#if defined(_M_X64) || defined(__x86_64)
	#define MT_SIMD_ARCH_X64 1
#else
	#define MT_SIMD_ARCH_X64 0
#endif

// Only MSVC exposes vector lane member fields (m128_f32, m256i_i32, ...).
#if defined(_MSC_VER) && !defined(__clang__)
	#define MT_SIMD_HAS_MSVC_VECTOR_MEMBERS 1
#else
	#define MT_SIMD_HAS_MSVC_VECTOR_MEMBERS 0
#endif

// GCC/Clang need software shims for non-standard SIMD helper intrinsics.
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
	#define MT_SIMD_USE_PORTABLE_X86_SHIMS 0
#else
	#define MT_SIMD_USE_PORTABLE_X86_SHIMS 1
#endif

// Math backend selection for transcendental SIMD functions.
// MT_USE_SVML is the only backend switch; all other builds use native code paths.
#ifndef MT_USE_SVML
	#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		#define MT_USE_SVML 1
	#else
		#define MT_USE_SVML 0
	#endif
#endif

#if MT_USE_SVML && !MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
	#error "MT_USE_SVML is only supported with MSVC."
#endif

namespace mt::environment {




//Setup some compiler constants
#if defined(_MSC_VER)
	constexpr static bool is_visual_studio = true;
#else
	constexpr bool is_visual_studio = false;
#endif

#if defined(__clang__)
	constexpr bool is_clang = true;
#else
	constexpr static bool is_clang = false;
#endif

#if defined(__GNUC__)
	constexpr bool is_gcc = true;
#else
	constexpr static bool is_gcc = false;
#endif


#if defined(__EMSCRIPTEN__)
	constexpr static bool is_emscripten = true;
#else
	constexpr static bool is_emscripten = false;
#endif
	// Backward-compatible typo alias.
	constexpr static bool is_emsctipten = is_emscripten;
	constexpr static bool emsctipten = is_emscripten;


//Check the arhitecture
#if MT_SIMD_ARCH_X64
	constexpr static bool is_x64 = true;
#else
	constexpr static bool is_x64 = false;
#endif


//MSVC++ does not define SSE macros 
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS && MT_SIMD_ARCH_X64
	#if !defined(__SSE__)
		#define __SSE__ 1
	#endif
	#if !defined(__SSE2__)
		#define __SSE2__ 1
	#endif

	// I don't know a way to detect SSE3 and SSE4 and compile time, but it exists if AVX does
	#if defined(__AVX__)
		#define __SSE3__ 1
		#define __SSSE3__ 1
		#define __SSE4_1__ 1
		#define __SSE4_2__ 1
	#endif
#endif

//FMA is not defined by visual studio, so we need AVX2 support to compile with it.
#if !defined(__FMA__) && defined(__AVX2__)
	#define __FMA__ 1
#endif


//Setup some constexpr variables that we can use to provide some consistancy with different compilers.

#if defined(__SSE__) || defined(__AVX__)
	constexpr static bool compiler_has_sse = true;
#else
	constexpr static bool compiler_has_sse = false;
#endif

#if defined(__SSE2__) || defined(__AVX__)
	constexpr static bool compiler_has_sse2 = true;
#else
	constexpr static bool compiler_has_sse2 = false;
#endif

#if defined(__SSE3__) || defined(__AVX__)
	constexpr static bool compiler_has_sse3 = true;
#else
	constexpr static bool compiler_has_sse3 = false;
#endif

#if defined(__SSSE3__) || defined(__AVX__)
	constexpr static bool compiler_has_ssse3 = true;
#else
	constexpr static bool compiler_has_ssse3 = false;
#endif

#if defined(__SSE4_1__) || defined(__AVX__)
	constexpr static bool compiler_has_sse4_1 = true;
#else
	constexpr static bool compiler_has_sse4_1 = false;
#endif

#if defined(__SSE4_2__) || defined(__AVX__)
	constexpr static bool compiler_has_sse4_2 = true;
#else
	constexpr static bool compiler_has_sse4_2 = false;
#endif

#if defined(__AVX__)
	constexpr static bool compiler_has_avx = true;
#else
	constexpr static bool compiler_has_avx = false;
#endif

#if defined(__AVX2__)
	constexpr static bool compiler_has_avx2 = true;
#else
	constexpr static bool compiler_has_avx2 = false;
#endif

#if defined(__FMA__)
	constexpr static bool compiler_has_fma = true;
#else
	constexpr static bool compiler_has_fma = false;
#endif

#if defined(__F16C__)
	constexpr static bool compiler_has_f16c = true;
#else
	constexpr static bool compiler_has_f16c = false;
#endif

	// Selected math backend.
	constexpr static bool use_svml = (MT_USE_SVML != 0);


// Compile-time gating for full SIMD type declarations.
// MSVC is intentionally permissive to support runtime dispatch builds.
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
	#define MT_SIMD_ALLOW_LEVEL3_TYPES 1
	#define MT_SIMD_ALLOW_LEVEL4_TYPES 1
#else
	#if defined(__AVX2__)
		#define MT_SIMD_ALLOW_LEVEL3_TYPES 1
	#else
		#define MT_SIMD_ALLOW_LEVEL3_TYPES 0
	#endif

	#if defined(__AVX512F__) && defined(__AVX512DQ__)
		#define MT_SIMD_ALLOW_LEVEL4_TYPES 1
	#else
		#define MT_SIMD_ALLOW_LEVEL4_TYPES 0
	#endif
#endif

#if defined(MT_FORCE_COMPILER_NO_AVX512)
	constexpr static bool compiler_has_avx512f = false;
#elif defined(__AVX512F__)
	constexpr static bool compiler_has_avx512f = true;
#else
	constexpr static bool compiler_has_avx512f = false;
#endif

#if defined(MT_FORCE_COMPILER_NO_AVX512)
	constexpr static bool compiler_has_avx512dq = false;
#elif defined(__AVX512DQ__)
	constexpr static bool compiler_has_avx512dq = true;
#else
	constexpr static bool compiler_has_avx512dq = false;
#endif

#if defined(MT_FORCE_COMPILER_NO_AVX512)
	constexpr static bool compiler_has_avx512vl = false;
#elif defined(__AVX512VL__)
	constexpr static bool compiler_has_avx512vl = true;
#else
	constexpr static bool compiler_has_avx512vl = false;
#endif


#if defined(MT_FORCE_COMPILER_NO_AVX512)
	constexpr static bool compiler_has_avx512bw = false;
#elif defined(__AVX512BW__)
	constexpr static bool compiler_has_avx512bw = true;
#else
	constexpr static bool compiler_has_avx512bw = false;
#endif

#if defined(MT_FORCE_COMPILER_NO_AVX512)
	constexpr static bool compiler_has_avx512cd = false;
#elif defined(__AVX512CD__)
	constexpr static bool compiler_has_avx512cd = true;
#else
	constexpr static bool compiler_has_avx512cd = false;
#endif

	// Compiler-mode x86_64 microarchitecture level checks.
	constexpr static bool compiler_is_level_1 = compiler_has_sse && compiler_has_sse2;
	constexpr static bool compiler_is_level_2 = compiler_is_level_1 && compiler_has_sse3 && compiler_has_ssse3 && compiler_has_sse4_1 && compiler_has_sse4_2;
	constexpr static bool compiler_is_level_3 = compiler_is_level_2 && compiler_has_avx && compiler_has_avx2 && compiler_has_fma && compiler_has_f16c;
	constexpr static bool compiler_is_level_4 = compiler_is_level_3 && compiler_has_avx512f && compiler_has_avx512dq && compiler_has_avx512vl && compiler_has_avx512bw && compiler_has_avx512cd;









}


 
