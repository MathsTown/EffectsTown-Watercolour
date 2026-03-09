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

Basic SIMD Types for 64-bit floats:

FallbackFloat64		- Works on all build modes and CPUs.  Forwards most requests to the standard library.

Simd128Float64		- x86_64 Microarchitecture Level 1 - Works on all x86_64 CPUs.
					- Requires SSE and SSE2 support.  Will use SSE4.1 instructions when __SSE4_1__ or __AVX__ defined.

Simd256Float64		- x86_64 Microarchitecture Level 3.
					- Requires AVX, AVX2 and FMA support.

Simd512Float64		- x86_64 Microarchitecture Level 4.
					- Requires AVX512F, AVX512DQ, ACX512VL, AVX512CD, AVX512BW

SimdNativeFloat64	- A Typedef referring to one of the above types.  Chosen based on compiler support/mode.
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
I've included FallbackFloat64 for use with Emscripen, but use SimdNativeFloat64 as SIMD support will be added soon.



*********************************************************************************************************/



#include "environment.h"
#include "simd-cpuid.h"
#include "simd-concepts.h"
#include "simd-mask.h"
#include "simd-uint64.h"


#include <cmath>
#include <cstring>





/**************************************************************************************************
 * Fallback 64-Bit Floating Point
 *
 * ************************************************************************************************/
struct FallbackFloat64 {
	double v;

	typedef double F;				//The type of the underlying data.
	typedef bool MaskType;
	typedef FallbackUInt64 U;		//The type if cast to an unsigned int.
	typedef FallbackUInt64 U64;		//The type of a 64-bit unsigned int.

	FallbackFloat64() = default;
	FallbackFloat64(F a) : v(a) {};




	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported() { return true; }

#if MT_SIMD_ARCH_X64
	//Performs a runtime CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static bool cpu_supported(CpuInformation) { return true; }

#endif

	//Performs a compile time CPU check to see if this type is supported.  Checks this type ONLY (integers in same the same level may not be supported) 
	static constexpr bool compiler_supported() {
		return true;
	}

	//*****Access Elements*****
	F element(int) const { return v; }
	void set_element(int, F value) { v = value; }
	static constexpr int size_of_element() { return sizeof(double); }
	static constexpr int number_of_elements() { return 1; }

	//*****Addition Operators*****
	FallbackFloat64& operator+=(const FallbackFloat64& rhs) noexcept { v += rhs.v; return *this; }
	FallbackFloat64& operator+=(double rhs) noexcept { v += rhs;	return *this; }

	//*****Subtraction Operators*****
	FallbackFloat64& operator-=(const FallbackFloat64& rhs) noexcept { v -= rhs.v; return *this; }
	FallbackFloat64& operator-=(double rhs) noexcept { v -= rhs;	return *this; }

	//*****Multiplication Operators*****
	FallbackFloat64& operator*=(const FallbackFloat64& rhs) noexcept { v *= rhs.v; return *this; }
	FallbackFloat64& operator*=(double rhs) noexcept { v *= rhs; return *this; }

	//*****Division Operators*****
	FallbackFloat64& operator/=(const FallbackFloat64& rhs) noexcept { v /= rhs.v; return *this; }
	FallbackFloat64& operator/=(double rhs) noexcept { v /= rhs;	return *this; }

	//*****Negate Operator*****
	FallbackFloat64 operator-() const noexcept { return FallbackFloat64(-v); }

	//*****Make Functions****
	static FallbackFloat64 make_sequential(F first) { return FallbackFloat64(first); }
	static FallbackFloat64 make_set1(F v) { return FallbackFloat64(v); }

	static FallbackFloat64 make_from_uints_52bits(FallbackUInt64 i) {
		auto x = i.v & 0b0000000000001111111111111111111111111111111111111111111111111111; //mask of 52-bits.
		auto f = static_cast<F>(x);
		return FallbackFloat64(f);
	}


	//*****Cast Functions****
	FallbackUInt64 bitcast_to_uint() const { return FallbackUInt64(std::bit_cast<uint64_t>(this->v)); }

	



};

