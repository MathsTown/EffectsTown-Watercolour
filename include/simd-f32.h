#pragma once
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

Basic SIMD Types for 32-bit floats:

FallbackFloat32		- Works on all build modes and CPUs.  Forwards most requests to the standard library.

Simd128Float32		- x86_64 Microarchitecture Level 1 - Works on all x86_64 CPUs. 
					- Requires SSE and SSE2 support.  Will use SSE4.1 instructions when __SSE4_1__ or __AVX__ defined.

Simd256Float32		- x86_64 Microarchitecture Level 3.
					- Requires AVX, AVX2 and FMA support.

Simd512Float32		- x86_64 Microarchitecture Level 4.
					- Requires AVX512F, AVX512DQ, ACX512VL, AVX512CD, AVX512BW

SimdNativeFloat32	- A Typedef referring to one of the above types.  Chosen based on compiler support/mode.
                    - Just use this type in your code if you are building for a specific platform.


Checking CPU Support:
Unless you are using a SimdNative typedef, you must check for CPU support before using any of these types.
- MSVC - You may check at runtime or compile time.  (compile time checks generally results in much faster code)
- GCC/Clang - You must check at compile time (due to compiler limitations)

Types reqpresenting floats, doubles, ints, longs etc are arranged in microarchitecture level groups.
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

WASM Support:
I've included FallbackFloat32 for use with Emscripen, but use SimdNativeFloat32 as SIMD support will be added soon.


*********************************************************************************************************/


#include <cmath>
#include <cstring>

#include "environment.h"
#include "simd-cpuid.h"
#include "simd-concepts.h"
#include "simd-mask.h"
#include "simd-uint32.h"
#include "simd-uint64.h"
#include "simd-wasm-helpers.h"

/***************************************************************************************************************************************************************************************************
 * Fallback to a single 32 bit float
 **************************************************************************************************************************************************************************************************/
struct FallbackFloat32 {
	float v;

	typedef float F;
	typedef FallbackUInt32 U;
	typedef FallbackUInt64 U64;
	typedef bool MaskType;

	FallbackFloat32() = default;
	FallbackFloat32(float a) : v(a) {};

	
	//*****Support Informtion*****
	
	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported() {return true;}

	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported(CpuInformation) {return true;}

	//Performs a compile time CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static constexpr bool compiler_supported() {
		return true;
	}

	//*****Access Elements*****
	static constexpr int size_of_element() { return sizeof(float); }
	static constexpr int number_of_elements() { return 1; }	
	F element(int)  const { return v; }
	void set_element(int, F value) { v = value; }

	//*****Addition Operators*****
	FallbackFloat32& operator+=(const FallbackFloat32& rhs) noexcept { v += rhs.v; return *this; }
	FallbackFloat32& operator+=(float rhs) noexcept { v += rhs;	return *this; }

	//*****Subtraction Operators*****
	FallbackFloat32& operator-=(const FallbackFloat32& rhs) noexcept { v -= rhs.v; return *this; }
	FallbackFloat32& operator-=(float rhs) noexcept { v -= rhs;	return *this; }

	//*****Multiplication Operators*****
	FallbackFloat32& operator*=(const FallbackFloat32& rhs) noexcept { v *= rhs.v; return *this; }
	FallbackFloat32& operator*=(float rhs) noexcept { v *= rhs; return *this; }

	//*****Division Operators*****
	FallbackFloat32& operator/=(const FallbackFloat32& rhs) noexcept { v /= rhs.v; return *this; }
	FallbackFloat32& operator/=(float rhs) noexcept { v /= rhs;	return *this; }

	//*****Negate Operators*****
	FallbackFloat32 operator-() const noexcept { return FallbackFloat32(-v); }

	//*****Make Functions****
	static FallbackFloat32 make_sequential(F first) { return FallbackFloat32(first); }
	static FallbackFloat32 make_set1(F v) { return FallbackFloat32(v); }
	static FallbackFloat32 make_from_int32(FallbackUInt32 i) { return FallbackFloat32(static_cast<float>(i.v)); }

	//*****Cast Functions****
	FallbackUInt32 bitcast_to_uint() const noexcept { return FallbackUInt32(std::bit_cast<uint32_t>(this->v)); }

	

};

