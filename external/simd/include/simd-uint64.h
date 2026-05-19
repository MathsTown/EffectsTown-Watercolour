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

Basic SIMD Types for 64-bit Unsigned Integers:

FallbackUInt64		- Works on all build modes and CPUs.  Forwards most requests to the standard library.

Simd128UInt64		- x86_64 Microarchitecture Level 1 - Works on all x86_64 CPUs.
					- Requires SSE and SSE2 support.  Will use SSE4.1 instructions when __SSE4_1__ or __AVX__ defined.

Simd256UInt64		- x86_64 Microarchitecture Level 3.
					- Requires AVX, AVX2 and FMA support.

Simd512UInt64		- x86_64 Microarchitecture Level 4.
					- Requires AVX512F, AVX512DQ, AVX512VL, AVX512CD, AVX512BW

SimdNativeUInt64	- A Typedef referring to one of the above types.  Chosen based on compiler support/mode.
					- Just use this type in your code if you are building for a specific platform.


Checking CPU Support:
Unless you are using a SimdNative typedef, you must check for CPU support before using any of these types.
- MSVC - You may check at runtime or compile time.  (compile time checks generally results in much faster code)
- GCC/Clang - You must check at compile time (due to compiler limitations)

Types representing floats, doubles, ints, longs etc are arranged in microarchitecture level groups.
Generally CPUs have more SIMD support for floats than ints.
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
- When the compiler detects higher levels of support, such as SSE4.1 (level 2), more optimised instructions may be chosen when available.
- Runtime checks are only meaningful for builds intended to run across mixed CPU capabilities, but separate compilation in recommended.

WASM Support:
I've included FallbackUInt64 for use with Emscripen, but use SimdNativeUInt64 as SIMD support will be added soon.

*********************************************************************************************************/
#pragma once

#include "simd-cpuid.h"
#include "simd-concepts.h"
#include "simd-common.h"
#include "simd-mask.h"
#include "simd-wasm-helpers.h"

#include <stdint.h>
#include <bit>
#include <algorithm>
#include <cstring>

/**************************************************************************************************
* Fallback I64 type.
* Contains 1 x 64bit Unsigned Integers
*
* This is a fallback for cpus that are not yet supported.
*
* It may be useful to evaluate a single value from the same code base, as it offers the same interface
* as the SIMD types.
*
* ************************************************************************************************/
namespace mt {

struct FallbackUInt64 {
	uint64_t v;
	typedef uint64_t F;
	typedef bool MaskType;

	FallbackUInt64() = default;
	FallbackUInt64(uint64_t a) : v(a) {};

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
	static constexpr int size_of_element() { return sizeof(uint64_t); }
	static constexpr int number_of_elements() { return 1; }	
	F element(int) const { return v; }
	void set_element(int, F value) { v = value; }

	//*****Make Functions****
	static FallbackUInt64 make_sequential(uint64_t first) { return FallbackUInt64(first); }
	static FallbackUInt64 make_set1(uint64_t v) { return FallbackUInt64(v); }


	//*****Addition Operators*****
	FallbackUInt64& operator+=(const FallbackUInt64& rhs) noexcept { v += rhs.v; return *this; }
	FallbackUInt64& operator+=(uint64_t rhs) noexcept { v += rhs; return *this; }

	//*****Subtraction Operators*****
	FallbackUInt64& operator-=(const FallbackUInt64& rhs) noexcept { v -= rhs.v; return *this; }
	FallbackUInt64& operator-=(uint64_t rhs) noexcept { v -= rhs; return *this; }

	//*****Multiplication Operators*****
	FallbackUInt64& operator*=(const FallbackUInt64& rhs) noexcept { v *= rhs.v; return *this; }
	FallbackUInt64& operator*=(uint64_t rhs) noexcept { v *= rhs; return *this; }

	//*****Division Operators*****
	FallbackUInt64& operator/=(const FallbackUInt64& rhs) noexcept { v /= rhs.v; return *this; }
	FallbackUInt64& operator/=(uint64_t rhs) noexcept { v /= rhs;	return *this; }

