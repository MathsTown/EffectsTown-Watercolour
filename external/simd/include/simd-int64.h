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

Basic SIMD Types for 64-bit Signed Integers:

FallbackInt64		- Works on all build modes and CPUs.  Forwards most requests to the standard library.

Simd128Int64		- x86_64 Microarchitecture Level 1 - Works on all x86_64 CPUs.
					- Requires SSE and SSE2 support.  Will use SSE4.1 instructions when __SSE4_1__ or __AVX__ defined.

Simd256Int64		- x86_64 Microarchitecture Level 3.
					- Requires AVX, AVX2 and FMA support.

Simd512Int64		- x86_64 Microarchitecture Level 4.
					- Requires AVX512F, AVX512DQ, AVX512VL, AVX512CD, AVX512BW

SimdNativeInt64	- A Typedef referring to one of the above types.  Chosen based on compiler support/mode.
					- Just use this type in your code if you are building for a specific platform.


Checking CPU Support:
Unless you are using a SimdNative typedef, you must check for CPU support before using any of these types.
- MSVC - You may check at runtime or compile time.  (compile time checks generally results in much faster code)
- GCC/Clang - You must check at compile time (due to compiler limitations)

Types representing floats, doubles, ints, longs etc are arranged in microarchitecture level groups.
Generally CPUs have more SIMD support for floats than ints (and 32-bit is better than 64-bit).
Ensure the CPU supports the full "level" if you need to use more than one type.


To check support at compile time:
	- For GCC you may need pre-processor defines to gate the calling site.

To check support at run time:
	- Use cpu_supported()

Runtime detection notes:
Visual studio will support compiling all types and switching at runtime. However this often results in slower
code than compiling with full compiler support.  Visual studio will optimise AVX code well when build support is enabled.
If you are able, I recommend distributing code at different support levels. (1,3,4). Let the user choose which to download,
or your installer can make the switch.  It is also possible to dynamically load different .dlls

- Simd128/256/512 describe lane shape and API width and correspond to level 1, 2 & 4.
- When the compiler detects higher levels of support, such as SSE4.1 (level 2), more optimised instructions may be chosen.
- Runtime checks are only meaningful for builds intended to run across mixed CPU capabilities, but separate compilation in recommended.

WASM Support:
I've included FallbackInt64 for use with Emscripen.


*********************************************************************************************************/
#pragma once


#include <stdint.h>
#include <algorithm>
#include <cstring>
#include "simd-cpuid.h"
#include "simd-concepts.h"
#include "simd-common.h"
#include "simd-mask.h"
#include "simd-wasm-helpers.h"

/**************************************************************************************************
* Fallback Int64 type.
* Contains 1 x 64bit Signed Integer
*
* This is a fallback for cpus that are not yet supported.
*
* It may be useful to evaluate a single value from the same code base, as it offers the same interface
* as the SIMD types.
*
* ************************************************************************************************/
namespace mt {

struct FallbackInt64 {
	int64_t v;
	typedef int64_t F;
	typedef bool MaskType;
	FallbackInt64() = default;
	FallbackInt64(int64_t a) : v(a) {};

	//*****Support Informtion*****

	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported() { return true; }

	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported(CpuInformation) { return true; }

	//Performs a compile time CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static constexpr bool compiler_supported() {
		return true;
	}

	//*****Elements*****
	static constexpr int size_of_element() { return sizeof(int64_t); }
	static constexpr int number_of_elements() { return 1; }
	F element(int) const { return v; }
	void set_element(int, F value) { v = value; }

	//*****Make Functions****
	static FallbackInt64 make_sequential(int64_t first) { return FallbackInt64(first); }
	static FallbackInt64 make_set1(int64_t v) { return FallbackInt64(v); }


	//*****Addition Operators*****
	FallbackInt64& operator+=(const FallbackInt64& rhs) noexcept { v += rhs.v; return *this; }
	FallbackInt64& operator+=(int64_t rhs) noexcept { v += rhs; return *this; }

	//*****Subtraction Operators*****
	FallbackInt64& operator-=(const FallbackInt64& rhs) noexcept { v -= rhs.v; return *this; }
	FallbackInt64& operator-=(int64_t rhs) noexcept { v -= rhs; return *this; }

	//*****Multiplication Operators*****
	FallbackInt64& operator*=(const FallbackInt64& rhs) noexcept { v *= rhs.v; return *this; }
	FallbackInt64& operator*=(int64_t rhs) noexcept { v *= rhs; return *this; }

	//*****Division Operators*****
	FallbackInt64& operator/=(const FallbackInt64& rhs) noexcept { v /= rhs.v; return *this; }
	FallbackInt64& operator/=(int64_t rhs) noexcept { v /= rhs;	return *this; }

	//*****Negate Operators*****
	FallbackInt64 operator-() const noexcept { return FallbackInt64(-v); }

	//*****Bitwise Logic Operators*****
	FallbackInt64& operator&=(const FallbackInt64& rhs) noexcept { v &= rhs.v; return *this; }
	FallbackInt64& operator|=(const FallbackInt64& rhs) noexcept { v |= rhs.v; return *this; }
	FallbackInt64& operator^=(const FallbackInt64& rhs) noexcept { v ^= rhs.v; return *this; }

};