//*****Addition Operators*****
inline static FallbackFloat32 operator+(FallbackFloat32  lhs, const FallbackFloat32& rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackFloat32 operator+(FallbackFloat32  lhs, float rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackFloat32 operator+(float lhs, FallbackFloat32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static FallbackFloat32 operator-(FallbackFloat32  lhs, const FallbackFloat32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackFloat32 operator-(FallbackFloat32  lhs, float rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackFloat32 operator-(const float lhs, const FallbackFloat32& rhs) noexcept { return FallbackFloat32(lhs - rhs.v); }

//*****Multiplication Operators*****
inline static FallbackFloat32 operator*(FallbackFloat32  lhs, const FallbackFloat32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackFloat32 operator*(FallbackFloat32  lhs, float rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackFloat32 operator*(float lhs, FallbackFloat32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static FallbackFloat32 operator/(FallbackFloat32  lhs, const FallbackFloat32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static FallbackFloat32 operator/(FallbackFloat32  lhs, float rhs) noexcept { lhs /= rhs; return lhs; }
inline static FallbackFloat32 operator/(const float lhs, const FallbackFloat32& rhs) noexcept { return FallbackFloat32(lhs / rhs.v); }


//*****Fused Multiply Add Fallbacks*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static FallbackFloat32 fma(const FallbackFloat32  a, const FallbackFloat32 b, const FallbackFloat32 c) { 
	//return a * b + c; 
	return std::fma(a.v, b.v, c.v);
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static FallbackFloat32 fms(const FallbackFloat32  a, const FallbackFloat32 b, const FallbackFloat32 c) {
	//return a * b - c; 
	return std::fma(a.v, b.v, -c.v);
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static FallbackFloat32 fnma(const FallbackFloat32  a, const FallbackFloat32 b, const FallbackFloat32 c) { 
	//return -a * b + c; 
	return std::fma(-a.v, b.v, c.v);
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static FallbackFloat32 fnms(const FallbackFloat32  a, const FallbackFloat32 b, const FallbackFloat32 c) { 
	//return -a * b - c; 
	return std::fma(-a.v, b.v, -c.v);
}




//*****Rounding Functions*****
inline static FallbackFloat32 floor(FallbackFloat32 a) { return  FallbackFloat32(std::floor(a.v)); }
inline static FallbackFloat32 ceil(FallbackFloat32 a) { return  FallbackFloat32(std::ceil(a.v)); }
inline static FallbackFloat32 trunc(FallbackFloat32 a) { return  FallbackFloat32(std::trunc(a.v)); }
inline static FallbackFloat32 round(FallbackFloat32 a) { return  FallbackFloat32(std::round(a.v)); }
inline static FallbackFloat32 fract(FallbackFloat32 a) { return a - floor(a); }


//*****Min/Max*****
inline static FallbackFloat32 min(FallbackFloat32 a, FallbackFloat32 b) { return FallbackFloat32(std::min(a.v, b.v)); }
inline static FallbackFloat32 max(FallbackFloat32 a, FallbackFloat32 b) { return FallbackFloat32(std::max(a.v, b.v)); }

//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static FallbackFloat32 clamp(const FallbackFloat32 a) noexcept {
	return std::min(std::max(a.v, 0.0f), 1.0f);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static FallbackFloat32 clamp(const FallbackFloat32 a, const FallbackFloat32 min_f, const FallbackFloat32 max_f) noexcept {
	return std::min(std::max(a.v, min_f.v), max_f.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static FallbackFloat32 clamp(const FallbackFloat32 a, const float min_f, const float max_f) noexcept {
	return std::min(std::max(a.v, min_f), max_f);
}



//*****Approximate Functions*****
inline static FallbackFloat32 reciprocal_approx(FallbackFloat32 a) noexcept { return FallbackFloat32(1.0f / a.v); }

//*****Mathematical Functions*****
inline static FallbackFloat32 sqrt(FallbackFloat32 a) { return FallbackFloat32(std::sqrt(a.v)); }
inline static FallbackFloat32 abs(FallbackFloat32 a) { return FallbackFloat32(std::abs(a.v)); }
inline static FallbackFloat32 pow(FallbackFloat32 a, FallbackFloat32 b) { return FallbackFloat32(std::pow(a.v,b.v)); }
inline static FallbackFloat32 exp(FallbackFloat32 a) { return FallbackFloat32(std::exp(a.v)); }
inline static FallbackFloat32 exp2(FallbackFloat32 a) { return FallbackFloat32(std::exp2(a.v)); }
inline static FallbackFloat32 exp10(FallbackFloat32 a) { return FallbackFloat32(std::pow(10.0f,a.v)); }
inline static FallbackFloat32 expm1(FallbackFloat32 a) { return FallbackFloat32(std::expm1(a.v)); }
inline static FallbackFloat32 log(FallbackFloat32 a) { return FallbackFloat32(std::log(a.v)); }
inline static FallbackFloat32 log1p(FallbackFloat32 a) { return FallbackFloat32(std::log1p(a.v)); }
inline static FallbackFloat32 log2(FallbackFloat32 a) { return FallbackFloat32(std::log2(a.v)); }
inline static FallbackFloat32 log10(FallbackFloat32 a) { return FallbackFloat32(std::log10(a.v)); }
inline static FallbackFloat32 cbrt(FallbackFloat32 a) { return FallbackFloat32(std::cbrt(a.v)); }
inline static FallbackFloat32 hypot(FallbackFloat32 a, FallbackFloat32 b) { return FallbackFloat32(std::hypot(a.v, b.v)); }

inline static FallbackFloat32 sin(FallbackFloat32 a) { return FallbackFloat32(std::sin(a.v)); }
inline static FallbackFloat32 cos(FallbackFloat32 a) { return FallbackFloat32(std::cos(a.v)); }
inline static FallbackFloat32 tan(FallbackFloat32 a) { return FallbackFloat32(std::tan(a.v)); }
inline static FallbackFloat32 asin(FallbackFloat32 a) { return FallbackFloat32(std::asin(a.v)); }
inline static FallbackFloat32 acos(FallbackFloat32 a) { return FallbackFloat32(std::acos(a.v)); }
inline static FallbackFloat32 atan(FallbackFloat32 a) { return FallbackFloat32(std::atan(a.v)); }
inline static FallbackFloat32 atan2(FallbackFloat32 y, FallbackFloat32 x) { return FallbackFloat32(std::atan2(y.v, x.v)); }
inline static FallbackFloat32 sinh(FallbackFloat32 a) { return FallbackFloat32(std::sinh(a.v)); }
inline static FallbackFloat32 cosh(FallbackFloat32 a) { return FallbackFloat32(std::cosh(a.v)); }
inline static FallbackFloat32 tanh(FallbackFloat32 a) { return FallbackFloat32(std::tanh(a.v)); }
inline static FallbackFloat32 asinh(FallbackFloat32 a) { return FallbackFloat32(std::asinh(a.v)); }
inline static FallbackFloat32 acosh(FallbackFloat32 a) { return FallbackFloat32(std::acosh(a.v)); }
inline static FallbackFloat32 atanh(FallbackFloat32 a) { return FallbackFloat32(std::atanh(a.v)); }


//*****Conditional Functions *****

//Compare if 2 values are equal and return a mask.
inline static bool compare_equal(const FallbackFloat32 a, const FallbackFloat32 b) noexcept { return (a.v == b.v); }
inline static bool compare_less(const FallbackFloat32 a, const FallbackFloat32 b) noexcept { return(a.v < b.v); }
inline static bool compare_less_equal(const FallbackFloat32 a, const FallbackFloat32 b) noexcept { return (a.v <= b.v); }
inline static bool compare_greater(const FallbackFloat32 a, const FallbackFloat32 b) noexcept { return (a.v > b.v); }
inline static bool compare_greater_equal(const FallbackFloat32 a, const FallbackFloat32 b) noexcept { return (a.v >= b.v); }
inline static bool isnan(const FallbackFloat32 a) noexcept { return std::isnan(a.v); }

//Blend two values together based on mask.  First argument if zero. Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static FallbackFloat32 blend(const FallbackFloat32 if_false, const FallbackFloat32 if_true, bool mask) noexcept {
	return (mask) ? if_true : if_false;
}






//***************** x86_64 only code ******************
#if MT_SIMD_ARCH_X64
#include <immintrin.h>


/**************************************************************************************************
 * Compiler Compatability Layer
 * MSCV intrinsics are sometime a little more feature rich than GCC and Clang.  
 * This section provides shims and patches for compiler incompatible behaviour.
 * ************************************************************************************************/

namespace mt::simd_detail_f32 {
	// Portability layer: GCC/Clang cannot index SIMD lanes via MSVC vector members.
	inline float lane_get(__m128 v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m128_f32[i];
#else
		alignas(16) float lanes[4];
		_mm_storeu_ps(lanes, v);
		return lanes[i];
#endif
	}
	inline __m128 lane_set(__m128 v, int i, float value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m128_f32[i] = value;
		return v;
#else
		alignas(16) float lanes[4];
		_mm_storeu_ps(lanes, v);
		lanes[i] = value;
		return _mm_loadu_ps(lanes);
#endif
	}
	inline float lane_get(const __m256& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m256_f32[i];
#else
		alignas(32) float lanes[8];
		_mm256_storeu_ps(lanes, v);
		return lanes[i];
#endif
	}
	inline void lane_set(__m256& v, int i, float value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m256_f32[i] = value;
#else
		alignas(32) float lanes[8];
		_mm256_storeu_ps(lanes, v);
		lanes[i] = value;
		v = _mm256_loadu_ps(lanes);
#endif
	}
	inline float lane_get(const __m512& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m512_f32[i];
#else
		alignas(64) float lanes[16];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	inline void lane_set(__m512& v, int i, float value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m512_f32[i] = value;
#else
		alignas(64) float lanes[16];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

#if MT_SIMD_USE_PORTABLE_X86_SHIMS
	template <typename Fn>
	inline __m128 map_unary(__m128 v, Fn fn) {
		alignas(16) float in[4], out[4];
		_mm_storeu_ps(in, v);
		for (int i = 0; i < 4; ++i) out[i] = fn(in[i]);
		return _mm_loadu_ps(out);
	}
	template <typename Fn>
	inline __m256 map_unary(const __m256& v, Fn fn) {
		alignas(32) float in[8], out[8];
		_mm256_storeu_ps(in, v);
		for (int i = 0; i < 8; ++i) out[i] = fn(in[i]);
		return _mm256_loadu_ps(out);
	}
	template <typename Fn>
	inline __m512 map_unary(const __m512& v, Fn fn) {
		alignas(64) float in[16], out[16];
		std::memcpy(in, &v, sizeof(v));
		for (int i = 0; i < 16; ++i) out[i] = fn(in[i]);
		__m512 out_v;
		std::memcpy(&out_v, out, sizeof(out_v));
		return out_v;
	}
	template <typename Fn>
	inline __m128 map_binary(__m128 a, __m128 b, Fn fn) {
		alignas(16) float lhs[4], rhs[4], out[4];
		_mm_storeu_ps(lhs, a); _mm_storeu_ps(rhs, b);
		for (int i = 0; i < 4; ++i) out[i] = fn(lhs[i], rhs[i]);
		return _mm_loadu_ps(out);
	}
	template <typename Fn>
	inline __m256 map_binary(const __m256& a, const __m256& b, Fn fn) {
		alignas(32) float lhs[8], rhs[8], out[8];
		_mm256_storeu_ps(lhs, a); _mm256_storeu_ps(rhs, b);
		for (int i = 0; i < 8; ++i) out[i] = fn(lhs[i], rhs[i]);
		return _mm256_loadu_ps(out);
	}
	template <typename Fn>
	inline __m512 map_binary(const __m512& a, const __m512& b, Fn fn) {
		alignas(64) float lhs[16], rhs[16], out[16];
		std::memcpy(lhs, &a, sizeof(a));
		std::memcpy(rhs, &b, sizeof(b));
		for (int i = 0; i < 16; ++i) out[i] = fn(lhs[i], rhs[i]);
		__m512 out_v;
		std::memcpy(&out_v, out, sizeof(out_v));
		return out_v;
	}
#endif

#if MT_SIMD_USE_PORTABLE_X86_SHIMS && !MT_USE_SVML
	#define MT_F32_UNARY(name, expr) \
		inline __m128 name##_ps(__m128 v) { return map_unary(v, [](float x) { return (expr); }); } \
		inline __m256 name##_ps(__m256 v) { return map_unary(v, [](float x) { return (expr); }); } \
		inline __m512 name##_ps(__m512 v) { return map_unary(v, [](float x) { return (expr); }); }

	#define MT_F32_BINARY(name, expr) \
		inline __m128 name##_ps(__m128 a, __m128 b) { return map_binary(a, b, [](float x, float y) { return (expr); }); } \
		inline __m256 name##_ps(__m256 a, __m256 b) { return map_binary(a, b, [](float x, float y) { return (expr); }); } \
		inline __m512 name##_ps(__m512 a, __m512 b) { return map_binary(a, b, [](float x, float y) { return (expr); }); }

	MT_F32_UNARY(trunc, std::trunc(x))
	MT_F32_UNARY(round, std::round(x))
	MT_F32_UNARY(floor, std::floor(x))
	MT_F32_UNARY(ceil, std::ceil(x))
	MT_F32_UNARY(exp, std::exp(x))
	MT_F32_UNARY(exp2, std::exp2(x))
	MT_F32_UNARY(exp10, std::pow(10.0f, x))
	MT_F32_UNARY(expm1, std::expm1(x))
	MT_F32_UNARY(log, std::log(x))
	MT_F32_UNARY(log1p, std::log1p(x))
	MT_F32_UNARY(log2, std::log2(x))
	MT_F32_UNARY(log10, std::log10(x))
	MT_F32_UNARY(cbrt, std::cbrt(x))
	MT_F32_UNARY(sin, std::sin(x))
	MT_F32_UNARY(cos, std::cos(x))
	MT_F32_UNARY(tan, std::tan(x))
	MT_F32_UNARY(asin, std::asin(x))
	MT_F32_UNARY(acos, std::acos(x))
	MT_F32_UNARY(atan, std::atan(x))
	MT_F32_UNARY(sinh, std::sinh(x))
	MT_F32_UNARY(cosh, std::cosh(x))
	MT_F32_UNARY(tanh, std::tanh(x))
	MT_F32_UNARY(asinh, std::asinh(x))
	MT_F32_UNARY(acosh, std::acosh(x))
	MT_F32_UNARY(atanh, std::atanh(x))
	MT_F32_BINARY(pow, std::pow(x, y))
	MT_F32_BINARY(hypot, std::hypot(x, y))
	MT_F32_BINARY(atan2, std::atan2(x, y))

	#undef MT_F32_UNARY
	#undef MT_F32_BINARY
#endif

}

#if MT_SIMD_USE_PORTABLE_X86_SHIMS
// Portability shim: these math names are not standard hardware intrinsics on GCC/Clang.
#define _mm_trunc_ps mt::simd_detail_f32::trunc_ps
#define _mm256_trunc_ps mt::simd_detail_f32::trunc_ps
#define _mm512_trunc_ps mt::simd_detail_f32::trunc_ps
#define _mm512_floor_ps mt::simd_detail_f32::floor_ps
#define _mm512_ceil_ps mt::simd_detail_f32::ceil_ps
#define _mm_pow_ps mt::simd_detail_f32::pow_ps
#define _mm256_pow_ps mt::simd_detail_f32::pow_ps
#define _mm512_pow_ps mt::simd_detail_f32::pow_ps
#define _mm_exp_ps mt::simd_detail_f32::exp_ps
#define _mm256_exp_ps mt::simd_detail_f32::exp_ps
#define _mm512_exp_ps mt::simd_detail_f32::exp_ps
#define _mm_exp2_ps mt::simd_detail_f32::exp2_ps
#define _mm256_exp2_ps mt::simd_detail_f32::exp2_ps
#define _mm512_exp2_ps mt::simd_detail_f32::exp2_ps
#define _mm_exp10_ps mt::simd_detail_f32::exp10_ps
#define _mm256_exp10_ps mt::simd_detail_f32::exp10_ps
#define _mm512_exp10_ps mt::simd_detail_f32::exp10_ps
#define _mm_expm1_ps mt::simd_detail_f32::expm1_ps
#define _mm256_expm1_ps mt::simd_detail_f32::expm1_ps
#define _mm512_expm1_ps mt::simd_detail_f32::expm1_ps
#define _mm_log_ps mt::simd_detail_f32::log_ps
#define _mm256_log_ps mt::simd_detail_f32::log_ps
#define _mm512_log_ps mt::simd_detail_f32::log_ps
#define _mm_log1p_ps mt::simd_detail_f32::log1p_ps
#define _mm256_log1p_ps mt::simd_detail_f32::log1p_ps
#define _mm512_log1p_ps mt::simd_detail_f32::log1p_ps
#define _mm_log2_ps mt::simd_detail_f32::log2_ps
#define _mm256_log2_ps mt::simd_detail_f32::log2_ps
#define _mm512_log2_ps mt::simd_detail_f32::log2_ps
#define _mm_log10_ps mt::simd_detail_f32::log10_ps
#define _mm256_log10_ps mt::simd_detail_f32::log10_ps
#define _mm512_log10_ps mt::simd_detail_f32::log10_ps
#define _mm_cbrt_ps mt::simd_detail_f32::cbrt_ps
#define _mm256_cbrt_ps mt::simd_detail_f32::cbrt_ps
#define _mm512_cbrt_ps mt::simd_detail_f32::cbrt_ps
#define _mm_hypot_ps mt::simd_detail_f32::hypot_ps
#define _mm256_hypot_ps mt::simd_detail_f32::hypot_ps
#define _mm512_hypot_ps mt::simd_detail_f32::hypot_ps
#define _mm_sin_ps mt::simd_detail_f32::sin_ps
#define _mm256_sin_ps mt::simd_detail_f32::sin_ps
#define _mm512_sin_ps mt::simd_detail_f32::sin_ps
#define _mm_cos_ps mt::simd_detail_f32::cos_ps
#define _mm256_cos_ps mt::simd_detail_f32::cos_ps
#define _mm512_cos_ps mt::simd_detail_f32::cos_ps
#define _mm_tan_ps mt::simd_detail_f32::tan_ps
#define _mm256_tan_ps mt::simd_detail_f32::tan_ps
#define _mm512_tan_ps mt::simd_detail_f32::tan_ps
#define _mm_asin_ps mt::simd_detail_f32::asin_ps
#define _mm256_asin_ps mt::simd_detail_f32::asin_ps
#define _mm512_asin_ps mt::simd_detail_f32::asin_ps
#define _mm_acos_ps mt::simd_detail_f32::acos_ps
#define _mm256_acos_ps mt::simd_detail_f32::acos_ps
#define _mm512_acos_ps mt::simd_detail_f32::acos_ps
#define _mm_atan_ps mt::simd_detail_f32::atan_ps
#define _mm256_atan_ps mt::simd_detail_f32::atan_ps
#define _mm512_atan_ps mt::simd_detail_f32::atan_ps
#define _mm_atan2_ps mt::simd_detail_f32::atan2_ps
#define _mm256_atan2_ps mt::simd_detail_f32::atan2_ps
#define _mm512_atan2_ps mt::simd_detail_f32::atan2_ps
#define _mm_sinh_ps mt::simd_detail_f32::sinh_ps
#define _mm256_sinh_ps mt::simd_detail_f32::sinh_ps
#define _mm512_sinh_ps mt::simd_detail_f32::sinh_ps
#define _mm_cosh_ps mt::simd_detail_f32::cosh_ps
#define _mm256_cosh_ps mt::simd_detail_f32::cosh_ps
#define _mm512_cosh_ps mt::simd_detail_f32::cosh_ps
#define _mm_tanh_ps mt::simd_detail_f32::tanh_ps
#define _mm256_tanh_ps mt::simd_detail_f32::tanh_ps
#define _mm512_tanh_ps mt::simd_detail_f32::tanh_ps
#define _mm_asinh_ps mt::simd_detail_f32::asinh_ps
#define _mm256_asinh_ps mt::simd_detail_f32::asinh_ps
#define _mm512_asinh_ps mt::simd_detail_f32::asinh_ps
#define _mm_acosh_ps mt::simd_detail_f32::acosh_ps
#define _mm256_acosh_ps mt::simd_detail_f32::acosh_ps
#define _mm512_acosh_ps mt::simd_detail_f32::acosh_ps
#define _mm_atanh_ps mt::simd_detail_f32::atanh_ps
#define _mm256_atanh_ps mt::simd_detail_f32::atanh_ps
#define _mm512_atanh_ps mt::simd_detail_f32::atanh_ps
#endif



/****************************************************************************************************************************************************************************************************
 * SIMD 512 type.  Contains 16 x 32bit Floats
 * Requires AVX-512F support.
 * **************************************************************************************************************************************************************************************************/
#if MT_SIMD_ALLOW_LEVEL4_TYPES
struct Simd512Float32 {
	__m512 v;

	typedef float F;
	typedef __mmask16 MaskType;
	typedef Simd512UInt32 U;
	

	Simd512Float32() = default;
	Simd512Float32(__m512 a) : v(a) {};
	Simd512Float32(F a) : v(_mm512_set1_ps(a)) {};

	
	//*****Support Informtion*****
	
	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same class may not be supported) 
	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}

	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same class may not be supported) 
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.is_level_4();
	}

	//Performs a compile time support. Checks this type ONLY (integers in same class may not be supported) 
	static constexpr bool compiler_supported() {
		return mt::environment::compiler_can_use_x86_64_level_4_types;
	}

	static constexpr int size_of_element() { return sizeof(float); }
	static constexpr int number_of_elements() { return 16; }

	//*****Access Elements*****
	F element(int i)  const { return mt::simd_detail_f32::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_f32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd512Float32& operator+=(const Simd512Float32& rhs) noexcept { v = _mm512_add_ps(v, rhs.v); return *this; }
	Simd512Float32& operator+=(float rhs) noexcept { v = _mm512_add_ps(v, _mm512_set1_ps(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd512Float32& operator-=(const Simd512Float32& rhs) noexcept { v = _mm512_sub_ps(v, rhs.v); return *this; }
	Simd512Float32& operator-=(float rhs) noexcept { v = _mm512_sub_ps(v, _mm512_set1_ps(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd512Float32& operator*=(const Simd512Float32& rhs) noexcept { v = _mm512_mul_ps(v, rhs.v); return *this; }
	Simd512Float32& operator*=(float rhs) noexcept { v = _mm512_mul_ps(v, _mm512_set1_ps(rhs)); return *this; }

	//*****Division Operators*****
	Simd512Float32& operator/=(const Simd512Float32& rhs) noexcept { v = _mm512_div_ps(v, rhs.v); return *this; }
	Simd512Float32& operator/=(float rhs) noexcept { v = _mm512_div_ps(v, _mm512_set1_ps(rhs));	return *this; }

	//*****Negate Operators*****
	Simd512Float32 operator-() const noexcept { return Simd512Float32(_mm512_sub_ps(_mm512_setzero_ps(), v)); }

	//*****Make Functions****
	static Simd512Float32 make_sequential(F first) { return Simd512Float32(_mm512_set_ps(first+15.0f, first + 14.0f, first + 13.0f, first + 12.0f, first + 11.0f, first + 10.0f, first + 9.0f, first + 8.0f, first + 7.0f, first + 6.0f, first + 5.0f, first + 4.0f, first + 3.0f, first + 2.0f, first + 1.0f, first)); }
	static Simd512Float32 make_set1(F v) { return Simd512Float32(_mm512_set1_ps(v)); }
	

	static Simd512Float32 make_from_int32(Simd512UInt32 i) { return Simd512Float32(_mm512_cvtepu32_ps(i.v)); }

	//*****Cast Functions****

	//Converts to an unsigned integer.  No check is performed to see if that type is supported.
	Simd512UInt32 bitcast_to_uint() const { return Simd512UInt32(_mm512_castps_si512(this->v)); }
	

	

};

//*****Addition Operators*****
inline static Simd512Float32 operator+(Simd512Float32  lhs, const Simd512Float32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Float32 operator+(Simd512Float32  lhs, float rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Float32 operator+(float lhs, Simd512Float32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd512Float32 operator-(Simd512Float32  lhs, const Simd512Float32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Float32 operator-(Simd512Float32  lhs, float rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Float32 operator-(const float lhs, const Simd512Float32& rhs) noexcept { return Simd512Float32(_mm512_sub_ps(_mm512_set1_ps(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd512Float32 operator*(Simd512Float32  lhs, const Simd512Float32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Float32 operator*(Simd512Float32  lhs, float rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Float32 operator*(float lhs, Simd512Float32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd512Float32 operator/(Simd512Float32  lhs, const Simd512Float32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd512Float32 operator/(Simd512Float32  lhs, float rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd512Float32 operator/(const float lhs, const Simd512Float32& rhs) noexcept { return Simd512Float32(_mm512_div_ps(_mm512_set1_ps(lhs), rhs.v)); }


//*****Fused Multiply Add Instructions*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd512Float32 fma(const Simd512Float32  a, const Simd512Float32 b, const Simd512Float32 c) {
	return _mm512_fmadd_ps(a.v, b.v, c.v);
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd512Float32 fms(const Simd512Float32  a, const Simd512Float32 b, const Simd512Float32 c) {
	return _mm512_fmsub_ps(a.v, b.v, c.v);
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd512Float32 fnma(const Simd512Float32  a, const Simd512Float32 b, const Simd512Float32 c) {
	return _mm512_fnmadd_ps(a.v, b.v, c.v);
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd512Float32 fnms(const Simd512Float32  a, const Simd512Float32 b, const Simd512Float32 c) {
	return _mm512_fnmsub_ps(a.v, b.v, c.v);
}



//*****Rounding Functions*****
[[nodiscard("Value calculated and not used (floor)")]]
inline static Simd512Float32 floor(Simd512Float32 a) noexcept { return  Simd512Float32(_mm512_floor_ps(a.v)); }
[[nodiscard("Value calculated and not used (ceil)")]]
inline static Simd512Float32 ceil(Simd512Float32 a)  noexcept { return  Simd512Float32(_mm512_ceil_ps(a.v)); }
[[nodiscard("Value calculated and not used (trunc)")]]
inline static Simd512Float32 trunc(Simd512Float32 a) noexcept { return  Simd512Float32(_mm512_trunc_ps(a.v)); }
[[nodiscard("Value calculated and not used (round)")]]
inline static Simd512Float32 round(Simd512Float32 a) noexcept { return  Simd512Float32(_mm512_roundscale_ps(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)); }
[[nodiscard("Value calculated and not used (fract)")]]
inline static Simd512Float32 fract(Simd512Float32 a) noexcept { return a - floor(a); }

//*****Min/Max*****
[[nodiscard("Value calculated and not used (min)")]]
inline static Simd512Float32 min(Simd512Float32 a, Simd512Float32 b) noexcept { return Simd512Float32(_mm512_min_ps(a.v, b.v)); }
[[nodiscard("Value calculated and not used (max)")]]
inline static Simd512Float32 max(Simd512Float32 a, Simd512Float32 b) noexcept { return Simd512Float32(_mm512_max_ps(a.v, b.v)); }

//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd512Float32 clamp(const Simd512Float32 a) noexcept {
	const auto zero = _mm512_setzero_ps();
	const auto one = _mm512_set1_ps(1.0);
	return _mm512_min_ps(_mm512_max_ps(a.v, zero), one);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd512Float32 clamp(const Simd512Float32 a, const Simd512Float32 min, const Simd512Float32 max) noexcept {
	return _mm512_min_ps(_mm512_max_ps(a.v, min.v), max.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd512Float32 clamp(const Simd512Float32 a, const float min_f, const float max_f) noexcept {
	const auto min = _mm512_set1_ps(min_f);
	const auto max = _mm512_set1_ps(max_f);
	return _mm512_min_ps(_mm512_max_ps(a.v, min), max);
}



//*****Approximate Functions*****
[[nodiscard("Value calculated and not used ()")]]
inline static Simd512Float32 reciprocal_approx(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_rcp14_ps(a.v)); }



//*****512-bit Mathematical Functions*****
[[nodiscard("Value calculated and not used (sqrt)")]]
inline static Simd512Float32 sqrt(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_sqrt_ps(a.v)); }

[[nodiscard("Value calculated and not used (pow)")]]
inline static Simd512Float32 pow(Simd512Float32 a, Simd512Float32 b) noexcept { return Simd512Float32(_mm512_pow_ps(a.v,b.v)); }

[[nodiscard("Value calculated and not used (abs)")]]
inline static Simd512Float32 abs(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_abs_ps(a.v)); }

//Calculate e^x
[[nodiscard("Value calculated and not used (exp)")]]
inline static Simd512Float32 exp(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_exp_ps(a.v)); }

//Calculate 2^x
[[nodiscard("Value calculated and not used (exp2)")]]
inline static Simd512Float32 exp2(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_exp2_ps(a.v)); }

//Calculate 10^x
[[nodiscard("Value calculated and not used (exp10)")]]
inline static Simd512Float32 exp10(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_exp10_ps(a.v)); }

//Calculate (e^x)-1.0
[[nodiscard("Value calculated and not used (exp_minus1)")]]
inline static Simd512Float32 expm1(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_expm1_ps(a.v)); }

//Calulate natural log(x)
[[nodiscard("Value calculated and not used (log)")]]
inline static Simd512Float32 log(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_log_ps(a.v)); }

//Calulate log(1.0 + x)
[[nodiscard("Value calculated and not used (log1p)")]]
inline static Simd512Float32 log1p(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_log1p_ps(a.v)); }

//Calculate log_1(x)
[[nodiscard("Value calculated and not used (log2)")]]
inline static Simd512Float32 log2(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_log2_ps(a.v)); }

//Calculate log_10(x)
[[nodiscard("Value calculated and not used (log10)")]]
inline static Simd512Float32 log10(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_log10_ps(a.v)); }

//Calculate cube root
[[nodiscard("Value calculated and not used (cbrt)")]]
inline static Simd512Float32 cbrt(const Simd512Float32 a) noexcept { return Simd512Float32(_mm512_cbrt_ps(a.v)); }

//Calculate hypot(x).  That is: sqrt(a^2 + b^2) while avoiding overflow.
[[nodiscard("Value calculated and not used (hypot)")]]
inline static Simd512Float32 hypot(const Simd512Float32 a, const Simd512Float32 b) noexcept { return Simd512Float32(_mm512_hypot_ps(a.v, b.v)); }





//*****Trigonometric Functions *****
[[nodiscard("Value calculated and not used (sin)")]]
inline static Simd512Float32 sin(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_sin_ps(a.v)); }

[[nodiscard("Value calculated and not used (cos)")]]
inline static Simd512Float32 cos(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_cos_ps(a.v)); }

[[nodiscard("Value calculated and not used (tan)")]]
inline static Simd512Float32 tan(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_tan_ps(a.v)); }

[[nodiscard("Value calculated and not used (asin)")]]
inline static Simd512Float32 asin(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_asin_ps(a.v)); }

[[nodiscard("Value calculated and not used (acos)")]]
inline static Simd512Float32 acos(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_acos_ps(a.v)); }

[[nodiscard("Value calculated and not used (atan)")]]
inline static Simd512Float32 atan(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_atan_ps(a.v)); }

[[nodiscard("Value calculated and not used (atan2)")]]
inline static Simd512Float32 atan2(Simd512Float32 a, Simd512Float32 b) noexcept { return Simd512Float32(_mm512_atan2_ps(a.v, b.v)); }

[[nodiscard("Value calculated and not used (sinh)")]]
inline static Simd512Float32 sinh(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_sinh_ps(a.v)); }

[[nodiscard("Value calculated and not used (cosh)")]]
inline static Simd512Float32 cosh(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_cosh_ps(a.v)); }

[[nodiscard("Value calculated and not used (tanh)")]]
inline static Simd512Float32 tanh(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_tanh_ps(a.v)); }

[[nodiscard("Value calculated and not used (asinh)")]]
inline static Simd512Float32 asinh(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_asinh_ps(a.v)); }

[[nodiscard("Value calculated and not used (acosh)")]]
inline static Simd512Float32 acosh(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_acosh_ps(a.v)); }

[[nodiscard("Value calculated and not used (atanh)")]]
inline static Simd512Float32 atanh(Simd512Float32 a) noexcept { return Simd512Float32(_mm512_atanh_ps(a.v)); }

//*****AVX-512 Conditional Functions *****

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_equal)")]]
inline static __mmask16 compare_equal(const Simd512Float32 a, const Simd512Float32 b) noexcept { return _mm512_cmp_ps_mask(a.v, b.v, _CMP_EQ_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_less)")]]
inline static __mmask16 compare_less(const Simd512Float32 a, const Simd512Float32 b) noexcept { return _mm512_cmp_ps_mask(a.v, b.v, _CMP_LT_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_less_equal)")]]
inline static __mmask16 compare_less_equal(const Simd512Float32 a, const Simd512Float32 b) noexcept { return _mm512_cmp_ps_mask(a.v, b.v, _CMP_LE_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_greater)")]]
inline static __mmask16 compare_greater(const Simd512Float32 a, const Simd512Float32 b) noexcept { return _mm512_cmp_ps_mask(a.v, b.v, _CMP_GT_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_greater_equal)")]]
inline static __mmask16 compare_greater_equal(const Simd512Float32 a, const Simd512Float32 b) noexcept { return _mm512_cmp_ps_mask(a.v, b.v, _CMP_GE_OQ); }

//Compare to nan
[[nodiscard("Value Calculated and not used (compare_is_nan)")]]
inline static __mmask16 isnan(const Simd512Float32 a) noexcept { return _mm512_cmp_ps_mask(a.v, a.v, _CMP_UNORD_Q); }


//Blend two values together based on mask.First argument if zero.Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd512Float32 blend(const Simd512Float32 if_false, const Simd512Float32 if_true, __mmask16 mask) noexcept {
	return Simd512Float32(_mm512_mask_blend_ps(mask, if_false.v, if_true.v));
}



/***************************************************************************************************************************************************************************************************
 * SIMD 256 type.  Contains 8 x 32bit Floats
 * Requires AVX support.
 * *************************************************************************************************************************************************************************************************/
#endif // MT_SIMD_ALLOW_LEVEL4_TYPES

#if MT_SIMD_ALLOW_LEVEL3_TYPES
struct Simd256Float32 {
	__m256 v;
	typedef float F;
	typedef __m256 MaskType;
	typedef Simd256UInt32 U;
	typedef Simd256UInt64 U64;


	//*****Constructors*****
	Simd256Float32() = default;
	Simd256Float32(__m256 a) : v(a) {};
	Simd256Float32(F a) :  v(_mm256_set1_ps(a)) {};

	//*****Support Informtion*****
	
	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}
	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.is_level_3();
	}

	//Performs a compile time support. Checks this type ONLY (integers in same class may not be supported) 
	static constexpr bool compiler_supported() {
		return mt::environment::compiler_can_use_x86_64_level_3_types;
	}

	static constexpr int size_of_element() { return sizeof(float); }
	static constexpr int number_of_elements() { return 8; }

	//*****Access Elements*****
	F element(int i) const { return mt::simd_detail_f32::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_f32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd256Float32& operator+=(const Simd256Float32& rhs) noexcept { v = _mm256_add_ps(v, rhs.v); return *this; }
	Simd256Float32& operator+=(float rhs) noexcept { v = _mm256_add_ps(v, _mm256_set1_ps(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd256Float32& operator-=(const Simd256Float32& rhs) noexcept { v = _mm256_sub_ps(v, rhs.v); return *this; }
	Simd256Float32& operator-=(float rhs) noexcept { v = _mm256_sub_ps(v, _mm256_set1_ps(rhs));	return *this; }
	
	//*****Multiplication Operators*****
	Simd256Float32& operator*=(const Simd256Float32& rhs) noexcept { v = _mm256_mul_ps(v, rhs.v); return *this; }
	Simd256Float32& operator*=(float rhs) noexcept { v = _mm256_mul_ps(v, _mm256_set1_ps(rhs)); return *this; }

	//*****Division Operators*****
	Simd256Float32& operator/=(const Simd256Float32& rhs) noexcept { v = _mm256_div_ps(v, rhs.v); return *this; }
	Simd256Float32& operator/=(float rhs) noexcept { v = _mm256_div_ps(v, _mm256_set1_ps(rhs));	return *this; }

	//*****Negate Operators*****
	Simd256Float32 operator-() const noexcept { return Simd256Float32(_mm256_sub_ps(_mm256_setzero_ps(), v)); }


	//*****Make Functions****
	static Simd256Float32 make_sequential(F first) { return Simd256Float32(_mm256_set_ps(first+7.0f, first + 6.0f, first + 5.0f, first + 4.0f, first + 3.0f, first + 2.0f, first + 1.0f, first)); }
	static Simd256Float32 make_set1(F v) { return Simd256Float32(_mm256_set1_ps(v)); }
	

	static Simd256Float32 make_from_int32(Simd256UInt32 i) {return Simd256Float32(_mm256_cvtepi32_ps(i.v));}

	//*****Cast Functions****
	
	//Warning: Requires additional CPU features (AVX2)
	Simd256UInt32 bitcast_to_uint() const { return Simd256UInt32(_mm256_castps_si256(this->v)); } 
	

	


};

//*****Addition Operators*****
inline static Simd256Float32 operator+(Simd256Float32  lhs, const Simd256Float32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Float32 operator+(Simd256Float32  lhs, float rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Float32 operator+(float lhs, Simd256Float32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd256Float32 operator-(Simd256Float32  lhs, const Simd256Float32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Float32 operator-(Simd256Float32  lhs, float rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Float32 operator-(const float lhs, const Simd256Float32& rhs) noexcept { return Simd256Float32(_mm256_sub_ps(_mm256_set1_ps(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd256Float32 operator*(Simd256Float32  lhs, const Simd256Float32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Float32 operator*(Simd256Float32  lhs, float rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Float32 operator*(float lhs, Simd256Float32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd256Float32 operator/(Simd256Float32  lhs, const Simd256Float32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd256Float32 operator/(Simd256Float32  lhs, float rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd256Float32 operator/(const float lhs, const Simd256Float32& rhs) noexcept { return Simd256Float32(_mm256_div_ps(_mm256_set1_ps(lhs), rhs.v)); }

//*****Fused Multiply Add Instructions*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd256Float32 fma(const Simd256Float32  a, const Simd256Float32 b, const Simd256Float32 c) {
	return _mm256_fmadd_ps(a.v, b.v, c.v);
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd256Float32 fms(const Simd256Float32  a, const Simd256Float32 b, const Simd256Float32 c) {
	return _mm256_fmsub_ps(a.v, b.v, c.v);
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd256Float32 fnma(const Simd256Float32  a, const Simd256Float32 b, const Simd256Float32 c) {
	return _mm256_fnmadd_ps(a.v, b.v, c.v);
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd256Float32 fnms(const Simd256Float32  a, const Simd256Float32 b, const Simd256Float32 c) {
	return _mm256_fnmsub_ps(a.v, b.v, c.v);
}



//*****Rounding Functions*****
[[nodiscard("Value calculated and not used (floor)")]]
inline static Simd256Float32 floor(Simd256Float32 a) noexcept {return Simd256Float32(_mm256_floor_ps(a.v));}

[[nodiscard("Value calculated and not used (ceil)")]]
inline static Simd256Float32 ceil(Simd256Float32 a) noexcept { return Simd256Float32(_mm256_ceil_ps(a.v));}

[[nodiscard("Value calculated and not used (trunc)")]]
inline static Simd256Float32 trunc(Simd256Float32 a) noexcept {return Simd256Float32(_mm256_trunc_ps(a.v));}

[[nodiscard("Value calculated and not used (round)")]]
inline static Simd256Float32 round(Simd256Float32 a) noexcept {return Simd256Float32(_mm256_round_ps(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)); }

[[nodiscard("Value calculated and not used (fract)")]]
inline static Simd256Float32 fract(Simd256Float32 a) noexcept {return a - floor(a);}

//*****Min/Max*****
[[nodiscard("Value calculated and not used (min)")]]
inline static Simd256Float32 min(const Simd256Float32 a, const Simd256Float32 b)  noexcept {return Simd256Float32(_mm256_min_ps(a.v, b.v)); }

[[nodiscard("Value calculated and not used (max)")]]
inline static Simd256Float32 max(const Simd256Float32 a, const Simd256Float32 b)  noexcept {return Simd256Float32(_mm256_max_ps(a.v, b.v));}

//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd256Float32 clamp(const Simd256Float32 a) noexcept {
	const auto zero = _mm256_setzero_ps();
	const auto one = _mm256_set1_ps(1.0);
	return _mm256_min_ps(_mm256_max_ps(a.v, zero), one);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd256Float32 clamp(const Simd256Float32 a, const Simd256Float32 min, const Simd256Float32 max) noexcept {
	return _mm256_min_ps(_mm256_max_ps(a.v, min.v), max.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd256Float32 clamp(const Simd256Float32 a, const float min_f, const float max_f) noexcept {
	const auto min = _mm256_set1_ps(min_f);
	const auto max = _mm256_set1_ps(max_f);
	return _mm256_min_ps(_mm256_max_ps(a.v, min), max);
}



//*****Approximate Functions*****
[[nodiscard("Value calculated and not used (reciprocal_approx)")]]
inline static Simd256Float32 reciprocal_approx(const Simd256Float32 a) noexcept {
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd256Float32(_mm256_rcp14_ps(a.v));
	}
	else {
		return Simd256Float32(_mm256_rcp_ps(a.v));
	}
}



//*****256-bit SIMD Mathematical Functions*****
[[nodiscard("Value calculated and not used (sqrt)")]] 
inline static Simd256Float32 sqrt(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_sqrt_ps(a.v));}

[[nodiscard("Value calculated and not used (pow)")]]
inline static Simd256Float32 pow(Simd256Float32 a, Simd256Float32 b) noexcept { return Simd256Float32(_mm256_pow_ps(a.v, b.v)); }

[[nodiscard("Value Calculated and not used (abs)")]]
inline static Simd256Float32 abs(const Simd256Float32 a) noexcept {	
	//No AVX for abs so we just flip the bit.
	const auto r = _mm256_and_ps(_mm256_set1_ps(std::bit_cast<float>(0x7FFFFFFF)), a.v); 
	return Simd256Float32(r);
}


//Calculate e^x
[[nodiscard("Value calculated and not used (exp)")]]
inline static Simd256Float32 exp(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_exp_ps(a.v)); }

//Calculate 2^x
[[nodiscard("Value calculated and not used (exp2)")]]
inline static Simd256Float32 exp2(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_exp2_ps(a.v)); }

//Calculate 10^x
[[nodiscard("Value calculated and not used (exp10)")]]
inline static Simd256Float32 exp10(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_exp10_ps(a.v)); }

//Calculate (e^x)-1.0
[[nodiscard("Value calculated and not used (exp_minus1)")]]
inline static Simd256Float32 expm1(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_expm1_ps(a.v)); }

//Calulate natural log(x)
[[nodiscard("Value calculated and not used (log)")]]
inline static Simd256Float32 log(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_log_ps(a.v)); }

//Calulate log(1.0 + x)
[[nodiscard("Value calculated and not used (log1p)")]]
inline static Simd256Float32 log1p(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_log1p_ps(a.v)); }

//Calculate log_1(x)
[[nodiscard("Value calculated and not used (log2)")]]
inline static Simd256Float32 log2(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_log2_ps(a.v)); }

//Calculate log_10(x)
[[nodiscard("Value calculated and not used (log10)")]]
inline static Simd256Float32 log10(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_log10_ps(a.v)); }

//Calculate cube root
[[nodiscard("Value calculated and not used (cbrt)")]]
inline static Simd256Float32 cbrt(const Simd256Float32 a) noexcept { return Simd256Float32(_mm256_cbrt_ps(a.v)); }

//Calculate hypot(x).  That is: sqrt(a^2 + b^2) while avoiding overflow.
[[nodiscard("Value calculated and not used (hypot)")]]
inline static Simd256Float32 hypot(const Simd256Float32 a, const Simd256Float32 b) noexcept { return Simd256Float32(_mm256_hypot_ps(a.v, b.v)); }




//*****Trigonometric Functions *****

[[nodiscard("Value Calculated and not used (sin)")]]
inline static Simd256Float32 sin(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_sin_ps(a.v));}

[[nodiscard("Value Calculated and not used (cos)")]]
inline static Simd256Float32 cos(const Simd256Float32 a)  noexcept {return Simd256Float32(_mm256_cos_ps(a.v));}

[[nodiscard("Value Calculated and not used (tan)")]]
inline static Simd256Float32 tan(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_tan_ps(a.v));}

[[nodiscard("Value Calculated and not used (asin)")]]
inline static Simd256Float32 asin(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_asin_ps(a.v));}

[[nodiscard("Value Calculated and not used (acos)")]]
inline static Simd256Float32 acos(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_acos_ps(a.v));}

[[nodiscard("Value Calculated and not used (atan)")]]
inline static Simd256Float32 atan(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_atan_ps(a.v));}

[[nodiscard("Value Calculated and not used (atan2)")]]
inline static Simd256Float32 atan2(const Simd256Float32 a, const Simd256Float32 b) noexcept {return Simd256Float32(_mm256_atan2_ps(a.v, b.v));}

[[nodiscard("Value Calculated and not used (sinh)")]]
inline static Simd256Float32 sinh(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_sinh_ps(a.v));}

[[nodiscard("Value Calculated and not used (cosh)")]]
inline static Simd256Float32 cosh(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_cosh_ps(a.v)); }

[[nodiscard("Value Calculated and not used (tanh)")]]
inline static Simd256Float32 tanh(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_tanh_ps(a.v));}

[[nodiscard("Value Calculated and not used (asinh)")]]
inline static Simd256Float32 asinh(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_asinh_ps(a.v));}

[[nodiscard("Value Calculated and not used (acosh)")]]
inline static Simd256Float32 acosh(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_acosh_ps(a.v));}

[[nodiscard("Value Calculated and not used (atanh)")]]
inline static Simd256Float32 atanh(const Simd256Float32 a) noexcept {return Simd256Float32(_mm256_atanh_ps(a.v));}

//*****Conditional Functions *****

//Compare ordered.
inline static __m256 compare_equal(const Simd256Float32 a, const Simd256Float32 b) noexcept { return _mm256_cmp_ps(a.v, b.v, _CMP_EQ_OQ); }
inline static __m256 compare_less(const Simd256Float32 a, const Simd256Float32 b) noexcept { return _mm256_cmp_ps(a.v, b.v,  _CMP_LT_OS); }
inline static __m256 compare_less_equal(const Simd256Float32 a, const Simd256Float32 b) noexcept { return _mm256_cmp_ps(a.v, b.v, _CMP_LE_OS); }
inline static __m256 compare_greater(const Simd256Float32 a, const Simd256Float32 b) noexcept { return _mm256_cmp_ps(a.v, b.v, _CMP_GT_OS); }
inline static __m256 compare_greater_equal(const Simd256Float32 a, const Simd256Float32 b) noexcept { return _mm256_cmp_ps(a.v, b.v, _CMP_GE_OS); }
inline static __m256 isnan(const Simd256Float32 a) noexcept { return _mm256_cmp_ps(a.v, a.v, _CMP_UNORD_Q); }

//Blend two values together based on mask.First argument if zero.Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd256Float32 blend(const Simd256Float32 if_false, const Simd256Float32 if_true, __m256 mask) noexcept {
	return Simd256Float32(_mm256_blendv_ps(if_false.v, if_true.v, mask));	
}


/***************************************************************************************************************************************************************************************************
 * SIMD 128 type.  Contains 4 x 32bit Floats
 * Requires SSE2 support.  
 * (Improved performance if compiled with SSE4.1)
 * *************************************************************************************************************************************************************************************************/
#endif // MT_SIMD_ALLOW_LEVEL3_TYPES

struct Simd128Float32 {
	__m128 v;
	typedef float F;
	typedef __m128 MaskType;
	typedef Simd128UInt32 U;
	typedef Simd128UInt64 U64;


	//*****Constructors*****
	Simd128Float32() = default;
	Simd128Float32(__m128 a) : v(a) {};
	Simd128Float32(F a) : v(_mm_set1_ps(a)) {};

	//*****Support Informtion*****

	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported() {
		CpuInformation cpuid{};
		return cpu_supported(cpuid);
	}
	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported(CpuInformation cpuid) {
		return cpuid.is_level_1();
	}

	//Performs a compile time support. Checks this type ONLY (integers in same class may not be supported) 
	static constexpr bool compiler_supported() {
		return mt::environment::compiler_can_use_x86_64_level_1_types;
	}

	static constexpr int size_of_element() { return sizeof(float); }
	static constexpr int number_of_elements() { return 4; }

	//*****Access Elements*****
	F element(int i)  const { return mt::simd_detail_f32::lane_get(v, i); }
	void set_element(int i, F value) { v = mt::simd_detail_f32::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd128Float32& operator+=(const Simd128Float32& rhs) noexcept { v = _mm_add_ps(v, rhs.v); return *this; } //SSE1
	Simd128Float32& operator+=(float rhs) noexcept { v = _mm_add_ps(v, _mm_set1_ps(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd128Float32& operator-=(const Simd128Float32& rhs) noexcept { v = _mm_sub_ps(v, rhs.v); return *this; }//SSE1
	Simd128Float32& operator-=(float rhs) noexcept { v = _mm_sub_ps(v, _mm_set1_ps(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd128Float32& operator*=(const Simd128Float32& rhs) noexcept { v = _mm_mul_ps(v, rhs.v); return *this; } //SSE1
	Simd128Float32& operator*=(float rhs) noexcept { v = _mm_mul_ps(v, _mm_set1_ps(rhs)); return *this; }

	//*****Division Operators*****
	Simd128Float32& operator/=(const Simd128Float32& rhs) noexcept { v = _mm_div_ps(v, rhs.v); return *this; } //SSE1
	Simd128Float32& operator/=(float rhs) noexcept { v = _mm_div_ps(v, _mm_set1_ps(rhs));	return *this; }

	//*****Negate Operators*****
	Simd128Float32 operator-() const noexcept { return Simd128Float32(_mm_sub_ps(_mm_setzero_ps(), v)); }


	//*****Make Functions****
	static Simd128Float32 make_sequential(F first) { return Simd128Float32(_mm_set_ps(first + 3.0f, first + 2.0f, first + 1.0f, first)); }
	static Simd128Float32 make_set1(F v) { return Simd128Float32(_mm_set1_ps(v)); }


	static Simd128Float32 make_from_int32(Simd128UInt32 i) { return Simd128Float32(_mm_cvtepi32_ps(i.v)); } //SSE2

	//*****Cast Functions****
	Simd128UInt32 bitcast_to_uint() const { return Simd128UInt32(_mm_castps_si128(this->v)); } //SSE2
	

	

};

//*****Addition Operators*****
inline static Simd128Float32 operator+(Simd128Float32  lhs, const Simd128Float32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Float32 operator+(Simd128Float32  lhs, float rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Float32 operator+(float lhs, Simd128Float32 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd128Float32 operator-(Simd128Float32  lhs, const Simd128Float32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Float32 operator-(Simd128Float32  lhs, float rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Float32 operator-(const float lhs, const Simd128Float32& rhs) noexcept { return Simd128Float32(_mm_sub_ps(_mm_set1_ps(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd128Float32 operator*(Simd128Float32  lhs, const Simd128Float32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Float32 operator*(Simd128Float32  lhs, float rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Float32 operator*(float lhs, Simd128Float32 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd128Float32 operator/(Simd128Float32  lhs, const Simd128Float32& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd128Float32 operator/(Simd128Float32  lhs, float rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Float32 operator/(const float lhs, const Simd128Float32& rhs) noexcept { return Simd128Float32(_mm_div_ps(_mm_set1_ps(lhs), rhs.v)); }

//*****Rounding Functions*****
[[nodiscard("Value calculated and not used (floor)")]]
inline static Simd128Float32 floor(Simd128Float32 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float32(_mm_floor_ps(a.v)); //SSE4.1
	}
	else {
		return Simd128Float32(_mm_set_ps(std::floor(a.element(3)), std::floor(a.element(2)), std::floor(a.element(1)), std::floor(a.element(0))));
	}
} 

[[nodiscard("Value calculated and not used (ceil)")]]
inline static Simd128Float32 ceil(Simd128Float32 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float32(_mm_ceil_ps(a.v)); //SSE4.1
	}
	else {
		return Simd128Float32(_mm_set_ps(std::ceil(a.element(3)), std::ceil(a.element(2)), std::ceil(a.element(1)), std::ceil(a.element(0))));
	}
}

[[nodiscard("Value calculated and not used (trunc)")]]
inline static Simd128Float32 trunc(Simd128Float32 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float32(_mm_round_ps(a.v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)); //SSE4.1
	}
	else {
		return Simd128Float32(_mm_set_ps(std::trunc(a.element(3)), std::trunc(a.element(2)), std::trunc(a.element(1)), std::trunc(a.element(0))));
	}
}

[[nodiscard("Value calculated and not used (round)")]]
inline static Simd128Float32 round(Simd128Float32 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float32(_mm_round_ps(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)); //SSE4.1
	}
	else {
		return Simd128Float32(_mm_set_ps(std::round(a.element(3)), std::round(a.element(2)), std::round(a.element(1)), std::round(a.element(0))));
	}
}


[[nodiscard("Value calculated and not used (fract)")]]
inline static Simd128Float32 fract(Simd128Float32 a) noexcept { return a - floor(a); } 



//*****Fused Multiply Add Simd128s*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd128Float32 fma(const Simd128Float32  a, const Simd128Float32 b, const Simd128Float32 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fmadd_ps(a.v, b.v, c.v);  //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float32(_mm_set_ps(
			std::fma(a.element(3), b.element(3), c.element(3)),
			std::fma(a.element(2), b.element(2), c.element(2)),
			std::fma(a.element(1), b.element(1), c.element(1)),
			std::fma(a.element(0), b.element(0), c.element(0))
		));
	}
} 

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd128Float32 fms(const Simd128Float32  a, const Simd128Float32 b, const Simd128Float32 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fmsub_ps(a.v, b.v, c.v);  //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float32(_mm_set_ps(
			std::fma(a.element(3), b.element(3), -c.element(3)),
			std::fma(a.element(2), b.element(2), -c.element(2)),
			std::fma(a.element(1), b.element(1), -c.element(1)),
			std::fma(a.element(0), b.element(0), -c.element(0))
		));
	}
} 

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd128Float32 fnma(const Simd128Float32  a, const Simd128Float32 b, const Simd128Float32 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fnmadd_ps(a.v, b.v, c.v);  //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float32(_mm_set_ps(
			std::fma(-a.element(3), b.element(3), c.element(3)),
			std::fma(-a.element(2), b.element(2), c.element(2)),
			std::fma(-a.element(1), b.element(1), c.element(1)),
			std::fma(-a.element(0), b.element(0), c.element(0))
		));
	}
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd128Float32 fnms(const Simd128Float32  a, const Simd128Float32 b, const Simd128Float32 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fnmsub_ps(a.v, b.v, c.v); //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float32(_mm_set_ps(
			std::fma(-a.element(3), b.element(3), -c.element(3)),
			std::fma(-a.element(2), b.element(2), -c.element(2)),
			std::fma(-a.element(1), b.element(1), -c.element(1)),
			std::fma(-a.element(0), b.element(0), -c.element(0))
		));
	}
}



//**********Min/Max*v*********
[[nodiscard("Value calculated and not used (min)")]]
inline static Simd128Float32 min(const Simd128Float32 a, const Simd128Float32 b)  noexcept { return Simd128Float32(_mm_min_ps(a.v, b.v)); } //SSE1

[[nodiscard("Value calculated and not used (max)")]]
inline static Simd128Float32 max(const Simd128Float32 a, const Simd128Float32 b)  noexcept { return Simd128Float32(_mm_max_ps(a.v, b.v)); } //SSE1

//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float32 clamp(const Simd128Float32 a) noexcept {
	const auto zero = _mm_setzero_ps();
	const auto one = _mm_set1_ps(1.0);
	return _mm_min_ps(_mm_max_ps(a.v, zero), one);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float32 clamp(const Simd128Float32 a, const Simd128Float32 min, const Simd128Float32 max) noexcept {
	return _mm_min_ps(_mm_max_ps(a.v, min.v), max.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float32 clamp(const Simd128Float32 a, const float min_f, const float max_f) noexcept {
	const auto min = _mm_set1_ps(min_f);
	const auto max = _mm_set1_ps(max_f);
	return _mm_min_ps(_mm_max_ps(a.v, min), max);
}



//*****Approximate Functions*****
[[nodiscard("Value calculated and not used (reciprocal_approx)")]]
inline static Simd128Float32 reciprocal_approx(const Simd128Float32 a) noexcept {
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd128Float32(_mm_rcp14_ps(a.v));
	}
	else {
		return Simd128Float32(_mm_rcp_ps(a.v));
	}
}




//*****128-bit SIMD Mathematical Functions*****

//Calculate square root.
[[nodiscard("Value calculated and not used (sqrt)")]]
inline static Simd128Float32 sqrt(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_sqrt_ps(a.v)); } //sse

//Calculating a raised to the power of b
[[nodiscard("Value calculated and not used (pow)")]]
inline static Simd128Float32 pow(Simd128Float32 a, Simd128Float32 b) noexcept { return Simd128Float32(_mm_pow_ps(a.v, b.v)); }

//Calculate the absoulte value.  Performed by unsetting the sign bit.
[[nodiscard("Value Calculated and not used (abs)")]]
inline static Simd128Float32 abs(const Simd128Float32 a) noexcept {
	//No SSE for abs so we just flip the bit.
	const auto r = _mm_and_ps(_mm_set1_ps(std::bit_cast<float>(0x7FFFFFFF)), a.v);
	return Simd128Float32(r);
}

//Calculate e^x
[[nodiscard("Value calculated and not used (exp)")]]
inline static Simd128Float32 exp(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_exp_ps(a.v)); } //sse

//Calculate 2^x
[[nodiscard("Value calculated and not used (exp2)")]]
inline static Simd128Float32 exp2(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_exp2_ps(a.v)); } //sse

//Calculate 10^x
[[nodiscard("Value calculated and not used (exp10)")]]
inline static Simd128Float32 exp10(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_exp10_ps(a.v)); } //sse

//Calculate (e^x)-1.0
[[nodiscard("Value calculated and not used (exp_minus1)")]]
inline static Simd128Float32 expm1(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_expm1_ps(a.v)); } //sse

//Calulate natural log(x)
[[nodiscard("Value calculated and not used (log)")]]
inline static Simd128Float32 log(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_log_ps(a.v)); } //sse

//Calulate log(1.0 + x)
[[nodiscard("Value calculated and not used (log1p)")]]
inline static Simd128Float32 log1p(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_log1p_ps(a.v)); } //sse

//Calculate log_1(x)
[[nodiscard("Value calculated and not used (log2)")]]
inline static Simd128Float32 log2(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_log2_ps(a.v)); } //sse

//Calculate log_10(x)
[[nodiscard("Value calculated and not used (log10)")]]
inline static Simd128Float32 log10(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_log10_ps(a.v)); } //sse

//Calculate cube root
[[nodiscard("Value calculated and not used (cbrt)")]]
inline static Simd128Float32 cbrt(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_cbrt_ps(a.v)); } //sse


//Calculate hypot(x).  That is: sqrt(a^2 + b^2) while avoiding overflow.
[[nodiscard("Value calculated and not used (hypot)")]]
inline static Simd128Float32 hypot(const Simd128Float32 a, const Simd128Float32 b) noexcept { return Simd128Float32(_mm_hypot_ps(a.v, b.v)); } //sse



//*****Trigonometric Functions *****
[[nodiscard("Value Calculated and not used (sin)")]]
inline static Simd128Float32 sin(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_sin_ps(a.v)); }  //SSE

[[nodiscard("Value Calculated and not used (cos)")]]
inline static Simd128Float32 cos(const Simd128Float32 a)  noexcept { return Simd128Float32(_mm_cos_ps(a.v)); }

[[nodiscard("Value Calculated and not used (tan)")]]
inline static Simd128Float32 tan(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_tan_ps(a.v)); }

[[nodiscard("Value Calculated and not used (asin)")]]
inline static Simd128Float32 asin(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_asin_ps(a.v)); }

[[nodiscard("Value Calculated and not used (acos)")]]
inline static Simd128Float32 acos(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_acos_ps(a.v)); }

[[nodiscard("Value Calculated and not used (atan)")]]
inline static Simd128Float32 atan(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_atan_ps(a.v)); }

[[nodiscard("Value Calculated and not used (atan2)")]]
inline static Simd128Float32 atan2(const Simd128Float32 a, const Simd128Float32 b) noexcept { return Simd128Float32(_mm_atan2_ps(a.v, b.v)); }

[[nodiscard("Value Calculated and not used (sinh)")]]
inline static Simd128Float32 sinh(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_sinh_ps(a.v)); }

[[nodiscard("Value Calculated and not used (cosh)")]]
inline static Simd128Float32 cosh(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_cosh_ps(a.v)); }

[[nodiscard("Value Calculated and not used (tanh)")]]
inline static Simd128Float32 tanh(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_tanh_ps(a.v)); }

[[nodiscard("Value Calculated and not used (asinh)")]]
inline static Simd128Float32 asinh(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_asinh_ps(a.v)); }

[[nodiscard("Value Calculated and not used (acosh)")]]
inline static Simd128Float32 acosh(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_acosh_ps(a.v)); }

