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

Basic SIMD Types for 32-bit Unsigned Integers:

FallbackUInt32		- Works on all build modes and CPUs.  Forwards most requests to the standard library.

Simd128UInt32		- x86_64 Microarchitecture Level 1 - Works on all x86_64 CPUs.
					- Requires SSE and SSE2 support.  Will use SSE4.1 instructions when __SSE4_1__ or __AVX__ defined.

Simd256UInt32		- x86_64 Microarchitecture Level 3.
					- Requires AVX, AVX2 and FMA support.

Simd512UInt32		- x86_64 Microarchitecture Level 4.
					- Requires AVX512F, AVX512DQ, AVX512VL, AVX512CD, AVX512BW

SimdNativeUInt32	- A Typedef referring to one of the above types.  Chosen based on compiler support/mode.
					- Just use this type in your code if you are building for a specific platform.


Checking CPU Support:
Unless you are using a SimdNative typedef, you must check for CPU support before using any of these types.
- MSVC - You may check at runtime or compile time.  (compile time checks generally results in much faster code)
- GCC/Clang - You must check at compile time (due to compiler limitations)

Types representing floats, doubles, ints, longs etc are arranged in microarchitecture level groups.
Generally CPUs have more SIMD support for floats than ints (and 32 bit is better than 64-bit).
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
I've included FallbackUInt32 for use with Emscripen, but use SimdNativeUInt32 as SIMD support will be added soon.


*********************************************************************************************************/
#pragma once


#include <stdint.h>
#include <algorithm>
#include <cstring>
#include "simd-cpuid.h"
#include "simd-concepts.h"
#include "simd-mask.h"
#include "simd-wasm-helpers.h"

/**************************************************************************************************
* Fallback I32 type.
* Contains 1 x 32bit Unsigned Integers
*
* This is a fallback for cpus that are not yet supported.
*
* It may be useful to evaluate a single value from the same code base, as it offers the same interface
* as the SIMD types.
*
* ************************************************************************************************/
struct FallbackUInt32 {
	uint32_t v;
	typedef uint32_t F;
	typedef bool MaskType;
	FallbackUInt32() = default;
	FallbackUInt32(uint32_t a) : v(a) {};

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
	static constexpr int size_of_element() { return sizeof(uint32_t); }
	static constexpr int number_of_elements() { return 1; }
	F element(int) const { return v; }
	void set_element(int, F value) { v = value; }

	//*****Make Functions****
	static FallbackUInt32 make_sequential(uint32_t first) { return FallbackUInt32(first); }
	static FallbackUInt32 make_set1(uint32_t v) { return FallbackUInt32(v); }


	//*****Addition Operators*****
	FallbackUInt32& operator+=(const FallbackUInt32& rhs) noexcept { v += rhs.v; return *this; }
	FallbackUInt32& operator+=(uint32_t rhs) noexcept { v += rhs; return *this; }

	//*****Subtraction Operators*****
	FallbackUInt32& operator-=(const FallbackUInt32& rhs) noexcept { v -= rhs.v; return *this; }
	FallbackUInt32& operator-=(uint32_t rhs) noexcept { v -= rhs; return *this; }

	//*****Multiplication Operators*****
	FallbackUInt32& operator*=(const FallbackUInt32& rhs) noexcept { v *= rhs.v; return *this; }
	FallbackUInt32& operator*=(uint32_t rhs) noexcept { v *= rhs; return *this; }

	//*****Division Operators*****
	FallbackUInt32& operator/=(const FallbackUInt32& rhs) noexcept { v /= rhs.v; return *this; }
	FallbackUInt32& operator/=(uint32_t rhs) noexcept { v /= rhs;	return *this; }

	//*****Bitwise Logic Operators*****
	FallbackUInt32& operator&=(const FallbackUInt32& rhs) noexcept { v &= rhs.v; return *this; }
	FallbackUInt32& operator|=(const FallbackUInt32& rhs) noexcept { v |= rhs.v; return *this; }
	FallbackUInt32& operator^=(const FallbackUInt32& rhs) noexcept { v ^= rhs.v; return *this; }

	//*****Mathematical*****

};

//*****Addition Operators*****
inline static FallbackUInt32 operator+(FallbackUInt32  lhs, const FallbackUInt32& rhs) noexcept { lhs += rhs; return lhs; }