//*****Addition Operators*****
inline static FallbackInt64 operator+(FallbackInt64  lhs, const FallbackInt64& rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackInt64 operator+(FallbackInt64  lhs, int64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackInt64 operator+(int64_t lhs, FallbackInt64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static FallbackInt64 operator-(FallbackInt64  lhs, const FallbackInt64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackInt64 operator-(FallbackInt64  lhs, int64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackInt64 operator-(const int64_t lhs, FallbackInt64 rhs) noexcept { rhs.v = lhs - rhs.v; return rhs; }

//*****Multiplication Operators*****
inline static FallbackInt64 operator*(FallbackInt64  lhs, const FallbackInt64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackInt64 operator*(FallbackInt64  lhs, int64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackInt64 operator*(int64_t lhs, FallbackInt64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static FallbackInt64 operator/(FallbackInt64  lhs, const FallbackInt64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static FallbackInt64 operator/(FallbackInt64  lhs, int64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static FallbackInt64 operator/(const int64_t lhs, FallbackInt64 rhs) noexcept { rhs.v = lhs / rhs.v; return rhs; }


//*****Bitwise Logic Operators*****
inline static FallbackInt64 operator&(const FallbackInt64& lhs, const FallbackInt64& rhs) noexcept { return FallbackInt64(lhs.v & rhs.v); }
inline static FallbackInt64 operator|(const FallbackInt64& lhs, const FallbackInt64& rhs) noexcept { return FallbackInt64(lhs.v | rhs.v); }
inline static FallbackInt64 operator^(const FallbackInt64& lhs, const FallbackInt64& rhs) noexcept { return FallbackInt64(lhs.v ^ rhs.v); }
inline static FallbackInt64 operator~(FallbackInt64 lhs) noexcept { return FallbackInt64(~lhs.v); }


//*****Shifting Operators*****
inline static FallbackInt64 operator<<(FallbackInt64 lhs, int bits) noexcept { lhs.v <<= bits; return lhs; }
inline static FallbackInt64 operator>>(FallbackInt64 lhs, int bits) noexcept { lhs.v >>= bits; return lhs; }


//*****Min/Max*****
inline static FallbackInt64 min(FallbackInt64 a, FallbackInt64 b) { return FallbackInt64(std::min(a.v, b.v)); }
inline static FallbackInt64 max(FallbackInt64 a, FallbackInt64 b) { return FallbackInt64(std::max(a.v, b.v)); }

//*****Math Operators*****
inline static FallbackInt64 abs(FallbackInt64 a) noexcept {
	// std::abs(INT64_MIN) is UB; use unsigned two's-complement math.
	const uint64_t ux = static_cast<uint64_t>(a.v);
	const uint64_t sign = ux >> 63;
	const uint64_t mask = 0ull - sign;
	const uint64_t mag = (ux ^ mask) + sign;
	return FallbackInt64(static_cast<int64_t>(mag));
}

//*****Conditional Functions*****
inline static bool compare_equal(const FallbackInt64 a, const FallbackInt64 b) noexcept { return a.v == b.v; }
inline static bool compare_less(const FallbackInt64 a, const FallbackInt64 b) noexcept { return a.v < b.v; }
inline static bool compare_less_equal(const FallbackInt64 a, const FallbackInt64 b) noexcept { return a.v <= b.v; }
inline static bool compare_greater(const FallbackInt64 a, const FallbackInt64 b) noexcept { return a.v > b.v; }
inline static bool compare_greater_equal(const FallbackInt64 a, const FallbackInt64 b) noexcept { return a.v >= b.v; }
inline static FallbackInt64 blend(const FallbackInt64 if_false, const FallbackInt64 if_true, bool mask) noexcept {
	return mask ? if_true : if_false;
}


//***************** x86_64 only code ******************
#if MT_SIMD_ARCH_X64
#include <immintrin.h>


/**************************************************************************************************
 * Compiler Compatability Layer
 * MSCV intrinsics are sometime a little more feature rich than GCC and Clang.  
 * This section provides shims and patches for compiler incompatible behaviour.
 * ************************************************************************************************/
namespace simd_detail_i64 {
	// Portability layer: GCC/Clang cannot index SIMD lanes via MSVC vector members.
	inline int64_t lane_get(__m128i v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m128i_i64[i];
#else
		alignas(16) int64_t lanes[2];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline __m128i lane_set(__m128i v, int i, int64_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m128i_i64[i] = value;
		return v;
#else
		alignas(16) int64_t lanes[2];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		lanes[i] = value;
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(lanes));
#endif
	}
	inline int64_t lane_get(const __m256i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m256i_i64[i];
#else
		alignas(32) int64_t lanes[4];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline void lane_set(__m256i& v, int i, int64_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m256i_i64[i] = value;
#else
		alignas(32) int64_t lanes[4];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		lanes[i] = value;
		v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lanes));
#endif
	}
	inline int64_t lane_get(const __m512i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m512i_i64[i];
#else
		alignas(64) int64_t lanes[8];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	inline void lane_set(__m512i& v, int i, int64_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m512i_i64[i] = value;
#else
		alignas(64) int64_t lanes[8];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

	// Fallback: integer SIMD divide intrinsics are non-standard across compilers.
	inline __m128i div_epi64(__m128i a, __m128i b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm_div_epi64(a, b);
#else
		alignas(16) int64_t lhs[2], rhs[2], out[2];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lhs), a);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(rhs), b);
		for (int i = 0; i < 2; ++i) out[i] = lhs[i] / rhs[i];
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(out));
#endif
	}
	inline __m256i div_epi64(const __m256i& a, const __m256i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm256_div_epi64(a, b);
#else
		alignas(32) int64_t lhs[4], rhs[4], out[4];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lhs), a);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(rhs), b);
		for (int i = 0; i < 4; ++i) out[i] = lhs[i] / rhs[i];
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out));
#endif
	}
	inline __m512i div_epi64(const __m512i& a, const __m512i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm512_div_epi64(a, b);
#else
		alignas(64) int64_t lhs[8], rhs[8], out[8];
		std::memcpy(lhs, &a, sizeof(a));
		std::memcpy(rhs, &b, sizeof(b));
		for (int i = 0; i < 8; ++i) out[i] = lhs[i] / rhs[i];
		__m512i out_v;
		std::memcpy(&out_v, out, sizeof(out_v));
		return out_v;
#endif
	}
}



#if MT_SIMD_ALLOW_LEVEL4_TYPES
/**************************************************************************************************
 * SIMD 512 type.  Contains 16 x 64bit Signed Integers
 * Requires AVX-512F
 * ************************************************************************************************/
struct Simd512Int64 {
	__m512i v;
	typedef int64_t F;
	typedef __mmask8 MaskType;

	Simd512Int64() = default;
	Simd512Int64(__m512i a) : v(a) {};
	Simd512Int64(F a) : v(_mm512_set1_epi64(a)) {};

	//*****Support Informtion*****
	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.is_level_4();
	}

	//Performs a compile time support. Checks this type ONLY (integers in same class may not be supported) 
	static constexpr bool compiler_supported() {
		return mt::environment::compiler_can_use_x86_64_level_4_types;
	}

	//*****Elements*****
	static constexpr int size_of_element() { return sizeof(int64_t); }
	static constexpr int number_of_elements() { return 8; }
	F element(int i) const { return mt::simd_detail_i64::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_i64::lane_set(v, i, value); }

	//*****Make Functions****
	static Simd512Int64 make_sequential(int64_t first) {
		const uint64_t base = static_cast<uint64_t>(first);
		return Simd512Int64(_mm512_set_epi64(
			static_cast<int64_t>(base + 7ull), static_cast<int64_t>(base + 6ull), static_cast<int64_t>(base + 5ull), static_cast<int64_t>(base + 4ull),
			static_cast<int64_t>(base + 3ull), static_cast<int64_t>(base + 2ull), static_cast<int64_t>(base + 1ull), static_cast<int64_t>(base)));
	}
	static Simd512Int64 make_set1(int64_t v) { return Simd512Int64(_mm512_set1_epi64(v)); }


	//*****Addition Operators*****
	Simd512Int64& operator+=(const Simd512Int64& rhs) noexcept { v = _mm512_add_epi64(v, rhs.v); return *this; }
	Simd512Int64& operator+=(int64_t rhs) noexcept { v = _mm512_add_epi64(v, _mm512_set1_epi64(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd512Int64& operator-=(const Simd512Int64& rhs) noexcept { v = _mm512_sub_epi64(v, rhs.v); return *this; }
	Simd512Int64& operator-=(int64_t rhs) noexcept { v = _mm512_sub_epi64(v, _mm512_set1_epi64(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd512Int64& operator*=(const Simd512Int64& rhs) noexcept { v = _mm512_mullo_epi64(v, rhs.v); return *this; }
	Simd512Int64& operator*=(int64_t rhs) noexcept { v = _mm512_mullo_epi64(v, _mm512_set1_epi64(rhs)); return *this; }

	//*****Division Operators*****
	Simd512Int64& operator/=(const Simd512Int64& rhs) noexcept { v = mt::simd_detail_i64::div_epi64(v, rhs.v); return *this; }
	Simd512Int64& operator/=(int64_t rhs) noexcept {		
		v = mt::simd_detail_i64::div_epi64(v, _mm512_set1_epi64(rhs));
		return *this;
		
	}

	//*****Negate Operators*****
	Simd512Int64 operator-() const noexcept {
		return Simd512Int64(_mm512_sub_epi64(_mm512_setzero_si512(), v));
	}

	//*****Bitwise Logic Operators*****
	Simd512Int64& operator&=(const Simd512Int64& rhs) noexcept { v = _mm512_and_si512(v, rhs.v); return *this; }
	Simd512Int64& operator|=(const Simd512Int64& rhs) noexcept { v = _mm512_or_si512(v, rhs.v); return *this; }
	Simd512Int64& operator^=(const Simd512Int64& rhs) noexcept { v = _mm512_xor_si512(v, rhs.v); return *this; }




};

//*****Addition Operators*****
inline static Simd512Int64 operator+(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Int64 operator+(Simd512Int64  lhs, int64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Int64 operator+(int64_t lhs, Simd512Int64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd512Int64 operator-(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Int64 operator-(Simd512Int64  lhs, int64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Int64 operator-(const int64_t lhs, const Simd512Int64& rhs) noexcept { return Simd512Int64(_mm512_sub_epi64(_mm512_set1_epi64(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd512Int64 operator*(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Int64 operator*(Simd512Int64  lhs, int64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Int64 operator*(int64_t lhs, Simd512Int64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd512Int64 operator/(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd512Int64 operator/(Simd512Int64  lhs, int64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd512Int64 operator/(const int64_t lhs, const Simd512Int64& rhs) noexcept { return Simd512Int64(mt::simd_detail_i64::div_epi64(_mm512_set1_epi64(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd512Int64 operator&(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd512Int64 operator|(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd512Int64 operator^(Simd512Int64  lhs, const Simd512Int64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd512Int64 operator~(const Simd512Int64& lhs) noexcept { return Simd512Int64(_mm512_xor_epi64(lhs.v, _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF))); } //No bitwise not in AVX512 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd512Int64 operator<<(const Simd512Int64& lhs, int bits) noexcept { return Simd512Int64(_mm512_slli_epi64(lhs.v, bits)); }

//Arithmatic Shift is used for signed integers
inline static Simd512Int64 operator>>(const Simd512Int64& lhs, int bits) noexcept { return Simd512Int64(_mm512_srai_epi64(lhs.v, bits)); }



//*****Min/Max*****
inline static Simd512Int64 min(Simd512Int64 a, Simd512Int64 b) { return Simd512Int64(_mm512_min_epi64(a.v, b.v)); }
inline static Simd512Int64 max(Simd512Int64 a, Simd512Int64 b) { return Simd512Int64(_mm512_max_epi64(a.v, b.v)); }


//*****Mathematical*****
inline static Simd512Int64 abs(Simd512Int64 a) { return Simd512Int64(_mm512_abs_epi64(a.v)); }

//*****Conditional Functions*****
inline static __mmask8 compare_equal(const Simd512Int64 a, const Simd512Int64 b) noexcept {
	return _mm512_cmp_epi64_mask(a.v, b.v, _MM_CMPINT_EQ);
}
inline static __mmask8 compare_less(const Simd512Int64 a, const Simd512Int64 b) noexcept {
	return _mm512_cmp_epi64_mask(a.v, b.v, _MM_CMPINT_LT);
}
inline static __mmask8 compare_less_equal(const Simd512Int64 a, const Simd512Int64 b) noexcept {
	return _mm512_cmp_epi64_mask(a.v, b.v, _MM_CMPINT_LE);
}
inline static __mmask8 compare_greater(const Simd512Int64 a, const Simd512Int64 b) noexcept {
	return _mm512_cmp_epi64_mask(a.v, b.v, _MM_CMPINT_NLE);
}
inline static __mmask8 compare_greater_equal(const Simd512Int64 a, const Simd512Int64 b) noexcept {
	return _mm512_cmp_epi64_mask(a.v, b.v, _MM_CMPINT_NLT);
}
inline static Simd512Int64 blend(const Simd512Int64 if_false, const Simd512Int64 if_true, __mmask8 mask) noexcept {
	return Simd512Int64(_mm512_mask_blend_epi64(mask, if_false.v, if_true.v));
}

#endif // MT_SIMD_ALLOW_LEVEL4_TYPES

#if MT_SIMD_ALLOW_LEVEL3_TYPES
/**************************************************************************************************
 * SIMD 256 type.  Contains 8 x 64bit Signed Integers
 * Requires AVX2 support.
 * ************************************************************************************************/
struct Simd256Int64 {
	__m256i v;
	typedef int64_t F;
	typedef __m256i MaskType;

	Simd256Int64() = default;
	Simd256Int64(__m256i a) : v(a) {};
	Simd256Int64(F a) : v(_mm256_set1_epi64x(a)) {};

	//*****Support Informtion*****
	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.is_level_3();
	}

	//Performs a compile time support. Checks this type ONLY (integers in same class may not be supported) 
	static constexpr bool compiler_supported() {
		return mt::environment::compiler_can_use_x86_64_level_3_types;
	}

	//*****Elements*****
	static constexpr int size_of_element() { return sizeof(int64_t); }
	static constexpr int number_of_elements() { return 4; }
	F element(int i) const { return mt::simd_detail_i64::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_i64::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd256Int64& operator+=(const Simd256Int64& rhs) noexcept { v = _mm256_add_epi64(v, rhs.v); return *this; }
	Simd256Int64& operator+=(int64_t rhs) noexcept { v = _mm256_add_epi64(v, _mm256_set1_epi64x(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd256Int64& operator-=(const Simd256Int64& rhs) noexcept { v = _mm256_sub_epi64(v, rhs.v); return *this; }
	Simd256Int64& operator-=(int64_t rhs) noexcept { v = _mm256_sub_epi64(v, _mm256_set1_epi64x(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd256Int64& operator*=(const Simd256Int64& rhs) noexcept {
		if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
			//There is a slight chance a user is using this struct but has compiled the code with avx512dq and avx512vl enabled.
			this->v = _mm256_mullo_epi64(v, rhs.v);
			return *this;
		}
		else {
			//Multiplication (i64*i64->i64) is not implemented for AVX2.
			//Algorithm:
			//		If x*y = [a,b]*[c,d] (where a,b,c,d are digits/dwords).
			//			   = (2^64) ac + (2^32) (ad + bc) + bd
			//				 (We ignore upper 64bits so, we can ignore the (2^64) term.)
			//
			//Given lhs = [a,b] and rhs=[c,d]
			auto digit1 = _mm256_mul_epu32(v, rhs.v);											//=[0+carry, bd]	: Calculate bd (i32*i32->i64)
			auto rhs_swap = _mm256_shuffle_epi32(rhs.v, 0xB1);									//=[d,c]			: Swaps the low and high dwords on RHS . 
			auto ad_bc = _mm256_mullo_epi32(v, rhs_swap);										//=[ad,bc]			: Multiply every dword together (i32*i32->i32).          
			auto bc_00 = _mm256_slli_epi64(ad_bc, 32);											//=[bc,0]			: Shift Left to put bc in the upper dword.
			auto ad_plus_bc = _mm256_add_epi32(ad_bc, bc_00);									//=[ad+bc,bc]		: Perofrm addition in the upper dword
			auto digit2 = _mm256_and_si256(ad_plus_bc, _mm256_set1_epi64x(0xFFFFFFFF00000000)); //=[ad+bc,0]		: Zero lower dword using &
			this->v = _mm256_add_epi64(digit1, digit2);											//=[ad+dc+carry,bd] : Add digits to get final result. 
			return *this;
		}

	}
	Simd256Int64& operator*=(int64_t rhs) noexcept { *this *= Simd256Int64(_mm256_set1_epi64x(rhs)); return *this; }

	//*****Division Operators*****
	Simd256Int64& operator/=(const Simd256Int64& rhs) noexcept {
		v = mt::simd_detail_i64::div_epi64(v, rhs.v);
		return *this;
	}
	Simd256Int64& operator/=(int64_t rhs) noexcept {		
		v = mt::simd_detail_i64::div_epi64(v, _mm256_set1_epi64x(rhs));
		return *this;
	}

	//*****Negate Operators*****
	Simd256Int64 operator-() const noexcept {
		return Simd256Int64(_mm256_sub_epi64(_mm256_setzero_si256(), v));
	}

	//*****Bitwise Logic Operators*****
	Simd256Int64& operator&=(const Simd256Int64& rhs) noexcept { v = _mm256_and_si256(v, rhs.v); return *this; }
	Simd256Int64& operator|=(const Simd256Int64& rhs) noexcept { v = _mm256_or_si256(v, rhs.v); return *this; }
	Simd256Int64& operator^=(const Simd256Int64& rhs) noexcept { v = _mm256_xor_si256(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd256Int64 make_sequential(int64_t first) {
		const uint64_t base = static_cast<uint64_t>(first);
		return Simd256Int64(_mm256_set_epi64x(
			static_cast<int64_t>(base + 3ull), static_cast<int64_t>(base + 2ull), static_cast<int64_t>(base + 1ull), static_cast<int64_t>(base)));
	}
	static Simd256Int64 make_set1(int64_t v) { return Simd256Int64(_mm256_set1_epi64x(v)); }


};

//*****Addition Operators*****
inline static Simd256Int64 operator+(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Int64 operator+(Simd256Int64  lhs, int64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Int64 operator+(int64_t lhs, Simd256Int64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd256Int64 operator-(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Int64 operator-(Simd256Int64  lhs, int64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Int64 operator-(const int64_t lhs, const Simd256Int64& rhs) noexcept { return Simd256Int64(_mm256_sub_epi64(_mm256_set1_epi64x(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd256Int64 operator*(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Int64 operator*(Simd256Int64  lhs, int64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Int64 operator*(int64_t lhs, Simd256Int64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline Simd256Int64 operator/(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline Simd256Int64 operator/(Simd256Int64  lhs, int64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline Simd256Int64 operator/(const int64_t lhs, const Simd256Int64& rhs) noexcept { return Simd256Int64(mt::simd_detail_i64::div_epi64(_mm256_set1_epi64x(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd256Int64 operator&(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd256Int64 operator|(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd256Int64 operator^(Simd256Int64  lhs, const Simd256Int64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd256Int64 operator~(const Simd256Int64& lhs) noexcept { return Simd256Int64(_mm256_xor_si256(lhs.v, _mm256_cmpeq_epi64(lhs.v, lhs.v))); } //No bitwise not in AVX2 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd256Int64 operator<<(const Simd256Int64& lhs, int bits) noexcept { return Simd256Int64(_mm256_slli_epi64(lhs.v, bits)); }
inline static Simd256Int64 operator>>(const Simd256Int64& lhs, int bits) noexcept {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd256Int64(_mm256_srai_epi64(lhs.v, bits)); //AVX-512
	}
	else {
		//AVX2 fallback: emulate arithmetic shift with logical shift + sign fill mask.
		const int n = (bits <= 0) ? 0 : ((bits >= 64) ? 63 : bits);
		if (n == 0) {
			return lhs;
		}
		const auto shift_r = _mm_cvtsi64_si128(static_cast<int64_t>(n));
		const auto shift_l = _mm_cvtsi64_si128(static_cast<int64_t>(64 - n));
		const auto logical = _mm256_srl_epi64(lhs.v, shift_r);
		const auto sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), lhs.v);
		const auto fill = _mm256_sll_epi64(sign, shift_l);
		return Simd256Int64(_mm256_or_si256(logical, fill));
	}
}


//*****Min/Max*****
inline static Simd256Int64 min(Simd256Int64 a, Simd256Int64 b) {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd256Int64(_mm256_min_epi64(a.v, b.v)); 
	}
	else {
		const auto pick_b = _mm256_cmpgt_epi64(a.v, b.v); // a > b
		const auto keep_a = _mm256_xor_si256(pick_b, _mm256_set1_epi64x(-1));
		return Simd256Int64(_mm256_or_si256(_mm256_and_si256(a.v, keep_a), _mm256_and_si256(b.v, pick_b)));
	}
}
inline static Simd256Int64 max(Simd256Int64 a, Simd256Int64 b) {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd256Int64(_mm256_max_epi64(a.v, b.v)); 
	}
	else {
		const auto pick_a = _mm256_cmpgt_epi64(a.v, b.v); // a > b
		const auto keep_b = _mm256_xor_si256(pick_a, _mm256_set1_epi64x(-1));
		return Simd256Int64(_mm256_or_si256(_mm256_and_si256(a.v, pick_a), _mm256_and_si256(b.v, keep_b)));
	}
}

//*****Mathematical*****
inline static Simd256Int64 abs(Simd256Int64 a) {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd256Int64(_mm256_abs_epi64(a.v));
	}
	else {
		const auto sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), a.v);
		return Simd256Int64(_mm256_sub_epi64(_mm256_xor_si256(a.v, sign), sign));
	}
}

//*****Conditional Functions*****
inline static __m256i compare_equal(const Simd256Int64 a, const Simd256Int64 b) noexcept { return _mm256_cmpeq_epi64(a.v, b.v); }
inline static __m256i compare_less(const Simd256Int64 a, const Simd256Int64 b) noexcept { return _mm256_cmpgt_epi64(b.v, a.v); }
inline static __m256i compare_less_equal(const Simd256Int64 a, const Simd256Int64 b) noexcept {
	return _mm256_xor_si256(_mm256_cmpgt_epi64(a.v, b.v), _mm256_set1_epi64x(-1));
}
inline static __m256i compare_greater(const Simd256Int64 a, const Simd256Int64 b) noexcept { return _mm256_cmpgt_epi64(a.v, b.v); }
inline static __m256i compare_greater_equal(const Simd256Int64 a, const Simd256Int64 b) noexcept {
	return _mm256_xor_si256(_mm256_cmpgt_epi64(b.v, a.v), _mm256_set1_epi64x(-1));
}
inline static Simd256Int64 blend(const Simd256Int64 if_false, const Simd256Int64 if_true, __m256i mask) noexcept {
	return Simd256Int64(_mm256_blendv_epi8(if_false.v, if_true.v, mask));
}








#endif // MT_SIMD_ALLOW_LEVEL3_TYPES

/**************************************************************************************************
*SIMD 128 type.Contains 4 x 64bit Signed Integers
* Requires SSE2 support.
* (will be faster with SSE4.1 enabled at compile time)
* ************************************************************************************************/
struct Simd128Int64 {
	__m128i v;
	typedef int64_t F;
	typedef __m128i MaskType;

	Simd128Int64() = default;
	Simd128Int64(__m128i a) : v(a) {};
	Simd128Int64(F a) : v(_mm_set1_epi64x(a)) {};

	//*****Support Informtion*****
	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.is_level_1();
	}

	//Performs a compile time support. Checks this type ONLY (integers in same class may not be supported) 
	static constexpr bool compiler_supported() {
		return mt::environment::compiler_can_use_x86_64_level_1_types;
	}

	//*****Elements*****
	static constexpr int size_of_element() { return sizeof(int64_t); }
	static constexpr int number_of_elements() { return 2; }
	F element(int i) const { return mt::simd_detail_i64::lane_get(v, i); }
	void set_element(int i, F value) { v = mt::simd_detail_i64::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd128Int64& operator+=(const Simd128Int64& rhs) noexcept { v = _mm_add_epi64(v, rhs.v); return *this; }
	Simd128Int64& operator+=(int64_t rhs) noexcept { v = _mm_add_epi64(v, _mm_set1_epi64x(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd128Int64& operator-=(const Simd128Int64& rhs) noexcept { v = _mm_sub_epi64(v, rhs.v); return *this; }
	Simd128Int64& operator-=(int64_t rhs) noexcept { v = _mm_sub_epi64(v, _mm_set1_epi64x(rhs));	return *this; } 

	//*****Multiplication Operators*****
	Simd128Int64& operator*=(const Simd128Int64& rhs) noexcept {
		if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
			v = _mm_mullo_epi64(v, rhs.v); //AVX-512
			return *this;
		}
		else {
			v = _mm_set_epi64x(element(1) * rhs.element(1), element(0) * rhs.element(0));
			return *this;
		}
	}

	Simd128Int64& operator*=(int64_t rhs) noexcept { *this *= Simd128Int64(_mm_set1_epi64x(rhs)); return *this; }

	//*****Division Operators*****
	Simd128Int64& operator/=(const Simd128Int64& rhs) noexcept { v = mt::simd_detail_i64::div_epi64(v, rhs.v); return *this; }
	Simd128Int64& operator/=(int64_t rhs) noexcept { v = mt::simd_detail_i64::div_epi64(v, _mm_set1_epi64x(rhs));	return *this; } //SSE

	//*****Negate Operators*****
	Simd128Int64 operator-() const noexcept {
		return Simd128Int64(_mm_sub_epi64(_mm_setzero_si128(), v));
	}

	//*****Bitwise Logic Operators*****
	Simd128Int64& operator&=(const Simd128Int64& rhs) noexcept { v = _mm_and_si128(v, rhs.v); return *this; } //SSE2
	Simd128Int64& operator|=(const Simd128Int64& rhs) noexcept { v = _mm_or_si128(v, rhs.v); return *this; }
	Simd128Int64& operator^=(const Simd128Int64& rhs) noexcept { v = _mm_xor_si128(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd128Int64 make_sequential(int64_t first) {
		const uint64_t base = static_cast<uint64_t>(first);
		return Simd128Int64(_mm_set_epi64x(static_cast<int64_t>(base + 1ull), static_cast<int64_t>(base)));
	}
	static Simd128Int64 make_set1(int64_t v) { return Simd128Int64(_mm_set1_epi64x(v)); }






};

//*****Addition Operators*****
inline static Simd128Int64 operator+(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int64 operator+(Simd128Int64  lhs, int64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int64 operator+(int64_t lhs, Simd128Int64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd128Int64 operator-(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int64 operator-(Simd128Int64  lhs, int64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int64 operator-(const int64_t lhs, const Simd128Int64& rhs) noexcept { return Simd128Int64(_mm_sub_epi64(_mm_set1_epi64x(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd128Int64 operator*(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int64 operator*(Simd128Int64  lhs, int64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int64 operator*(int64_t lhs, Simd128Int64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd128Int64 operator/(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd128Int64 operator/(Simd128Int64  lhs, int64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Int64 operator/(const int64_t lhs, const Simd128Int64& rhs) noexcept { return Simd128Int64(mt::simd_detail_i64::div_epi64(_mm_set1_epi64x(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd128Int64 operator&(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128Int64 operator|(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128Int64 operator^(Simd128Int64  lhs, const Simd128Int64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128Int64 operator~(const Simd128Int64& lhs) noexcept { return Simd128Int64(_mm_xor_si128(lhs.v, _mm_set1_epi32(-1))); }


//*****Shifting Operators*****
inline static Simd128Int64 operator<<(const Simd128Int64& lhs, const int bits) noexcept { return Simd128Int64(_mm_slli_epi64(lhs.v, bits)); } //SSE2
//Arithmatic Shift is used for signed integers
inline static Simd128Int64 operator>>(const Simd128Int64& lhs, const int bits) noexcept { 
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd128Int64(_mm_srai_epi64(lhs.v, bits)); //AVX-512
	}
	else if constexpr (mt::environment::compiler_has_sse4_2) {
		const int n = (bits <= 0) ? 0 : ((bits >= 64) ? 63 : bits);
		if (n == 0) {
			return lhs;
		}
		const auto shift_r = _mm_cvtsi64_si128(static_cast<int64_t>(n));
		const auto shift_l = _mm_cvtsi64_si128(static_cast<int64_t>(64 - n));
		const auto logical = _mm_srl_epi64(lhs.v, shift_r);
		const auto sign = _mm_cmpgt_epi64(_mm_setzero_si128(), lhs.v);
		const auto fill = _mm_sll_epi64(sign, shift_l);
		return Simd128Int64(_mm_or_si128(logical, fill));
	}
	else {
		//No Arithmatic Shift Right for SSE or AVX2
		auto m1 = lhs.element(1) >> bits;
		auto m0 = lhs.element(0) >> bits;
		return Simd128Int64(_mm_set_epi64x(m1, m0));
	}
}




//*****Min/Max*****
inline static Simd128Int64 min(Simd128Int64 a, Simd128Int64 b) {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd128Int64(_mm_min_epi64(a.v, b.v)); //AVX-512
	}
	else if constexpr (mt::environment::compiler_has_sse4_2) {
		const auto pick_b = _mm_cmpgt_epi64(a.v, b.v); // a > b
		const auto keep_a = _mm_xor_si128(pick_b, _mm_set1_epi64x(-1));
		return Simd128Int64(_mm_or_si128(_mm_and_si128(a.v, keep_a), _mm_and_si128(b.v, pick_b)));
	}
	else {
		//No min/max or compare for Signed ints in SSE2 so we will just unroll.
		auto m1 = std::min(a.element(1), b.element(1));
		auto m0 = std::min(a.element(0), b.element(0));
		return Simd128Int64(_mm_set_epi64x(m1, m0));
	}
}



inline static Simd128Int64 max(Simd128Int64 a, Simd128Int64 b) {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd128Int64(_mm_max_epi64(a.v, b.v));  //avx-512
	}
	else if constexpr (mt::environment::compiler_has_sse4_2) {
		const auto pick_a = _mm_cmpgt_epi64(a.v, b.v); // a > b
		const auto keep_b = _mm_xor_si128(pick_a, _mm_set1_epi64x(-1));
		return Simd128Int64(_mm_or_si128(_mm_and_si128(a.v, pick_a), _mm_and_si128(b.v, keep_b)));
	}
	else {
		//No min/max or compare for Signed ints in SSE2 so we will just unroll.
		auto m1 = std::max(a.element(1), b.element(1));
		auto m0 = std::max(a.element(0), b.element(0));
		return Simd128Int64(_mm_set_epi64x( m1, m0));
	}
}

//*****Mathematical*****
inline static Simd128Int64 abs(Simd128Int64 a) {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f) {
		return Simd128Int64(_mm_abs_epi64(a.v));  //avx-512
	}
	else if constexpr (mt::environment::compiler_has_sse4_2) {
		const auto sign = _mm_cmpgt_epi64(_mm_setzero_si128(), a.v);
		return Simd128Int64(_mm_sub_epi64(_mm_xor_si128(a.v, sign), sign));
	}
	else {
		//No i64 abs in SSE2/AVX2; use scalar lanes.
		const auto m1 = abs(FallbackInt64(a.element(1))).v;
		const auto m0 = abs(FallbackInt64(a.element(0))).v;
		return Simd128Int64(_mm_set_epi64x(m1, m0));
	}
}

//*****Conditional Functions*****
inline static __m128i compare_equal(const Simd128Int64 a, const Simd128Int64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return _mm_cmpeq_epi64(a.v, b.v);
	}
	else {
		const int64_t m1 = (a.element(1) == b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) == b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_less(const Simd128Int64 a, const Simd128Int64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		return _mm_cmpgt_epi64(b.v, a.v);
	}
	else {
		const int64_t m1 = (a.element(1) < b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) < b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_less_equal(const Simd128Int64 a, const Simd128Int64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		return _mm_xor_si128(_mm_cmpgt_epi64(a.v, b.v), _mm_set1_epi64x(-1));
	}
	else {
		const int64_t m1 = (a.element(1) <= b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) <= b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_greater(const Simd128Int64 a, const Simd128Int64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		return _mm_cmpgt_epi64(a.v, b.v);
	}
	else {
		const int64_t m1 = (a.element(1) > b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) > b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_greater_equal(const Simd128Int64 a, const Simd128Int64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		return _mm_xor_si128(_mm_cmpgt_epi64(b.v, a.v), _mm_set1_epi64x(-1));
	}
	else {
		const int64_t m1 = (a.element(1) >= b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) >= b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static Simd128Int64 blend(const Simd128Int64 if_false, const Simd128Int64 if_true, __m128i mask) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Int64(_mm_blendv_epi8(if_false.v, if_true.v, mask));
	}
	else {
		return Simd128Int64(_mm_or_si128(_mm_andnot_si128(mask, if_false.v), _mm_and_si128(mask, if_true.v)));
	}
}


#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)

struct Simd128Int64 {
	v128_t v;
	typedef int64_t F;
	typedef v128_t MaskType;
	Simd128Int64() = default;
	Simd128Int64(v128_t a) : v(a) {}
	Simd128Int64(F a) : v(mt::simd_wasm_detail::splat<int64_t>(a)) {}

	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.has_wasm_simd();
	}
	static constexpr bool compiler_supported() {
		return mt::environment::is_wasm_simd_level_1;
	}

	static constexpr int size_of_element() { return sizeof(int64_t); }
	static constexpr int number_of_elements() { return 2; }
	F element(int i) const { return mt::simd_wasm_detail::lane_get<int64_t, 2>(v, i); }
	void set_element(int i, F value) { v = mt::simd_wasm_detail::lane_set<int64_t, 2>(v, i, value); }
	static Simd128Int64 make_sequential(int64_t first) { return Simd128Int64(mt::simd_wasm_detail::make_sequential<int64_t, 2>(first)); }
	static Simd128Int64 make_set1(int64_t value) { return Simd128Int64(mt::simd_wasm_detail::splat<int64_t>(value)); }

	Simd128Int64& operator+=(const Simd128Int64& rhs) noexcept { v = wasm_i64x2_add(v, rhs.v); return *this; }
	Simd128Int64& operator+=(int64_t rhs) noexcept { v = wasm_i64x2_add(v, make_set1(rhs).v); return *this; }
	Simd128Int64& operator-=(const Simd128Int64& rhs) noexcept { v = wasm_i64x2_sub(v, rhs.v); return *this; }
	Simd128Int64& operator-=(int64_t rhs) noexcept { v = wasm_i64x2_sub(v, make_set1(rhs).v); return *this; }
	Simd128Int64& operator*=(const Simd128Int64& rhs) noexcept { v = wasm_i64x2_mul(v, rhs.v); return *this; }
	Simd128Int64& operator*=(int64_t rhs) noexcept { v = wasm_i64x2_mul(v, make_set1(rhs).v); return *this; }
	Simd128Int64& operator/=(const Simd128Int64& rhs) noexcept {
		v = mt::simd_wasm_detail::map_binary<int64_t, 2>(v, rhs.v, [](int64_t a, int64_t b) { return a / b; });
		return *this;
	}
	Simd128Int64& operator/=(int64_t rhs) noexcept { return *this /= make_set1(rhs); }
	Simd128Int64 operator-() const noexcept { return Simd128Int64(wasm_i64x2_neg(v)); }
	Simd128Int64& operator&=(const Simd128Int64& rhs) noexcept { v = wasm_v128_and(v, rhs.v); return *this; }
	Simd128Int64& operator|=(const Simd128Int64& rhs) noexcept { v = wasm_v128_or(v, rhs.v); return *this; }
	Simd128Int64& operator^=(const Simd128Int64& rhs) noexcept { v = wasm_v128_xor(v, rhs.v); return *this; }
};

inline static Simd128Int64 operator+(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int64 operator+(Simd128Int64 lhs, int64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int64 operator+(int64_t lhs, Simd128Int64 rhs) noexcept { rhs += lhs; return rhs; }
inline static Simd128Int64 operator-(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int64 operator-(Simd128Int64 lhs, int64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int64 operator-(int64_t lhs, const Simd128Int64& rhs) noexcept { return Simd128Int64::make_set1(lhs) - rhs; }
inline static Simd128Int64 operator*(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int64 operator*(Simd128Int64 lhs, int64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int64 operator*(int64_t lhs, Simd128Int64 rhs) noexcept { rhs *= lhs; return rhs; }
inline static Simd128Int64 operator/(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Int64 operator/(Simd128Int64 lhs, int64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Int64 operator/(int64_t lhs, const Simd128Int64& rhs) noexcept { return Simd128Int64::make_set1(lhs) / rhs; }
inline static Simd128Int64 operator&(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128Int64 operator|(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128Int64 operator^(Simd128Int64 lhs, const Simd128Int64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128Int64 operator~(const Simd128Int64& lhs) noexcept { return Simd128Int64(wasm_v128_not(lhs.v)); }
inline static Simd128Int64 operator<<(const Simd128Int64& lhs, const int bits) noexcept { return Simd128Int64(wasm_i64x2_shl(lhs.v, static_cast<uint32_t>(bits))); }
inline static Simd128Int64 operator>>(const Simd128Int64& lhs, const int bits) noexcept { return Simd128Int64(wasm_i64x2_shr(lhs.v, static_cast<uint32_t>(bits))); }

inline static Simd128Int64 min(Simd128Int64 a, Simd128Int64 b) {
	return Simd128Int64(mt::simd_wasm_detail::map_binary<int64_t, 2>(a.v, b.v, [](int64_t x, int64_t y) { return std::min(x, y); }));
}
inline static Simd128Int64 max(Simd128Int64 a, Simd128Int64 b) {
	return Simd128Int64(mt::simd_wasm_detail::map_binary<int64_t, 2>(a.v, b.v, [](int64_t x, int64_t y) { return std::max(x, y); }));
}
inline static Simd128Int64 abs(Simd128Int64 a) { return Simd128Int64(wasm_i64x2_abs(a.v)); }

inline static v128_t compare_equal(const Simd128Int64 a, const Simd128Int64 b) noexcept { return wasm_i64x2_eq(a.v, b.v); }
inline static v128_t compare_less(const Simd128Int64 a, const Simd128Int64 b) noexcept { return wasm_i64x2_lt(a.v, b.v); }
inline static v128_t compare_less_equal(const Simd128Int64 a, const Simd128Int64 b) noexcept { return wasm_i64x2_le(a.v, b.v); }
inline static v128_t compare_greater(const Simd128Int64 a, const Simd128Int64 b) noexcept { return wasm_i64x2_gt(a.v, b.v); }
inline static v128_t compare_greater_equal(const Simd128Int64 a, const Simd128Int64 b) noexcept { return wasm_i64x2_ge(a.v, b.v); }
inline static Simd128Int64 blend(const Simd128Int64 if_false, const Simd128Int64 if_true, v128_t mask) noexcept {
	return Simd128Int64(wasm_v128_bitselect(if_true.v, if_false.v, mask));
}

#endif //x86_64 / wasm




/**************************************************************************************************
 * Check that each type implements the desired types from simd-concepts.h
 * (Not sure why this fails on intelisense, but compliles ok.)
 * ************************************************************************************************/
static_assert(Simd<FallbackInt64>, "FallbackInt64 does not implement the concept Simd");
static_assert(SimdSigned<FallbackInt64>, "FallbackInt64 does not implement the concept SimdSigned");
static_assert(SimdInt<FallbackInt64>, "FallbackInt64 does not implement the concept SimdInt");
static_assert(SimdInt64<FallbackInt64>, "FallbackInt64 does not implement the concept SimdInt64");
static_assert(SimdCompareOps<FallbackInt64>, "FallbackInt64 does not implement the concept SimdCompareOps");

#if MT_SIMD_ARCH_X64
static_assert(Simd<Simd128Int64>, "Simd128Int64 does not implement the concept Simd");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(Simd<Simd256Int64>, "Simd256Int64 does not implement the concept Simd");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(Simd<Simd512Int64>, "Simd512Int64 does not implement the concept Simd");
#endif

static_assert(SimdSigned<Simd128Int64>, "Simd128Int64 does not implement the concept SimdSigned");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdSigned<Simd256Int64>, "Simd256Int64 does not implement the concept SimdSigned");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdSigned<Simd512Int64>, "Simd512Int64 does not implement the concept SimdSigned");
#endif

static_assert(SimdInt<Simd128Int64>, "Simd128Int64 does not implement the concept SimdInt");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdInt<Simd256Int64>, "Simd256Int64 does not implement the concept SimdInt");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdInt<Simd512Int64>, "Simd512Int64 does not implement the concept SimdInt");
#endif



static_assert(SimdInt64<Simd128Int64>, "Simd128Int64 does not implement the concept SimdInt64");
static_assert(SimdCompareOps<Simd128Int64>, "Simd128Int64 does not implement the concept SimdCompareOps");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdInt64<Simd256Int64>, "Simd256Int64 does not implement the concept SimdInt64");
static_assert(SimdCompareOps<Simd256Int64>, "Simd256Int64 does not implement the concept SimdCompareOps");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdInt64<Simd512Int64>, "Simd512Int64 does not implement the concept SimdInt64");
static_assert(SimdCompareOps<Simd512Int64>, "Simd512Int64 does not implement the concept SimdCompareOps");
#endif
#endif



/**************************************************************************************************
 Define SimdNativeInt64 as the best supported type at compile time.
*************************************************************************************************/
#if MT_SIMD_ARCH_X64
#if MT_SIMD_ALLOW_LEVEL4_TYPES
typedef Simd512Int64 SimdNativeInt64;
#elif MT_SIMD_ALLOW_LEVEL3_TYPES
typedef Simd256Int64 SimdNativeInt64;
#else
typedef Simd128Int64 SimdNativeInt64;
#endif
#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)
typedef Simd128Int64 SimdNativeInt64;
#else
typedef FallbackInt64 SimdNativeInt64;
#endif

} // namespace mt
