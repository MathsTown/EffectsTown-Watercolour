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

Basic SIMD Types for 32-bit Signed Integers:

FallbackInt32		- Works on all build modes and CPUs.  Forwards most requests to the standard library.

Simd128Int32		- x86_64 Microarchitecture Level 1 - Works on all x86_64 CPUs.
					- Requires SSE and SSE2 support.  Will use SSE4.1 instructions when __SSE4_1__ or __AVX__ defined.

Simd256Int32		- x86_64 Microarchitecture Level 3.
					- Requires AVX, AVX2 and FMA support.

Simd512Int32		- x86_64 Microarchitecture Level 4.
					- Requires AVX512F, AVX512DQ, AVX512VL, AVX512CD, AVX512BW

SimdNativeInt32	- A Typedef referring to one of the above types.  Chosen based on compiler support/mode.
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
- When the compiler detects higher levels of support, such as SSE4.1 (level 2), more optimised instructions may be chosen.
- Runtime checks are only meaningful for builds intended to run across mixed CPU capabilities, but separate compilation in recommended.

WASM Support:
I've included FallbackInt32 for use with Emscripen.


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
* Fallback Int32 type.
* Contains 1 x 32bit Signed Integer
*
* This is a fallback for cpus that are not yet supported.
*
* It may be useful to evaluate a single value from the same code base, as it offers the same interface
* as the SIMD types.
*
* ************************************************************************************************/
struct FallbackInt32 {
	int32_t v;
	typedef int32_t F;
	typedef bool MaskType;
	FallbackInt32() = default;
	FallbackInt32(int32_t a) : v(a) {};

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
	static constexpr int size_of_element() { return sizeof(int32_t); }
	static constexpr int number_of_elements() { return 1; }
	F element(int) const { return v; }
	void set_element(int, F value) { v = value; }

	//*****Make Functions****
	static FallbackInt32 make_sequential(int32_t first) { return FallbackInt32(first); }
	static FallbackInt32 make_set1(int32_t v) { return FallbackInt32(v); }


	//*****Addition Operators*****
	FallbackInt32& operator+=(const FallbackInt32& rhs) noexcept { v += rhs.v; return *this; }
	FallbackInt32& operator+=(int32_t rhs) noexcept { v += rhs; return *this; }

	//*****Subtraction Operators*****
	FallbackInt32& operator-=(const FallbackInt32& rhs) noexcept { v -= rhs.v; return *this; }
	FallbackInt32& operator-=(int32_t rhs) noexcept { v -= rhs; return *this; }

	//*****Multiplication Operators*****
	FallbackInt32& operator*=(const FallbackInt32& rhs) noexcept { v *= rhs.v; return *this; }
	FallbackInt32& operator*=(int32_t rhs) noexcept { v *= rhs; return *this; }

	//*****Division Operators*****
	FallbackInt32& operator/=(const FallbackInt32& rhs) noexcept { v /= rhs.v; return *this; }
	FallbackInt32& operator/=(int32_t rhs) noexcept { v /= rhs;	return *this; }

	//*****Negate Operators*****
	FallbackInt32 operator-() const noexcept { return FallbackInt32(-v); }

	//*****Bitwise Logic Operators*****
	FallbackInt32& operator&=(const FallbackInt32& rhs) noexcept { v &= rhs.v; return *this; }
	FallbackInt32& operator|=(const FallbackInt32& rhs) noexcept { v |= rhs.v; return *this; }
	FallbackInt32& operator^=(const FallbackInt32& rhs) noexcept { v ^= rhs.v; return *this; }

};