//*****Addition Operators*****
inline static FallbackFloat64 operator+(FallbackFloat64  lhs, const FallbackFloat64& rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackFloat64 operator+(FallbackFloat64  lhs, double rhs) noexcept { lhs += rhs; return lhs; }
inline static FallbackFloat64 operator+(double lhs, FallbackFloat64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static FallbackFloat64 operator-(FallbackFloat64  lhs, const FallbackFloat64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackFloat64 operator-(FallbackFloat64  lhs, double rhs) noexcept { lhs -= rhs; return lhs; }
inline static FallbackFloat64 operator-(const double lhs, const FallbackFloat64& rhs) noexcept { return FallbackFloat64(lhs - rhs.v); }

//*****Multiplication Operators*****
inline static FallbackFloat64 operator*(FallbackFloat64  lhs, const FallbackFloat64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackFloat64 operator*(FallbackFloat64  lhs, double rhs) noexcept { lhs *= rhs; return lhs; }
inline static FallbackFloat64 operator*(double lhs, FallbackFloat64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static FallbackFloat64 operator/(FallbackFloat64  lhs, const FallbackFloat64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static FallbackFloat64 operator/(FallbackFloat64  lhs, double rhs) noexcept { lhs /= rhs; return lhs; }
inline static FallbackFloat64 operator/(const double lhs, const FallbackFloat64& rhs) noexcept { return FallbackFloat64(lhs / rhs.v); }

//*****Fused Multiply Add Fallbacks*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static FallbackFloat64 fma(const FallbackFloat64  a, const FallbackFloat64 b, const FallbackFloat64 c) { 
	return std::fma(a.v, b.v, c.v);
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static FallbackFloat64 fms(const FallbackFloat64  a, const FallbackFloat64 b, const FallbackFloat64 c) { 
	return std::fma(a.v, b.v, -c.v);
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static FallbackFloat64 fnma(const FallbackFloat64  a, const FallbackFloat64 b, const FallbackFloat64 c) { 
	return std::fma(-a.v, b.v, c.v);
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static FallbackFloat64 fnms(const FallbackFloat64  a, const FallbackFloat64 b, const FallbackFloat64 c) { 
	return std::fma(-a.v, b.v, -c.v);
}

//*****Rounding Functions*****
inline static FallbackFloat64 floor(FallbackFloat64 a) { return  FallbackFloat64(std::floor(a.v)); }
inline static FallbackFloat64 ceil(FallbackFloat64 a) { return  FallbackFloat64(std::ceil(a.v)); }
inline static FallbackFloat64 trunc(FallbackFloat64 a) { return  FallbackFloat64(std::trunc(a.v)); }
inline static FallbackFloat64 round(FallbackFloat64 a) { return  FallbackFloat64(std::round(a.v)); }
inline static FallbackFloat64 fract(FallbackFloat64 a) { return a - floor(a); }


//*****Min/Max*****
inline static FallbackFloat64 min(FallbackFloat64 a, FallbackFloat64 b) { return FallbackFloat64(std::min(a.v, b.v)); }
inline static FallbackFloat64 max(FallbackFloat64 a, FallbackFloat64 b) { return FallbackFloat64(std::max(a.v, b.v)); }

//*****Approximate Functions*****
inline static FallbackFloat64 reciprocal_approx(FallbackFloat64 a) { return FallbackFloat64(1.0 / a.v); }

//*****Mathematical Functions*****
inline static FallbackFloat64 sqrt(FallbackFloat64 a) { return FallbackFloat64(std::sqrt(a.v)); }
inline static FallbackFloat64 abs(FallbackFloat64 a) { return FallbackFloat64(std::abs(a.v)); }
inline static FallbackFloat64 pow(FallbackFloat64 a, FallbackFloat64 b) { return FallbackFloat64(std::pow(a.v, b.v)); }
inline static FallbackFloat64 exp(FallbackFloat64 a) { return FallbackFloat64(std::exp(a.v)); }
inline static FallbackFloat64 exp2(FallbackFloat64 a) { return FallbackFloat64(std::exp2(a.v)); }
inline static FallbackFloat64 exp10(FallbackFloat64 a) { return FallbackFloat64(std::pow(10.0, a.v)); }
inline static FallbackFloat64 expm1(FallbackFloat64 a) { return FallbackFloat64(std::expm1(a.v)); }
inline static FallbackFloat64 log(FallbackFloat64 a) { return FallbackFloat64(std::log(a.v)); }
inline static FallbackFloat64 log1p(FallbackFloat64 a) { return FallbackFloat64(std::log1p(a.v)); }
inline static FallbackFloat64 log2(FallbackFloat64 a) { return FallbackFloat64(std::log2(a.v)); }
inline static FallbackFloat64 log10(FallbackFloat64 a) { return FallbackFloat64(std::log10(a.v)); }
inline static FallbackFloat64 cbrt(FallbackFloat64 a) { return FallbackFloat64(std::cbrt(a.v)); }
inline static FallbackFloat64 hypot(FallbackFloat64 a, FallbackFloat64 b) { return FallbackFloat64(std::hypot(a.v, b.v)); }

inline static FallbackFloat64 sin(FallbackFloat64 a) { return FallbackFloat64(std::sin(a.v)); }
inline static FallbackFloat64 cos(FallbackFloat64 a) { return FallbackFloat64(std::cos(a.v)); }
inline static FallbackFloat64 tan(FallbackFloat64 a) { return FallbackFloat64(std::tan(a.v)); }
inline static FallbackFloat64 asin(FallbackFloat64 a) { return FallbackFloat64(std::asin(a.v)); }
inline static FallbackFloat64 acos(FallbackFloat64 a) { return FallbackFloat64(std::acos(a.v)); }
inline static FallbackFloat64 atan(FallbackFloat64 a) { return FallbackFloat64(std::atan(a.v)); }
inline static FallbackFloat64 atan2(FallbackFloat64 y, FallbackFloat64 x) { return FallbackFloat64(std::atan2(y.v, x.v)); }
inline static FallbackFloat64 sinh(FallbackFloat64 a) { return FallbackFloat64(std::sinh(a.v)); }
inline static FallbackFloat64 cosh(FallbackFloat64 a) { return FallbackFloat64(std::cosh(a.v)); }
inline static FallbackFloat64 tanh(FallbackFloat64 a) { return FallbackFloat64(std::tanh(a.v)); }
inline static FallbackFloat64 asinh(FallbackFloat64 a) { return FallbackFloat64(std::asinh(a.v)); }
inline static FallbackFloat64 acosh(FallbackFloat64 a) { return FallbackFloat64(std::acosh(a.v)); }
inline static FallbackFloat64 atanh(FallbackFloat64 a) { return FallbackFloat64(std::atanh(a.v)); }


//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static FallbackFloat64 clamp(const FallbackFloat64 a) noexcept {
	return std::min(std::max(a.v, 0.0), 1.0);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static FallbackFloat64 clamp(const FallbackFloat64 a, const FallbackFloat64 min_f, const FallbackFloat64 max_f) noexcept {
	return std::min(std::max(a.v, min_f.v), max_f.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static FallbackFloat64 clamp(const FallbackFloat64 a, const double min_f, const double max_f) noexcept {
	return std::min(std::max(a.v, min_f), max_f);
}

//*****Conditional Functions *****

//Compare if 2 values are equal and return a mask.
inline static bool compare_equal(const FallbackFloat64 a, const FallbackFloat64 b) noexcept { return (a.v == b.v); }
inline static bool compare_less(const FallbackFloat64 a, const FallbackFloat64 b) noexcept { return(a.v < b.v); }
inline static bool compare_less_equal(const FallbackFloat64 a, const FallbackFloat64 b) noexcept { return (a.v <= b.v); }
inline static bool compare_greater(const FallbackFloat64 a, const FallbackFloat64 b) noexcept { return (a.v > b.v); }
inline static bool compare_greater_equal(const FallbackFloat64 a, const FallbackFloat64 b) noexcept { return (a.v >= b.v); }
inline static bool isnan(const FallbackFloat64 a) noexcept { return std::isnan(a.v); }

//Blend two values together based on mask.  First argument if zero. Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static FallbackFloat64 blend(const FallbackFloat64 if_false, const FallbackFloat64 if_true, bool mask) noexcept {
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

namespace mt::simd_detail_f64 {
	// Portability layer: GCC/Clang cannot index SIMD lanes via MSVC vector members.
	inline double lane_get(__m128d v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m128d_f64[i];
#else
		alignas(16) double lanes[2];
		_mm_storeu_pd(lanes, v);
		return lanes[i];
#endif
	}
	inline __m128d lane_set(__m128d v, int i, double value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m128d_f64[i] = value;
		return v;
#else
		alignas(16) double lanes[2];
		_mm_storeu_pd(lanes, v);
		lanes[i] = value;
		return _mm_loadu_pd(lanes);
#endif
	}
	inline double lane_get(const __m256d& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m256d_f64[i];
#else
		alignas(32) double lanes[4];
		_mm256_storeu_pd(lanes, v);
		return lanes[i];
#endif
	}
	inline void lane_set(__m256d& v, int i, double value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m256d_f64[i] = value;
#else
		alignas(32) double lanes[4];
		_mm256_storeu_pd(lanes, v);
		lanes[i] = value;
		v = _mm256_loadu_pd(lanes);
#endif
	}
	inline double lane_get(const __m512d& v, int i) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return v.m512d_f64[i];
#else
		alignas(64) double lanes[8];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	inline void lane_set(__m512d& v, int i, double value) noexcept {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		v.m512d_f64[i] = value;
#else
		alignas(64) double lanes[8];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

#if MT_SIMD_USE_PORTABLE_X86_SHIMS
	template <typename Fn>
	inline __m128d map_unary(__m128d v, Fn fn) {
		alignas(16) double in[2], out[2];
		_mm_storeu_pd(in, v);
		for (int i = 0; i < 2; ++i) out[i] = fn(in[i]);
		return _mm_loadu_pd(out);
	}
	template <typename Fn>
	inline __m256d map_unary(const __m256d& v, Fn fn) {
		alignas(32) double in[4], out[4];
		_mm256_storeu_pd(in, v);
		for (int i = 0; i < 4; ++i) out[i] = fn(in[i]);
		return _mm256_loadu_pd(out);
	}
	template <typename Fn>
	inline __m512d map_unary(const __m512d& v, Fn fn) {
		alignas(64) double in[8], out[8];
		std::memcpy(in, &v, sizeof(v));
		for (int i = 0; i < 8; ++i) out[i] = fn(in[i]);
		__m512d out_v;
		std::memcpy(&out_v, out, sizeof(out_v));
		return out_v;
	}
	template <typename Fn>
	inline __m128d map_binary(__m128d a, __m128d b, Fn fn) {
		alignas(16) double lhs[2], rhs[2], out[2];
		_mm_storeu_pd(lhs, a); _mm_storeu_pd(rhs, b);
		for (int i = 0; i < 2; ++i) out[i] = fn(lhs[i], rhs[i]);
		return _mm_loadu_pd(out);
	}
	template <typename Fn>
	inline __m256d map_binary(const __m256d& a, const __m256d& b, Fn fn) {
		alignas(32) double lhs[4], rhs[4], out[4];
		_mm256_storeu_pd(lhs, a); _mm256_storeu_pd(rhs, b);
		for (int i = 0; i < 4; ++i) out[i] = fn(lhs[i], rhs[i]);
		return _mm256_loadu_pd(out);
	}
	template <typename Fn>
	inline __m512d map_binary(const __m512d& a, const __m512d& b, Fn fn) {
		alignas(64) double lhs[8], rhs[8], out[8];
		std::memcpy(lhs, &a, sizeof(a));
		std::memcpy(rhs, &b, sizeof(b));
		for (int i = 0; i < 8; ++i) out[i] = fn(lhs[i], rhs[i]);
		__m512d out_v;
		std::memcpy(&out_v, out, sizeof(out_v));
		return out_v;
	}
#endif

#if MT_SIMD_USE_PORTABLE_X86_SHIMS && !MT_USE_SVML
	#define MT_F64_UNARY(name, expr) \
		inline __m128d name##_pd(__m128d v) { return map_unary(v, [](double x) { return (expr); }); } \
		inline __m256d name##_pd(__m256d v) { return map_unary(v, [](double x) { return (expr); }); } \
		inline __m512d name##_pd(__m512d v) { return map_unary(v, [](double x) { return (expr); }); }

	#define MT_F64_BINARY(name, expr) \
		inline __m128d name##_pd(__m128d a, __m128d b) { return map_binary(a, b, [](double x, double y) { return (expr); }); } \
		inline __m256d name##_pd(__m256d a, __m256d b) { return map_binary(a, b, [](double x, double y) { return (expr); }); } \
		inline __m512d name##_pd(__m512d a, __m512d b) { return map_binary(a, b, [](double x, double y) { return (expr); }); }

	MT_F64_UNARY(trunc, std::trunc(x))
	MT_F64_UNARY(round, std::round(x))
	MT_F64_UNARY(floor, std::floor(x))
	MT_F64_UNARY(ceil, std::ceil(x))
	MT_F64_UNARY(exp, std::exp(x))
	MT_F64_UNARY(exp2, std::exp2(x))
	MT_F64_UNARY(exp10, std::pow(10.0, x))
	MT_F64_UNARY(expm1, std::expm1(x))
	MT_F64_UNARY(log, std::log(x))
	MT_F64_UNARY(log1p, std::log1p(x))
	MT_F64_UNARY(log2, std::log2(x))
	MT_F64_UNARY(log10, std::log10(x))
	MT_F64_UNARY(cbrt, std::cbrt(x))
	MT_F64_UNARY(sin, std::sin(x))
	MT_F64_UNARY(cos, std::cos(x))
	MT_F64_UNARY(tan, std::tan(x))
	MT_F64_UNARY(asin, std::asin(x))
	MT_F64_UNARY(acos, std::acos(x))
	MT_F64_UNARY(atan, std::atan(x))
	MT_F64_UNARY(sinh, std::sinh(x))
	MT_F64_UNARY(cosh, std::cosh(x))
	MT_F64_UNARY(tanh, std::tanh(x))
	MT_F64_UNARY(asinh, std::asinh(x))
	MT_F64_UNARY(acosh, std::acosh(x))
	MT_F64_UNARY(atanh, std::atanh(x))
	MT_F64_UNARY(sind, std::sin(x * (3.14159265358979323846 / 180.0)))
	MT_F64_UNARY(cosd, std::cos(x * (3.14159265358979323846 / 180.0)))
	MT_F64_UNARY(tand, std::tan(x * (3.14159265358979323846 / 180.0)))
	MT_F64_BINARY(pow, std::pow(x, y))
	MT_F64_BINARY(hypot, std::hypot(x, y))
	MT_F64_BINARY(atan2, std::atan2(x, y))

	#undef MT_F64_UNARY
	#undef MT_F64_BINARY
#endif

}

#if MT_SIMD_USE_PORTABLE_X86_SHIMS
// Portability shim: these math names are not standard hardware intrinsics on GCC/Clang.
#define _mm_trunc_pd mt::simd_detail_f64::trunc_pd
#define _mm256_trunc_pd mt::simd_detail_f64::trunc_pd
#define _mm512_trunc_pd mt::simd_detail_f64::trunc_pd
#define _mm512_floor_pd mt::simd_detail_f64::floor_pd
#define _mm512_ceil_pd mt::simd_detail_f64::ceil_pd
#define _mm_pow_pd mt::simd_detail_f64::pow_pd
#define _mm256_pow_pd mt::simd_detail_f64::pow_pd
#define _mm512_pow_pd mt::simd_detail_f64::pow_pd
#define _mm_exp_pd mt::simd_detail_f64::exp_pd
#define _mm256_exp_pd mt::simd_detail_f64::exp_pd
#define _mm512_exp_pd mt::simd_detail_f64::exp_pd
#define _mm_exp2_pd mt::simd_detail_f64::exp2_pd
#define _mm256_exp2_pd mt::simd_detail_f64::exp2_pd
#define _mm512_exp2_pd mt::simd_detail_f64::exp2_pd
#define _mm_exp10_pd mt::simd_detail_f64::exp10_pd
#define _mm256_exp10_pd mt::simd_detail_f64::exp10_pd
#define _mm512_exp10_pd mt::simd_detail_f64::exp10_pd
#define _mm_expm1_pd mt::simd_detail_f64::expm1_pd
#define _mm256_expm1_pd mt::simd_detail_f64::expm1_pd
#define _mm512_expm1_pd mt::simd_detail_f64::expm1_pd
#define _mm_log_pd mt::simd_detail_f64::log_pd
#define _mm256_log_pd mt::simd_detail_f64::log_pd
#define _mm512_log_pd mt::simd_detail_f64::log_pd
#define _mm_log1p_pd mt::simd_detail_f64::log1p_pd
#define _mm256_log1p_pd mt::simd_detail_f64::log1p_pd
#define _mm512_log1p_pd mt::simd_detail_f64::log1p_pd
#define _mm_log2_pd mt::simd_detail_f64::log2_pd
#define _mm256_log2_pd mt::simd_detail_f64::log2_pd
#define _mm512_log2_pd mt::simd_detail_f64::log2_pd
#define _mm_log10_pd mt::simd_detail_f64::log10_pd
#define _mm256_log10_pd mt::simd_detail_f64::log10_pd
#define _mm512_log10_pd mt::simd_detail_f64::log10_pd
#define _mm_cbrt_pd mt::simd_detail_f64::cbrt_pd
#define _mm256_cbrt_pd mt::simd_detail_f64::cbrt_pd
#define _mm512_cbrt_pd mt::simd_detail_f64::cbrt_pd
#define _mm_hypot_pd mt::simd_detail_f64::hypot_pd
#define _mm256_hypot_pd mt::simd_detail_f64::hypot_pd
#define _mm512_hypot_pd mt::simd_detail_f64::hypot_pd
#define _mm_sin_pd mt::simd_detail_f64::sin_pd
#define _mm256_sin_pd mt::simd_detail_f64::sin_pd
#define _mm512_sin_pd mt::simd_detail_f64::sin_pd
#define _mm_cos_pd mt::simd_detail_f64::cos_pd
#define _mm256_cos_pd mt::simd_detail_f64::cos_pd
#define _mm512_cos_pd mt::simd_detail_f64::cos_pd
#define _mm_tan_pd mt::simd_detail_f64::tan_pd
#define _mm256_tan_pd mt::simd_detail_f64::tan_pd
#define _mm512_tan_pd mt::simd_detail_f64::tan_pd
#define _mm_asin_pd mt::simd_detail_f64::asin_pd
#define _mm256_asin_pd mt::simd_detail_f64::asin_pd
#define _mm512_asin_pd mt::simd_detail_f64::asin_pd
#define _mm_acos_pd mt::simd_detail_f64::acos_pd
#define _mm256_acos_pd mt::simd_detail_f64::acos_pd
#define _mm512_acos_pd mt::simd_detail_f64::acos_pd
#define _mm_atan_pd mt::simd_detail_f64::atan_pd
#define _mm256_atan_pd mt::simd_detail_f64::atan_pd
#define _mm512_atan_pd mt::simd_detail_f64::atan_pd
#define _mm_atan2_pd mt::simd_detail_f64::atan2_pd
#define _mm256_atan2_pd mt::simd_detail_f64::atan2_pd
#define _mm512_atan2_pd mt::simd_detail_f64::atan2_pd
#define _mm_sinh_pd mt::simd_detail_f64::sinh_pd
#define _mm256_sinh_pd mt::simd_detail_f64::sinh_pd
#define _mm512_sinh_pd mt::simd_detail_f64::sinh_pd
#define _mm_cosh_pd mt::simd_detail_f64::cosh_pd
#define _mm256_cosh_pd mt::simd_detail_f64::cosh_pd
#define _mm512_cosh_pd mt::simd_detail_f64::cosh_pd
#define _mm_tanh_pd mt::simd_detail_f64::tanh_pd
#define _mm256_tanh_pd mt::simd_detail_f64::tanh_pd
#define _mm512_tanh_pd mt::simd_detail_f64::tanh_pd
#define _mm_asinh_pd mt::simd_detail_f64::asinh_pd
#define _mm256_asinh_pd mt::simd_detail_f64::asinh_pd
#define _mm512_asinh_pd mt::simd_detail_f64::asinh_pd
#define _mm_acosh_pd mt::simd_detail_f64::acosh_pd
#define _mm256_acosh_pd mt::simd_detail_f64::acosh_pd
#define _mm512_acosh_pd mt::simd_detail_f64::acosh_pd
#define _mm_atanh_pd mt::simd_detail_f64::atanh_pd
#define _mm256_atanh_pd mt::simd_detail_f64::atanh_pd
#define _mm512_atanh_pd mt::simd_detail_f64::atanh_pd
#define _mm_sind_pd mt::simd_detail_f64::sind_pd
#define _mm_cosd_pd mt::simd_detail_f64::cosd_pd
#define _mm_tand_pd mt::simd_detail_f64::tand_pd
#endif



/****************************************************************************************************************************************************************************************************
 * SIMD 512 type.  Contains 16 x 64bit Floats
 * Requires AVX-512F support.
 * **************************************************************************************************************************************************************************************************/
#if MT_SIMD_ALLOW_LEVEL4_TYPES
struct Simd512Float64 {
	__m512d v;
	typedef __mmask8 MaskType;
	typedef double F;	
	typedef Simd512UInt64 U;
	typedef Simd512UInt64 U64;		//The type of a 64-bit unsigned int.

	Simd512Float64() = default;
	Simd512Float64(__m512d a) : v(a) {};
	Simd512Float64(F a) : v(_mm512_set1_pd(a)) {}

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
		return mt::environment::compiler_is_level_4;
	}

	static constexpr int size_of_element() { return sizeof(double); }
	static constexpr int number_of_elements() { return 8; }

	//*****Access Elements*****
	F element(int i) const {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		return mt::simd_detail_f64::lane_get(v, i);
#else
		alignas(32) double lanes[4];
		std::memcpy(lanes, &v, sizeof(v));
		return lanes[i];
#endif
	}
	void set_element(int i, F value) {
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
		mt::simd_detail_f64::lane_set(v, i, value);
#else
		alignas(32) double lanes[4];
		std::memcpy(lanes, &v, sizeof(v));
		lanes[i] = value;
		std::memcpy(&v, lanes, sizeof(v));
#endif
	}

	//*****Addition Operators*****
	Simd512Float64& operator+=(const Simd512Float64& rhs) noexcept { v = _mm512_add_pd(v, rhs.v); return *this; }
	Simd512Float64& operator+=(double rhs) noexcept { v = _mm512_add_pd(v, _mm512_set1_pd(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd512Float64& operator-=(const Simd512Float64& rhs) noexcept { v = _mm512_sub_pd(v, rhs.v); return *this; }
	Simd512Float64& operator-=(double rhs) noexcept { v = _mm512_sub_pd(v, _mm512_set1_pd(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd512Float64& operator*=(const Simd512Float64& rhs) noexcept { v = _mm512_mul_pd(v, rhs.v); return *this; }
	Simd512Float64& operator*=(double rhs) noexcept { v = _mm512_mul_pd(v, _mm512_set1_pd(rhs)); return *this; }

	//*****Division Operators*****
	Simd512Float64& operator/=(const Simd512Float64& rhs) noexcept { v = _mm512_div_pd(v, rhs.v); return *this; }
	Simd512Float64& operator/=(double rhs) noexcept { v = _mm512_div_pd(v, _mm512_set1_pd(rhs));	return *this; }

	//*****Negate Operator*****
	Simd512Float64 operator-() const noexcept { return Simd512Float64(_mm512_sub_pd(_mm512_setzero_pd(), v)); }

	//*****Make Functions****
	static Simd512Float64 make_sequential(F first) { return Simd512Float64(_mm512_set_pd(first + 7.0f, first + 6.0f, first + 5.0f, first + 4.0f, first + 3.0f, first + 2.0f, first + 1.0f, first)); }
	static Simd512Float64 make_set1(F v) { return _mm512_set1_pd(v); }

	static Simd512Float64 make_from_uints_52bits(Simd512UInt64 i) {
		auto x = _mm512_and_si512(i.v, _mm512_set1_epi64(0b0000000000001111111111111111111111111111111111111111111111111111)); //mask of 52-bits.
		auto u = _mm512_cvtepu64_pd(x); //avx-512dq instruction
		return Simd512Float64(u);
	}

	//*****Cast Functions****

	//Warning: Returned type requires additional CPU features (AVX-512DQ)
	Simd512UInt64 bitcast_to_uint() const { return Simd512UInt64(_mm512_castpd_si512(this->v)); }

	



};

//*****Addition Operators*****
inline static Simd512Float64 operator+(Simd512Float64 lhs, const Simd512Float64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Float64 operator+(Simd512Float64 lhs, double rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd512Float64 operator+(double lhs, Simd512Float64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd512Float64 operator-(Simd512Float64  lhs, const Simd512Float64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Float64 operator-(Simd512Float64  lhs, double rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd512Float64 operator-(const double lhs, const Simd512Float64& rhs) noexcept { return Simd512Float64(_mm512_sub_pd(_mm512_set1_pd(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd512Float64 operator*(Simd512Float64  lhs, const Simd512Float64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Float64 operator*(Simd512Float64  lhs, double rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd512Float64 operator*(double lhs, Simd512Float64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd512Float64 operator/(Simd512Float64  lhs, const Simd512Float64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd512Float64 operator/(Simd512Float64  lhs, double rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd512Float64 operator/(const double lhs, const Simd512Float64& rhs) noexcept { return Simd512Float64(_mm512_div_pd(_mm512_set1_pd(lhs), rhs.v)); }

//*****Fused Multiply Add Instructions*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd512Float64 fma(const Simd512Float64  a, const Simd512Float64 b, const Simd512Float64 c) {
	return _mm512_fmadd_pd(a.v, b.v, c.v);
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd512Float64 fms(const Simd512Float64  a, const Simd512Float64 b, const Simd512Float64 c) {
	return _mm512_fmsub_pd(a.v, b.v, c.v);
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd512Float64 fnma(const Simd512Float64  a, const Simd512Float64 b, const Simd512Float64 c) {
	return _mm512_fnmadd_pd(a.v, b.v, c.v);
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd512Float64 fnms(const Simd512Float64  a, const Simd512Float64 b, const Simd512Float64 c) {
	return _mm512_fnmsub_pd(a.v, b.v, c.v);
}

//*****Rounding Functions*****
inline static Simd512Float64 floor(Simd512Float64 a) {return  Simd512Float64(_mm512_floor_pd(a.v)); }
inline static Simd512Float64 ceil(Simd512Float64 a) { return  Simd512Float64(_mm512_ceil_pd(a.v)); }
inline static Simd512Float64 trunc(Simd512Float64 a) { return  Simd512Float64(_mm512_trunc_pd(a.v)); }
inline static Simd512Float64 round(Simd512Float64 a) { return  Simd512Float64(_mm512_roundscale_pd(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)); }
inline static Simd512Float64 fract(Simd512Float64 a) { return a - floor(a);}

//*****Min/Max*****
inline static Simd512Float64 min(Simd512Float64 a, Simd512Float64 b) { return Simd512Float64(_mm512_min_pd(a.v,b.v)); }
inline static Simd512Float64 max(Simd512Float64 a, Simd512Float64 b) { return Simd512Float64(_mm512_max_pd(a.v, b.v)); }


//*****Approximate Functions*****
inline static Simd512Float64 reciprocal_approx(Simd512Float64 a) { return Simd512Float64(_mm512_rcp14_pd(a.v)); }

//*****512-bit Mathematical Functions*****
inline static Simd512Float64 sqrt(Simd512Float64 a) { return Simd512Float64(_mm512_sqrt_pd(a.v)); }

//Calculate a raised to the power of b
[[nodiscard("Value calculated and not used (pow)")]]
inline static Simd512Float64 pow(Simd512Float64 a, Simd512Float64 b) noexcept { return Simd512Float64(_mm512_pow_pd(a.v, b.v)); }

//Calculate e^x
[[nodiscard("Value calculated and not used (exp)")]]
inline static Simd512Float64 exp(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_exp_pd(a.v)); } 

//Calculate 2^x
[[nodiscard("Value calculated and not used (exp2)")]]
inline static Simd512Float64 exp2(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_exp2_pd(a.v)); } 

//Calculate 10^x
[[nodiscard("Value calculated and not used (exp10)")]]
inline static Simd512Float64 exp10(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_exp10_pd(a.v)); } 

//Calculate (e^x)-1.0
[[nodiscard("Value calculated and not used (exp_minus1)")]]
inline static Simd512Float64 expm1(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_expm1_pd(a.v)); } 

//Calulate natural log(x)
[[nodiscard("Value calculated and not used (log)")]]
inline static Simd512Float64 log(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_log_pd(a.v)); } 

//Calulate log(1.0 + x)
[[nodiscard("Value calculated and not used (log1p)")]]
inline static Simd512Float64 log1p(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_log1p_pd(a.v)); } 

//Calculate log_1(x)
[[nodiscard("Value calculated and not used (log2)")]]
inline static Simd512Float64 log2(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_log2_pd(a.v)); } 

//Calculate log_10(x)
[[nodiscard("Value calculated and not used (log10)")]]
inline static Simd512Float64 log10(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_log10_pd(a.v)); } 

//Calculate cube root
[[nodiscard("Value calculated and not used (cbrt)")]]
inline static Simd512Float64 cbrt(const Simd512Float64 a) noexcept { return Simd512Float64(_mm512_cbrt_pd(a.v)); } 

//Calculate hypot(x).  That is: sqrt(a^2 + b^2) while avoiding overflow.
[[nodiscard("Value calculated and not used (hypot)")]]
inline static Simd512Float64 hypot(const Simd512Float64 a, const Simd512Float64 b) noexcept { return Simd512Float64(_mm512_hypot_pd(a.v, b.v)); } 

inline static Simd512Float64 sin(Simd512Float64 a) { return Simd512Float64(_mm512_sin_pd(a.v)); }
inline static Simd512Float64 cos(Simd512Float64 a) { return Simd512Float64(_mm512_cos_pd(a.v)); }
inline static Simd512Float64 tan(Simd512Float64 a) { return Simd512Float64(_mm512_tan_pd(a.v)); }
inline static Simd512Float64 asin(Simd512Float64 a) { return Simd512Float64(_mm512_asin_pd(a.v)); }
inline static Simd512Float64 acos(Simd512Float64 a) { return Simd512Float64(_mm512_acos_pd(a.v)); }
inline static Simd512Float64 atan(Simd512Float64 a) { return Simd512Float64(_mm512_atan_pd(a.v)); }
inline static Simd512Float64 atan2(Simd512Float64 a, Simd512Float64 b) { return Simd512Float64(_mm512_atan2_pd(a.v, b.v)); }
inline static Simd512Float64 sinh(Simd512Float64 a) { return Simd512Float64(_mm512_sinh_pd(a.v)); }
inline static Simd512Float64 cosh(Simd512Float64 a) { return Simd512Float64(_mm512_cosh_pd(a.v)); }
inline static Simd512Float64 tanh(Simd512Float64 a) { return Simd512Float64(_mm512_tanh_pd(a.v)); }
inline static Simd512Float64 asinh(Simd512Float64 a) { return Simd512Float64(_mm512_asinh_pd(a.v)); }
inline static Simd512Float64 acosh(Simd512Float64 a) { return Simd512Float64(_mm512_acosh_pd(a.v)); }
inline static Simd512Float64 atanh(Simd512Float64 a) { return Simd512Float64(_mm512_atanh_pd(a.v)); }
inline static Simd512Float64 abs(Simd512Float64 a) { return Simd512Float64(_mm512_abs_pd(a.v)); }


//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd512Float64 clamp(const Simd512Float64 a) noexcept {
	const auto zero = _mm512_setzero_pd();
	const auto one = _mm512_set1_pd(1.0);
	return _mm512_min_pd(_mm512_max_pd(a.v, zero), one);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd512Float64 clamp(const Simd512Float64 a, const Simd512Float64 min, const Simd512Float64 max) noexcept {
	return _mm512_min_pd(_mm512_max_pd(a.v, min.v), max.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd512Float64 clamp(const Simd512Float64 a, const float min_f, const float max_f) noexcept {
	const auto min = _mm512_set1_pd(min_f);
	const auto max = _mm512_set1_pd(max_f);
	return _mm512_min_pd(_mm512_max_pd(a.v, min), max);
}

//*****AVX-512 Conditional Functions *****

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_equal)")]]
inline static __mmask8 compare_equal(const Simd512Float64 a, const Simd512Float64 b) noexcept { return _mm512_cmp_pd_mask(a.v, b.v, _CMP_EQ_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_less)")]]
inline static __mmask8 compare_less(const Simd512Float64 a, const Simd512Float64 b) noexcept { return _mm512_cmp_pd_mask(a.v, b.v, _CMP_LT_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_less_equal)")]]
inline static __mmask8 compare_less_equal(const Simd512Float64 a, const Simd512Float64 b) noexcept { return _mm512_cmp_pd_mask(a.v, b.v, _CMP_LE_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_greater)")]]
inline static __mmask8 compare_greater(const Simd512Float64 a, const Simd512Float64 b) noexcept { return _mm512_cmp_pd_mask(a.v, b.v, _CMP_GT_OQ); }

//Compare ordered.
[[nodiscard("Value Calculated and not used (compare_greater_equal)")]]
inline static __mmask8 compare_greater_equal(const Simd512Float64 a, const Simd512Float64 b) noexcept { return _mm512_cmp_pd_mask(a.v, b.v, _CMP_GE_OQ); }

//Compare to nan
[[nodiscard("Value Calculated and not used (compare_is_nan)")]]
inline static __mmask8 isnan(const Simd512Float64 a) noexcept { return _mm512_cmp_pd_mask(a.v, a.v, _CMP_UNORD_Q); }


//Blend two values together based on mask.First argument if zero.Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd512Float64 blend(const Simd512Float64 if_false, const Simd512Float64 if_true, __mmask8 mask) noexcept {
	return Simd512Float64(_mm512_mask_blend_pd(mask, if_false.v, if_true.v));
}


/*inline static bool test_all_true(__m256d mask) {
	return _mm256_testc_pd(mask, _mm256_set1_pd(std::bit_cast<double>(0xFFFFFFFFFFFFFFFF)));
}*/

/**************************************************************************************************
 * SIMD 256 type.  Contains 8 x 64bit Floats
 * Requires AVX support.
 * ************************************************************************************************/
#endif // MT_SIMD_ALLOW_LEVEL4_TYPES

#if MT_SIMD_ALLOW_LEVEL3_TYPES
struct Simd256Float64 {
	__m256d v;

	typedef double F;
	typedef __m256d MaskType;
	typedef Simd256UInt64 U;
	typedef Simd256UInt64 U64;

	Simd256Float64() = default;
	Simd256Float64(__m256d a) : v(a) {}
	Simd256Float64(F a) : v(_mm256_set1_pd(a)) {}
	
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
		return mt::environment::compiler_is_level_3;
	}

	static constexpr int size_of_element() { return sizeof(double); }
	static constexpr int number_of_elements() { return 4; }

	//*****Access Elements*****
	F element(int i) const { return mt::simd_detail_f64::lane_get(v, i); }
	void set_element(int i, F value) { mt::simd_detail_f64::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd256Float64& operator+=(const Simd256Float64& rhs) noexcept { v = _mm256_add_pd(v, rhs.v); return *this; }
	Simd256Float64& operator+=(double rhs) noexcept { v = _mm256_add_pd(v, _mm256_set1_pd(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd256Float64& operator-=(const Simd256Float64& rhs) noexcept { v = _mm256_sub_pd(v, rhs.v); return *this; }
	Simd256Float64& operator-=(double rhs) noexcept { v = _mm256_sub_pd(v, _mm256_set1_pd(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd256Float64& operator*=(const Simd256Float64& rhs) noexcept { v = _mm256_mul_pd(v, rhs.v); return *this; }
	Simd256Float64& operator*=(double rhs) noexcept { v = _mm256_mul_pd(v, _mm256_set1_pd(rhs)); return *this; }

	//*****Division Operators*****
	Simd256Float64& operator/=(const Simd256Float64& rhs) noexcept { v = _mm256_div_pd(v, rhs.v); return *this; }
	Simd256Float64& operator/=(double rhs) noexcept { v = _mm256_div_pd(v, _mm256_set1_pd(rhs));	return *this; }


	//*****Negate Operator*****
	Simd256Float64 operator-() const noexcept { return Simd256Float64(_mm256_sub_pd(_mm256_setzero_pd(), v)); }

	//*****Make Functions****
	static Simd256Float64 make_sequential(F first) { return Simd256Float64(_mm256_set_pd(first + 3.0f, first + 2.0f, first + 1.0f, first)); }
	static Simd256Float64 make_set1(F v) { return _mm256_set1_pd(v); }
	
	//Convert uints that are less than 2^52 to double (this is quicker than full range)
	static Simd256Float64 make_from_uints_52bits(Simd256UInt64 i) {
		//From: https://stackoverflow.com/questions/41144668/how-to-efficiently-perform-double-int64-conversions-with-sse-avx

		auto x = _mm256_and_si256(i.v, _mm256_set1_epi64x(0b0000000000001111111111111111111111111111111111111111111111111111)); //mask of 52-bits.
		x = _mm256_or_si256(x, _mm256_castpd_si256(_mm256_set1_pd(0x0010000000000000)));
		auto u = _mm256_sub_pd(_mm256_castsi256_pd(x), _mm256_set1_pd(0x0010000000000000));
		return Simd256Float64(u);
	}



	//*****Cast Functions****

	//Warning: Requires additional CPU features (AVX2)
	Simd256UInt64 bitcast_to_uint() const { return Simd256UInt64(_mm256_castpd_si256(this->v)); }

	

};

//*****Addition Operators*****
inline static Simd256Float64 operator+(Simd256Float64  lhs, const Simd256Float64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Float64 operator+(Simd256Float64  lhs, double rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd256Float64 operator+(double lhs, Simd256Float64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd256Float64 operator-(Simd256Float64  lhs, const Simd256Float64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Float64 operator-(Simd256Float64  lhs, double rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd256Float64 operator-(const double lhs, const Simd256Float64& rhs) noexcept { return Simd256Float64(_mm256_sub_pd(_mm256_set1_pd(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd256Float64 operator*(Simd256Float64  lhs, const Simd256Float64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Float64 operator*(Simd256Float64  lhs, double rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd256Float64 operator*(double lhs, Simd256Float64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd256Float64 operator/(Simd256Float64  lhs, const Simd256Float64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd256Float64 operator/(Simd256Float64  lhs, double rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd256Float64 operator/(const double lhs, const Simd256Float64& rhs) noexcept { return Simd256Float64(_mm256_div_pd(_mm256_set1_pd(lhs), rhs.v)); }

//*****Fused Multiply Add Instructions*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd256Float64 fma(const Simd256Float64  a, const Simd256Float64 b, const Simd256Float64 c) {
	return _mm256_fmadd_pd(a.v, b.v, c.v);
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd256Float64 fms(const Simd256Float64  a, const Simd256Float64 b, const Simd256Float64 c) {
	return _mm256_fmsub_pd(a.v, b.v, c.v);
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd256Float64 fnma(const Simd256Float64  a, const Simd256Float64 b, const Simd256Float64 c) {
	return _mm256_fnmadd_pd(a.v, b.v, c.v);
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd256Float64 fnms(const Simd256Float64  a, const Simd256Float64 b, const Simd256Float64 c) {
	return _mm256_fnmsub_pd(a.v, b.v, c.v);
}

//*****Rounding Functions*****
inline static Simd256Float64 floor(Simd256Float64 a) { return  Simd256Float64(_mm256_floor_pd(a.v)); }
inline static Simd256Float64 ceil(Simd256Float64 a) { return  Simd256Float64(_mm256_ceil_pd(a.v)); }
inline static Simd256Float64 trunc(Simd256Float64 a) { return  Simd256Float64(_mm256_trunc_pd(a.v)); }
inline static Simd256Float64 round(Simd256Float64 a) { return  Simd256Float64(_mm256_round_pd(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)); }
inline static Simd256Float64 fract(Simd256Float64 a) { return a - floor(a); }

//*****Min/Max*****
inline static Simd256Float64 min(Simd256Float64 a, Simd256Float64 b) { return Simd256Float64(_mm256_min_pd(a.v, b.v)); }
inline static Simd256Float64 max(Simd256Float64 a, Simd256Float64 b) { return Simd256Float64(_mm256_max_pd(a.v, b.v)); }



//*****Approximate Functions*****
inline static Simd256Float64 reciprocal_approx(Simd256Float64 a) {
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd256Float64(_mm256_rcp14_pd(a.v));
	}
	else {
		return Simd256Float64(1.0 / a);
	}
}



//*****256-bit Mathematical Functions*****
inline static Simd256Float64 sqrt(Simd256Float64 a) { return Simd256Float64(_mm256_sqrt_pd(a.v)); }


[[nodiscard("Value calculated and not used (pow)")]]
inline static Simd256Float64 pow(Simd256Float64 a, Simd256Float64 b) noexcept { return Simd256Float64(_mm256_pow_pd(a.v, b.v)); }

//Calculate e^x
[[nodiscard("Value calculated and not used (exp)")]]
inline static Simd256Float64 exp(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_exp_pd(a.v)); } 

//Calculate 2^x
[[nodiscard("Value calculated and not used (exp2)")]]
inline static Simd256Float64 exp2(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_exp2_pd(a.v)); } 

//Calculate 10^x
[[nodiscard("Value calculated and not used (exp10)")]]
inline static Simd256Float64 exp10(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_exp10_pd(a.v)); } 

//Calculate (e^x)-1.0
[[nodiscard("Value calculated and not used (exp_minus1)")]]
inline static Simd256Float64 expm1(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_expm1_pd(a.v)); } 

//Calulate natural log(x)
[[nodiscard("Value calculated and not used (log)")]]
inline static Simd256Float64 log(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_log_pd(a.v)); } 

//Calulate log(1.0 + x)
[[nodiscard("Value calculated and not used (log1p)")]]
inline static Simd256Float64 log1p(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_log1p_pd(a.v)); } 

//Calculate log_1(x)
[[nodiscard("Value calculated and not used (log2)")]]
inline static Simd256Float64 log2(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_log2_pd(a.v)); } 

//Calculate log_10(x)
[[nodiscard("Value calculated and not used (log10)")]]
inline static Simd256Float64 log10(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_log10_pd(a.v)); } 

//Calculate cube root
[[nodiscard("Value calculated and not used (cbrt)")]]
inline static Simd256Float64 cbrt(const Simd256Float64 a) noexcept { return Simd256Float64(_mm256_cbrt_pd(a.v)); } 

//Calculate hypot(x).  That is: sqrt(a^2 + b^2) while avoiding overflow.
[[nodiscard("Value calculated and not used (hypot)")]]
inline static Simd256Float64 hypot(const Simd256Float64 a, const Simd256Float64 b) noexcept { return Simd256Float64(_mm256_hypot_pd(a.v, b.v)); } 

inline static Simd256Float64 sin(Simd256Float64 a) { return Simd256Float64(_mm256_sin_pd(a.v)); }
inline static Simd256Float64 cos(Simd256Float64 a) { return Simd256Float64(_mm256_cos_pd(a.v)); }
inline static Simd256Float64 tan(Simd256Float64 a) { return Simd256Float64(_mm256_tan_pd(a.v)); }
inline static Simd256Float64 asin(Simd256Float64 a) { return Simd256Float64(_mm256_asin_pd(a.v)); }
inline static Simd256Float64 acos(Simd256Float64 a) { return Simd256Float64(_mm256_acos_pd(a.v)); }
inline static Simd256Float64 atan(Simd256Float64 a) { return Simd256Float64(_mm256_atan_pd(a.v)); }
inline static Simd256Float64 atan2(Simd256Float64 a, Simd256Float64 b) { return Simd256Float64(_mm256_atan2_pd(a.v, b.v)); }
inline static Simd256Float64 sinh(Simd256Float64 a) { return Simd256Float64(_mm256_sinh_pd(a.v)); }
inline static Simd256Float64 cosh(Simd256Float64 a) { return Simd256Float64(_mm256_cosh_pd(a.v)); }
inline static Simd256Float64 tanh(Simd256Float64 a) { return Simd256Float64(_mm256_tanh_pd(a.v)); }
inline static Simd256Float64 asinh(Simd256Float64 a) { return Simd256Float64(_mm256_asinh_pd(a.v)); }
inline static Simd256Float64 acosh(Simd256Float64 a) { return Simd256Float64(_mm256_acosh_pd(a.v)); }
inline static Simd256Float64 atanh(Simd256Float64 a) { return Simd256Float64(_mm256_atanh_pd(a.v)); }
inline static Simd256Float64 abs(Simd256Float64 a) {
	auto r = _mm256_and_pd(_mm256_set1_pd(std::bit_cast<double>(0x7FFFFFFFFFFFFFFFull)), a.v); //No AVX for abs
	return Simd256Float64(r);
}

//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd256Float64 clamp(const Simd256Float64 a) noexcept {
	const auto zero = _mm256_setzero_pd();
	const auto one = _mm256_set1_pd(1.0);
	return _mm256_min_pd(_mm256_max_pd(a.v, zero), one);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd256Float64 clamp(const Simd256Float64 a, const Simd256Float64 min, const Simd256Float64 max) noexcept {
	return _mm256_min_pd(_mm256_max_pd(a.v, min.v), max.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd256Float64 clamp(const Simd256Float64 a, const float min_f, const float max_f) noexcept {
	const auto min = _mm256_set1_pd(min_f);
	const auto max = _mm256_set1_pd(max_f);
	return _mm256_min_pd(_mm256_max_pd(a.v, min), max);
}


//*****Conditional Functions *****

//Compare ordered.
inline static __m256d compare_equal(const Simd256Float64 a, const Simd256Float64 b) noexcept { return _mm256_cmp_pd(a.v, b.v, _CMP_EQ_OQ); }
inline static __m256d compare_less(const Simd256Float64 a, const Simd256Float64 b) noexcept { return _mm256_cmp_pd(a.v, b.v, _CMP_LT_OS); }
inline static __m256d compare_less_equal(const Simd256Float64 a, const Simd256Float64 b) noexcept { return _mm256_cmp_pd(a.v, b.v, _CMP_LE_OS); }
inline static __m256d compare_greater(const Simd256Float64 a, const Simd256Float64 b) noexcept { return _mm256_cmp_pd(a.v, b.v, _CMP_GT_OS); }
inline static __m256d compare_greater_equal(const Simd256Float64 a, const Simd256Float64 b) noexcept { return _mm256_cmp_pd(a.v, b.v, _CMP_GE_OS); }
inline static __m256d isnan(const Simd256Float64 a) noexcept { return _mm256_cmp_pd(a.v, a.v, _CMP_UNORD_Q); }

//Blend two values together based on mask.First argument if zero.Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd256Float64 blend(const Simd256Float64 if_false, const Simd256Float64 if_true, __m256d mask) noexcept {
	return Simd256Float64(_mm256_blendv_pd(if_false.v, if_true.v, mask));
}



/***************************************************************************************************************************************************************************************************
 * SIMD 128 type.  Contains 2 x 64bit Floats
 * Requires SSE2 support.  
 
 * *************************************************************************************************************************************************************************************************/
#endif // MT_SIMD_ALLOW_LEVEL3_TYPES

struct Simd128Float64 {
	__m128d v;
	typedef double F;
	typedef __m128d MaskType;
	typedef Simd128UInt32 U;
	typedef Simd128UInt64 U64;


	//*****Constructors*****
	Simd128Float64() = default;
	Simd128Float64(__m128d a) : v(a) {};
	Simd128Float64(F a) : v(_mm_set1_pd(a)) {};

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
		return mt::environment::compiler_is_level_1;
	}


	static constexpr int size_of_element() { return sizeof(double); }
	static constexpr int number_of_elements() { return 2; }

	//*****Access Elements*****
	F element(int i)  const { return mt::simd_detail_f64::lane_get(v, i); }
	void set_element(int i, F value) { v = mt::simd_detail_f64::lane_set(v, i, value); }

	//*****Addition Operators*****
	Simd128Float64& operator+=(const Simd128Float64& rhs) noexcept { v = _mm_add_pd(v, rhs.v); return *this; } //SSE1
	Simd128Float64& operator+=(double rhs) noexcept { v = _mm_add_pd(v, _mm_set1_pd(rhs));	return *this; }

	//*****Subtraction Operators*****
	Simd128Float64& operator-=(const Simd128Float64& rhs) noexcept { v = _mm_sub_pd(v, rhs.v); return *this; }//SSE1
	Simd128Float64& operator-=(double rhs) noexcept { v = _mm_sub_pd(v, _mm_set1_pd(rhs));	return *this; }

	//*****Multiplication Operators*****
	Simd128Float64& operator*=(const Simd128Float64& rhs) noexcept { v = _mm_mul_pd(v, rhs.v); return *this; } //SSE1
	Simd128Float64& operator*=(double rhs) noexcept { v = _mm_mul_pd(v, _mm_set1_pd(rhs)); return *this; }

	//*****Division Operators*****
	Simd128Float64& operator/=(const Simd128Float64& rhs) noexcept { v = _mm_div_pd(v, rhs.v); return *this; } //SSE1
	Simd128Float64& operator/=(double rhs) noexcept { v = _mm_div_pd(v, _mm_set1_pd(rhs));	return *this; }

	//*****Negate Operators*****
	Simd128Float64 operator-() const noexcept { return Simd128Float64(_mm_sub_pd(_mm_setzero_pd(), v)); }


	//*****Make Functions****
	static Simd128Float64 make_sequential(F first) { return Simd128Float64(_mm_set_pd(first + 1.0f, first)); }
	static Simd128Float64 make_set1(F v) { return Simd128Float64(_mm_set1_pd(v)); }


	//static Simd128Float64 make_from_int64(Simd128UInt64 i) { return Simd128Float64(_mm_cvtepi64_pd(i.v)); } //SSE2

	//*****Cast Functions****

	//Warning: May requires additional CPU features 
	Simd128UInt64 bitcast_to_uint() const { return Simd128UInt64(_mm_castpd_si128(this->v)); } //SSE2

	

};

//*****Addition Operators*****
inline static Simd128Float64 operator+(Simd128Float64  lhs, const Simd128Float64& rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Float64 operator+(Simd128Float64  lhs, double rhs) noexcept { lhs += rhs; return lhs; }
inline static Simd128Float64 operator+(double lhs, Simd128Float64 rhs) noexcept { rhs += lhs; return rhs; }

//*****Subtraction Operators*****
inline static Simd128Float64 operator-(Simd128Float64  lhs, const Simd128Float64& rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Float64 operator-(Simd128Float64  lhs, double rhs) noexcept { lhs -= rhs; return lhs; }
inline static Simd128Float64 operator-(const double lhs, const Simd128Float64& rhs) noexcept { return Simd128Float64(_mm_sub_pd(_mm_set1_pd(lhs), rhs.v)); }

//*****Multiplication Operators*****
inline static Simd128Float64 operator*(Simd128Float64  lhs, const Simd128Float64& rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Float64 operator*(Simd128Float64  lhs, double rhs) noexcept { lhs *= rhs; return lhs; }
inline static Simd128Float64 operator*(double lhs, Simd128Float64 rhs) noexcept { rhs *= lhs; return rhs; }

//*****Division Operators*****
inline static Simd128Float64 operator/(Simd128Float64  lhs, const Simd128Float64& rhs) noexcept { lhs /= rhs;	return lhs; }
inline static Simd128Float64 operator/(Simd128Float64  lhs, double rhs) noexcept { lhs /= rhs; return lhs; }
inline static Simd128Float64 operator/(const double lhs, const Simd128Float64& rhs) noexcept { return Simd128Float64(_mm_div_pd(_mm_set1_pd(lhs), rhs.v)); }

//*****Rounding Functions*****
[[nodiscard("Value calculated and not used (floor)")]]
inline static Simd128Float64 floor(Simd128Float64 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float64(_mm_floor_pd(a.v)); //SSE4.1
	}
	else {
		return Simd128Float64(_mm_set_pd(std::floor(a.element(1)), std::floor(a.element(0))));
	}
}

[[nodiscard("Value calculated and not used (ceil)")]]
inline static Simd128Float64 ceil(Simd128Float64 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float64(_mm_ceil_pd(a.v)); //SSE4.1
	}
	else {
		return Simd128Float64(_mm_set_pd( std::ceil(a.element(1)), std::ceil(a.element(0))));
	}
}

[[nodiscard("Value calculated and not used (trunc)")]]
inline static Simd128Float64 trunc(Simd128Float64 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float64(_mm_round_pd(a.v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)); //SSE4.1
	}
	else {
		return Simd128Float64(_mm_set_pd(std::trunc(a.element(1)), std::trunc(a.element(0))));
	}
}

[[nodiscard("Value calculated and not used (round)")]]
inline static Simd128Float64 round(Simd128Float64 a) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float64(_mm_round_pd(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)); //SSE4.1
	}
	else {
		return Simd128Float64(_mm_set_pd( std::round(a.element(1)), std::round(a.element(0))));
	}
}


[[nodiscard("Value calculated and not used (fract)")]]
inline static Simd128Float64 fract(Simd128Float64 a) noexcept { return a - floor(a); }



//*****Fused Multiply Add Simd128s*****
// Fused Multiply Add (a*b+c)
[[nodiscard("Value calculated and not used (fma)")]]
inline static Simd128Float64 fma(const Simd128Float64  a, const Simd128Float64 b, const Simd128Float64 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fmadd_pd(a.v, b.v, c.v);  //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float64(_mm_set_pd(
			std::fma(a.element(1), b.element(1), c.element(1)),
			std::fma(a.element(0), b.element(0), c.element(0))
		));
	}
}

// Fused Multiply Subtract (a*b-c)
[[nodiscard("Value calculated and not used (fms)")]]
inline static Simd128Float64 fms(const Simd128Float64  a, const Simd128Float64 b, const Simd128Float64 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fmsub_pd(a.v, b.v, c.v);  //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float64(_mm_set_pd(
			std::fma(a.element(1), b.element(1), -c.element(1)),
			std::fma(a.element(0), b.element(0), -c.element(0))
		));
	}
}

// Fused Negative Multiply Add (-a*b+c)
[[nodiscard("Value calculated and not used (fnma)")]]
inline static Simd128Float64 fnma(const Simd128Float64  a, const Simd128Float64 b, const Simd128Float64 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fnmadd_pd(a.v, b.v, c.v);  //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float64(_mm_set_pd(
			std::fma(-a.element(1), b.element(1), c.element(1)),
			std::fma(-a.element(0), b.element(0), c.element(0))
		));
	}
}

// Fused Negative Multiply Subtract (-a*b-c)
[[nodiscard("Value calculated and not used (fnms)")]]
inline static Simd128Float64 fnms(const Simd128Float64  a, const Simd128Float64 b, const Simd128Float64 c) {
	if constexpr (mt::environment::compiler_has_fma) {
		return _mm_fnmsub_pd(a.v, b.v, c.v); //We are compiling to level 3, but using 128 simd.
	}
	else {
		return Simd128Float64(_mm_set_pd(
			std::fma(-a.element(1), b.element(1), -c.element(1)),
			std::fma(-a.element(0), b.element(0), -c.element(0))
		));
	}
}



//**********Min/Max*v*********
[[nodiscard("Value calculated and not used (min)")]]
inline static Simd128Float64 min(const Simd128Float64 a, const Simd128Float64 b)  noexcept { return Simd128Float64(_mm_min_pd(a.v, b.v)); } 

[[nodiscard("Value calculated and not used (max)")]]
inline static Simd128Float64 max(const Simd128Float64 a, const Simd128Float64 b)  noexcept { return Simd128Float64(_mm_max_pd(a.v, b.v)); } 

//Clamp a value between 0.0 and 1.0
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float64 clamp(const Simd128Float64 a) noexcept {
	const auto zero = _mm_setzero_pd();
	const auto one = _mm_set1_pd(1.0);
	return _mm_min_pd(_mm_max_pd(a.v, zero), one);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float64 clamp(const Simd128Float64 a, const Simd128Float64 min, const Simd128Float64 max) noexcept {
	return _mm_min_pd(_mm_max_pd(a.v, min.v), max.v);
}

//Clamp a value between min and max
[[nodiscard("Value calculated and not used (clamp)")]]
inline static Simd128Float64 clamp(const Simd128Float64 a, const float min_f, const float max_f) noexcept {
	const auto min = _mm_set1_pd(min_f);
	const auto max = _mm_set1_pd(max_f);
	return _mm_min_pd(_mm_max_pd(a.v, min), max);
}



//*****Approximate Functions*****
[[nodiscard("Value calculated and not used (reciprocal_approx)")]]
inline static Simd128Float64 reciprocal_approx(const Simd128Float64 a) noexcept {
	if constexpr (mt::environment::compiler_has_avx512f && mt::environment::compiler_has_avx512vl) {
		return Simd128Float64(_mm_rcp14_pd(a.v));
	}
	else {
		return Simd128Float64(1.0 / a.v);
	}
}

//*****128-bit Mathematical Functions*****
[[nodiscard("Value calculated and not used (sqrt)")]]
inline static Simd128Float64 sqrt(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_sqrt_pd(a.v)); } //sse2

[[nodiscard("Value Calculated and not used (abs)")]]
inline static Simd128Float64 abs(const Simd128Float64 a) noexcept {
	//No SSE for abs so we just flip the bit.
	const auto r = _mm_and_pd(_mm_set1_pd(std::bit_cast<double>(0x7FFFFFFFFFFFFFFFull)), a.v);
	return Simd128Float64(r);
}
//Calculating a raised to the power of b
[[nodiscard("Value calculated and not used (pow)")]]
inline static Simd128Float64 pow(Simd128Float64 a, Simd128Float64 b) noexcept { return Simd128Float64(_mm_pow_pd(a.v, b.v)); }

//Calculate e^x
[[nodiscard("Value calculated and not used (exp)")]]
inline static Simd128Float64 exp(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_exp_pd(a.v)); } //sse

//Calculate 2^x
[[nodiscard("Value calculated and not used (exp2)")]]
inline static Simd128Float64 exp2(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_exp2_pd(a.v)); } //sse

//Calculate 10^x
[[nodiscard("Value calculated and not used (exp10)")]]
inline static Simd128Float64 exp10(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_exp10_pd(a.v)); } //sse

//Calculate (e^x)-1.0
[[nodiscard("Value calculated and not used (exp_minus1)")]]
inline static Simd128Float64 expm1(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_expm1_pd(a.v)); } //sse

//Calulate natural log(x)
[[nodiscard("Value calculated and not used (log)")]]
inline static Simd128Float64 log(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_log_pd(a.v)); } //sse

//Calulate log(1.0 + x)
[[nodiscard("Value calculated and not used (log1p)")]]
inline static Simd128Float64 log1p(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_log1p_pd(a.v)); } //sse

//Calculate log_2(x)
[[nodiscard("Value calculated and not used (log2)")]]
inline static Simd128Float64 log2(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_log2_pd(a.v)); } //sse

//Calculate log_10(x)
[[nodiscard("Value calculated and not used (log10)")]]
inline static Simd128Float64 log10(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_log10_pd(a.v)); } //sse

//Calculate cube root
[[nodiscard("Value calculated and not used (cbrt)")]]
inline static Simd128Float64 cbrt(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_cbrt_pd(a.v)); } //sse


//Calculate hypot(x).  That is: sqrt(a^2 + b^2) while avoiding overflow.
[[nodiscard("Value calculated and not used (hypot)")]]
inline static Simd128Float64 hypot(const Simd128Float64 a, const Simd128Float64 b) noexcept { return Simd128Float64(_mm_hypot_pd(a.v, b.v)); } //sse


//*****Trigonometric Functions *****
[[nodiscard("Value Calculated and not used (sin)")]]
inline static Simd128Float64 sin(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_sin_pd(a.v)); }  //SSE

[[nodiscard("Value Calculated and not used (cos)")]]
inline static Simd128Float64 cos(const Simd128Float64 a)  noexcept { return Simd128Float64(_mm_cos_pd(a.v)); }

[[nodiscard("Value Calculated and not used (tan)")]]
inline static Simd128Float64 tan(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_tan_pd(a.v)); }

[[nodiscard("Value Calculated and not used (asin)")]]
inline static Simd128Float64 asin(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_asin_pd(a.v)); }

[[nodiscard("Value Calculated and not used (acos)")]]
inline static Simd128Float64 acos(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_acos_pd(a.v)); }

[[nodiscard("Value Calculated and not used (atan)")]]
inline static Simd128Float64 atan(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_atan_pd(a.v)); }

[[nodiscard("Value Calculated and not used (atan2)")]]
inline static Simd128Float64 atan2(const Simd128Float64 a, const Simd128Float64 b) noexcept { return Simd128Float64(_mm_atan2_pd(a.v, b.v)); }

[[nodiscard("Value Calculated and not used (sinh)")]]
inline static Simd128Float64 sinh(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_sinh_pd(a.v)); }

[[nodiscard("Value Calculated and not used (cosh)")]]
inline static Simd128Float64 cosh(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_cosh_pd(a.v)); }

[[nodiscard("Value Calculated and not used (tanh)")]]
inline static Simd128Float64 tanh(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_tanh_pd(a.v)); }

[[nodiscard("Value Calculated and not used (asinh)")]]
inline static Simd128Float64 asinh(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_asinh_pd(a.v)); }

[[nodiscard("Value Calculated and not used (acosh)")]]
inline static Simd128Float64 acosh(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_acosh_pd(a.v)); }

[[nodiscard("Value Calculated and not used (atanh)")]]
inline static Simd128Float64 atanh(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_atanh_pd(a.v)); } //SSE

//Calculate sin(x) where x is in degrees.
[[nodiscard("Value Calculated and not used (sind)")]]
inline static Simd128Float64 sind(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_sind_pd(a.v)); }  //SSE

//Calculate cos(x) where x is in degrees.
[[nodiscard("Value Calculated and not used (cosd)")]]
inline static Simd128Float64 cosd(const Simd128Float64 a)  noexcept { return Simd128Float64(_mm_cosd_pd(a.v)); }

//Calculate tan(x) where x is in degrees.
[[nodiscard("Value Calculated and not used (tand)")]]
inline static Simd128Float64 tand(const Simd128Float64 a) noexcept { return Simd128Float64(_mm_tand_pd(a.v)); }


//*****Conditional Functions *****

//Compare if 2 values are equal and return a mask.
inline static __m128d compare_equal(const Simd128Float64 a, const Simd128Float64 b) noexcept { return _mm_cmpeq_pd(a.v, b.v); }
inline static __m128d compare_less(const Simd128Float64 a, const Simd128Float64 b) noexcept { return _mm_cmplt_pd(a.v, b.v); }
inline static __m128d compare_less_equal(const Simd128Float64 a, const Simd128Float64 b) noexcept { return _mm_cmple_pd(a.v, b.v); }
inline static __m128d compare_greater(const Simd128Float64 a, const Simd128Float64 b) noexcept { return _mm_cmpgt_pd(a.v, b.v); }
inline static __m128d compare_greater_equal(const Simd128Float64 a, const Simd128Float64 b) noexcept { return _mm_cmpge_pd(a.v, b.v); }
inline static __m128d isnan(const Simd128Float64 a) noexcept { return _mm_cmpunord_pd(a.v, a.v); }

//Blend two values together based on mask.  First argument if zero. Second argument if 1.
//Note: the if_false argument is first!!
[[nodiscard("Value Calculated and not used (blend)")]]
inline static Simd128Float64 blend(const Simd128Float64 if_false, const Simd128Float64 if_true, __m128d mask) noexcept {
	if constexpr (mt::environment::compiler_has_sse4_1) {
		return Simd128Float64(_mm_blendv_pd(if_false.v, if_true.v, mask));
	}
	else {
		return Simd128Float64(_mm_or_pd(_mm_andnot_pd(mask, if_false.v), _mm_and_pd(mask, if_true.v)));
	}
}




#endif //x86_64


/**************************************************************************************************
 * Templated Functions for all types
 * ************************************************************************************************/

 //If values a and b are equal return if_true, otherwise return if_false.
template <SimdFloat64 T>
[[nodiscard("Value Calculated and not used (if_equal)")]]
inline static T if_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_equal(value_a, value_b));
}

template <SimdFloat64 T>
[[nodiscard("Value Calculated and not used (if_less)")]]
inline static T if_less(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less(value_a, value_b));
}

template <SimdFloat64 T>
[[nodiscard("Value Calculated and not used (if_less_equal)")]]
inline static T if_less_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_less_equal(value_a, value_b));
}

template <SimdFloat64 T>
[[nodiscard("Value Calculated and not used (if_greater)")]]
inline static T if_greater(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater(value_a, value_b));
}


template <SimdFloat64 T>
[[nodiscard("Value Calculated and not used (if_greater_equal)")]]
inline static T if_greater_equal(const T value_a, const T value_b, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, compare_greater_equal(value_a, value_b));
}


template <SimdFloat64 T>
[[nodiscard("Value Calculated and not used (if_nan)")]]
inline static T if_nan(const T value_a, const T if_true, const T if_false) noexcept {
	return blend(if_false, if_true, isnan(value_a));
}


/**************************************************************************************************
 * MASK OPS
 * ************************************************************************************************/
#if MT_SIMD_HAS_MSVC_VECTOR_MEMBERS
inline static __m128d operator&(__m128d  lhs, const __m128d rhs) noexcept { return _mm_and_pd(lhs, rhs); }
inline static __m128d operator|(__m128d  lhs, const __m128d rhs) noexcept { return _mm_or_pd(lhs, rhs); }
inline static __m128d operator^(__m128d  lhs, const __m128d rhs) noexcept { return _mm_xor_pd(lhs, rhs); }
inline static __m128d operator~(__m128d  lhs) noexcept { return _mm_xor_pd(lhs, _mm_cmpeq_pd(lhs, lhs)); }

inline static __m256d operator&(__m256d  lhs, const __m256d rhs) noexcept { return _mm256_and_pd(lhs, rhs); }
inline static __m256d operator|(__m256d  lhs, const __m256d rhs) noexcept { return _mm256_or_pd(lhs, rhs); }
inline static __m256d operator^(__m256d  lhs, const __m256d rhs) noexcept { return _mm256_xor_pd(lhs, rhs); }
inline static __m256d operator~(__m256d  lhs) noexcept { return _mm256_xor_pd(lhs, _mm256_xor_pd(lhs, _mm256_set1_pd(std::bit_cast<double>(0xFFFFFFFFFFFFFFFF)))); }
#endif


/**************************************************************************************************
 * Check that each type implements the desired types from simd-concepts.h
 * (Not sure why this fails on intelisense, but compliles ok.)
 * ************************************************************************************************/
static_assert(Simd<FallbackFloat64>, "FallbackFloat64 does not implement the concept SIMD");
static_assert(SimdReal<FallbackFloat64>, "FallbackFloat64 does not implement the concept SimdReal");
static_assert(SimdFloat<FallbackFloat64>, "FallbackFloat64 does not implement the concept SimdFloat");
static_assert(SimdFloat64<FallbackFloat64>, "FallbackFloat64 does not implement the concept SimdFloat64");
static_assert(SimdMath<FallbackFloat64>, "FallbackFloat64 does not implement the concept SimdFloat64");
static_assert(SimdCompareOps<FallbackFloat64>, "FallbackFloat64 does not implement the concept SimdCompareOps");

#if MT_SIMD_ARCH_X64
static_assert(Simd<Simd128Float64>, "Simd128Float64 does not implement the concept SIMD");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(Simd<Simd256Float64>, "Simd256Float64 does not implement the concept SIMD");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(Simd<Simd512Float64>, "Simd512Float64 does not implement the concept SIMD");
#endif

//Real
static_assert(SimdReal<Simd128Float64>, "Simd128Float64 does not implement the concept SimdReal");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdReal<Simd256Float64>, "Simd256Float64 does not implement the concept SimdReal");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdReal<Simd512Float64>, "Simd512Float64 does not implement the concept SimdReal");
#endif