[[nodiscard("Value Calculated and not used (atanh)")]]
inline static Simd128Float32 atanh(const Simd128Float32 a) noexcept { return Simd128Float32(_mm_atanh_ps(a.v)); } //SSE



//*****Conditional Functions *****

//Compare if 2 values are equal and return a mask.
inline static __m128 compare_equal(const Simd128Float32 a, const Simd128Float32 b) noexcept { return _mm_cmpeq_ps(a.v, b.v); }
inline static __m128 compare_less(const Simd128Float32 a, const Simd128Float32 b) noexcept { return _mm_cmplt_ps(a.v, b.v); }
inline static __m128 compare_less_equal(const Simd128Float32 a, const Simd128Float32 b) noexcept { return _mm_cmple_ps(a.v, b.v); }
inline static __m128 compare_greater(const Simd128Float32 a, const Simd128Float32 b) noexcept { return _mm_cmpgt_ps(a.v, b.v); }
inline static __m128 compare_greater_equal(const Simd128Float32 a, const Simd128Float32 b) noexcept { return _mm_cmpge_ps(a.v, b.v); }
inline static __m128 isnan(const Simd128Float32 a) noexcept { return _mm_cmpunord_ps(a.v, a.v); }

//Blend two values together based on mask.  First argument if zero. Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd128Float32 blend(const Simd128Float32 if_false, const Simd128Float32 if_true, __m128 mask) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float32(_mm_blendv_ps(if_false.v, if_true.v, mask));
	}
	else {
		return Simd128Float32(_mm_or_ps(_mm_andnot_ps(mask,if_false.v), _mm_and_ps(mask, if_true.v)));
	}
}

