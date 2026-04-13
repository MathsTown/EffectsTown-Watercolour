#pragma once
/********************************************************************************************************

Authors:        (c) 2023-2026 Maths Town

Licence:        The MIT License

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
*********************************************************************************************************/

#include "environment.h"
#include <stdint.h>

namespace mt {

inline static bool test_all_false(bool mask) noexcept { return !mask; }
inline static bool test_all_true(bool mask) noexcept { return mask; }

#if MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)
#include <wasm_simd128.h>

inline static bool test_all_false(v128_t mask) noexcept { return wasm_i8x16_bitmask(mask) == 0; }
inline static bool test_all_true(v128_t mask) noexcept { return wasm_i8x16_bitmask(mask) == 0xFFFF; }
#endif

#if MT_SIMD_ARCH_X64
#include <immintrin.h>

inline static bool test_all_false(__mmask8 mask) noexcept { return mask == 0; }
inline static bool test_all_true(__mmask8 mask) noexcept { return mask == static_cast<__mmask8>(0xFFu); }

inline static bool test_all_false(__mmask16 mask) noexcept { return mask == 0; }
inline static bool test_all_true(__mmask16 mask) noexcept { return mask == static_cast<__mmask16>(0xFFFFu); }

inline static bool test_all_false(__m128i mask) noexcept { return _mm_movemask_epi8(mask) == 0; }
inline static bool test_all_true(__m128i mask) noexcept { return _mm_movemask_epi8(mask) == 0xFFFF; }

inline static bool test_all_false(__m256i mask) noexcept { return _mm256_movemask_epi8(mask) == 0; }
inline static bool test_all_true(__m256i mask) noexcept {
	return static_cast<uint32_t>(_mm256_movemask_epi8(mask)) == 0xFFFFFFFFu;
}

inline static bool test_all_false(__m128 mask) noexcept { return _mm_movemask_ps(mask) == 0; }
inline static bool test_all_true(__m128 mask) noexcept { return _mm_movemask_ps(mask) == 0xF; }

inline static bool test_all_false(__m256 mask) noexcept { return _mm256_movemask_ps(mask) == 0; }
inline static bool test_all_true(__m256 mask) noexcept { return _mm256_movemask_ps(mask) == 0xFF; }

inline static bool test_all_false(__m128d mask) noexcept { return _mm_movemask_pd(mask) == 0; }
inline static bool test_all_true(__m128d mask) noexcept { return _mm_movemask_pd(mask) == 0x3; }

inline static bool test_all_false(__m256d mask) noexcept { return _mm256_movemask_pd(mask) == 0; }
inline static bool test_all_true(__m256d mask) noexcept { return _mm256_movemask_pd(mask) == 0xF; }
#endif

} // namespace mt