//Float
static_assert(SimdFloat<Simd128Float64>, "Simd128Float64 does not implement the concept SimdFloat");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdFloat<Simd256Float64>, "Simd256Float64 does not implement the concept SimdFloat");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdFloat<Simd512Float64>, "Simd512Float64 does not implement the concept SimdFloat");
#endif

//Float64
static_assert(SimdFloat64<Simd128Float64>, "Simd128Float64 does not implement the concept SimdFloat64");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdFloat64<Simd256Float64>, "Simd256Float64 does not implement the concept SimdFloat64");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdFloat64<Simd512Float64>, "Simd512Float64 does not implement the concept SimdFloat64");
#endif

//Uint conversion support.
static_assert(SimdFloatToInt<Simd128Float64>, "Simd128Float64 does not implement the concept SimdFloatToInt");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdFloatToInt<Simd256Float64>, "Simd256Float64 does not implement the concept SimdFloatToInt");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdFloatToInt<Simd512Float64>, "Simd512Float64 does not implement the concept SimdFloatToInt");
#endif

//SIMD Math Support
static_assert(SimdMath<Simd128Float64>, "Simd128Float64 does not implement the concept SimdMath");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdMath<Simd256Float64>, "Simd256Float64 does not implement the concept SimdMath");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdMath<Simd512Float64>, "Simd512Float64 does not implement the concept SimdMath");
#endif