#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)

struct Simd128Float32 {
	v128_t v;

	typedef float F;
	typedef v128_t MaskType;
	typedef Simd128UInt32 U;
	typedef Simd128UInt64 U64;

	Simd128Float32() = default;
	Simd128Float32(v128_t a) : v(a) {}
	Simd128Float32(F a) : v(mt::simd_wasm_detail::splat<float>(a)) {}

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

	static constexpr int size_of_element() { return sizeof(float); }
	static constexpr int number_of_elements() { return 4; }

	F element(int i) const { return mt::simd_wasm_detail::lane_get<float, 4>(v, i); }
	void set_element(int i, F value) { v = mt::simd_wasm_detail::lane_set<float, 4>(v, i, value); }

	Simd128Float32& operator+=(const Simd128Float32& rhs) noexcept { v = wasm_f32x4_add(v, rhs.v); return *this; }
	Simd128Float32& operator+=(float rhs) noexcept { v = wasm_f32x4_add(v, Simd128Float32(rhs).v); return *this; }
	Simd128Float32& operator-=(const Simd128Float32& rhs) noexcept { v = wasm_f32x4_sub(v, rhs.v); return *this; }
	Simd128Float32& operator-=(float rhs) noexcept { v = wasm_f32x4_sub(v, Simd128Float32(rhs).v); return *this; }
	Simd128Float32& operator*=(const Simd128Float32& rhs) noexcept { v = wasm_f32x4_mul(v, rhs.v); return *this; }
	Simd128Float32& operator*=(float rhs) noexcept { v = wasm_f32x4_mul(v, Simd128Float32(rhs).v); return *this; }
	Simd128Float32& operator/=(const Simd128Float32& rhs) noexcept { v = wasm_f32x4_div(v, rhs.v); return *this; }
	Simd128Float32& operator/=(float rhs) noexcept { v = wasm_f32x4_div(v, Simd128Float32(rhs).v); return *this; }
	Simd128Float32 operator-() const noexcept { return Simd128Float32(wasm_f32x4_neg(v)); }