	//*****Bitwise Logic Operators*****
	FallbackUInt64& operator&=(const FallbackUInt64& rhs) noexcept { v &= rhs.v; return *this; }
	FallbackUInt64& operator|=(const FallbackUInt64& rhs) noexcept { v |= rhs.v; return *this; }
	FallbackUInt64& operator^=(const FallbackUInt64& rhs) noexcept { v ^= rhs.v; return *this; }

};

//*****Addition Operators*****
inline static FallbackUInt64 operator+(FallbackUInt64  lhs, const FallbackUInt64& rhs) noexcept { lhs += rhs; return lhs; }


inline static FallbackUInt64 operator+(FallbackUInt64  lhs, uint64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackUInt64 operator+(uint64_t lhs, FallbackUInt64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static FallbackUInt64 operator-(FallbackUInt64  lhs, const FallbackUInt64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackUInt64 operator-(FallbackUInt64  lhs, uint64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackUInt64 operator-(const uint64_t lhs, FallbackUInt64 rhs) noexcept { rhs.v = lhs - rhs.v; return rhs; }

//*****Multiplication Operators*****
inline static FallbackUInt64 operator*(FallbackUInt64  lhs, const FallbackUInt64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackUInt64 operator*(FallbackUInt64  lhs, uint64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackUInt64 operator*(uint64_t lhs, FallbackUInt64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static FallbackUInt64 operator/(FallbackUInt64  lhs, const FallbackUInt64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static FallbackUInt64 operator/(FallbackUInt64  lhs, uint64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static FallbackUInt64 operator/(const uint64_t lhs, FallbackUInt64 rhs) noexcept { rhs.v = lhs / rhs.v; return rhs; }


//*****Bitwise Logic Operators*****
inline static FallbackUInt64 operator&(const FallbackUInt64& lhs, const FallbackUInt64& rhs) noexcept { return FallbackUInt64(lhs.v & rhs.v); }
inline static FallbackUInt64 operator|(const FallbackUInt64& lhs, const FallbackUInt64& rhs) noexcept { return FallbackUInt64(lhs.v | rhs.v); }
inline static FallbackUInt64 operator^(const FallbackUInt64& lhs, const FallbackUInt64& rhs) noexcept { return FallbackUInt64(lhs.v ^ rhs.v); }
inline static FallbackUInt64 operator~(FallbackUInt64 lhs) noexcept { return FallbackUInt64(~lhs.v); }


//*****Shifting Operators*****
inline static FallbackUInt64 operator<<(FallbackUInt64 lhs, int bits) noexcept { lhs.v <<= bits; return lhs; }
inline static FallbackUInt64 operator>>(FallbackUInt64 lhs, int bits) noexcept { lhs.v >>= bits; return lhs; }

inline static FallbackUInt64 rotl(const FallbackUInt64& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 63u;
	if (n == 0u) {
		return a;
	}
	return (a << static_cast<int>(n)) | (a >> static_cast<int>(64u - n));
};
inline static FallbackUInt64 rotr(const FallbackUInt64& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 63u;
	if (n == 0u) {
		return a;
	}
	return (a >> static_cast<int>(n)) | (a << static_cast<int>(64u - n));
};

//*****Min/Max*****
inline static FallbackUInt64 min(FallbackUInt64 a, FallbackUInt64 b) { return FallbackUInt64(std::min(a.v, b.v)); }
inline static FallbackUInt64 max(FallbackUInt64 a, FallbackUInt64 b) { return FallbackUInt64(std::max(a.v, b.v)); }

//*****Conditional Functions*****
inline static bool compare_equal(const FallbackUInt64 a, const FallbackUInt64 b) noexcept { return a.v == b.v; }
inline static bool compare_less(const FallbackUInt64 a, const FallbackUInt64 b) noexcept { return a.v < b.v; }
inline static bool compare_less_equal(const FallbackUInt64 a, const FallbackUInt64 b) noexcept { return a.v <= b.v; }
inline static bool compare_greater(const FallbackUInt64 a, const FallbackUInt64 b) noexcept { return a.v > b.v; }
inline static bool compare_greater_equal(const FallbackUInt64 a, const FallbackUInt64 b) noexcept { return a.v >= b.v; }
inline static FallbackUInt64 blend(const FallbackUInt64 if_false, const FallbackUInt64 if_true, bool mask) noexcept {
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
namespace simd_detail_u64 {
	// Portability layer: GCC/Clang cannot index SIMD lanes via MSVC vector members.
	inline uint64_t lane_get(__m128i v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m128i_u64[i];
#else
		alignas(16) uint64_t lanes[2];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline __m128i lane_set(__m128i v, int i, uint64_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m128i_u64[i] = value;
		return v;
#else
		alignas(16) uint64_t lanes[2];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		lanes[i] = value;
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(lanes));
#endif
	}

	inline uint64_t lane_get(const __m256i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m256i_u64[i];
#else
		alignas(32) uint64_t lanes[4];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline void lane_set(__m256i& v, int i, uint64_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m256i_u64[i] = value;
#else
		alignas(32) uint64_t lanes[4];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		lanes[i] = value;
		v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lanes));
#endif
	}

	inline uint64_t lane_get(const __m512i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m512i_u64[i];
#else
		alignas(64) uint64_t lanes[8];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	inline void lane_set(__m512i& v, int i, uint64_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m512i_u64[i] = value;
#else
		alignas(64) uint64_t lanes[8];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

	// Fallback: integer SIMD divide intrinsics are non-standard across compilers.
	inline __m128i div_epu64(__m128i a, __m128i b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm_div_epu64(a, b);
#else
		alignas(16) uint64_t lhs[2], rhs[2], out[2];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lhs), a);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(rhs), b);
		for (int i = 0; i < 2; ++i) out[i] = lhs[i] / rhs[i];
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(out));
#endif
	}
	inline __m256i div_epu64(const __m256i& a, const __m256i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm256_div_epu64(a, b);
#else
		alignas(32) uint64_t lhs[4], rhs[4], out[4];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lhs), a);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(rhs), b);
		for (int i = 0; i < 4; ++i) out[i] = lhs[i] / rhs[i];
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out));
#endif
	}
	inline __m512i div_epu64(const __m512i& a, const __m512i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm512_div_epu64(a, b);
#else
		alignas(64) uint64_t lhs[8], rhs[8], out[8];
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
 * SIMD 512 type.  Contains 8 x 64bit Unsigned Integers
 * Requires AVX-512F and AVX-512DQ support.
 * ************************************************************************************************/
struct Simd512UInt64 {
	__m512i v;
	typedef uint64_t F;
	typedef __mmask8 MaskType;

	Simd512UInt64() = default;
	Simd512UInt64(__m512i a) : v(a) {};
	Simd512UInt64(F a) : v(_mm512_set1_epi64(a)) {}

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
	F element(int i) const { return mt::simd_detail_u64::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_u64::lane_set(v, i, value); }
	static constexpr int size_of_element() { return sizeof(uint64_t); }
	static constexpr int number_of_elements() { return 8; }

	//*****Make Functions****
	static Simd512UInt64 make_sequential(uint64_t first) { return Simd512UInt64(_mm512_set_epi64(first + 7, first + 6, first + 5, first + 4, first + 3, first + 2, first + 1, first)); }
	static Simd512UInt64 make_set1(uint64_t v) { return Simd512UInt64(_mm512_set1_epi64(static_cast<long long>(v))); }


	//*****Addition Operators*****
	Simd512UInt64& operator+=(const Simd512UInt64& rhs) noexcept { v = _mm512_add_epi64(v, rhs.v); return *this; }
	Simd512UInt64& operator+=(uint64_t rhs) noexcept { v = _mm512_add_epi64(v, _mm512_set1_epi64(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd512UInt64& operator-=(const Simd512UInt64& rhs) noexcept { v = _mm512_sub_epi64(v, rhs.v); return *this; }
	Simd512UInt64& operator-=(uint64_t rhs) noexcept { v = _mm512_sub_epi64(v, _mm512_set1_epi64(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd512UInt64& operator*=(const Simd512UInt64& rhs) noexcept { v = _mm512_mullo_epi64(v, rhs.v); return *this; }
	Simd512UInt64& operator*=(uint64_t rhs) noexcept { v = _mm512_mullo_epi64(v, _mm512_set1_epi64(rhs)); return *this; }

	//*****Division Operators*****
	Simd512UInt64& operator/=(const Simd512UInt64& rhs) noexcept { v = mt::simd_detail_u64::div_epu64(v, rhs.v); return *this; }
	Simd512UInt64& operator/=(uint64_t rhs) noexcept { v = mt::simd_detail_u64::div_epu64(v, _mm512_set1_epi64(rhs));	return *this; }

	//*****Bitwise Logic Operators*****
	Simd512UInt64& operator&=(const Simd512UInt64& rhs) noexcept {v= _mm512_and_si512(v, rhs.v); return *this; }
	Simd512UInt64& operator|=(const Simd512UInt64& rhs) noexcept {v= _mm512_or_si512(v, rhs.v); return *this; }
	Simd512UInt64& operator^=(const Simd512UInt64& rhs) noexcept {v= _mm512_xor_si512(v, rhs.v); return *this; }



};

//*****Addition Operators*****
inline static Simd512UInt64 operator+(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512UInt64 operator+(Simd512UInt64  lhs, uint64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512UInt64 operator+(uint64_t lhs, Simd512UInt64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd512UInt64 operator-(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512UInt64 operator-(Simd512UInt64  lhs, uint64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512UInt64 operator-(const uint64_t lhs, const Simd512UInt64& rhs) noexcept { return Simd512UInt64(_mm512_sub_epi64(_mm512_set1_epi64(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd512UInt64 operator*(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512UInt64 operator*(Simd512UInt64  lhs, uint64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512UInt64 operator*(uint64_t lhs, Simd512UInt64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd512UInt64 operator/(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd512UInt64 operator/(Simd512UInt64  lhs, uint64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd512UInt64 operator/(const uint64_t lhs, const Simd512UInt64& rhs) noexcept { return Simd512UInt64(mt::simd_detail_u64::div_epu64(_mm512_set1_epi64(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd512UInt64 operator&(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd512UInt64 operator|(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd512UInt64 operator^(Simd512UInt64  lhs, const Simd512UInt64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd512UInt64 operator~(const Simd512UInt64& lhs) noexcept { return Simd512UInt64(_mm512_xor_epi64(lhs.v, _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF))); } //No bitwise not in AVX512 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd512UInt64 operator<<(const Simd512UInt64& lhs, int bits) noexcept { return Simd512UInt64(_mm512_slli_epi64(lhs.v, bits)); }
inline static Simd512UInt64 operator>>(const Simd512UInt64& lhs, int bits) noexcept { return Simd512UInt64(_mm512_srli_epi64(lhs.v, bits)); }

inline static Simd512UInt64 rotl(const Simd512UInt64& a, const int bits) noexcept {
	const int n = bits & 63;
	return Simd512UInt64(_mm512_rolv_epi64(a.v, _mm512_set1_epi64(n)));
}
inline static Simd512UInt64 rotr(const Simd512UInt64& a, const int bits) noexcept {
	const int n = bits & 63;
	return Simd512UInt64(_mm512_rorv_epi64(a.v, _mm512_set1_epi64(n)));
}

//*****Min/Max*****
inline static Simd512UInt64 min(Simd512UInt64 a, Simd512UInt64 b) { return Simd512UInt64(_mm512_min_epu64(a.v, b.v)); }
inline static Simd512UInt64 max(Simd512UInt64 a, Simd512UInt64 b) { return Simd512UInt64(_mm512_max_epu64(a.v, b.v)); }

//*****Conditional Functions*****
inline static __mmask8 compare_equal(const Simd512UInt64 a, const Simd512UInt64 b) noexcept {
	return _mm512_cmp_epu64_mask(a.v, b.v, _MM_CMPINT_EQ);
}
inline static __mmask8 compare_less(const Simd512UInt64 a, const Simd512UInt64 b) noexcept {
	return _mm512_cmp_epu64_mask(a.v, b.v, _MM_CMPINT_LT);
}
inline static __mmask8 compare_less_equal(const Simd512UInt64 a, const Simd512UInt64 b) noexcept {
	return _mm512_cmp_epu64_mask(a.v, b.v, _MM_CMPINT_LE);
}
inline static __mmask8 compare_greater(const Simd512UInt64 a, const Simd512UInt64 b) noexcept {
	return _mm512_cmp_epu64_mask(a.v, b.v, _MM_CMPINT_NLE);
}
inline static __mmask8 compare_greater_equal(const Simd512UInt64 a, const Simd512UInt64 b) noexcept {
	return _mm512_cmp_epu64_mask(a.v, b.v, _MM_CMPINT_NLT);
}
inline static Simd512UInt64 blend(const Simd512UInt64 if_false, const Simd512UInt64 if_true, __mmask8 mask) noexcept {
	return Simd512UInt64(_mm512_mask_blend_epi64(mask, if_false.v, if_true.v));
}



#endif // MT_SIMD_ALLOW_LEVEL4_TYPES

#if MT_SIMD_ALLOW_LEVEL3_TYPES
/**************************************************************************************************
 * SIMD 256 type.  Contains 4 x 64bit Unsigned Integers
 * Requires AVX2 support.
 * ************************************************************************************************/
struct Simd256UInt64 {
	__m256i v;

	typedef uint64_t F;
	typedef __m256i MaskType;

	Simd256UInt64() = default;
	Simd256UInt64(__m256i a) : v(a) {};
	Simd256UInt64(F a) : v(_mm256_set1_epi64x(a)) {}

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
	static constexpr int size_of_element() { return sizeof(uint64_t); }
	static constexpr int number_of_elements() { return 4; }	
	F element(int i) const { return mt::simd_detail_u64::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_u64::lane_set(v, i, value); }
	

	//*****Addition Operators*****
	Simd256UInt64& operator+=(const Simd256UInt64& rhs) noexcept {v = _mm256_add_epi64(v, rhs.v); return *this;}
	Simd256UInt64& operator+=(const uint64_t rhs) noexcept {v = _mm256_add_epi64(v, _mm256_set1_epi64x(rhs));	return *this;}

	//*****Subtraction Operators*****
	Simd256UInt64& operator-=(const Simd256UInt64& rhs) noexcept {v = _mm256_sub_epi64(v, rhs.v); return *this;}
	Simd256UInt64& operator-=(const uint64_t rhs) noexcept {v = _mm256_sub_epi64(v, _mm256_set1_epi64x(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd256UInt64& operator*=(const Simd256UInt64& rhs) noexcept {
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

	Simd256UInt64& operator*=(uint64_t rhs) noexcept { *this *= Simd256UInt64(_mm256_set1_epi64x(rhs)); return *this; }

	//*****Division Operators*****
	Simd256UInt64& operator/=(const Simd256UInt64& rhs) noexcept { v = mt::simd_detail_u64::div_epu64(v, rhs.v); return *this; }
	Simd256UInt64& operator/=(uint64_t rhs) noexcept { v = mt::simd_detail_u64::div_epu64(v, _mm256_set1_epi64x(rhs));	return *this; }

	//*****Bitwise Logic Operators*****
	Simd256UInt64& operator&=(const Simd256UInt64& rhs) noexcept {v=_mm256_and_si256(v, rhs.v);return *this;}
	Simd256UInt64& operator|=(const Simd256UInt64& rhs) noexcept {v=_mm256_or_si256(v, rhs.v); return *this;}
	Simd256UInt64& operator^=(const Simd256UInt64&rhs) noexcept {v=_mm256_xor_si256(v, rhs.v);return *this;}

	//*****Make Functions****
	static Simd256UInt64 make_sequential(uint64_t first) noexcept { return Simd256UInt64(_mm256_set_epi64x(first + 3, first + 2, first + 1, first)); }
	static Simd256UInt64 make_set1(uint64_t v) noexcept { return Simd256UInt64(_mm256_set1_epi64x(static_cast<long long>(v))); }
	


};

//*****Addition Operators*****
inline static Simd256UInt64 operator+(Simd256UInt64  lhs, const Simd256UInt64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256UInt64 operator+(Simd256UInt64  lhs, uint64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256UInt64 operator+(uint64_t lhs, Simd256UInt64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd256UInt64 operator-(Simd256UInt64  lhs, const Simd256UInt64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256UInt64 operator-(Simd256UInt64  lhs, uint64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256UInt64 operator-(const uint64_t lhs, const Simd256UInt64&  rhs) noexcept { return Simd256UInt64(_mm256_sub_epi64(_mm256_set1_epi64x(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd256UInt64 operator*(Simd256UInt64  lhs, const Simd256UInt64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256UInt64 operator*(Simd256UInt64  lhs, uint64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256UInt64 operator*(uint64_t lhs, Simd256UInt64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd256UInt64 operator/(Simd256UInt64  lhs, const Simd256UInt64 & rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd256UInt64 operator/(Simd256UInt64  lhs, uint64_t rhs) noexcept {lhs /= rhs; return lhs; }
inline static Simd256UInt64 operator/(const uint64_t lhs, const Simd256UInt64& rhs) noexcept { return Simd256UInt64(mt::simd_detail_u64::div_epu64(_mm256_set1_epi64x(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd256UInt64 operator&(Simd256UInt64  lhs, const Simd256UInt64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd256UInt64 operator|(Simd256UInt64  lhs, const Simd256UInt64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd256UInt64 operator^(Simd256UInt64  lhs, const Simd256UInt64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd256UInt64 operator~(const Simd256UInt64& lhs) noexcept { return Simd256UInt64(_mm256_xor_si256(lhs.v, _mm256_set1_epi64x(0xFFFFFFFFFFFFFFFF))); } //No bitwise not in AVX2 so we use xor with a constant..
	

//*****Shifting Operators*****
inline static Simd256UInt64 operator<<(const Simd256UInt64& lhs, int bits) noexcept { return Simd256UInt64(_mm256_slli_epi64(lhs.v, bits)); }
inline static Simd256UInt64 operator>>(const Simd256UInt64& lhs, int bits) noexcept { return Simd256UInt64(_mm256_srli_epi64(lhs.v, bits)); }
inline static Simd256UInt64 rotl(const Simd256UInt64& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 63u;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd256UInt64(_mm256_rolv_epi64(a.v, _mm256_set1_epi64x(static_cast<int64_t>(n))));
	}
	if (n == 0u) {
		return a;
	}
	return (a << static_cast<int>(n)) | (a >> static_cast<int>(64u - n));
};
inline static Simd256UInt64 rotr(const Simd256UInt64& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 63u;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd256UInt64(_mm256_rorv_epi64(a.v, _mm256_set1_epi64x(static_cast<int64_t>(n))));
	}
	if (n == 0u) {
		return a;
	}
	return (a >> static_cast<int>(n)) | (a << static_cast<int>(64u - n));
};

//*****Min/Max*****
inline static Simd256UInt64 min(Simd256UInt64 a, Simd256UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
		return Simd256UInt64(_mm256_min_epu64(a.v, b.v));
	}
	else {
		const auto sign = _mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000ull));
		const auto a_bias = _mm256_xor_si256(a.v, sign);
		const auto b_bias = _mm256_xor_si256(b.v, sign);
		const auto pick_b = _mm256_cmpgt_epi64(a_bias, b_bias); // a > b (unsigned)
		const auto keep_a = _mm256_xor_si256(pick_b, _mm256_set1_epi64x(-1));
		return Simd256UInt64(_mm256_or_si256(_mm256_and_si256(a.v, keep_a), _mm256_and_si256(b.v, pick_b)));
	}
}
inline static Simd256UInt64 max(Simd256UInt64 a, Simd256UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
		return Simd256UInt64(_mm256_max_epu64(a.v, b.v));
	}
	else {
		const auto sign = _mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000ull));
		const auto a_bias = _mm256_xor_si256(a.v, sign);
		const auto b_bias = _mm256_xor_si256(b.v, sign);
		const auto pick_a = _mm256_cmpgt_epi64(a_bias, b_bias); // a > b (unsigned)
		const auto keep_b = _mm256_xor_si256(pick_a, _mm256_set1_epi64x(-1));
		return Simd256UInt64(_mm256_or_si256(_mm256_and_si256(a.v, pick_a), _mm256_and_si256(b.v, keep_b)));
	}
}

//*****Conditional Functions*****
inline static __m256i compare_equal(const Simd256UInt64 a, const Simd256UInt64 b) noexcept { return _mm256_cmpeq_epi64(a.v, b.v); }
inline static __m256i compare_greater(const Simd256UInt64 a, const Simd256UInt64 b) noexcept {
	const auto sign = _mm256_set1_epi64x(static_cast<int64_t>(0x8000000000000000ull));
	const auto a_bias = _mm256_xor_si256(a.v, sign);
	const auto b_bias = _mm256_xor_si256(b.v, sign);
	return _mm256_cmpgt_epi64(a_bias, b_bias);
}
inline static __m256i compare_less(const Simd256UInt64 a, const Simd256UInt64 b) noexcept { return compare_greater(b, a); }
inline static __m256i compare_less_equal(const Simd256UInt64 a, const Simd256UInt64 b) noexcept {
	return _mm256_xor_si256(compare_greater(a, b), _mm256_set1_epi64x(-1));
}
inline static __m256i compare_greater_equal(const Simd256UInt64 a, const Simd256UInt64 b) noexcept {
	return _mm256_xor_si256(compare_greater(b, a), _mm256_set1_epi64x(-1));
}
inline static Simd256UInt64 blend(const Simd256UInt64 if_false, const Simd256UInt64 if_true, __m256i mask) noexcept {
	return Simd256UInt64(_mm256_blendv_epi8(if_false.v, if_true.v, mask));
}






#endif // MT_SIMD_ALLOW_LEVEL3_TYPES

/**************************************************************************************************
 * SIMD 128 type.  Contains 4 x 64bit Unsigned Integers
 * Requires SSE support.
 * 
 * //BROKEN:  CURRENTLY REQUIRES SSE`4.1
 * ************************************************************************************************/
struct Simd128UInt64 {
	__m128i v;

	typedef uint64_t F;
	typedef __m128i MaskType;

	Simd128UInt64() = default;
	Simd128UInt64(__m128i a) : v(a) {};
	Simd128UInt64(F a) : v(_mm_set1_epi64x(a)) {}

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


	static constexpr int size_of_element() { return sizeof(uint64_t); }
	static constexpr int number_of_elements() { return 2; }

	//*****Elements*****
	F element(int i) const { return mt::simd_detail_u64::lane_get(v, i); }
	void set_element(int i, F value) { v = mt::simd_detail_u64::lane_set(v, i, value); }


	//*****Addition Operators*****
	Simd128UInt64& operator+=(const Simd128UInt64& rhs) noexcept { v = _mm_add_epi64(v, rhs.v); return *this; } //sse2
	Simd128UInt64& operator+=(const uint64_t rhs) noexcept { v = _mm_add_epi64(v, _mm_set1_epi64x(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd128UInt64& operator-=(const Simd128UInt64& rhs) noexcept { v = _mm_sub_epi64(v, rhs.v); return *this; } //sse2
	Simd128UInt64& operator-=(const uint64_t rhs) noexcept { v = _mm_sub_epi64(v, _mm_set1_epi64x(rhs));	return *this; }



	//*****Multiplication Operators*****
	Simd128UInt64& operator*=(const Simd128UInt64& rhs) noexcept {
		if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
			//There is a slight chance a user is using this struct but has compiled the code with avx512dq and avx512vl enabled.
			this->v = _mm_mullo_epi64(v, rhs.v);
			return *this;
		} 
		else {
			//Not supported, we will just unroll as there are only 2 values anyway.			
			const auto m1 = element(1) * rhs.element(1);
			const auto m0 = element(0) * rhs.element(0);
			v = _mm_set_epi64x(m1, m0);			
			return *this;
		}

	}

	Simd128UInt64& operator*=(uint64_t rhs) noexcept { *this *= Simd128UInt64(_mm_set1_epi64x(rhs)); return *this; }

	//*****Division Operators*****
	Simd128UInt64& operator/=(const Simd128UInt64& rhs) noexcept { v = mt::simd_detail_u64::div_epu64(v, rhs.v); return *this; } //sse
	Simd128UInt64& operator/=(uint64_t rhs) noexcept { v = mt::simd_detail_u64::div_epu64(v, _mm_set1_epi64x(rhs));	return *this; }

	//*****Bitwise Logic Operators*****
	Simd128UInt64& operator&=(const Simd128UInt64& rhs) noexcept { v = _mm_and_si128(v, rhs.v); return *this; } //sse2
	Simd128UInt64& operator|=(const Simd128UInt64& rhs) noexcept { v = _mm_or_si128(v, rhs.v); return *this; }
	Simd128UInt64& operator^=(const Simd128UInt64& rhs) noexcept { v = _mm_xor_si128(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd128UInt64 make_sequential(uint64_t first) noexcept { return Simd128UInt64(_mm_set_epi64x(first + 1, first)); }
	static Simd128UInt64 make_set1(uint64_t v) noexcept { return Simd128UInt64(_mm_set1_epi64x(static_cast<long long>(v))); }

	private:

    //32-bit mullo multiply using only sse2
	__m128i software_mullo_epu32(__m128i a, __m128i b) {
		auto result02 = _mm_mul_epu32(a, b);  //Multiply words 0 and 2.  
		auto result13 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));  //Multiply words 1 and 3, by shifting them into 0,2.
		return  _mm_unpacklo_epi32(_mm_shuffle_epi32(result02, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(result13, _MM_SHUFFLE(0, 0, 2, 0))); // shuffle and pack
	}

};

//*****Addition Operators*****
inline static Simd128UInt64 operator+(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt64 operator+(Simd128UInt64  lhs, uint64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt64 operator+(uint64_t lhs, Simd128UInt64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd128UInt64 operator-(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt64 operator-(Simd128UInt64  lhs, uint64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt64 operator-(const uint64_t lhs, const Simd128UInt64& rhs) noexcept { return Simd128UInt64(_mm_sub_epi64(_mm_set1_epi64x(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd128UInt64 operator*(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt64 operator*(Simd128UInt64  lhs, uint64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt64 operator*(uint64_t lhs, Simd128UInt64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd128UInt64 operator/(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd128UInt64 operator/(Simd128UInt64  lhs, uint64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128UInt64 operator/(const uint64_t lhs, const Simd128UInt64& rhs) noexcept { return Simd128UInt64(mt::simd_detail_u64::div_epu64(_mm_set1_epi64x(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd128UInt64 operator&(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128UInt64 operator|(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128UInt64 operator^(Simd128UInt64  lhs, const Simd128UInt64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128UInt64 operator~(const Simd128UInt64& lhs) noexcept { return Simd128UInt64(_mm_xor_si128(lhs.v, _mm_set1_epi64x(0xFFFFFFFFFFFFFFFF))); } 


//*****Shifting Operators*****
inline static Simd128UInt64 operator<<(const Simd128UInt64& lhs, int bits) noexcept { return Simd128UInt64(_mm_slli_epi64(lhs.v, bits)); } //sse2
inline static Simd128UInt64 operator>>(const Simd128UInt64& lhs, int bits) noexcept { return Simd128UInt64(_mm_srli_epi64(lhs.v, bits)); }

inline static Simd128UInt64 rotl(const Simd128UInt64& a, int bits) {
	const int n = bits & 63;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd128UInt64(_mm_rolv_epi64(a.v, _mm_set1_epi64x(n)));
	}
	else {
		if (n == 0) {
			return a;
		}
		return (a << n) | (a >> (64 - n));
	}
};


inline static Simd128UInt64 rotr(const Simd128UInt64& a, int bits) {
	const int n = bits & 63;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd128UInt64(_mm_rorv_epi64(a.v, _mm_set1_epi64x(n)));
	}
	else {
		if (n == 0) {
			return a;
		}
		return (a >> n) | (a << (64 - n));
	}
};

//*****Min/Max*****
inline static Simd128UInt64 min(Simd128UInt64 a, Simd128UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
		return Simd128UInt64(_mm_min_epu64(a.v, b.v)); //AVX-512
	}
	else if constexpr (mt::environment::compiler_has_sse4_2) {
		const auto sign = _mm_set1_epi64x(static_cast<int64_t>(0x8000000000000000ull));
		const auto a_bias = _mm_xor_si128(a.v, sign);
		const auto b_bias = _mm_xor_si128(b.v, sign);
		const auto pick_b = _mm_cmpgt_epi64(a_bias, b_bias); // a > b (unsigned)
		const auto keep_a = _mm_xor_si128(pick_b, _mm_set1_epi64x(-1));
		return Simd128UInt64(_mm_or_si128(_mm_and_si128(a.v, keep_a), _mm_and_si128(b.v, pick_b)));
	}
	else {
		//No min/max or compare for unsigned ints in SSE2 so we will just unroll.
		auto m1 = std::min(mt::simd_detail_u64::lane_get(a.v, 1), mt::simd_detail_u64::lane_get(b.v, 1));
		auto m0 = std::min(mt::simd_detail_u64::lane_get(a.v, 0), mt::simd_detail_u64::lane_get(b.v, 0));
		return Simd128UInt64(_mm_set_epi64x(m1, m0));
	}
}

	
inline static Simd128UInt64 max(Simd128UInt64 a, Simd128UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512dq) {
		return Simd128UInt64(_mm_max_epu64(a.v, b.v)); //AVX-512
	}
	else if constexpr (mt::environment::compiler_has_sse4_2) {
		const auto sign = _mm_set1_epi64x(static_cast<int64_t>(0x8000000000000000ull));
		const auto a_bias = _mm_xor_si128(a.v, sign);
		const auto b_bias = _mm_xor_si128(b.v, sign);
		const auto pick_a = _mm_cmpgt_epi64(a_bias, b_bias); // a > b (unsigned)
		const auto keep_b = _mm_xor_si128(pick_a, _mm_set1_epi64x(-1));
		return Simd128UInt64(_mm_or_si128(_mm_and_si128(a.v, pick_a), _mm_and_si128(b.v, keep_b)));
	}
	else {
		//No min/max or compare for unsigned ints in SSE2 so we will just unroll.
		auto m1 = std::max(mt::simd_detail_u64::lane_get(a.v, 1), mt::simd_detail_u64::lane_get(b.v, 1));
		auto m0 = std::max(mt::simd_detail_u64::lane_get(a.v, 0), mt::simd_detail_u64::lane_get(b.v, 0));
		return Simd128UInt64(_mm_set_epi64x(m1, m0));
	}
}

//*****Conditional Functions*****
inline static __m128i compare_equal(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return _mm_cmpeq_epi64(a.v, b.v);
	}
	else {
		const int64_t m1 = (a.element(1) == b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) == b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_greater(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		const auto sign = _mm_set1_epi64x(static_cast<int64_t>(0x8000000000000000ull));
		const auto a_bias = _mm_xor_si128(a.v, sign);
		const auto b_bias = _mm_xor_si128(b.v, sign);
		return _mm_cmpgt_epi64(a_bias, b_bias);
	}
	else {
		const int64_t m1 = (a.element(1) > b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) > b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_less(const Simd128UInt64 a, const Simd128UInt64 b) noexcept { return compare_greater(b, a); }
inline static __m128i compare_less_equal(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		return _mm_xor_si128(compare_greater(a, b), _mm_set1_epi64x(-1));
	}
	else {
		const int64_t m1 = (a.element(1) <= b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) <= b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static __m128i compare_greater_equal(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_2) {
		return _mm_xor_si128(compare_greater(b, a), _mm_set1_epi64x(-1));
	}
	else {
		const int64_t m1 = (a.element(1) >= b.element(1)) ? -1ll : 0ll;
		const int64_t m0 = (a.element(0) >= b.element(0)) ? -1ll : 0ll;
		return _mm_set_epi64x(m1, m0);
	}
}
inline static Simd128UInt64 blend(const Simd128UInt64 if_false, const Simd128UInt64 if_true, __m128i mask) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128UInt64(_mm_blendv_epi8(if_false.v, if_true.v, mask));
	}
	else {
		return Simd128UInt64(_mm_or_si128(_mm_andnot_si128(mask, if_false.v), _mm_and_si128(mask, if_true.v)));
	}
}


#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)

struct Simd128UInt64 {
	v128_t v;
	typedef uint64_t F;
	typedef v128_t MaskType;

	Simd128UInt64() = default;
	Simd128UInt64(v128_t a) : v(a) {}
	Simd128UInt64(F a) : v(mt::simd_wasm_detail::splat<uint64_t>(a)) {}

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

	static constexpr int size_of_element() { return sizeof(uint64_t); }
	static constexpr int number_of_elements() { return 2; }
	F element(int i) const { return mt::simd_wasm_detail::lane_get<uint64_t, 2>(v, i); }
	void set_element(int i, F value) { v = mt::simd_wasm_detail::lane_set<uint64_t, 2>(v, i, value); }
	static Simd128UInt64 make_sequential(uint64_t first) noexcept { return Simd128UInt64(mt::simd_wasm_detail::make_sequential<uint64_t, 2>(first)); }
	static Simd128UInt64 make_set1(uint64_t value) noexcept { return Simd128UInt64(mt::simd_wasm_detail::splat<uint64_t>(value)); }

	Simd128UInt64& operator+=(const Simd128UInt64& rhs) noexcept { v = wasm_i64x2_add(v, rhs.v); return *this; }
	Simd128UInt64& operator+=(uint64_t rhs) noexcept { v = wasm_i64x2_add(v, make_set1(rhs).v); return *this; }
	Simd128UInt64& operator-=(const Simd128UInt64& rhs) noexcept { v = wasm_i64x2_sub(v, rhs.v); return *this; }
	Simd128UInt64& operator-=(uint64_t rhs) noexcept { v = wasm_i64x2_sub(v, make_set1(rhs).v); return *this; }
	Simd128UInt64& operator*=(const Simd128UInt64& rhs) noexcept { v = wasm_i64x2_mul(v, rhs.v); return *this; }
	Simd128UInt64& operator*=(uint64_t rhs) noexcept { v = wasm_i64x2_mul(v, make_set1(rhs).v); return *this; }
	Simd128UInt64& operator/=(const Simd128UInt64& rhs) noexcept {
		v = mt::simd_wasm_detail::map_binary<uint64_t, 2>(v, rhs.v, [](uint64_t a, uint64_t b) { return a / b; });
		return *this;
	}
	Simd128UInt64& operator/=(uint64_t rhs) noexcept { return *this /= make_set1(rhs); }
	Simd128UInt64& operator&=(const Simd128UInt64& rhs) noexcept { v = wasm_v128_and(v, rhs.v); return *this; }
	Simd128UInt64& operator|=(const Simd128UInt64& rhs) noexcept { v = wasm_v128_or(v, rhs.v); return *this; }
	Simd128UInt64& operator^=(const Simd128UInt64& rhs) noexcept { v = wasm_v128_xor(v, rhs.v); return *this; }
};

inline static Simd128UInt64 operator+(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt64 operator+(Simd128UInt64 lhs, uint64_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt64 operator+(uint64_t lhs, Simd128UInt64 rhs) noexcept { rhs += lhs; return rhs; }
inline static Simd128UInt64 operator-(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt64 operator-(Simd128UInt64 lhs, uint64_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt64 operator-(uint64_t lhs, const Simd128UInt64& rhs) noexcept { return Simd128UInt64::make_set1(lhs) - rhs; }
inline static Simd128UInt64 operator*(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt64 operator*(Simd128UInt64 lhs, uint64_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt64 operator*(uint64_t lhs, Simd128UInt64 rhs) noexcept { rhs *= lhs; return rhs; }
inline static Simd128UInt64 operator/(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128UInt64 operator/(Simd128UInt64 lhs, uint64_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128UInt64 operator/(uint64_t lhs, const Simd128UInt64& rhs) noexcept { return Simd128UInt64::make_set1(lhs) / rhs; }
inline static Simd128UInt64 operator&(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128UInt64 operator|(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128UInt64 operator^(Simd128UInt64 lhs, const Simd128UInt64& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128UInt64 operator~(const Simd128UInt64& lhs) noexcept { return Simd128UInt64(wasm_v128_not(lhs.v)); }
inline static Simd128UInt64 operator<<(const Simd128UInt64& lhs, int bits) noexcept { return Simd128UInt64(wasm_i64x2_shl(lhs.v, static_cast<uint32_t>(bits))); }
inline static Simd128UInt64 operator>>(const Simd128UInt64& lhs, int bits) noexcept { return Simd128UInt64(wasm_u64x2_shr(lhs.v, static_cast<uint32_t>(bits))); }

inline static Simd128UInt64 rotl(const Simd128UInt64& a, int bits) {
	const int n = bits & 63;
	if (n == 0) {
		return a;
	}
	return (a << n) | (a >> (64 - n));
}

inline static Simd128UInt64 rotr(const Simd128UInt64& a, int bits) {
	const int n = bits & 63;
	if (n == 0) {
		return a;
	}
	return (a >> n) | (a << (64 - n));
}

inline static Simd128UInt64 min(Simd128UInt64 a, Simd128UInt64 b) noexcept {
	return Simd128UInt64(mt::simd_wasm_detail::map_binary<uint64_t, 2>(a.v, b.v, [](uint64_t x, uint64_t y) { return std::min(x, y); }));
}
inline static Simd128UInt64 max(Simd128UInt64 a, Simd128UInt64 b) noexcept {
	return Simd128UInt64(mt::simd_wasm_detail::map_binary<uint64_t, 2>(a.v, b.v, [](uint64_t x, uint64_t y) { return std::max(x, y); }));
}

inline static v128_t compare_equal(const Simd128UInt64 a, const Simd128UInt64 b) noexcept { return wasm_i64x2_eq(a.v, b.v); }
inline static v128_t bias_for_unsigned_compare() noexcept {
	return mt::simd_wasm_detail::splat<uint64_t>(0x8000000000000000ull);
}
inline static v128_t compare_greater(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	const v128_t bias = bias_for_unsigned_compare();
	return wasm_i64x2_gt(wasm_v128_xor(a.v, bias), wasm_v128_xor(b.v, bias));
}
inline static v128_t compare_less(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	const v128_t bias = bias_for_unsigned_compare();
	return wasm_i64x2_lt(wasm_v128_xor(a.v, bias), wasm_v128_xor(b.v, bias));
}
inline static v128_t compare_less_equal(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	const v128_t bias = bias_for_unsigned_compare();
	return wasm_i64x2_le(wasm_v128_xor(a.v, bias), wasm_v128_xor(b.v, bias));
}
inline static v128_t compare_greater_equal(const Simd128UInt64 a, const Simd128UInt64 b) noexcept {
	const v128_t bias = bias_for_unsigned_compare();
	return wasm_i64x2_ge(wasm_v128_xor(a.v, bias), wasm_v128_xor(b.v, bias));
}
inline static Simd128UInt64 blend(const Simd128UInt64 if_false, const Simd128UInt64 if_true, v128_t mask) noexcept {
	return Simd128UInt64(wasm_v128_bitselect(if_true.v, if_false.v, mask));
}

#endif //x86_64 / wasm


/**************************************************************************************************
 * Check that each type implements the desired types from simd-concepts.h
 * ************************************************************************************************/
static_assert(Simd<FallbackUInt64>, "FallbackUInt64 does not implement the concept Simd");
static_assert(SimdUInt<FallbackUInt64>, "FallbackUInt64 does not implement the concept SimdUInt");
static_assert(SimdUInt64<FallbackUInt64>, "FallbackUInt64 does not implement the concept SimdUInt64");
static_assert(SimdCompareOps<FallbackUInt64>, "FallbackUInt64 does not implement the concept SimdCompareOps");


#if MT_SIMD_ARCH_X64
static_assert(Simd<Simd128UInt64>, "Simd128UInt64 does not implement the concept Simd");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(Simd<Simd256UInt64>, "Simd256UInt64 does not implement the concept Simd");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(Simd<Simd512UInt64>, "Simd512UInt64 does not implement the concept Simd");
#endif

static_assert(SimdUInt<Simd128UInt64>, "Simd128UInt64 does not implement the concept SimdUInt");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdUInt<Simd256UInt64>, "Simd256UInt64 does not implement the concept SimdUInt");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdUInt<Simd512UInt64>, "Simd512UInt64 does not implement the concept SimdUInt");
#endif

static_assert(SimdUInt64<Simd128UInt64>, "Simd128UInt64 does not implement the concept SimdUInt64");
static_assert(SimdCompareOps<Simd128UInt64>, "Simd128UInt64 does not implement the concept SimdCompareOps");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdUInt64<Simd256UInt64>, "Simd256UInt64 does not implement the concept SimdUInt64");
static_assert(SimdCompareOps<Simd256UInt64>, "Simd256UInt64 does not implement the concept SimdCompareOps");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdUInt64<Simd512UInt64>, "Simd512UInt64 does not implement the concept SimdUInt64");
static_assert(SimdCompareOps<Simd512UInt64>, "Simd512UInt64 does not implement the concept SimdCompareOps");
#endif
#endif


/**************************************************************************************************
 Define SimdNativeUInt64 as the best supported type at compile time.
*************************************************************************************************/
#if MT_SIMD_ARCH_X64
	#if MT_SIMD_ALLOW_LEVEL4_TYPES
		typedef Simd512UInt64 SimdNativeUInt64;
	#elif MT_SIMD_ALLOW_LEVEL3_TYPES
		typedef Simd256UInt64 SimdNativeUInt64;
	#else
		typedef Simd128UInt64 SimdNativeUInt64;
	#endif
#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)
typedef Simd128UInt64 SimdNativeUInt64;
#else
typedef FallbackUInt64 SimdNativeUInt64;
#endif

} // namespace mt