//Compare Ops
static_assert(SimdCompareOps<Simd128Float64>, "Simd128Float64 does not implement the concept SimdCompareOps");
#if MT_SIMD_ALLOW_LEVEL3_TYPES
static_assert(SimdCompareOps<Simd256Float64>, "Simd256Float64 does not implement the concept SimdCompareOps");
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
static_assert(SimdCompareOps<Simd512Float64>, "Simd512Float64 does not implement the concept SimdCompareOps");
#endif


#endif


/**************************************************************************************************
 Define SimdNativeFloat64 as the best supported type at compile time.
 (Based on microarchitecture level so that integers are also supported)
*************************************************************************************************/
#if MT_SIMD_ARCH_X64
#if MT_SIMD_ALLOW_LEVEL4_TYPES
	typedef Simd512Float64 SimdNativeFloat64;
	#else
#if MT_SIMD_ALLOW_LEVEL3_TYPES
	typedef Simd256Float64 SimdNativeFloat64;
	#else
	#if defined(__SSE4_1__) && defined(__SSE4_1__) && defined(__SSE3__) && defined(__SSSE3__) 
	typedef Simd128Float64 SimdNativeFloat64;
	#else
	typedef Simd128Float64 SimdNativeFloat64;
	#endif	
	#endif	
	#endif
#else
	//not x64
	typedef FallbackFloat64 SimdNativeFloat64;
#endif