	static Simd128Float32 make_sequential(F first) { return Simd128Float32(mt::simd_wasm_detail::make_sequential<float, 4>(first)); }
	static Simd128Float32 make_set1(F value) { return Simd128Float32(mt::simd_wasm_detail::splat<float>(value)); }
	static Simd128Float32 make_from_int32(Simd128UInt32 i) { return Simd128Float32(wasm_f32x4_convert_i32x4(i.v)); }

	Simd128UInt32 bitcast_to_uint() const { return Simd128UInt32(v); }
};

inline static Simd128Float32 operator+(Simd128Float32 lhs, const Simd128Float32& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Float32 operator+(Simd128Float32 lhs, float rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Float32 operator+(float lhs, Simd128Float32 rhs) noexcept { rhs += lhs; return rhs; }
inline static Simd128Float32 operator-(Simd128Float32 lhs, const Simd128Float32& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Float32 operator-(Simd128Float32 lhs, float rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Float32 operator-(const float lhs, const Simd128Float32& rhs) noexcept { return Simd128Float32(lhs) - rhs; }
inline static Simd128Float32 operator*(Simd128Float32 lhs, const Simd128Float32& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Float32 operator*(Simd128Float32 lhs, float rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Float32 operator*(float lhs, Simd128Float32 rhs) noexcept { rhs *= lhs; return rhs; }
inline static Simd128Float32 operator/(Simd128Float32 lhs, const Simd128Float32& rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Float32 operator/(Simd128Float32 lhs, float rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Float32 operator/(const float lhs, const Simd128Float32& rhs) noexcept { return Simd128Float32(lhs) / rhs; }

[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd128Float32 fma(const Simd128Float32 a, const Simd128Float32 b, const Simd128Float32 c) {
	return Simd128Float32(mt::simd_wasm_detail::map_ternary<float, 4>(a.v, b.v, c.v, [](float x, float y, float z) { return std::fma(x, y, z); }));
}
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd128Float32 fms(const Simd128Float32 a, const Simd128Float32 b, const Simd128Float32 c) {
	return Simd128Float32(mt::simd_wasm_detail::map_ternary<float, 4>(a.v, b.v, c.v, [](float x, float y, float z) { return std::fma(x, y, -z); }));
}
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd128Float32 fnma(const Simd128Float32 a, const Simd128Float32 b, const Simd128Float32 c) {
	return Simd128Float32(mt::simd_wasm_detail::map_ternary<float, 4>(a.v, b.v, c.v, [](float x, float y, float z) { return std::fma(-x, y, z); }));
}
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd128Float32 fnms(const Simd128Float32 a, const Simd128Float32 b, const Simd128Float32 c) {
	return Simd128Float32(mt::simd_wasm_detail::map_ternary<float, 4>(a.v, b.v, c.v, [](float x, float y, float z) { return std::fma(-x, y, -z); }));
}

[[nodiscard("Value calculated and not used (floor)")]]
inline static Simd128Float32 floor(Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_floor(a.v)); }
[[nodiscard("Value calculated and not used (ceil)")]]
inline static Simd128Float32 ceil(Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_ceil(a.v)); }
[[nodiscard("Value calculated and not used (trunc)")]]
inline static Simd128Float32 trunc(Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_trunc(a.v)); }
[[nodiscard("Value calculated and not used (round)")]]
inline static Simd128Float32 round(Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_nearest(a.v)); }
[[nodiscard("Value calculated and not used (fract)")]]
inline static Simd128Float32 fract(Simd128Float32 a) noexcept { return a - floor(a); }
[[nodiscard("Value calculated and not used (min)")]]
inline static Simd128Float32 min(const Simd128Float32 a, const Simd128Float32 b) noexcept { return Simd128Float32(wasm_f32x4_min(a.v, b.v)); }
[[nodiscard("Value calculated and not used (max)")]]
inline static Simd128Float32 max(const Simd128Float32 a, const Simd128Float32 b) noexcept { return Simd128Float32(wasm_f32x4_max(a.v, b.v)); }
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float32 clamp(const Simd128Float32 a) noexcept { return max(Simd128Float32(0.0f), min(a, Simd128Float32(1.0f))); }
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float32 clamp(const Simd128Float32 a, const Simd128Float32 min_v, const Simd128Float32 max_v) noexcept {
	return Simd128Float32(mt::simd_wasm_detail::clamp_vector<float, 4>(a.v, min_v.v, max_v.v));
}
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float32 clamp(const Simd128Float32 a, const float min_f, const float max_f) noexcept {
	return clamp(a, Simd128Float32(min_f), Simd128Float32(max_f));
}
[[nodiscard("Value calculated and not used (reciprocal_approx)")]]
inline static Simd128Float32 reciprocal_approx(const Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_div(Simd128Float32(1.0f).v, a.v)); }
[[nodiscard("Value calculated and not used (sqrt)")]]
inline static Simd128Float32 sqrt(const Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_sqrt(a.v)); }
[[nodiscard("Value Calculated and not used (abs)")]]
inline static Simd128Float32 abs(const Simd128Float32 a) noexcept { return Simd128Float32(wasm_f32x4_abs(a.v)); }

#define MT_WASM_F32_UNARY_STD(name, fn) \
[[nodiscard("Value calculated and not used (" #name ")")]] \
inline static Simd128Float32 name(const Simd128Float32 a) noexcept { \
	return Simd128Float32(mt::simd_wasm_detail::map_unary<float, 4>(a.v, [](float x) { return std::fn(x); })); \
}

#define MT_WASM_F32_BINARY_STD(name, fn) \
[[nodiscard("Value calculated and not used (" #name ")")]] \
inline static Simd128Float32 name(const Simd128Float32 a, const Simd128Float32 b) noexcept { \
	return Simd128Float32(mt::simd_wasm_detail::map_binary<float, 4>(a.v, b.v, [](float x, float y) { return std::fn(x, y); })); \
}

MT_WASM_F32_BINARY_STD(pow, pow)
MT_WASM_F32_UNARY_STD(exp, exp)
MT_WASM_F32_UNARY_STD(exp2, exp2)
MT_WASM_F32_UNARY_STD(expm1, expm1)
MT_WASM_F32_UNARY_STD(log, log)
MT_WASM_F32_UNARY_STD(log1p, log1p)
MT_WASM_F32_UNARY_STD(log2, log2)
MT_WASM_F32_UNARY_STD(log10, log10)
MT_WASM_F32_UNARY_STD(cbrt, cbrt)
MT_WASM_F32_BINARY_STD(hypot, hypot)
MT_WASM_F32_UNARY_STD(sin, sin)
MT_WASM_F32_UNARY_STD(cos, cos)
MT_WASM_F32_UNARY_STD(tan, tan)
MT_WASM_F32_UNARY_STD(asin, asin)
MT_WASM_F32_UNARY_STD(acos, acos)
MT_WASM_F32_UNARY_STD(atan, atan)
MT_WASM_F32_BINARY_STD(atan2, atan2)
MT_WASM_F32_UNARY_STD(sinh, sinh)
MT_WASM_F32_UNARY_STD(cosh, cosh)
MT_WASM_F32_UNARY_STD(tanh, tanh)
MT_WASM_F32_UNARY_STD(asinh, asinh)
MT_WASM_F32_UNARY_STD(acosh, acosh)
MT_WASM_F32_UNARY_STD(atanh, atanh)
inline static Simd128Float32 exp10(const Simd128Float32 a) noexcept {
	return Simd128Float32(mt::simd_wasm_detail::map_unary<float, 4>(a.v, [](float x) { return std::pow(10.0f, x); }));
}

#undef MT_WASM_F32_UNARY_STD
#undef MT_WASM_F32_BINARY_STD

inline static v128_t compare_equal(const Simd128Float32 a, const Simd128Float32 b) noexcept { return wasm_f32x4_eq(a.v, b.v); }
inline static v128_t compare_less(const Simd128Float32 a, const Simd128Float32 b) noexcept { return wasm_f32x4_lt(a.v, b.v); }
inline static v128_t compare_less_equal(const Simd128Float32 a, const Simd128Float32 b) noexcept { return wasm_f32x4_le(a.v, b.v); }
inline static v128_t compare_greater(const Simd128Float32 a, const Simd128Float32 b) noexcept { return wasm_f32x4_gt(a.v, b.v); }
inline static v128_t compare_greater_equal(const Simd128Float32 a, const Simd128Float32 b) noexcept { return wasm_f32x4_ge(a.v, b.v); }
inline static v128_t isnan(const Simd128Float32 a) noexcept { return wasm_f32x4_ne(a.v, a.v); }

[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd128Float32 blend(const Simd128Float32 if_false, const Simd128Float32 if_true, v128_t mask) noexcept {
	return Simd128Float32(wasm_v128_bitselect(if_true.v, if_false.v, mask));
}

#endif

/**************************************************************************************************
 * Templated Functions for all types
 * ************************************************************************************************/

//If values a and b are equal return if_true, otherwise return if_false.
template <SimdFloat32 T> 
[[nodiscard("Value Calculated and not used (if_equal)")]]
inline static T if_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_equal(value_a, value_b));
}

template <SimdFloat32 T>
[[nodiscard("Value Calculated and not used (if_less)")]]
inline static T if_less(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less(value_a, value_b));
}

template <SimdFloat32 T>
[[nodiscard("Value Calculated and not used (if_less_equal)")]]
inline static T if_less_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less_equal(value_a, value_b));
}

template <SimdFloat32 T>
[[nodiscard("Value Calculated and not used (if_greater)")]]
inline static T if_greater(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater(value_a, value_b));
}


template <SimdFloat32 T>
[[nodiscard("Value Calculated and not used (if_greater_equal)")]]
inline static T if_greater_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater_equal(value_a, value_b));
}


template <SimdFloat32 T>
[[nodiscard("Value Calculated and not used (if_nan)")]]
inline static T if_nan(const T value_a, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, isnan(value_a));
}



/**************************************************************************************************
 * MASK OPS
 * ************************************************************************************************/
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
inline static __m128 operator&(__m128  lhs, const __m128 rhs) noexcept { return _mm_and_ps(lhs,rhs); }
inline static __m128 operator|(__m128  lhs, const __m128 rhs) noexcept { return _mm_or_ps(lhs, rhs); }
inline static __m128 operator^(__m128  lhs, const __m128 rhs) noexcept { return _mm_xor_ps(lhs, rhs); }
inline static __m128 operator~(__m128  lhs) noexcept { return _mm_xor_ps(lhs, _mm_cmpeq_ps(lhs, lhs)); }

inline static __m256 operator&(__m256  lhs, const __m256 rhs) noexcept { return _mm256_and_ps(lhs, rhs); }
inline static __m256 operator|(__m256  lhs, const __m256 rhs) noexcept { return _mm256_or_ps(lhs, rhs); }
inline static __m256 operator^(__m256  lhs, const __m256 rhs) noexcept { return _mm256_xor_ps(lhs, rhs); }
inline static __m256 operator~(__m256  lhs) noexcept { return _mm256_xor_ps(lhs, _mm256_xor_ps(lhs, _mm256_set1_ps(std::bit_cast<float>(0xFFFFFFFF)))); }
#endif


/**************************************************************************************************
 * Check that each type implements the desired types from simd-concepts.h
 * (Not sure why this fails on intelisense, but compliles ok.)
 * ************************************************************************************************/
static_assert(Simd<FallbackFloat32>,"FallbackFloat32 does not implement the concept SIMD");
static_assert(SimdReal<FallbackFloat32>, "FallbackFloat32 does not implement the concept SimdReal");
static_assert(SimdFloat<FallbackFloat32>, "FallbackFloat32 does not implement the concept SimdFloat");
static_assert(SimdFloat32<FallbackFloat32>, "FallbackFloat32 does not implement the concept SimdFloat32");
static_assert(SimdFloatToInt<FallbackFloat32>, "FallbackFloat32 does not implement the concept SimdFloatToInt");
static_assert(SimdMath<FallbackFloat32>, "FallbackFloat32 does not implement the concept SimdMath");
static_assert(SimdCompareOps<FallbackFloat32>, "FallbackFloat32 does not implement the concept SimdCompareOps");


#if MT_SIMD_ARCH_X64
static_assert(Simd<Simd128Float32>, "Simd256Float32 does not implement the concept SIMD");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(Simd<Simd256Float32>, "Simd256Float32 does not implement the concept SIMD");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(Simd<Simd512Float32>, "Simd512Float32 does not implement the concept SIMD");
#endif


static_assert(SimdReal<Simd128Float32>, "Simd256Float32 does not implement the concept SimdReal");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdReal<Simd256Float32>, "Simd256Float32 does not implement the concept SimdReal");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdReal<Simd512Float32>, "Simd512Float32 does not implement the concept SimdReal");
#endif