//*****Addition Operators*****
inline static FallbackInt32 operator+(FallbackInt32  lhs, const FallbackInt32& rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackInt32 operator+(FallbackInt32  lhs, int32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackInt32 operator+(int32_t lhs, FallbackInt32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static FallbackInt32 operator-(FallbackInt32  lhs, const FallbackInt32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackInt32 operator-(FallbackInt32  lhs, int32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackInt32 operator-(const int32_t lhs, FallbackInt32 rhs) noexcept { rhs.v = lhs - rhs.v; return rhs; }

//*****Multiplication Operators*****
inline static FallbackInt32 operator*(FallbackInt32  lhs, const FallbackInt32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackInt32 operator*(FallbackInt32  lhs, int32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackInt32 operator*(int32_t lhs, FallbackInt32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static FallbackInt32 operator/(FallbackInt32  lhs, const FallbackInt32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static FallbackInt32 operator/(FallbackInt32  lhs, int32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static FallbackInt32 operator/(const int32_t lhs, FallbackInt32 rhs) noexcept { rhs.v = lhs / rhs.v; return rhs; }


//*****Bitwise Logic Operators*****
inline static FallbackInt32 operator&(const FallbackInt32& lhs, const FallbackInt32& rhs) noexcept { return FallbackInt32(lhs.v & rhs.v); }
inline static FallbackInt32 operator|(const FallbackInt32& lhs, const FallbackInt32& rhs) noexcept { return FallbackInt32(lhs.v | rhs.v); }
inline static FallbackInt32 operator^(const FallbackInt32& lhs, const FallbackInt32& rhs) noexcept { return FallbackInt32(lhs.v ^ rhs.v); }
inline static FallbackInt32 operator~(FallbackInt32 lhs) noexcept { return FallbackInt32(~lhs.v); }


//*****Shifting Operators*****
inline static FallbackInt32 operator<<(FallbackInt32 lhs, int bits) noexcept { lhs.v <<= bits; return lhs; }
inline static FallbackInt32 operator>>(FallbackInt32 lhs, int bits) noexcept { lhs.v >>= bits; return lhs; }

//*****Min/Max*****
inline static FallbackInt32 min(FallbackInt32 a, FallbackInt32 b) { return FallbackInt32(std::min(a.v, b.v)); }
inline static FallbackInt32 max(FallbackInt32 a, FallbackInt32 b) { return FallbackInt32(std::max(a.v, b.v)); }

//*****Math Operators*****
inline static FallbackInt32 abs(FallbackInt32 a) noexcept {
	// std::abs(INT_MIN) is UB; use unsigned two's-complement math. This ensures the same results as SSE/AVX code.
	const uint32_t ux = static_cast<uint32_t>(a.v);
	const uint32_t sign = ux >> 31;
	const uint32_t mask = 0u - sign;
	const uint32_t mag = (ux ^ mask) + sign;
	return FallbackInt32(static_cast<int32_t>(mag));
}

//*****Conditional Functions*****
inline static bool compare_equal(const FallbackInt32 a, const FallbackInt32 b) noexcept { return a.v == b.v; }
inline static bool compare_less(const FallbackInt32 a, const FallbackInt32 b) noexcept { return a.v < b.v; }
inline static bool compare_less_equal(const FallbackInt32 a, const FallbackInt32 b) noexcept { return a.v <= b.v; }
inline static bool compare_greater(const FallbackInt32 a, const FallbackInt32 b) noexcept { return a.v > b.v; }
inline static bool compare_greater_equal(const FallbackInt32 a, const FallbackInt32 b) noexcept { return a.v >= b.v; }
inline static FallbackInt32 blend(const FallbackInt32 if_false, const FallbackInt32 if_true, bool mask) noexcept {
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
namespace mt::simd_detail_i32 {
	// Portability layer: GCC/Clang cannot index SIMD lanes via MSVC vector members.
	inline int32_t lane_get(__m128i v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m128i_i32[i];
#else
		alignas(16) int32_t lanes[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline __m128i lane_set(__m128i v, int i, int32_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m128i_i32[i] = value;
		return v;
#else
		alignas(16) int32_t lanes[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lanes), v);
		lanes[i] = value;
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(lanes));
#endif
	}

	inline int32_t lane_get(const __m256i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m256i_i32[i];
#else
		alignas(32) int32_t lanes[8];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		return lanes[i];
#endif
	}
	inline void lane_set(__m256i& v, int i, int32_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m256i_i32[i] = value;
#else
		alignas(32) int32_t lanes[8];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lanes), v);
		lanes[i] = value;
		v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lanes));
#endif
	}

	inline int32_t lane_get(const __m512i& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m512i_i32[i];
#else
		alignas(64) int32_t lanes[16];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	inline void lane_set(__m512i& v, int i, int32_t value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m512i_i32[i] = value;
#else
		alignas(64) int32_t lanes[16];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

	// Fallback: integer SIMD divide intrinsics are non-standard across compilers.
	inline __m128i div_epi32(__m128i a, __m128i b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm_div_epi32(a, b);
#else
		alignas(16) int32_t lhs[4], rhs[4], out[4];
		_mm_storeu_si128(reinterpret_cast<__m128i*>(lhs), a);
		_mm_storeu_si128(reinterpret_cast<__m128i*>(rhs), b);
		for (int i = 0; i < 4; ++i) out[i] = lhs[i] / rhs[i];
		return _mm_loadu_si128(reinterpret_cast<const __m128i*>(out));
#endif
	}
	inline __m256i div_epi32(const __m256i& a, const __m256i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm256_div_epi32(a, b);
#else
		alignas(32) int32_t lhs[8], rhs[8], out[8];
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(lhs), a);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(rhs), b);
		for (int i = 0; i < 8; ++i) out[i] = lhs[i] / rhs[i];
		return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out));
#endif
	}
	inline __m512i div_epi32(const __m512i& a, const __m512i& b) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return _mm512_div_epi32(a, b);
#else
		alignas(64) int32_t lhs[16], rhs[16], out[16];
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
 * SIMD 512 type.  Contains 16 x 32bit Signed Integers
 * Requires AVX-512F
 * ************************************************************************************************/
struct Simd512Int32 {
	__m512i v;
	typedef int32_t F;
	typedef __mmask16 MaskType;

	Simd512Int32() = default;
	Simd512Int32(__m512i a) : v(a) {};
	Simd512Int32(F a) : v(_mm512_set1_epi32(a)) {};

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
	static constexpr int size_of_element() { return sizeof(int32_t); }
	static constexpr int number_of_elements() { return 16; }
	F element(int i) const { return mt::simd_detail_i32::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_i32::lane_set(v, i, value); }

	//*****Make Functions****
	static Simd512Int32 make_sequential(int32_t first) {
		const uint32_t base = static_cast<uint32_t>(first);
		return Simd512Int32(_mm512_set_epi32(
			static_cast<int32_t>(base + 15u), static_cast<int32_t>(base + 14u), static_cast<int32_t>(base + 13u), static_cast<int32_t>(base + 12u),
			static_cast<int32_t>(base + 11u), static_cast<int32_t>(base + 10u), static_cast<int32_t>(base + 9u), static_cast<int32_t>(base + 8u),
			static_cast<int32_t>(base + 7u), static_cast<int32_t>(base + 6u), static_cast<int32_t>(base + 5u), static_cast<int32_t>(base + 4u),
			static_cast<int32_t>(base + 3u), static_cast<int32_t>(base + 2u), static_cast<int32_t>(base + 1u), static_cast<int32_t>(base)));
	}
	static Simd512Int32 make_set1(int32_t v) { return Simd512Int32(_mm512_set1_epi32(v)); }


	//*****Addition Operators*****
	Simd512Int32& operator+=(const Simd512Int32& rhs) noexcept { v = _mm512_add_epi32(v, rhs.v); return *this; }
	Simd512Int32& operator+=(int32_t rhs) noexcept { v = _mm512_add_epi32(v, _mm512_set1_epi32(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd512Int32& operator-=(const Simd512Int32& rhs) noexcept { v = _mm512_sub_epi32(v, rhs.v); return *this; }
	Simd512Int32& operator-=(int32_t rhs) noexcept { v = _mm512_sub_epi32(v, _mm512_set1_epi32(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd512Int32& operator*=(const Simd512Int32& rhs) noexcept { v = _mm512_mullo_epi32(v, rhs.v); return *this; }
	Simd512Int32& operator*=(int32_t rhs) noexcept { v = _mm512_mullo_epi32(v, _mm512_set1_epi32(rhs)); return *this; }

	//*****Division Operators*****
	Simd512Int32& operator/=(const Simd512Int32& rhs) noexcept { v = mt::simd_detail_i32::div_epi32(v, rhs.v); return *this; }
	Simd512Int32& operator/=(int32_t rhs) noexcept {
		if constexpr (mt::environment::compiler_has_avx512f) {
			v = mt::simd_detail_i32::div_epi32(v, _mm512_set1_epi32(rhs));
			return *this;
		}
		else {
			//I don't know why but visual studio linker was hanging when compiling this without AVX.
			//Since we wish to support runtime dispatch in visual studio, we fallback to scaler division in this case.
			//For future investigation.
			v = _mm512_set_epi32(
				mt::simd_detail_i32::lane_get(v, 15) / rhs,
				mt::simd_detail_i32::lane_get(v, 14) / rhs,
				mt::simd_detail_i32::lane_get(v, 13) / rhs,
				mt::simd_detail_i32::lane_get(v, 12) / rhs,
				mt::simd_detail_i32::lane_get(v, 11) / rhs,
				mt::simd_detail_i32::lane_get(v, 10) / rhs,
				mt::simd_detail_i32::lane_get(v, 9) / rhs,
				mt::simd_detail_i32::lane_get(v, 8) / rhs,
				mt::simd_detail_i32::lane_get(v, 7) / rhs,
				mt::simd_detail_i32::lane_get(v, 6) / rhs,
				mt::simd_detail_i32::lane_get(v, 5) / rhs,
				mt::simd_detail_i32::lane_get(v, 4) / rhs,
				mt::simd_detail_i32::lane_get(v, 3) / rhs,
				mt::simd_detail_i32::lane_get(v, 2) / rhs,
				mt::simd_detail_i32::lane_get(v, 1) / rhs,
				mt::simd_detail_i32::lane_get(v, 0) / rhs
			);
			return *this;
		}
	}

	//*****Negate Operators*****
	Simd512Int32 operator-() const noexcept {
		return Simd512Int32(_mm512_sub_epi32(_mm512_setzero_epi32(), v));
	}

	//*****Bitwise Logic Operators*****
	Simd512Int32& operator&=(const Simd512Int32& rhs) noexcept { v = _mm512_and_si512(v, rhs.v); return *this; }
	Simd512Int32& operator|=(const Simd512Int32& rhs) noexcept { v = _mm512_or_si512(v, rhs.v); return *this; }
	Simd512Int32& operator^=(const Simd512Int32& rhs) noexcept { v = _mm512_xor_si512(v, rhs.v); return *this; }




};

//*****Addition Operators*****
inline static Simd512Int32 operator+(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Int32 operator+(Simd512Int32  lhs, int32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Int32 operator+(int32_t lhs, Simd512Int32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd512Int32 operator-(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Int32 operator-(Simd512Int32  lhs, int32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Int32 operator-(const int32_t lhs, const Simd512Int32& rhs) noexcept { return Simd512Int32(_mm512_sub_epi32(_mm512_set1_epi32(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd512Int32 operator*(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Int32 operator*(Simd512Int32  lhs, int32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Int32 operator*(int32_t lhs, Simd512Int32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd512Int32 operator/(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd512Int32 operator/(Simd512Int32  lhs, int32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd512Int32 operator/(const int32_t lhs, const Simd512Int32& rhs) noexcept { return Simd512Int32(mt::simd_detail_i32::div_epi32(_mm512_set1_epi32(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd512Int32 operator&(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd512Int32 operator|(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd512Int32 operator^(Simd512Int32  lhs, const Simd512Int32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd512Int32 operator~(const Simd512Int32& lhs) noexcept { return Simd512Int32(_mm512_xor_epi32(lhs.v, _mm512_set1_epi32(0xFFFFFFFF))); } //No bitwise not in AVX512 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd512Int32 operator<<(const Simd512Int32& lhs, int bits) noexcept { return Simd512Int32(_mm512_slli_epi32(lhs.v, bits)); }
inline static Simd512Int32 operator>>(const Simd512Int32& lhs, int bits) noexcept { return Simd512Int32(_mm512_srai_epi32(lhs.v, bits)); }


//*****Min/Max*****
inline static Simd512Int32 min(Simd512Int32 a, Simd512Int32 b) { return Simd512Int32(_mm512_min_epi32(a.v, b.v)); }
inline static Simd512Int32 max(Simd512Int32 a, Simd512Int32 b) { return Simd512Int32(_mm512_max_epi32(a.v, b.v)); }


//*****Mathematical*****
inline static Simd512Int32 abs(Simd512Int32 a) { return Simd512Int32(_mm512_abs_epi32(a.v)); }

//*****Conditional Functions*****
inline static __mmask16 compare_equal(const Simd512Int32 a, const Simd512Int32 b) noexcept {
	return _mm512_cmp_epi32_mask(a.v, b.v, _MM_CMPINT_EQ);
}
inline static __mmask16 compare_less(const Simd512Int32 a, const Simd512Int32 b) noexcept {
	return _mm512_cmp_epi32_mask(a.v, b.v, _MM_CMPINT_LT);
}
inline static __mmask16 compare_less_equal(const Simd512Int32 a, const Simd512Int32 b) noexcept {
	return _mm512_cmp_epi32_mask(a.v, b.v, _MM_CMPINT_LE);
}
inline static __mmask16 compare_greater(const Simd512Int32 a, const Simd512Int32 b) noexcept {
	return _mm512_cmp_epi32_mask(a.v, b.v, _MM_CMPINT_NLE);
}
inline static __mmask16 compare_greater_equal(const Simd512Int32 a, const Simd512Int32 b) noexcept {
	return _mm512_cmp_epi32_mask(a.v, b.v, _MM_CMPINT_NLT);
}
inline static Simd512Int32 blend(const Simd512Int32 if_false, const Simd512Int32 if_true, __mmask16 mask) noexcept {
	return Simd512Int32(_mm512_mask_blend_epi32(mask, if_false.v, if_true.v));
}

#endif // MT_SIMD_ALLOW_LEVEL4_TYPES

#if MT_SIMD_ALLOW_LEVEL3_TYPES
/**************************************************************************************************
 * SIMD 256 type.  Contains 8 x 32bit Signed Integers
 * Requires AVX2 support.
 * ************************************************************************************************/
struct Simd256Int32 {
	__m256i v;
	typedef int32_t F;
	typedef __m256i MaskType;

	Simd256Int32() = default;
	Simd256Int32(__m256i a) : v(a) {};
	Simd256Int32(F a) : v(_mm256_set1_epi32(a)) {};

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
	static constexpr int size_of_element() { return sizeof(int32_t); }
	static constexpr int number_of_elements() { return 8; }
	F element(int i) const { return mt::simd_detail_i32::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_i32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd256Int32& operator+=(const Simd256Int32& rhs) noexcept { v = _mm256_add_epi32(v, rhs.v); return *this; }
	Simd256Int32& operator+=(int32_t rhs) noexcept { v = _mm256_add_epi32(v, _mm256_set1_epi32(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd256Int32& operator-=(const Simd256Int32& rhs) noexcept { v = _mm256_sub_epi32(v, rhs.v); return *this; }
	Simd256Int32& operator-=(int32_t rhs) noexcept { v = _mm256_sub_epi32(v, _mm256_set1_epi32(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd256Int32& operator*=(const Simd256Int32& rhs) noexcept { v = _mm256_mullo_epi32(v, rhs.v);	return *this; }
	Simd256Int32& operator*=(int32_t rhs) noexcept { *this *= Simd256Int32(_mm256_set1_epi32(rhs)); return *this; }

	//*****Division Operators*****
	Simd256Int32& operator/=(const Simd256Int32& rhs) noexcept { 
		v = mt::simd_detail_i32::div_epi32(v, rhs.v);
		return *this;
	}
	Simd256Int32& operator/=(int32_t rhs) noexcept { 
		if constexpr (mt::environment::compiler_has_avx2) {
			v = mt::simd_detail_i32::div_epi32(v, _mm256_set1_epi32(rhs));
			return *this;
		}else {
			//I don't know why but visual studio was hanging when compiling this without AVX.
			//Since we wish to support runtime dispatch in visual studio, we fallback to scaler division in this case.
			v = _mm256_set_epi32(
				mt::simd_detail_i32::lane_get(v, 7) / rhs,
				mt::simd_detail_i32::lane_get(v, 6) / rhs,
				mt::simd_detail_i32::lane_get(v, 5) / rhs,
				mt::simd_detail_i32::lane_get(v, 4) / rhs,
				mt::simd_detail_i32::lane_get(v, 3) / rhs,
				mt::simd_detail_i32::lane_get(v, 2) / rhs,
				mt::simd_detail_i32::lane_get(v, 1) / rhs,
				mt::simd_detail_i32::lane_get(v, 0) / rhs
			);
			return *this;
		}
		
	}

	//*****Negate Operators*****
	Simd256Int32 operator-() const noexcept {
		return Simd256Int32(_mm256_sub_epi32(_mm256_setzero_si256(), v));
	}

	//*****Bitwise Logic Operators*****
	Simd256Int32& operator&=(const Simd256Int32& rhs) noexcept { v = _mm256_and_si256(v, rhs.v); return *this; }
	Simd256Int32& operator|=(const Simd256Int32& rhs) noexcept { v = _mm256_or_si256(v, rhs.v); return *this; }
	Simd256Int32& operator^=(const Simd256Int32& rhs) noexcept { v = _mm256_xor_si256(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd256Int32 make_sequential(int32_t first) {
		const uint32_t base = static_cast<uint32_t>(first);
		return Simd256Int32(_mm256_set_epi32(
			static_cast<int32_t>(base + 7u), static_cast<int32_t>(base + 6u), static_cast<int32_t>(base + 5u), static_cast<int32_t>(base + 4u),
			static_cast<int32_t>(base + 3u), static_cast<int32_t>(base + 2u), static_cast<int32_t>(base + 1u), static_cast<int32_t>(base)));
	}
	static Simd256Int32 make_set1(int32_t v) { return Simd256Int32(_mm256_set1_epi32(v)); }


};

//*****Addition Operators*****
inline static Simd256Int32 operator+(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Int32 operator+(Simd256Int32  lhs, int32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Int32 operator+(int32_t lhs, Simd256Int32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd256Int32 operator-(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Int32 operator-(Simd256Int32  lhs, int32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Int32 operator-(const int32_t lhs, const Simd256Int32& rhs) noexcept { return Simd256Int32(_mm256_sub_epi32(_mm256_set1_epi32(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd256Int32 operator*(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Int32 operator*(Simd256Int32  lhs, int32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Int32 operator*(int32_t lhs, Simd256Int32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline Simd256Int32 operator/(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline Simd256Int32 operator/(Simd256Int32  lhs, int32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline Simd256Int32 operator/(const int32_t lhs, const Simd256Int32& rhs) noexcept { return Simd256Int32(mt::simd_detail_i32::div_epi32(_mm256_set1_epi32(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd256Int32 operator&(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd256Int32 operator|(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd256Int32 operator^(Simd256Int32  lhs, const Simd256Int32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd256Int32 operator~(const Simd256Int32& lhs) noexcept { return Simd256Int32(_mm256_xor_si256(lhs.v, _mm256_cmpeq_epi32(lhs.v, lhs.v))); } //No bitwise not in AVX2 so we use xor with a constant..


//*****Shifting Operators*****
inline static Simd256Int32 operator<<(const Simd256Int32& lhs, int bits) noexcept { return Simd256Int32(_mm256_slli_epi32(lhs.v, bits)); }
//Arithmatic Shift is used for signed integers
inline static Simd256Int32 operator>>(const Simd256Int32& lhs, int bits) noexcept { return Simd256Int32(_mm256_srai_epi32(lhs.v, bits)); }

//*****Min/Max*****
inline static Simd256Int32 min(Simd256Int32 a, Simd256Int32 b) { return Simd256Int32(_mm256_min_epi32(a.v, b.v)); }
inline static Simd256Int32 max(Simd256Int32 a, Simd256Int32 b) { return Simd256Int32(_mm256_max_epi32(a.v, b.v)); }

//*****Mathematical*****
inline static Simd256Int32 abs(Simd256Int32 a) { return Simd256Int32(_mm256_abs_epi32(a.v)); }

//*****Conditional Functions*****
inline static __m256i compare_equal(const Simd256Int32 a, const Simd256Int32 b) noexcept { return _mm256_cmpeq_epi32(a.v, b.v); }
inline static __m256i compare_less(const Simd256Int32 a, const Simd256Int32 b) noexcept { return _mm256_cmpgt_epi32(b.v, a.v); }
inline static __m256i compare_less_equal(const Simd256Int32 a, const Simd256Int32 b) noexcept {
	return _mm256_xor_si256(_mm256_cmpgt_epi32(a.v, b.v), _mm256_set1_epi32(-1));
}
inline static __m256i compare_greater(const Simd256Int32 a, const Simd256Int32 b) noexcept { return _mm256_cmpgt_epi32(a.v, b.v); }
inline static __m256i compare_greater_equal(const Simd256Int32 a, const Simd256Int32 b) noexcept {
	return _mm256_xor_si256(_mm256_cmpgt_epi32(b.v, a.v), _mm256_set1_epi32(-1));
}
inline static Simd256Int32 blend(const Simd256Int32 if_false, const Simd256Int32 if_true, __m256i mask) noexcept {
	return Simd256Int32(_mm256_blendv_epi8(if_false.v, if_true.v, mask));
}







#endif // MT_SIMD_ALLOW_LEVEL3_TYPES

/**************************************************************************************************
*SIMD 128 type.Contains 4 x 32bit Signed Integers
* Requires SSE2 support.
* (will be faster with SSE4.1 enabled at compile time)
* ************************************************************************************************/
struct Simd128Int32 {
	__m128i v;
	typedef int32_t F;
	typedef __m128i MaskType;

	Simd128Int32() = default;
	Simd128Int32(__m128i a) : v(a) {};
	Simd128Int32(F a) : v(_mm_set1_epi32(a)) {};

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
	static constexpr int size_of_element() { return sizeof(int32_t); }
	static constexpr int number_of_elements() { return 4; }
	F element(int i) const { return mt::simd_detail_i32::lane_get(v, i); }
	void set_element(int i, F value) { v = mt::simd_detail_i32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd128Int32& operator+=(const Simd128Int32& rhs) noexcept { v = _mm_add_epi32(v, rhs.v); return *this; }
	Simd128Int32& operator+=(int32_t rhs) noexcept { v = _mm_add_epi32(v, _mm_set1_epi32(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd128Int32& operator-=(const Simd128Int32& rhs) noexcept { v = _mm_sub_epi32(v, rhs.v); return *this; }
	Simd128Int32& operator-=(int32_t rhs) noexcept { v = _mm_sub_epi32(v, _mm_set1_epi32(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd128Int32& operator*=(const Simd128Int32& rhs) noexcept {
		if constexpr (mt::environment::compiler_has_sse4_1) {
			v = _mm_mullo_epi32(v, rhs.v); //SSE4.1
			return *this;
		}
		else {
			// SSE2 fallback: rebuild 4x low 32-bit products (same low bits for signed/unsigned multiply).
			const auto result02 = _mm_mul_epu32(v, rhs.v);
			const auto result13 = _mm_mul_epu32(_mm_srli_si128(v, 4), _mm_srli_si128(rhs.v, 4));
			v = _mm_unpacklo_epi32(_mm_shuffle_epi32(result02, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(result13, _MM_SHUFFLE(0, 0, 2, 0)));
			return *this;
		}
	}

	Simd128Int32& operator*=(int32_t rhs) noexcept { *this *= Simd128Int32(_mm_set1_epi32(rhs)); return *this; }

	//*****Division Operators*****
	Simd128Int32& operator/=(const Simd128Int32& rhs) noexcept { v = mt::simd_detail_i32::div_epi32(v, rhs.v); return *this; }
	Simd128Int32& operator/=(int32_t rhs) noexcept { v = mt::simd_detail_i32::div_epi32(v, _mm_set1_epi32(rhs));	return *this; } //SSE

	//*****Negate Operators*****
	Simd128Int32 operator-() const noexcept {
		return Simd128Int32(_mm_sub_epi32(_mm_setzero_si128(), v));
	}

	//*****Bitwise Logic Operators*****
	Simd128Int32& operator&=(const Simd128Int32& rhs) noexcept { v = _mm_and_si128(v, rhs.v); return *this; } //SSE2
	Simd128Int32& operator|=(const Simd128Int32& rhs) noexcept { v = _mm_or_si128(v, rhs.v); return *this; }
	Simd128Int32& operator^=(const Simd128Int32& rhs) noexcept { v = _mm_xor_si128(v, rhs.v); return *this; }

	//*****Make Functions****
	static Simd128Int32 make_sequential(int32_t first) {
		const uint32_t base = static_cast<uint32_t>(first);
		return Simd128Int32(_mm_set_epi32(
			static_cast<int32_t>(base + 3u), static_cast<int32_t>(base + 2u), static_cast<int32_t>(base + 1u), static_cast<int32_t>(base)));
	}
	static Simd128Int32 make_set1(int32_t v) { return Simd128Int32(_mm_set1_epi32(v)); }






};

//*****Addition Operators*****
inline static Simd128Int32 operator+(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int32 operator+(Simd128Int32  lhs, int32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int32 operator+(int32_t lhs, Simd128Int32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd128Int32 operator-(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int32 operator-(Simd128Int32  lhs, int32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int32 operator-(const int32_t lhs, const Simd128Int32& rhs) noexcept { return Simd128Int32(_mm_sub_epi32(_mm_set1_epi32(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd128Int32 operator*(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int32 operator*(Simd128Int32  lhs, int32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int32 operator*(int32_t lhs, Simd128Int32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd128Int32 operator/(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd128Int32 operator/(Simd128Int32  lhs, int32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Int32 operator/(const int32_t lhs, const Simd128Int32& rhs) noexcept { return Simd128Int32(mt::simd_detail_i32::div_epi32(_mm_set1_epi32(lhs), rhs.v)); }


//*****Bitwise Logic Operators*****
inline static Simd128Int32 operator&(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128Int32 operator|(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128Int32 operator^(Simd128Int32  lhs, const Simd128Int32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128Int32 operator~(const Simd128Int32& lhs) noexcept { return Simd128Int32(_mm_xor_si128(lhs.v, _mm_cmpeq_epi32(lhs.v, lhs.v))); }


//*****Shifting Operators*****
inline static Simd128Int32 operator<<(const Simd128Int32& lhs, const int bits) noexcept { return Simd128Int32(_mm_slli_epi32(lhs.v, bits)); } //SSE2
inline static Simd128Int32 operator>>(const Simd128Int32& lhs, const int bits) noexcept { return Simd128Int32(_mm_srai_epi32(lhs.v, bits)); }


//*****Min/Max*****
inline static Simd128Int32 min(Simd128Int32 a, Simd128Int32 b) {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Int32(_mm_min_epi32(a.v, b.v)); //SSE4.1
	}
	else {
		//No min/max or compare for Signed ints in SSE2 so we will just unroll.
		auto m3 = std::min(a.element(3), b.element(3));
		auto m2 = std::min(a.element(2), b.element(2));
		auto m1 = std::min(a.element(1), b.element(1));
		auto m0 = std::min(a.element(0), b.element(0));
		return Simd128Int32(_mm_set_epi32(m3, m2, m1, m0));
	}
}



inline static Simd128Int32 max(Simd128Int32 a, Simd128Int32 b) {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Int32(_mm_max_epi32(a.v, b.v));  //SSE4.1
	}
	else {
		//No min/max or compare for Signed ints in SSE2 so we will just unroll.
		auto m3 = std::max(a.element(3), b.element(3));
		auto m2 = std::max(a.element(2), b.element(2));
		auto m1 = std::max(a.element(1), b.element(1));
		auto m0 = std::max(a.element(0), b.element(0));
		return Simd128Int32(_mm_set_epi32(m3, m2, m1, m0));
	}
}

//*****Mathematical*****
inline static Simd128Int32 abs(Simd128Int32 a) {
	if constexpr (mt::environment::compiler_has_ssse3) {
		return Simd128Int32(_mm_abs_epi32(a.v));
	}
	else {
		//Not supported by SSE2, so we need to emulate it.
		//This clever little code sequence is thanks to Agner Fog.
		const auto sign = _mm_srai_epi32(a.v, 31); //shift right moving in sign bits
		const auto inv = _mm_xor_si128(a.v, sign);   // invert bits if negative
		const auto result = _mm_sub_epi32(inv, sign); //add 1 if needed
		return Simd128Int32(result);
	}
}

//*****Conditional Functions*****
inline static __m128i compare_equal(const Simd128Int32 a, const Simd128Int32 b) noexcept { return _mm_cmpeq_epi32(a.v, b.v); }
inline static __m128i compare_less(const Simd128Int32 a, const Simd128Int32 b) noexcept { return _mm_cmpgt_epi32(b.v, a.v); }
inline static __m128i compare_less_equal(const Simd128Int32 a, const Simd128Int32 b) noexcept {
	return _mm_xor_si128(_mm_cmpgt_epi32(a.v, b.v), _mm_set1_epi32(-1));
}
inline static __m128i compare_greater(const Simd128Int32 a, const Simd128Int32 b) noexcept { return _mm_cmpgt_epi32(a.v, b.v); }
inline static __m128i compare_greater_equal(const Simd128Int32 a, const Simd128Int32 b) noexcept {
	return _mm_xor_si128(_mm_cmpgt_epi32(b.v, a.v), _mm_set1_epi32(-1));
}
inline static Simd128Int32 blend(const Simd128Int32 if_false, const Simd128Int32 if_true, __m128i mask) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Int32(_mm_blendv_epi8(if_false.v, if_true.v, mask));
	}
	else {
		return Simd128Int32(_mm_or_si128(_mm_andnot_si128(mask, if_false.v), _mm_and_si128(mask, if_true.v)));
	}
}


#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)

struct Simd128Int32 {
	v128_t v;
	typedef int32_t F;
	typedef v128_t MaskType;
	Simd128Int32() = default;
	Simd128Int32(v128_t a) : v(a) {}
	Simd128Int32(F a) : v(mt::simd_wasm_detail::splat<int32_t>(a)) {}

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

	static constexpr int size_of_element() { return sizeof(int32_t); }
	static constexpr int number_of_elements() { return 4; }
	F element(int i) const { return mt::simd_wasm_detail::lane_get<int32_t, 4>(v, i); }
	void set_element(int i, F value) { v = mt::simd_wasm_detail::lane_set<int32_t, 4>(v, i, value); }
	static Simd128Int32 make_sequential(int32_t first) { return Simd128Int32(mt::simd_wasm_detail::make_sequential<int32_t, 4>(first)); }
	static Simd128Int32 make_set1(int32_t value) { return Simd128Int32(mt::simd_wasm_detail::splat<int32_t>(value)); }

	Simd128Int32& operator+=(const Simd128Int32& rhs) noexcept { v = wasm_i32x4_add(v, rhs.v); return *this; }
	Simd128Int32& operator+=(int32_t rhs) noexcept { v = wasm_i32x4_add(v, make_set1(rhs).v); return *this; }
	Simd128Int32& operator-=(const Simd128Int32& rhs) noexcept { v = wasm_i32x4_sub(v, rhs.v); return *this; }
	Simd128Int32& operator-=(int32_t rhs) noexcept { v = wasm_i32x4_sub(v, make_set1(rhs).v); return *this; }
	Simd128Int32& operator*=(const Simd128Int32& rhs) noexcept { v = wasm_i32x4_mul(v, rhs.v); return *this; }
	Simd128Int32& operator*=(int32_t rhs) noexcept { v = wasm_i32x4_mul(v, make_set1(rhs).v); return *this; }
	Simd128Int32& operator/=(const Simd128Int32& rhs) noexcept {
		v = mt::simd_wasm_detail::map_binary<int32_t, 4>(v, rhs.v, [](int32_t a, int32_t b) { return a / b; });
		return *this;
	}
	Simd128Int32& operator/=(int32_t rhs) noexcept { return *this /= make_set1(rhs); }
	Simd128Int32 operator-() const noexcept { return Simd128Int32(wasm_i32x4_neg(v)); }
	Simd128Int32& operator&=(const Simd128Int32& rhs) noexcept { v = wasm_v128_and(v, rhs.v); return *this; }
	Simd128Int32& operator|=(const Simd128Int32& rhs) noexcept { v = wasm_v128_or(v, rhs.v); return *this; }
	Simd128Int32& operator^=(const Simd128Int32& rhs) noexcept { v = wasm_v128_xor(v, rhs.v); return *this; }
};

inline static Simd128Int32 operator+(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int32 operator+(Simd128Int32 lhs, int32_t rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Int32 operator+(int32_t lhs, Simd128Int32 rhs) noexcept { rhs += lhs; return rhs; }
inline static Simd128Int32 operator-(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int32 operator-(Simd128Int32 lhs, int32_t rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Int32 operator-(int32_t lhs, const Simd128Int32& rhs) noexcept { return Simd128Int32::make_set1(lhs) - rhs; }
inline static Simd128Int32 operator*(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int32 operator*(Simd128Int32 lhs, int32_t rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Int32 operator*(int32_t lhs, Simd128Int32 rhs) noexcept { rhs *= lhs; return rhs; }
inline static Simd128Int32 operator/(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Int32 operator/(Simd128Int32 lhs, int32_t rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Int32 operator/(int32_t lhs, const Simd128Int32& rhs) noexcept { return Simd128Int32::make_set1(lhs) / rhs; }
inline static Simd128Int32 operator&(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs &= rhs; return lhs; }
inline static Simd128Int32 operator|(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs |= rhs; return lhs; }
inline static Simd128Int32 operator^(Simd128Int32 lhs, const Simd128Int32& rhs) noexcept { lhs ^= rhs; return lhs; }
inline static Simd128Int32 operator~(const Simd128Int32& lhs) noexcept { return Simd128Int32(wasm_v128_not(lhs.v)); }
inline static Simd128Int32 operator<<(const Simd128Int32& lhs, const int bits) noexcept { return Simd128Int32(wasm_i32x4_shl(lhs.v, static_cast<uint32_t>(bits))); }
inline static Simd128Int32 operator>>(const Simd128Int32& lhs, const int bits) noexcept { return Simd128Int32(wasm_i32x4_shr(lhs.v, static_cast<uint32_t>(bits))); }

inline static Simd128Int32 min(Simd128Int32 a, Simd128Int32 b) { return Simd128Int32(wasm_i32x4_min(a.v, b.v)); }
inline static Simd128Int32 max(Simd128Int32 a, Simd128Int32 b) { return Simd128Int32(wasm_i32x4_max(a.v, b.v)); }
inline static Simd128Int32 abs(Simd128Int32 a) { return Simd128Int32(wasm_i32x4_abs(a.v)); }

inline static v128_t compare_equal(const Simd128Int32 a, const Simd128Int32 b) noexcept { return wasm_i32x4_eq(a.v, b.v); }
inline static v128_t compare_less(const Simd128Int32 a, const Simd128Int32 b) noexcept { return wasm_i32x4_lt(a.v, b.v); }
inline static v128_t compare_less_equal(const Simd128Int32 a, const Simd128Int32 b) noexcept { return wasm_i32x4_le(a.v, b.v); }
inline static v128_t compare_greater(const Simd128Int32 a, const Simd128Int32 b) noexcept { return wasm_i32x4_gt(a.v, b.v); }
inline static v128_t compare_greater_equal(const Simd128Int32 a, const Simd128Int32 b) noexcept { return wasm_i32x4_ge(a.v, b.v); }
inline static Simd128Int32 blend(const Simd128Int32 if_false, const Simd128Int32 if_true, v128_t mask) noexcept {
	return Simd128Int32(wasm_v128_bitselect(if_true.v, if_false.v, mask));
}

#endif //x86_64 / wasm




/**************************************************************************************************
 * Templated Functions for all types
 * ************************************************************************************************/
template <SimdInt32 T>
[[nodiscard("Value Calculated and not used (if_equal)")]]
inline static T if_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_equal(value_a, value_b));
}

template <SimdInt32 T>
[[nodiscard("Value Calculated and not used (if_less)")]]
inline static T if_less(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less(value_a, value_b));
}

template <SimdInt32 T>
[[nodiscard("Value Calculated and not used (if_less_equal)")]]
inline static T if_less_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less_equal(value_a, value_b));
}

template <SimdInt32 T>
[[nodiscard("Value Calculated and not used (if_greater)")]]
inline static T if_greater(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater(value_a, value_b));
}

template <SimdInt32 T>
[[nodiscard("Value Calculated and not used (if_greater_equal)")]]
inline static T if_greater_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater_equal(value_a, value_b));
}



/**************************************************************************************************
 * Check that each type implements the desired types from simd-concepts.h
 * (Not sure why this fails on intelisense, but compliles ok.)
 * ************************************************************************************************/
static_assert(Simd<FallbackInt32>, "FallbackInt32 does not implement the concept Simd");
static_assert(SimdSigned<FallbackInt32>, "FallbackInt32 does not implement the concept SimdSigned");
static_assert(SimdInt<FallbackInt32>, "FallbackInt32 does not implement the concept SimdInt");
static_assert(SimdInt32<FallbackInt32>, "FallbackInt32 does not implement the concept SimdInt32");
static_assert(SimdCompareOps<FallbackInt32>, "FallbackInt32 does not implement the concept SimdCompareOps");

#if MT_SIMD_ARCH_X64
static_assert(Simd<Simd128Int32>, "Simd128Int32 does not implement the concept Simd");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(Simd<Simd256Int32>, "Simd256Int32 does not implement the concept Simd");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(Simd<Simd512Int32>, "Simd512Int32 does not implement the concept Simd");
#endif

static_assert(SimdSigned<Simd128Int32>, "Simd128Int32 does not implement the concept SimdSigned");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdSigned<Simd256Int32>, "Simd256Int32 does not implement the concept SimdSigned");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdSigned<Simd512Int32>, "Simd512Int32 does not implement the concept SimdSigned");
#endif

static_assert(SimdInt<Simd128Int32>, "Simd128Int32 does not implement the concept SimdInt");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdInt<Simd256Int32>, "Simd256Int32 does not implement the concept SimdInt");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdInt<Simd512Int32>, "Simd512Int32 does not implement the concept SimdInt");
#endif



static_assert(SimdInt32<Simd128Int32>, "Simd128Int32 does not implement the concept SimdInt32");
static_assert(SimdCompareOps<Simd128Int32>, "Simd128Int32 does not implement the concept SimdCompareOps");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdInt32<Simd256Int32>, "Simd256Int32 does not implement the concept SimdInt32");
static_assert(SimdCompareOps<Simd256Int32>, "Simd256Int32 does not implement the concept SimdCompareOps");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdInt32<Simd512Int32>, "Simd512Int32 does not implement the concept SimdInt32");
static_assert(SimdCompareOps<Simd512Int32>, "Simd512Int32 does not implement the concept SimdCompareOps");
#endif
#endif



/**************************************************************************************************
 Define SimdNativeInt32 as the best supported type at compile time.
*************************************************************************************************/
#if MT_SIMD_ARCH_X64
	#if MT_SIMD_ALLOW_LEVEL4_TYPES
		typedef Simd512Int32 SimdNativeInt32;
	#elif MT_SIMD_ALLOW_LEVEL3_TYPES
		typedef Simd256Int32 SimdNativeInt32;
	#else
		typedef Simd128Int32 SimdNativeInt32;
	#endif
#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)
	typedef Simd128Int32 SimdNativeInt32;
#else
	typedef FallbackInt32 SimdNativeInt32;
#endif