inline static FallbackUInt32 operator+(FallbackUInt32  lhs, uint32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackUInt32 operator+(uint32_t lhs, FallbackUInt32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static FallbackUInt32 operator-(FallbackUInt32  lhs, const FallbackUInt32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackUInt32 operator-(FallbackUInt32  lhs, uint32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackUInt32 operator-(const uint32_t lhs, FallbackUInt32 rhs) noexcept { rhs.v = lhs - rhs.v; return rhs; }

//*****Multiplication Operators*****
inline static FallbackUInt32 operator*(FallbackUInt32  lhs, const FallbackUInt32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackUInt32 operator*(FallbackUInt32  lhs, uint32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackUInt32 operator*(uint32_t lhs, FallbackUInt32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static FallbackUInt32 operator/(FallbackUInt32  lhs, const FallbackUInt32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static FallbackUInt32 operator/(FallbackUInt32  lhs, uint32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static FallbackUInt32 operator/(const uint32_t lhs, FallbackUInt32 rhs) noexcept { rhs.v = lhs / rhs.v; return rhs; }


//*****Bitwise Logic Operators*****
inline static FallbackUInt32 operator&(const FallbackUInt32& lhs, const FallbackUInt32& rhs) noexcept { return FallbackUInt32(lhs.v & rhs.v); }
inline static FallbackUInt32 operator|(const FallbackUInt32& lhs, const FallbackUInt32& rhs) noexcept { return FallbackUInt32(lhs.v | rhs.v); }
inline static FallbackUInt32 operator^(const FallbackUInt32& lhs, const FallbackUInt32& rhs) noexcept { return FallbackUInt32(lhs.v ^ rhs.v); }
inline static FallbackUInt32 operator~(FallbackUInt32 lhs) noexcept { return FallbackUInt32(~lhs.v); }


//*****Shifting Operators*****
inline static FallbackUInt32 operator<<(FallbackUInt32 lhs, int bits) noexcept { lhs.v <<= bits; return lhs; }
inline static FallbackUInt32 operator>>(FallbackUInt32 lhs, int bits) noexcept { lhs.v >>= bits; return lhs; }
inline static FallbackUInt32 rotl(const FallbackUInt32& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 31u;
	if (n == 0u) {
		return a;
	}
	return (a << static_cast<int>(n)) | (a >> static_cast<int>(32u - n));
};
inline static FallbackUInt32 rotr(const FallbackUInt32& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 31u;
	if (n == 0u) {
		return a;
	}
	return (a >> static_cast<int>(n)) | (a << static_cast<int>(32u - n));
}

//*****Min/Max*****
inline static FallbackUInt32 min(FallbackUInt32 a, FallbackUInt32 b) { return FallbackUInt32(std::min(a.v, b.v)); }
inline static FallbackUInt32 max(FallbackUInt32 a, FallbackUInt32 b) { return FallbackUInt32(std::max(a.v, b.v)); }

//*****Conditional Functions*****
inline static bool compare_equal(const FallbackUInt32 a, const FallbackUInt32 b) noexcept { return a.v == b.v; }
inline static bool compare_less(const FallbackUInt32 a, const FallbackUInt32 b) noexcept { return a.v < b.v; }
inline static bool compare_less_equal(const FallbackUInt32 a, const FallbackUInt32 b) noexcept { return a.v <= b.v; }
inline static bool compare_greater(const FallbackUInt32 a, const FallbackUInt32 b) noexcept { return a.v > b.v; }
inline static bool compare_greater_equal(const FallbackUInt32 a, const FallbackUInt32 b) noexcept { return a.v >= b.v; }
inline static FallbackUInt32 blend(const FallbackUInt32 if_false, const FallbackUInt32 if_true, bool mask) noexcept {
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
namespace mt::simd_detail_u32 {
	// Portability layer: GCC/Clang cannot index SIMD lanes via MSVC vector members.
	inline uint32_t lane_get(__m128i v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m128i_u32[i];
#else
		alignas(16) uint32_t lanes[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline __m128i lane_set(__m128i v, int i, uint32_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m128i_u32[i] = value;
		return v;
#else
		alignas(16) uint32_t lanes[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		lanes[i] = value;
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(lanes));
#endif
	}

	inline uint32_t lane_get(const __m256i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m256i_u32[i];
#else
		alignas(32) uint32_t lanes[8];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline void lane_set(__m256i& v, int i, uint32_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m256i_u32[i] = value;
#else
		alignas(32) uint32_t lanes[8];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		lanes[i] = value;
		v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lanes));
#endif
	}

	inline uint32_t lane_get(const __m512i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m512i_u32[i];
#else
		alignas(64) uint32_t lanes[16];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	inline void lane_set(__m512i& v, int i, uint32_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m512i_u32[i] = value;
#else
		alignas(64) uint32_t lanes[16];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

	// Fallback: integer SIMD divide intrinsics are non-standard across compilers.
	inline __m128i div_epu32(__m128i a, __m128i b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm_div_epu32(a, b);
#else
		alignas(16) uint32_t lhs[4];
		alignas(16) uint32_t rhs[4];
		alignas(16) uint32_t out[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lhs), a);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(rhs), b);
		for (int i = 0; i < 4; ++i) out[i] = lhs[i] / rhs[i];
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(out));
#endif
	}
	inline __m256i div_epu32(const __m256i& a, const __m256i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm256_div_epu32(a, b);
#else
		alignas(32) uint32_t lhs[8];
		alignas(32) uint32_t rhs[8];
		alignas(32) uint32_t out[8];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lhs), a);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(rhs), b);
		for (int i = 0; i < 8; ++i) out[i] = lhs[i] / rhs[i];
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out));
#endif
	}
	inline __m512i div_epu32(const __m512i& a, const __m512i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm512_div_epu32(a, b);
#else
		alignas(64) uint32_t lhs[16];
		alignas(64) uint32_t rhs[16];
		alignas(64) uint32_t out[16];
		std::memcpy(lhs, &a, sizeof(a));
		std::memcpy(rhs, &b, sizeof(b));
		for (int i = 0; i < 16; ++i) out[i] = lhs[i] / rhs[i];
		__m512i out_v;
		std::memcpy(&out_v, out, sizeof(out_v));
		return out_v;
#endif
	}
}



#if MT_SIMD_ALLOW_LEVEL4_TYPES
/**************************************************************************************************
 * SIMD 512 type.  Contains 16 x 32bit Unsigned Integers
 * Requires AVX-512F 
 * ************************************************************************************************/
struct Simd512UInt32 {
	__m512i v;
	typedef uint32_t F;
	typedef __mmask16 MaskType;

	Simd512UInt32() = default;
	Simd512UInt32(__m512i a) : v(a) {};
	Simd512UInt32(F a) : v(_mm512_set1_epi32(a)) {};

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
	static constexpr int size_of_element() { return sizeof(uint32_t); }
	static constexpr int number_of_elements() { return 16; }	
	F element(int i) const { return mt::simd_detail_u32::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_u32::lane_set(v, i, value); }

	//*****Make Functions****
	static Simd512UInt32 make_sequential(uint32_t first) { return Simd512UInt32(_mm512_set_epi32(first + 15, first + 14, first + 13, first + 12, first + 11, first + 10, first + 9, first + 8, first + 7, first + 6, first + 5, first + 4, first + 3, first + 2, first + 1, first)); }
	static Simd512UInt32 make_set1(uint32_t v) { return Simd512UInt32(_mm512_set1_epi32(v)); }


	//*****Addition Operators*****
	Simd512UInt32& operator+=(const Simd512UInt32& rhs) noexcept { v = _mm512_add_epi32(v, rhs.v); return *this; }
	Simd512UInt32& operator+=(uint32_t rhs) noexcept { v = _mm512_add_epi32(v, _mm512_set1_epi32(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd512UInt32& operator-=(const Simd512UInt32& rhs) noexcept { v = _mm512_sub_epi32(v, rhs.v); return *this; }
	Simd512UInt32& operator-=(uint32_t rhs) noexcept { v = _mm512_sub_epi32(v, _mm512_set1_epi32(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd512UInt32& operator*=(const Simd512UInt32& rhs) noexcept { v = _mm512_mullo_epi32(v, rhs.v); return *this; }
	Simd512UInt32& operator*=(uint32_t rhs) noexcept { v = _mm512_mullo_epi32(v, _mm512_set1_epi32(rhs)); return *this; }

	//*****Division Operators*****
	Simd512UInt32& operator/=(const Simd512UInt32& rhs) noexcept { v = mt::simd_detail_u32::div_epu32(v, rhs.v); return *this; }
	Simd512UInt32& operator/=(uint32_t rhs) noexcept { v = mt::simd_detail_u32::div_epu32(v, _mm512_set1_epi32(rhs));	return *this; }

	//*****Bitwise Logic Operators*****
	Simd512UInt32& operator&=(const Simd512UInt32& rhs) noexcept { v = _mm512_and_si512(v, rhs.v); return *this; }
	Simd512UInt32& operator|=(const Simd512UInt32& rhs) noexcept { v = _mm512_or_si512(v, rhs.v); return *this; }
	Simd512UInt32& operator^=(const Simd512UInt32& rhs) noexcept { v = _mm512_xor_si512(v, rhs.v); return *this; }

	//*****Mathematical*****

	
	
};

//*****Addition Operators*****
inline static Simd512UInt32 operator+(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512UInt32 operator+(Simd512UInt32  lhs, uint32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512UInt32 operator+(uint32_t lhs, Simd512UInt32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd512UInt32 operator-(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512UInt32 operator-(Simd512UInt32  lhs, uint32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512UInt32 operator-(const uint32_t lhs, const Simd512UInt32& rhs) noexcept { return Simd512UInt32(_mm512_sub_epi32(_mm512_set1_epi32(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd512UInt32 operator*(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512UInt32 operator*(Simd512UInt32  lhs, uint32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512UInt32 operator*(uint32_t lhs, Simd512UInt32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd512UInt32 operator/(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd512UInt32 operator/(Simd512UInt32  lhs, uint32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd512UInt32 operator/(const uint32_t lhs, const Simd512UInt32& rhs) noexcept { return Simd512UInt32(mt::simd_detail_u32::div_epu32(_mm512_set1_epi32(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd512UInt32 operator&(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd512UInt32 operator|(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd512UInt32 operator^(Simd512UInt32  lhs, const Simd512UInt32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd512UInt32 operator~(const Simd512UInt32& lhs) noexcept { return Simd512UInt32(_mm512_xor_epi32(lhs.v, _mm512_set1_epi32(0xFFFFFFFF))); } //No bitwise not in AVX512 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd512UInt32 operator<<(const Simd512UInt32& lhs, int bits) noexcept { return Simd512UInt32(_mm512_slli_epi32(lhs.v, bits)); }
inline static Simd512UInt32 operator>>(const Simd512UInt32& lhs, int bits) noexcept { return Simd512UInt32(_mm512_srli_epi32(lhs.v, bits)); }
inline static Simd512UInt32 rotl(const Simd512UInt32& a, const int bits) noexcept {
	const int n = bits & 31;
	return Simd512UInt32(_mm512_rolv_epi32(a.v, _mm512_set1_epi32(n)));
}
inline static Simd512UInt32 rotr(const Simd512UInt32& a, const int bits) noexcept {
	const int n = bits & 31;
	return Simd512UInt32(_mm512_rorv_epi32(a.v, _mm512_set1_epi32(n)));
}


//*****Min/Max*****
inline static Simd512UInt32 min(Simd512UInt32 a, Simd512UInt32 b) { return Simd512UInt32(_mm512_min_epu32(a.v, b.v)); }
inline static Simd512UInt32 max(Simd512UInt32 a, Simd512UInt32 b) { return Simd512UInt32(_mm512_max_epu32(a.v, b.v)); }

//*****Conditional Functions*****
inline static __mmask16 compare_equal(const Simd512UInt32 a, const Simd512UInt32 b) noexcept {
	return _mm512_cmp_epu32_mask(a.v, b.v, _MM_CMPINT_EQ);
}
inline static __mmask16 compare_less(const Simd512UInt32 a, const Simd512UInt32 b) noexcept {
	return _mm512_cmp_epu32_mask(a.v, b.v, _MM_CMPINT_LT);
}
inline static __mmask16 compare_less_equal(const Simd512UInt32 a, const Simd512UInt32 b) noexcept {
	return _mm512_cmp_epu32_mask(a.v, b.v, _MM_CMPINT_LE);
}
inline static __mmask16 compare_greater(const Simd512UInt32 a, const Simd512UInt32 b) noexcept {
	return _mm512_cmp_epu32_mask(a.v, b.v, _MM_CMPINT_NLE);
}
inline static __mmask16 compare_greater_equal(const Simd512UInt32 a, const Simd512UInt32 b) noexcept {
	return _mm512_cmp_epu32_mask(a.v, b.v, _MM_CMPINT_NLT);
}
inline static Simd512UInt32 blend(const Simd512UInt32 if_false, const Simd512UInt32 if_true, __mmask16 mask) noexcept {
	return Simd512UInt32(_mm512_mask_blend_epi32(mask, if_false.v, if_true.v));
}


#endif // MT_SIMD_ALLOW_LEVEL4_TYPES

#if MT_SIMD_ALLOW_LEVEL3_TYPES
/**************************************************************************************************
 * SIMD 256 type.  Contains 8 x 32bit Unsigned Integers
 * Requires AVX2 support.
 * ************************************************************************************************/
struct Simd256UInt32 {
	__m256i v;
	typedef uint32_t F;
	typedef __m256i MaskType;

	Simd256UInt32() = default;
	Simd256UInt32(__m256i a) : v(a) {};
	Simd256UInt32(F a) : v(_mm256_set1_epi32(a)) {};

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
	static constexpr int size_of_element() { return sizeof(uint32_t); }
	static constexpr int number_of_elements() { return 8; }	
	F element(int i) const { return mt::simd_detail_u32::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_u32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd256UInt32& operator+=(const Simd256UInt32& rhs) noexcept { v = _mm256_add_epi32(v, rhs.v); return *this; }
	Simd256UInt32& operator+=(uint32_t rhs) noexcept { v = _mm256_add_epi32(v, _mm256_set1_epi32(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd256UInt32& operator-=(const Simd256UInt32& rhs) noexcept { v = _mm256_sub_epi32(v, rhs.v); return *this; }
	Simd256UInt32& operator-=(uint32_t rhs) noexcept { v = _mm256_sub_epi32(v, _mm256_set1_epi32(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd256UInt32& operator*=(const Simd256UInt32& rhs) noexcept {v = _mm256_mullo_epi32(v, rhs.v);	return *this; }

	Simd256UInt32& operator*=(uint32_t rhs) noexcept { *this *= Simd256UInt32(_mm256_set1_epi32(rhs)); return *this; }

	//*****Division Operators*****
	Simd256UInt32& operator/=(const Simd256UInt32& rhs) noexcept { v = mt::simd_detail_u32::div_epu32(v, rhs.v); return *this; }
	Simd256UInt32& operator/=(uint32_t rhs) noexcept { v = mt::simd_detail_u32::div_epu32(v, _mm256_set1_epi32(rhs));	return *this; }

	//*****Bitwise Logic Operators*****
	Simd256UInt32& operator&=(const Simd256UInt32& rhs) noexcept { v = _mm256_and_si256(v, rhs.v); return *this; }
	Simd256UInt32& operator|=(const Simd256UInt32& rhs) noexcept { v = _mm256_or_si256(v, rhs.v); return *this; }
	Simd256UInt32& operator^=(const Simd256UInt32& rhs) noexcept { v = _mm256_xor_si256(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd256UInt32 make_sequential(uint32_t first) { return Simd256UInt32(_mm256_set_epi32(first + 7, first + 6, first + 5, first + 4, first + 3, first + 2, first + 1, first)); }
	static Simd256UInt32 make_set1(uint32_t v) { return Simd256UInt32(_mm256_set1_epi32(v)); }
	

	//*****Mathematical*****
	

	

};

//*****Addition Operators*****
inline static Simd256UInt32 operator+(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256UInt32 operator+(Simd256UInt32  lhs, uint32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256UInt32 operator+(uint32_t lhs, Simd256UInt32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd256UInt32 operator-(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256UInt32 operator-(Simd256UInt32  lhs, uint32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256UInt32 operator-(const uint32_t lhs, const Simd256UInt32& rhs) noexcept { return Simd256UInt32(_mm256_sub_epi32(_mm256_set1_epi32(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd256UInt32 operator*(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256UInt32 operator*(Simd256UInt32  lhs, uint32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256UInt32 operator*(uint32_t lhs, Simd256UInt32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd256UInt32 operator/(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd256UInt32 operator/(Simd256UInt32  lhs, uint32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd256UInt32 operator/(const uint32_t lhs, const Simd256UInt32& rhs) noexcept { return Simd256UInt32(mt::simd_detail_u32::div_epu32(_mm256_set1_epi32(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd256UInt32 operator&(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd256UInt32 operator|(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd256UInt32 operator^(Simd256UInt32  lhs, const Simd256UInt32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd256UInt32 operator~(const Simd256UInt32& lhs) noexcept { return Simd256UInt32(_mm256_xor_si256(lhs.v, _mm256_cmpeq_epi32(lhs.v, lhs.v))); } //No bitwise not in AVX2 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd256UInt32 operator<<(const Simd256UInt32& lhs, int bits) noexcept { return Simd256UInt32(_mm256_slli_epi32(lhs.v, bits)); }
inline static Simd256UInt32 operator>>(const Simd256UInt32& lhs, int bits) noexcept { return Simd256UInt32(_mm256_srli_epi32(lhs.v, bits)); }
inline static Simd256UInt32 rotl(const Simd256UInt32& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 31u;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd256UInt32(_mm256_rolv_epi32(a.v, _mm256_set1_epi32(static_cast<int>(n))));
	}
	if (n == 0u) {
		return a;
	}
	return (a << static_cast<int>(n)) | (a >> static_cast<int>(32u - n));
};
inline static Simd256UInt32 rotr(const Simd256UInt32& a, int bits) {
	const unsigned int n = static_cast<unsigned int>(bits) & 31u;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd256UInt32(_mm256_rorv_epi32(a.v, _mm256_set1_epi32(static_cast<int>(n))));
	}
	if (n == 0u) {
		return a;
	}
	return (a >> static_cast<int>(n)) | (a << static_cast<int>(32u - n));
};

//*****Min/Max*****
inline static Simd256UInt32 min(Simd256UInt32 a, Simd256UInt32 b) {  return Simd256UInt32(_mm256_min_epu32(a.v, b.v)); }
inline static Simd256UInt32 max(Simd256UInt32 a, Simd256UInt32 b) { return Simd256UInt32(_mm256_max_epu32(a.v, b.v)); }

//*****Conditional Functions*****
inline static __m256i compare_equal(const Simd256UInt32 a, const Simd256UInt32 b) noexcept { return _mm256_cmpeq_epi32(a.v, b.v); }
inline static __m256i compare_greater(const Simd256UInt32 a, const Simd256UInt32 b) noexcept {
	const auto sign = _mm256_set1_epi32(static_cast<int32_t>(0x80000000u));
	const auto a_bias = _mm256_xor_si256(a.v, sign);
	const auto b_bias = _mm256_xor_si256(b.v, sign);
	return _mm256_cmpgt_epi32(a_bias, b_bias);
}
inline static __m256i compare_less(const Simd256UInt32 a, const Simd256UInt32 b) noexcept { return compare_greater(b, a); }
inline static __m256i compare_less_equal(const Simd256UInt32 a, const Simd256UInt32 b) noexcept {
	return _mm256_xor_si256(compare_greater(a, b), _mm256_set1_epi32(-1));
}
inline static __m256i compare_greater_equal(const Simd256UInt32 a, const Simd256UInt32 b) noexcept {
	return _mm256_xor_si256(compare_greater(b, a), _mm256_set1_epi32(-1));
}
inline static Simd256UInt32 blend(const Simd256UInt32 if_false, const Simd256UInt32 if_true, __m256i mask) noexcept {
	return Simd256UInt32(_mm256_blendv_epi8(if_false.v, if_true.v, mask));
}









#endif // MT_SIMD_ALLOW_LEVEL3_TYPES

/**************************************************************************************************
*SIMD 128 type.Contains 4 x 32bit Unsigned Integers
* Requires SSE2 support.
* (will be faster with SSE4.1 enabled at compile time)
* ************************************************************************************************/
struct Simd128UInt32 {
	__m128i v;
	typedef uint32_t F;
	typedef __m128i MaskType;

	Simd128UInt32() = default;
	Simd128UInt32(__m128i a) : v(a) {};
	Simd128UInt32(F a) : v(_mm_set1_epi32(a)) {};

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
	static constexpr int size_of_element() { return sizeof(uint32_t); }
	static constexpr int number_of_elements() { return 4; }	
	F element(int i) const { return mt::simd_detail_u32::lane_get(v, i); }
	void set_element(int i, F value) { v = mt::simd_detail_u32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd128UInt32& operator+=(const Simd128UInt32& rhs) noexcept { v = _mm_add_epi32(v, rhs.v); return *this; }
	Simd128UInt32& operator+=(uint32_t rhs) noexcept { v = _mm_add_epi32(v, _mm_set1_epi32(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd128UInt32& operator-=(const Simd128UInt32& rhs) noexcept { v = _mm_sub_epi32(v, rhs.v); return *this; }
	Simd128UInt32& operator-=(uint32_t rhs) noexcept { v = _mm_sub_epi32(v, _mm_set1_epi32(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd128UInt32& operator*=(const Simd128UInt32& rhs) noexcept {
		if constexpr (mt::environment::compiler_has_sse4_1) {
			v = _mm_mullo_epi32(v, rhs.v); //SSE4.1
			return *this;
		}
		else {
			auto result02 = _mm_mul_epu32(v, rhs.v);  //Multiply words 0 and 2.  
			auto result13 = _mm_mul_epu32(_mm_srli_si128(v, 4), _mm_srli_si128(rhs.v, 4));  //Multiply words 1 and 3, by shifting them into 0,2.
			v = _mm_unpacklo_epi32(_mm_shuffle_epi32(result02, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(result13, _MM_SHUFFLE(0, 0, 2, 0))); // shuffle and pack
			return *this;
		}
	}  

	Simd128UInt32& operator*=(uint32_t rhs) noexcept { *this *= Simd128UInt32(_mm_set1_epi32(rhs)); return *this; } 

	//*****Division Operators*****
	Simd128UInt32& operator/=(const Simd128UInt32& rhs) noexcept { v = mt::simd_detail_u32::div_epu32(v, rhs.v); return *this; }
	Simd128UInt32& operator/=(uint32_t rhs) noexcept { v = mt::simd_detail_u32::div_epu32(v, _mm_set1_epi32(rhs));	return *this; } //SSE

	//*****Bitwise Logic Operators*****
	Simd128UInt32& operator&=(const Simd128UInt32& rhs) noexcept { v = _mm_and_si128(v, rhs.v); return *this; } //SSE2
	Simd128UInt32& operator|=(const Simd128UInt32& rhs) noexcept { v = _mm_or_si128(v, rhs.v); return *this; }
	Simd128UInt32& operator^=(const Simd128UInt32& rhs) noexcept { v = _mm_xor_si128(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd128UInt32 make_sequential(uint32_t first) { return Simd128UInt32(_mm_set_epi32(first + 3, first + 2, first + 1, first)); }
	static Simd128UInt32 make_set1(uint32_t v) { return Simd128UInt32(_mm_set1_epi32(v)); }


	//*****Mathematical*****




};

//*****Addition Operators*****
inline static Simd128UInt32 operator+(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt32 operator+(Simd128UInt32  lhs, uint32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt32 operator+(uint32_t lhs, Simd128UInt32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd128UInt32 operator-(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt32 operator-(Simd128UInt32  lhs, uint32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt32 operator-(const uint32_t lhs, const Simd128UInt32& rhs) noexcept { return Simd128UInt32(_mm_sub_epi32(_mm_set1_epi32(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd128UInt32 operator*(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt32 operator*(Simd128UInt32  lhs, uint32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt32 operator*(uint32_t lhs, Simd128UInt32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd128UInt32 operator/(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd128UInt32 operator/(Simd128UInt32  lhs, uint32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128UInt32 operator/(const uint32_t lhs, const Simd128UInt32& rhs) noexcept { return Simd128UInt32(mt::simd_detail_u32::div_epu32(_mm_set1_epi32(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd128UInt32 operator&(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128UInt32 operator|(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128UInt32 operator^(Simd128UInt32  lhs, const Simd128UInt32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128UInt32 operator~(const Simd128UInt32& lhs) noexcept { return Simd128UInt32(_mm_xor_si128(lhs.v, _mm_cmpeq_epi32(lhs.v,lhs.v))); } 


//*****Shifting Operators*****
inline static Simd128UInt32 operator<<(const Simd128UInt32& lhs, const int bits) noexcept { return Simd128UInt32(_mm_slli_epi32(lhs.v, bits)); } //SSE2
inline static Simd128UInt32 operator>>(const Simd128UInt32& lhs, const int bits) noexcept { return Simd128UInt32(_mm_srli_epi32(lhs.v, bits)); }

inline static Simd128UInt32 rotl(const Simd128UInt32& a, int bits) { 
	const int n = bits & 31;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd128UInt32(_mm_rolv_epi32(a.v, _mm_set1_epi32(n)));
	}
	else {
		if (n == 0) {
			return a;
		}
		return (a << n) | (a >> (32 - n));
	}
};


inline static Simd128UInt32 rotr(const Simd128UInt32& a, int bits) { 
	const int n = bits & 31;
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd128UInt32(_mm_rorv_epi32(a.v, _mm_set1_epi32(n)));
	}
	else {
		if (n == 0) {
			return a;
		}
		return (a >> n) | (a << (32 - n));
	}
};

//*****Min/Max*****
inline static Simd128UInt32 min(Simd128UInt32 a, Simd128UInt32 b) {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128UInt32(_mm_min_epu32(a.v, b.v)); //SSE4.1
	}
	else {
		//No min/max or compare for unsigned ints in SSE2 so we will just unroll.
		auto m3 = std::min(a.element(3), b.element(3));
		auto m2 = std::min(a.element(2), b.element(2));
		auto m1 = std::min(a.element(1), b.element(1));
		auto m0 = std::min(a.element(0), b.element(0));
		return Simd128UInt32(_mm_set_epi32(m3, m2, m1, m0));
	}
}



inline static Simd128UInt32 max(Simd128UInt32 a, Simd128UInt32 b) {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128UInt32(_mm_max_epu32(a.v, b.v));  //SSE4.1
	}
	else {
		//No min/max or compare for unsigned ints in SSE2 so we will just unroll.
		auto m3 = std::max(a.element(3), b.element(3));
		auto m2 = std::max(a.element(2), b.element(2));
		auto m1 = std::max(a.element(1), b.element(1));
		auto m0 = std::max(a.element(0), b.element(0));
		return Simd128UInt32(_mm_set_epi32(m3, m2, m1, m0));
	}
}

//*****Conditional Functions*****
inline static __m128i compare_equal(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return _mm_cmpeq_epi32(a.v, b.v); }
inline static __m128i compare_greater(const Simd128UInt32 a, const Simd128UInt32 b) noexcept {
	const auto sign = _mm_set1_epi32(static_cast<int32_t>(0x80000000u));
	const auto a_bias = _mm_xor_si128(a.v, sign);
	const auto b_bias = _mm_xor_si128(b.v, sign);
	return _mm_cmpgt_epi32(a_bias, b_bias);
}
inline static __m128i compare_less(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return compare_greater(b, a); }
inline static __m128i compare_less_equal(const Simd128UInt32 a, const Simd128UInt32 b) noexcept {
	return _mm_xor_si128(compare_greater(a, b), _mm_set1_epi32(-1));
}
inline static __m128i compare_greater_equal(const Simd128UInt32 a, const Simd128UInt32 b) noexcept {
	return _mm_xor_si128(compare_greater(b, a), _mm_set1_epi32(-1));
}
inline static Simd128UInt32 blend(const Simd128UInt32 if_false, const Simd128UInt32 if_true, __m128i mask) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128UInt32(_mm_blendv_epi8(if_false.v, if_true.v, mask));
	}
	else {
		return Simd128UInt32(_mm_or_si128(_mm_andnot_si128(mask, if_false.v), _mm_and_si128(mask, if_true.v)));
	}
}


#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)

struct Simd128UInt32 {
	v128_t v;
	typedef uint32_t F;
	typedef v128_t MaskType;

	Simd128UInt32() = default;
	Simd128UInt32(v128_t a) : v(a) {}
	Simd128UInt32(F a) : v(mt::simd_wasm_detail::splat<uint32_t>(a)) {}

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

	static constexpr int size_of_element() { return sizeof(uint32_t); }
	static constexpr int number_of_elements() { return 4; }
	F element(int i) const { return mt::simd_wasm_detail::lane_get<uint32_t, 4>(v, i); }
	void set_element(int i, F value) { v = mt::simd_wasm_detail::lane_set<uint32_t, 4>(v, i, value); }

	static Simd128UInt32 make_sequential(uint32_t first) { return Simd128UInt32(mt::simd_wasm_detail::make_sequential<uint32_t, 4>(first)); }
	static Simd128UInt32 make_set1(uint32_t value) { return Simd128UInt32(mt::simd_wasm_detail::splat<uint32_t>(value)); }

	Simd128UInt32& operator+=(const Simd128UInt32& rhs) noexcept { v = wasm_i32x4_add(v, rhs.v); return *this; }
	Simd128UInt32& operator+=(uint32_t rhs) noexcept { v = wasm_i32x4_add(v, make_set1(rhs).v); return *this; }
	Simd128UInt32& operator-=(const Simd128UInt32& rhs) noexcept { v = wasm_i32x4_sub(v, rhs.v); return *this; }
	Simd128UInt32& operator-=(uint32_t rhs) noexcept { v = wasm_i32x4_sub(v, make_set1(rhs).v); return *this; }
	Simd128UInt32& operator*=(const Simd128UInt32& rhs) noexcept { v = wasm_i32x4_mul(v, rhs.v); return *this; }
	Simd128UInt32& operator*=(uint32_t rhs) noexcept { v = wasm_i32x4_mul(v, make_set1(rhs).v); return *this; }
	Simd128UInt32& operator/=(const Simd128UInt32& rhs) noexcept {
		v = mt::simd_wasm_detail::map_binary<uint32_t, 4>(v, rhs.v, [](uint32_t a, uint32_t b) { return a / b; });
		return *this;
	}
	Simd128UInt32& operator/=(uint32_t rhs) noexcept { return *this /= make_set1(rhs); }
	Simd128UInt32& operator&=(const Simd128UInt32& rhs) noexcept { v = wasm_v128_and(v, rhs.v); return *this; }
	Simd128UInt32& operator|=(const Simd128UInt32& rhs) noexcept { v = wasm_v128_or(v, rhs.v); return *this; }
	Simd128UInt32& operator^=(const Simd128UInt32& rhs) noexcept { v = wasm_v128_xor(v, rhs.v); return *this; }
};

inline static Simd128UInt32 operator+(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt32 operator+(Simd128UInt32 lhs, uint32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128UInt32 operator+(uint32_t lhs, Simd128UInt32 rhs) noexcept { rhs += lhs; return rhs; }
inline static Simd128UInt32 operator-(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt32 operator-(Simd128UInt32 lhs, uint32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128UInt32 operator-(uint32_t lhs, const Simd128UInt32& rhs) noexcept { return Simd128UInt32::make_set1(lhs) - rhs; }
inline static Simd128UInt32 operator*(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt32 operator*(Simd128UInt32 lhs, uint32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128UInt32 operator*(uint32_t lhs, Simd128UInt32 rhs) noexcept { rhs *= lhs; return rhs; }
inline static Simd128UInt32 operator/(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128UInt32 operator/(Simd128UInt32 lhs, uint32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128UInt32 operator/(uint32_t lhs, const Simd128UInt32& rhs) noexcept { return Simd128UInt32::make_set1(lhs) / rhs; }
inline static Simd128UInt32 operator&(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128UInt32 operator|(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128UInt32 operator^(Simd128UInt32 lhs, const Simd128UInt32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128UInt32 operator~(const Simd128UInt32& lhs) noexcept { return Simd128UInt32(wasm_v128_not(lhs.v)); }
inline static Simd128UInt32 operator<<(const Simd128UInt32& lhs, const int bits) noexcept { return Simd128UInt32(wasm_i32x4_shl(lhs.v, static_cast<uint32_t>(bits))); }
inline static Simd128UInt32 operator>>(const Simd128UInt32& lhs, const int bits) noexcept { return Simd128UInt32(wasm_u32x4_shr(lhs.v, static_cast<uint32_t>(bits))); }

inline static Simd128UInt32 rotl(const Simd128UInt32& a, int bits) {
	const int n = bits & 31;
	if (n == 0) {
		return a;
	}
	return (a << n) | (a >> (32 - n));
}

inline static Simd128UInt32 rotr(const Simd128UInt32& a, int bits) {
	const int n = bits & 31;
	if (n == 0) {
		return a;
	}
	return (a >> n) | (a << (32 - n));
}

inline static Simd128UInt32 min(Simd128UInt32 a, Simd128UInt32 b) { return Simd128UInt32(wasm_u32x4_min(a.v, b.v)); }
inline static Simd128UInt32 max(Simd128UInt32 a, Simd128UInt32 b) { return Simd128UInt32(wasm_u32x4_max(a.v, b.v)); }

inline static v128_t compare_equal(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return wasm_i32x4_eq(a.v, b.v); }
inline static v128_t compare_greater(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return wasm_u32x4_gt(a.v, b.v); }
inline static v128_t compare_less(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return wasm_u32x4_lt(a.v, b.v); }
inline static v128_t compare_less_equal(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return wasm_u32x4_le(a.v, b.v); }
inline static v128_t compare_greater_equal(const Simd128UInt32 a, const Simd128UInt32 b) noexcept { return wasm_u32x4_ge(a.v, b.v); }
inline static Simd128UInt32 blend(const Simd128UInt32 if_false, const Simd128UInt32 if_true, v128_t mask) noexcept {
	return Simd128UInt32(wasm_v128_bitselect(if_true.v, if_false.v, mask));
}

#endif //x86_64 / wasm




/**************************************************************************************************
 * Templated Functions for all types
 * ************************************************************************************************/
template <SimdUInt32 T>
[[nodiscard("Value Calculated and not used (if_equal)")]]
inline static T if_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_equal(value_a, value_b));
}

template <SimdUInt32 T>
[[nodiscard("Value Calculated and not used (if_less)")]]
inline static T if_less(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less(value_a, value_b));
}

template <SimdUInt32 T>
[[nodiscard("Value Calculated and not used (if_less_equal)")]]
inline static T if_less_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less_equal(value_a, value_b));
}

template <SimdUInt32 T>
[[nodiscard("Value Calculated and not used (if_greater)")]]
inline static T if_greater(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater(value_a, value_b));
}

template <SimdUInt32 T>
[[nodiscard("Value Calculated and not used (if_greater_equal)")]]
inline static T if_greater_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater_equal(value_a, value_b));
}



/**************************************************************************************************
 * Check that each type implements the desired types from simd-concepts.h
 * (Not sure why this fails on intelisense, but compliles ok.)
 * ************************************************************************************************/
static_assert(Simd<FallbackUInt32>, "FallbackUInt32 does not implement the concept Simd");
static_assert(SimdUInt<FallbackUInt32>, "FallbackUInt32 does not implement the concept SimdUInt");
static_assert(SimdUInt32<FallbackUInt32>, "FallbackUInt32 does not implement the concept SimdUInt32");
static_assert(SimdCompareOps<FallbackUInt32>, "FallbackUInt32 does not implement the concept SimdCompareOps");

#if MT_SIMD_ARCH_X64
static_assert(Simd<Simd128UInt32>, "Simd128UInt32 does not implement the concept Simd");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(Simd<Simd256UInt32>, "Simd256UInt32 does not implement the concept Simd");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(Simd<Simd512UInt32>, "Simd512UInt32 does not implement the concept Simd");
#endif

static_assert(SimdUInt<Simd128UInt32>, "Simd128UInt32 does not implement the concept SimdUInt");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdUInt<Simd256UInt32>, "Simd256UInt32 does not implement the concept SimdUInt");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdUInt<Simd512UInt32>, "Simd512UInt32 does not implement the concept SimdUInt");
#endif



static_assert(SimdUInt32<Simd128UInt32>, "Simd128UInt32 does not implement the concept SimdUInt32");
static_assert(SimdCompareOps<Simd128UInt32>, "Simd128UInt32 does not implement the concept SimdCompareOps");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdUInt32<Simd256UInt32>, "Simd256UInt32 does not implement the concept SimdUInt32");
static_assert(SimdCompareOps<Simd256UInt32>, "Simd256UInt32 does not implement the concept SimdCompareOps");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdUInt32<Simd512UInt32>, "Simd512UInt32 does not implement the concept SimdUInt32");
static_assert(SimdCompareOps<Simd512UInt32>, "Simd512UInt32 does not implement the concept SimdCompareOps");
#endif
#endif



/**************************************************************************************************
 Define SimdNativeUInt32 as the best supported type at compile time.
*************************************************************************************************/
#if MT_SIMD_ARCH_X64
	#if MT_SIMD_ALLOW_LEVEL4_TYPES
		typedef Simd512UInt32 SimdNativeUInt32;
	#elif MT_SIMD_ALLOW_LEVEL3_TYPES
		typedef Simd256UInt32 SimdNativeUInt32;
	#else
		typedef Simd128UInt32 SimdNativeUInt32;
	#endif
#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)
	typedef Simd128UInt32 SimdNativeUInt32;
#else
	typedef FallbackUInt32 SimdNativeUInt32;
#endif