static_assert(SimdFloat<Simd128Float32>, "Simd256Float32 does not implement the concept SimdFloat");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdFloat<Simd256Float32>, "Simd256Float32 does not implement the concept SimdFloat");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdFloat<Simd512Float32>, "Simd512Float32 does not implement the concept SimdFloat");
#endif

static_assert(SimdFloat32<Simd128Float32>, "Simd128Float32 does not implement the concept SimdFloat32");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdFloat32<Simd256Float32>, "Simd256Float32 does not implement the concept SimdFloat32");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdFloat32<Simd512Float32>, "Simd512Float32 does not implement the concept SimdFloat32");
#endif

static_assert(SimdFloatToInt<Simd128Float32>, "Simd128Float32 does not implement the concept SimdFloatToInt");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdFloatToInt<Simd256Float32>, "Simd256Float32 does not implement the concept SimdFloatToInt");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdFloatToInt<Simd512Float32>, "Simd512Float32 does not implement the concept SimdFloatToInt");
#endif

//SIMD Math Support
static_assert(SimdMath<Simd128Float32>, "Simd128Float32 does not implement the concept SimdMath");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdMath<Simd256Float32>, "Simd256Float32 does not implement the concept SimdMath");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdMath<Simd512Float32>, "Simd512Float32 does not implement the concept SimdMath");
#endif

//Compare Ops
static_assert(SimdCompareOps<Simd128Float32>, "Simd128Float32 does not implement the concept SimdCompareOps");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdCompareOps<Simd256Float32>, "Simd256Float32 does not implement the concept SimdCompareOps");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdCompareOps<Simd512Float32>, "Simd512Float32 does not implement the concept SimdCompareOps");
#endif

#endif


/**************************************************************************************************
 Define SimdNativeFloat32 as the best supported type at compile time.  
 (Based on microarchitecture level so that integers are also supported)
 * ************************************************************************************************/
#if MT_SIMD_ARCH_X64
#if MT_SIMD_ALLOW_LEVEL4_TYPES
		typedef Simd512Float32 SimdNativeFloat32;
	#else
#if MT_SIMD_ALLOW_LEVEL3_TYPES
			typedef Simd256Float32 SimdNativeFloat32;
		#else
			#if defined(__SSE4_1__) && defined(__SSE4_1__) && defined(__SSE3__) && defined(__SSSE3__) 
				typedef Simd128Float32 SimdNativeFloat32;
			#else
				typedef Simd128Float32 SimdNativeFloat32;
			#endif	
		#endif	
	#endif
#elif MT_SIMD_ARCH_WASM && defined(__wasm_simd128__)
	typedef Simd128Float32 SimdNativeFloat32;
#else 
	//non x64
	typedef FallbackFloat32 SimdNativeFloat32;
#endif


