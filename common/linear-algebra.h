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

Description:

	A template driven Linear Algebra library.
	The template parameter should be a floating point type.
	The style of this library mimics the GLSL language

*********************************************************************************************************/
#pragma once

#include <concepts>
#include <cmath>
#include <type_traits>
#include "simd-concepts.h"
		

struct mat3;
struct mat4;



/**************************************************************************************************
 * 
 * ************************************************************************************************/
template <typename F>
constexpr inline F fract(F value){
	using std::floor; // Keep std overloads visible for scalar types; ADL can still select mt::floor for SIMD types.
	return value - floor(value); 	
}



/**************************************************************************************************
 * A 2-Vector (x,y)
 * ************************************************************************************************/
template <typename F>
struct vec2 {
	F x{};
	F y{};

	vec2() = default;
	vec2(F v) noexcept : x(v), y(v) {}
	vec2(F x1, F y1) noexcept : x(x1), y(y1) {}

	bool operator==(const vec2& rhs) const noexcept { return (x == rhs.x) && (y == rhs.y); }
	vec2 operator-() const noexcept { return vec2(-x, -y); }


	vec2& operator+=(const vec2& rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
	vec2& operator-=(const vec2& rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
	vec2& operator*=(const vec2& rhs) noexcept { x *= rhs.x; y *= rhs.y; return *this; }
	vec2& operator/=(const vec2& rhs) noexcept { x /= rhs.x; y /= rhs.y; return *this; }
	
	
	vec2& operator+=(const F rhs) noexcept { x += rhs; y += rhs; return *this; }
	vec2& operator-=(const F rhs) noexcept { x -= rhs; y -= rhs; return *this; }
	vec2& operator*=(const F rhs) noexcept { x *= rhs; y *= rhs; return *this; }
	vec2& operator/=(const F rhs) noexcept { x /= rhs; y /= rhs; return *this; }
	
	
	//Specilisations for SIMD Types
	//vec2& operator+=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x += F(rhs); y += F(rhs); return *this; }
	//vec2& operator-=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x -= F(rhs); y -= F(rhs); return *this; }
	//vec2& operator*=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x *= F(rhs); y *= F(rhs); return *this; }
	//vec2& operator/=(const float rhs) noexcept requires(mt::SimdFloat<F>) { x /= F(rhs); y /= F(rhs); return *this; }

	//vec2& operator+=(const vec2<float>& rhs) noexcept requires(mt::SimdFloat32<F>) { x += F(rhs.x); y += F(rhs.y); return *this; }
	
	//Calculate the magnitude (length) of the vector.
	[[nodiscard("Value Calculated and not used(magnitude)")]]
	inline F magnitude() const noexcept { 
		if constexpr (mt::SimdFloat<F>) {
			using std::fma;
			return sqrt(fma(x, x, y * y));
		}else {
			return sqrt(x * x + y * y);
		};
	}
	
	//Calculate the length of the vector (Note: This is an alias for magnitude()).
	[[nodiscard("Value Calculated and not used (length)")]]
	inline F length() const noexcept { return this->magnitude(); }
	
	[[nodiscard("Value Calculated and not used (normalize).  Note: This value is not calulated in place")]]
	inline vec2<F> normalize() const noexcept { const F m = magnitude(); return vec2(x / m, y / m); }

};



template <typename F> inline vec2<F> operator+(vec2<F> lhs, const vec2<F> & rhs) noexcept { lhs += rhs;	return lhs; }
template <typename F> inline vec2<F> operator-(vec2<F> lhs, const vec2<F> & rhs) noexcept { lhs -= rhs;	return lhs; }
template <typename F> inline vec2<F> operator*(vec2<F> lhs, const vec2<F> & rhs) noexcept { lhs *= rhs;	return lhs; }
template <typename F> inline vec2<F> operator/(vec2<F> lhs, const vec2<F> & rhs) noexcept { lhs /= rhs;	return lhs; }
template <typename F, typename F2> inline vec2<F> operator+(vec2<F> lhs, F2 rhs) noexcept { lhs += static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec2<F> operator-(vec2<F> lhs, F2 rhs) noexcept { lhs -= static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec2<F> operator*(vec2<F> lhs, F2 rhs) noexcept { lhs *= static_cast<F>(rhs);	return lhs; }

template <typename F, typename F2> inline vec2<F> operator/(vec2<F> lhs, F2 rhs) noexcept { lhs /= static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec2<F> operator+(F2 lhs, vec2<F> rhs) noexcept {return rhs + static_cast<F>(lhs); }
template <typename F, typename F2> inline vec2<F> operator-(F2 lhs, vec2<F> rhs) noexcept { return -rhs + static_cast<F>(lhs); }
template <typename F, typename F2> inline vec2<F> operator*(F2 lhs, vec2<F> rhs) noexcept { return rhs * static_cast<F>(lhs); }

//Dot Product of two vectors (a.x * b.x + a.y * b.y)
template <typename F> 
[[nodiscard("Value Calculated and not used (dot)")]]
inline static F dot(const vec2<F>& a, const vec2<F>& b) noexcept { 
	if constexpr (mt::SimdFloat<F>) {
		using std::fma;
		return fma(a.x, b.x, a.y * b.y);
	}else{
		return a.x * b.x + a.y * b.y;
	}
}


template <typename F> 
[[nodiscard("Value Calculated and not used (normalize).  Note: This value is not calulated in place")]]
inline vec2<F> normalize(const vec2<F>& v) noexcept { return v.normalize(); }

template <typename F> inline F distance(const vec2<F>& a, const vec2<F> & b) {
	using std::sqrt;
	return sqrt((b.x-a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y));
}

template <typename F> inline vec2<F> floor(const vec2<F>& a) {
	using std::floor;
	return vec2(floor(a.x), floor(a.y));
}
template <typename F> inline vec2<F> fract(const vec2<F>& a) { return vec2(fract(a.x), fract(a.y)); }
template <typename F> inline vec2<F> trunc(const vec2<F>& a) {
	using std::trunc;
	return vec2(trunc(a.x), trunc(a.y));
}
template <typename F> inline vec2<F> abs(const vec2<F>& a) {
	using std::abs;
	return vec2(abs(a.x), abs(a.y));
}
template <typename F> inline vec2<F> sqrt(const vec2<F>& a) {
	using std::sqrt;
	return vec2(sqrt(a.x), sqrt(a.y));
}
template <typename F> inline F length(const vec2<F>& a) { return a.magnitude(); }
template <typename F> inline F magnitude(const vec2<F>& a) { return a.magnitude(); }





/**************************************************************************************************
 * A 3-Vector (x,y,z)
 * ************************************************************************************************/
template <typename F>
struct vec3 {
	F x{};
	F y{};
	F z{};

	vec3() = default;
	vec3(F v) noexcept : x(v), y(v), z(v) {}
	vec3(F x1, F y1, F z1) noexcept : x(x1), y(y1),z(z1) {}
	
	vec3(const vec2<F>& v, F z1) noexcept : x(v.x), y(v.y), z(z1) {}
	vec3(F x1, const vec2<F>& v) noexcept : x(x1), y(v.x), z(v.y) {}

	bool operator==(const vec3<F>& rhs) const noexcept { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }

	vec3<F> operator-() const noexcept { return vec3<F>(-x, -y, -z); }
	vec3<F>& operator+=(const vec3<F>& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	vec3<F>& operator-=(const vec3<F>& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	vec3<F>& operator*=(const vec3<F>& rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	vec3<F>& operator/=(const vec3<F>& rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

	vec3<F>& operator+=(const F rhs) noexcept { x += rhs; y += rhs; z += rhs; return *this; }
	vec3<F>& operator-=(const F rhs) noexcept { x -= rhs; y -= rhs; z -= rhs; return *this; }
	vec3<F>& operator*=(const F rhs) noexcept { x *= rhs; y *= rhs; z *= rhs; return *this; }
	vec3<F>& operator/=(const F rhs) noexcept { x /= rhs; y /= rhs; z /= rhs; return *this; }

	inline F magnitude() const noexcept {
		using std::sqrt;
		return sqrt(x * x + y * y + z * z);
	}
	inline F length() const noexcept { return this->magnitude(); }
	
	[[nodiscard("Value Calculated and not used (normalize).  Note: This value is not calulated in place")]]
	inline vec3<F> normalize() const noexcept { const F m = magnitude(); return vec3(x / m, y / m, z / m); }




	inline vec2<F> xy() const noexcept { return vec2<F>(x, y); }
	inline vec2<F> yz() const noexcept { return vec2<F>(y, z); }
	inline vec2<F> xz() const noexcept { return vec2<F>(x, z); }
};
template <typename F> inline vec3<F> operator+(vec3<F> lhs, const vec3<F>& rhs) noexcept { lhs += rhs;	return lhs; }
template <typename F> inline vec3<F> operator-(vec3<F> lhs, const vec3<F>& rhs) noexcept { lhs -= rhs;	return lhs; }
template <typename F> inline vec3<F> operator*(vec3<F> lhs, const vec3<F>& rhs) noexcept { lhs *= rhs;	return lhs; }
template <typename F> inline vec3<F> operator/(vec3<F> lhs, const vec3<F>& rhs) noexcept { lhs /= rhs;	return lhs; }
template <typename F, typename F2> inline vec3<F> operator+(vec3<F> lhs, F2 rhs) noexcept { lhs += static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec3<F> operator-(vec3<F> lhs, F2 rhs) noexcept { lhs -= static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec3<F> operator*(vec3<F> lhs, F2 rhs) noexcept { lhs *= static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec3<F> operator/(vec3<F> lhs, F2 rhs) noexcept { lhs /= static_cast<F>(rhs);	return lhs; }
template <typename F> inline vec3<F> operator+(F lhs, vec3<F> rhs) noexcept { return rhs + lhs; }
template <typename F> inline vec3<F> operator-(F lhs, vec3<F> rhs) noexcept { return -rhs + lhs; }
template <typename F> inline vec3<F> operator*(F lhs, vec3<F> rhs) noexcept { return rhs * lhs; }


template <typename F> 
[[nodiscard("Value Calculated and not used (normalize).  Note: This value is not calulated in place")]]
inline vec3<F> normalize(const vec3<F>& v) noexcept {return v.normalize();}

template <typename F> inline static F dot(const vec3<F>& a, const vec3<F>& b) noexcept {return a.x * b.x + a.y * b.y + a.z * b.z;}
template <typename F> inline static vec3<F> cross(const vec3<F>& a, const vec3<F>& b) noexcept {return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);}
template <typename F> inline vec3<F> floor(const vec3<F>& a) {
	using std::floor;
	return vec3(floor(a.x), floor(a.y), floor(a.z));
}
template <typename F> inline vec3<F> fract(const vec3<F>& a) { return vec3(fract(a.x), fract(a.y), fract(a.z)); }
template <typename F> inline vec3<F> trunc(const vec3<F>& a) {
	using std::trunc;
	return vec3(trunc(a.x), trunc(a.y), trunc(a.z));
}
template <typename F> inline F length(const vec3<F>& a) { return a.magnitude(); }



/**************************************************************************************************
 * A 4-Vector (x,y,z,w)
 * ************************************************************************************************/
template <typename F>
struct vec4{
	F x{};
	F y{};
	F z{};
	F w{};

	vec4<F>() = default;
	vec4<F>(F v) noexcept : x(v), y(v), z(v), w(v) {}
	vec4<F>(F x1, F y1, F z1, F w1) noexcept : x(x1), y(y1), z(z1), w(w1) {}
	vec4<F>(const vec3<F>& xyz, F w1) noexcept : x(xyz.x), y(xyz.y), z(xyz.z),w(w1) {}
	vec4<F>(F x1, const vec3<F>& yzw) noexcept : x(x1), y(yzw.x), z(yzw.y), w(yzw.z) {}
	vec4<F>(const vec2<F>& xy, const vec2<F>& zw) noexcept : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
	vec4<F>(const vec2<F>& xy, F z1, F w1) noexcept : x(xy.x), y(xy.y), z(z1), w(w1) {}
	vec4<F>(F x1, F y1, const vec2<F>& zw) noexcept : x(x1), y(y1), z(zw.x), w(zw.y) {}

	bool operator==(const vec4<F>& rhs) const noexcept { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w==rhs.w); }

	vec4<F> operator-() const noexcept { return vec4<F>(-x, -y, -z, -w); }
	vec4<F>& operator+=(const vec4<F>& rhs) noexcept { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	vec4<F>& operator-=(const vec4<F>& rhs) noexcept { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	vec4<F>& operator*=(const vec4<F>& rhs) noexcept { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	vec4<F>& operator/=(const vec4<F>& rhs) noexcept { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

	vec4<F>& operator+=(const F rhs) noexcept { x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
	vec4<F>& operator-=(const F rhs) noexcept { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
	vec4<F>& operator*=(const F rhs) noexcept { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	vec4<F>& operator/=(const F rhs) noexcept { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	inline vec2<F> xy() const noexcept { return vec2<F>(x, y); }
	inline vec2<F> yz() const noexcept { return vec2<F>(y, z); }
	inline vec2<F> xz() const noexcept { return vec2<F>(x, z); }
	inline vec2<F> zw() const noexcept { return vec2<F>(z, w); }
	inline vec3<F> xyz() const noexcept { return vec3<F>(x, y, z); }
	inline vec3<F> yzw() const noexcept { return vec3<F>(y, z, w); }

	inline F magnitude() const noexcept {
		using std::sqrt;
		return sqrt(x * x + y*y + z*z+ w*w);
	}
	inline F length() const noexcept {
		using std::sqrt;
		return sqrt(x * x + y * y + z * z + w * w);
	}
	inline void normalize() noexcept { const F m = magnitude(); x /= m; y /= m; z /= m; w /= m; }
};
template <typename F> inline vec4<F> operator+(vec4<F> lhs, const vec4<F>& rhs) noexcept { lhs += rhs;	return lhs; }
template <typename F> inline vec4<F> operator-(vec4<F> lhs, const vec4<F>& rhs) noexcept { lhs -= rhs;	return lhs; }
template <typename F> inline vec4<F> operator*(vec4<F> lhs, const vec4<F>& rhs) noexcept { lhs *= rhs;	return lhs; }
template <typename F> inline vec4<F> operator/(vec4<F> lhs, const vec4<F>& rhs) noexcept { lhs /= rhs;	return lhs; }
template <typename F, typename F2> inline vec4<F> operator+(vec4<F> lhs, F2 rhs) noexcept { lhs += static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec4<F> operator-(vec4<F> lhs, F2 rhs) noexcept { lhs -= static_cast<F>(rhs);	return lhs; }
template <typename F, typename F2> inline vec4<F> operator*(vec4<F> lhs, F2 rhs) noexcept { 
	#pragma warning( suppress : 4244 )
	lhs *= static_cast<F>(rhs);	
	return lhs; 
}
template <typename F, typename F2> inline vec4<F> operator/(vec4<F> lhs, F2 rhs) noexcept { lhs /= static_cast<F>(rhs);	return lhs; }
template <typename F> inline vec4<F> operator+(F lhs, vec4<F> rhs) noexcept { return rhs + lhs; }
template <typename F> inline vec4<F> operator-(F lhs, vec4<F> rhs) noexcept { return -rhs + lhs; }
template <typename F> inline vec4<F> operator*(F lhs, vec4<F> rhs) noexcept { return rhs * lhs; }

template <typename F> inline static F dot(const vec4<F>& a, const vec4<F>& b) noexcept {return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;}
template <typename F> inline vec4<F> normalize(vec4<F> v) noexcept { v.normalize(); return v; }
template <typename F> inline vec4<F> floor(const vec4<F>& a) {
	using std::floor;
	return vec4(floor(a.x), floor(a.y), floor(a.z), floor(a.w));
}
template <typename F> inline vec4<F> fract(const vec4<F>& a) { return vec4(fract(a.x), fract(a.y), fract(a.z), fract(a.w)); }
template <typename F> inline vec4<F> trunc(const vec4<F>& a) {
	using std::trunc;
	return vec4(trunc(a.x), trunc(a.y), trunc(a.z), trunc(a.w));
}
template <typename F> inline F length(const vec4<F>& a) { return a.magnitude(); }



/*
//3x3 matrix.  Access elements using (row, col) operator
struct mat3 {
private:
	double n[3][3]{};
public:
	mat3() = default;
	mat3(double n00, double n01, double n02, double n10, double n11, double n12, double n20, double n21, double n22) noexcept {
		n[0][0] = n00;  n[0][1] = n10;  n[0][2] = n20;
		n[1][0] = n01;  n[1][1] = n11;  n[1][2] = n21;
		n[2][0] = n02;  n[2][1] = n12;  n[2][2] = n22;
	}
	//Create with 3 column vectors
	mat3(const vec3& a, const vec3& b, const vec3& c) noexcept {
		n[0][0] = a.x;  n[0][1] = a.y;  n[0][2] = a.z;
		n[1][0] = b.x;  n[1][1] = b.y;  n[1][2] = b.z;
		n[2][0] = c.x;  n[2][1] = c.y;  n[2][2] = c.z;
	}

	bool operator==(const mat3& rhs) const noexcept {  
		return
			n[0][0] == rhs.n[0][0] && n[0][1] == rhs.n[0][1] && n[0][2] == rhs.n[0][2] &&
			n[1][0] == rhs.n[1][0] && n[1][1] == rhs.n[1][1] && n[1][2] == rhs.n[1][2] &&
			n[2][0] == rhs.n[2][0] && n[2][1] == rhs.n[2][1] && n[2][2] == rhs.n[2][2];
	}

	//Access element by row & column
	inline double& operator()(int row, int col) noexcept { return (n[col][row]); }
	inline const double& operator()  (int row, int col) const noexcept { return (n[col][row]); }
	inline vec3& column(int col) noexcept { return *reinterpret_cast<vec3*>(n[col]); }

	mat3 operator-() const noexcept {
		const mat3& m = (*this);
		return mat3(
			-m(0, 0), -m(0, 1), -m(0, 2),
			-m(1, 0), -m(1, 1), -m(1, 2),
			-m(2, 0), -m(2, 1), -m(2, 2)
		);
		
		
	}
	   
	mat3& operator+=(const mat3& rhs) noexcept {
		n[0][0] += rhs.n[0][0];  n[0][1] += rhs.n[0][1];  n[0][2] += rhs.n[0][2];
		n[1][0] += rhs.n[1][0];  n[1][1] += rhs.n[1][1];  n[1][2] += rhs.n[1][2];
		n[2][0] += rhs.n[2][0];  n[2][1] += rhs.n[2][1];  n[2][2] += rhs.n[2][2];
		return *this; 
	}

	mat3& operator-=(const mat3& rhs) noexcept {
		n[0][0] -= rhs.n[0][0];  n[0][1] -= rhs.n[0][1];  n[0][2] -= rhs.n[0][2];
		n[1][0] -= rhs.n[1][0];  n[1][1] -= rhs.n[1][1];  n[1][2] -= rhs.n[1][2];
		n[2][0] -= rhs.n[2][0];  n[2][1] -= rhs.n[2][1];  n[2][2] -= rhs.n[2][2];
		return *this;
	}

	mat3& operator+=(const double rhs) noexcept {
		n[0][0] += rhs;  n[0][1] += rhs;  n[0][2] += rhs;
		n[1][0] += rhs;  n[1][1] += rhs;  n[1][2] += rhs;
		n[2][0] += rhs;  n[2][1] += rhs;  n[2][2] += rhs;
		return *this;
	}
	mat3& operator-=(const double rhs) noexcept {
		n[0][0] -= rhs;  n[0][1] -= rhs;  n[0][2] -= rhs;
		n[1][0] -= rhs;  n[1][1] -= rhs;  n[1][2] -= rhs;
		n[2][0] -= rhs;  n[2][1] -= rhs;  n[2][2] -= rhs;
		return *this;
	}
	mat3& operator*=(const double rhs) noexcept {
		n[0][0] *= rhs;  n[0][1] *= rhs;  n[0][2] *= rhs;
		n[1][0] *= rhs;  n[1][1] *= rhs;  n[1][2] *= rhs;
		n[2][0] *= rhs;  n[2][1] *= rhs;  n[2][2] *= rhs;
		return *this;
	}
	mat3& operator/=(const double rhs) noexcept {
		n[0][0] /= rhs;  n[0][1] /= rhs;  n[0][2] /= rhs;
		n[1][0] /= rhs;  n[1][1] /= rhs;  n[1][2] /= rhs;
		n[2][0] /= rhs;  n[2][1] /= rhs;  n[2][2] /= rhs;
		return *this;
	}

};
inline mat3 operator+(mat3 lhs, const mat3& rhs) noexcept { lhs += rhs;	return lhs; }
inline mat3 operator-(mat3 lhs, const mat3& rhs) noexcept { lhs -= rhs;	return lhs; }


inline mat3 operator+(mat3 lhs, double rhs) noexcept { lhs += rhs;	return lhs; }
inline mat3 operator-(mat3 lhs, double rhs) noexcept { lhs -= rhs;	return lhs; }
inline mat3 operator*(mat3 lhs, double rhs) noexcept { lhs *= rhs;	return lhs; }
inline mat3 operator/(mat3 lhs, double rhs) noexcept { lhs /= rhs;	return lhs; }
inline mat3 operator+(double lhs, mat3 rhs) noexcept { return rhs + lhs; }
inline mat3 operator-(double lhs, mat3 rhs) noexcept { return -rhs + lhs; }
inline mat3 operator*(double lhs, mat3 rhs) noexcept { return rhs * lhs; }

inline mat3 operator* (const mat3& lhs, const mat3& rhs) noexcept {
	return mat3(
		lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0),
		lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1),
		lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2),
		lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0),
		lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1),
		lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2),
		lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0),
		lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1),
		lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2)
	);
}


inline vec3 operator*(const mat3& m, const vec3& v) noexcept {
	return vec3(
		m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z,
		m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z,
		m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z
	);
}


//4x4 matrix.  Access elements using (row, col) operator
struct mat4 {
private:
	double n[4][4]{};
public:
	mat4() = default;
	mat4(double n00, double n01, double n02, double n03, double n10, double n11, double n12, double n13, double n20, double n21, double n22, double n23, double n30, double n31, double n32, double n33) noexcept {
		n[0][0] = n00;  n[0][1] = n10;  n[0][2] = n20; n[0][3] = n30;
		n[1][0] = n01;  n[1][1] = n11;  n[1][2] = n21; n[1][3] = n31;
		n[2][0] = n02;  n[2][1] = n12;  n[2][2] = n22; n[2][3] = n32;
		n[3][0] = n03;  n[3][1] = n13;  n[3][2] = n23; n[3][3] = n33;
	}
	//Create with 4 column vectors
	mat4(const vec4<F>& a, const vec4<F>& b, const vec4<F>& c, const vec4<F>& d) noexcept {
		n[0][0] = a.x;  n[0][1] = a.y;  n[0][2] = a.z;  n[0][3] = a.w;
		n[1][0] = b.x;  n[1][1] = b.y;  n[1][2] = b.z;  n[1][3] = b.w;
		n[2][0] = c.x;  n[2][1] = c.y;  n[2][2] = c.z;  n[2][3] = c.w;
		n[3][0] = d.x;  n[3][1] = d.y;  n[3][2] = d.z;  n[3][3] = d.w;
	}

	//Access element by row & column
	inline double& operator()(int row, int col) noexcept { return (n[col][row]); }
	inline const double& operator()  (int row, int col) const noexcept { return (n[col][row]); }
	inline vec4<F>& column(int col) noexcept { return *reinterpret_cast<vec4<F>*>(n[col]); }

	bool operator==(const mat4& rhs) const noexcept {
		return
			n[0][0] == rhs.n[0][0] && n[0][1] == rhs.n[0][1] && n[0][2] == rhs.n[0][2] && n[0][3] == rhs.n[0][3] &&
			n[1][0] == rhs.n[1][0] && n[1][1] == rhs.n[1][1] && n[1][2] == rhs.n[1][2] && n[1][3] == rhs.n[1][3] &&
			n[2][0] == rhs.n[2][0] && n[2][1] == rhs.n[2][1] && n[2][2] == rhs.n[2][2] && n[2][3] == rhs.n[2][3] &&
			n[3][0] == rhs.n[3][0] && n[3][1] == rhs.n[3][1] && n[3][2] == rhs.n[3][2] && n[3][3] == rhs.n[3][3];
	}

	mat4 operator-() const noexcept {
		const mat4& m = (*this);
		return mat4(
			-m(0, 0), -m(0, 1), -m(0, 2), -m(0, 3),
			-m(1, 0), -m(1, 1), -m(1, 2), -m(1, 3),
			-m(2, 0), -m(2, 1), -m(2, 2), -m(2, 3),
			-m(3, 0), -m(3, 1), -m(3, 2), -m(3, 3)
		);

	}

	mat4& operator+=(const mat4& rhs) noexcept {
		n[0][0] += rhs.n[0][0];  n[0][1] += rhs.n[0][1];  n[0][2] += rhs.n[0][2]; n[0][3] += rhs.n[0][3];
		n[1][0] += rhs.n[1][0];  n[1][1] += rhs.n[1][1];  n[1][2] += rhs.n[1][2]; n[1][3] += rhs.n[1][3];
		n[2][0] += rhs.n[2][0];  n[2][1] += rhs.n[2][1];  n[2][2] += rhs.n[2][2]; n[2][3] += rhs.n[2][3];
		n[3][0] += rhs.n[3][0];  n[3][1] += rhs.n[3][1];  n[3][2] += rhs.n[3][2]; n[3][3] += rhs.n[3][3];
		return *this;
	}

	mat4& operator-=(const mat4& rhs) noexcept {
		n[0][0] -= rhs.n[0][0];  n[0][1] -= rhs.n[0][1];  n[0][2] -= rhs.n[0][2]; n[0][3] -= rhs.n[0][3];
		n[1][0] -= rhs.n[1][0];  n[1][1] -= rhs.n[1][1];  n[1][2] -= rhs.n[1][2]; n[1][3] -= rhs.n[1][3];
		n[2][0] -= rhs.n[2][0];  n[2][1] -= rhs.n[2][1];  n[2][2] -= rhs.n[2][2]; n[2][3] -= rhs.n[2][3];
		n[3][0] -= rhs.n[3][0];  n[3][1] -= rhs.n[3][1];  n[3][2] -= rhs.n[3][2]; n[3][3] -= rhs.n[3][3];
		return *this;
	}

	mat4& operator+=(const double rhs) noexcept {
		n[0][0] += rhs;  n[0][1] += rhs;  n[0][2] += rhs; n[0][3] += rhs;
		n[1][0] += rhs;  n[1][1] += rhs;  n[1][2] += rhs; n[1][3] += rhs;
		n[2][0] += rhs;  n[2][1] += rhs;  n[2][2] += rhs; n[2][3] += rhs;
		n[3][0] += rhs;  n[3][1] += rhs;  n[3][2] += rhs; n[3][3] += rhs;
		return *this;
	}
	mat4& operator-=(const double rhs) noexcept {
		n[0][0] -= rhs;  n[0][1] -= rhs;  n[0][2] -= rhs; n[0][3] -= rhs;
		n[1][0] -= rhs;  n[1][1] -= rhs;  n[1][2] -= rhs; n[1][3] -= rhs;
		n[2][0] -= rhs;  n[2][1] -= rhs;  n[2][2] -= rhs; n[2][3] -= rhs;
		n[3][0] -= rhs;  n[3][1] -= rhs;  n[3][2] -= rhs; n[3][3] -= rhs;
		return *this;
	}
	mat4& operator*=(const double rhs) noexcept {
		n[0][0] *= rhs;  n[0][1] *= rhs;  n[0][2] *= rhs; n[0][3] *= rhs;
		n[1][0] *= rhs;  n[1][1] *= rhs;  n[1][2] *= rhs; n[1][3] *= rhs;
		n[2][0] *= rhs;  n[2][1] *= rhs;  n[2][2] *= rhs; n[2][3] *= rhs;
		n[3][0] *= rhs;  n[3][1] *= rhs;  n[3][2] *= rhs; n[3][3] *= rhs;
		return *this;
	}
	mat4& operator/=(const double rhs) noexcept {
		n[0][0] /= rhs;  n[0][1] /= rhs;  n[0][2] /= rhs; n[0][3] /= rhs;
		n[1][0] /= rhs;  n[1][1] /= rhs;  n[1][2] /= rhs; n[1][3] /= rhs;
		n[2][0] /= rhs;  n[2][1] /= rhs;  n[2][2] /= rhs; n[2][3] /= rhs;
		n[3][0] /= rhs;  n[3][1] /= rhs;  n[3][2] /= rhs; n[3][3] /= rhs;
		return *this;
	}


};
inline mat4 operator+(mat4 lhs, const mat4& rhs) noexcept { lhs += rhs;	return lhs; }
inline mat4 operator-(mat4 lhs, const mat4& rhs) noexcept { lhs -= rhs;	return lhs; }
inline mat4 operator* (const mat4& lhs, const mat4& rhs) noexcept {
	return mat4(
		lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0) + lhs(0, 3) * rhs(3, 0),
		lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1) + lhs(0, 3) * rhs(3, 1),
		lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2) + lhs(0, 3) * rhs(3, 2),
		lhs(0, 0) * rhs(0, 3) + lhs(0, 1) * rhs(1, 3) + lhs(0, 2) * rhs(2, 3) + lhs(0, 3) * rhs(3, 3),

		lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0) + lhs(1, 3) * rhs(3, 0),
		lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1) + lhs(1, 3) * rhs(3, 1),
		lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2) + lhs(1, 3) * rhs(3, 2),
		lhs(1, 0) * rhs(0, 3) + lhs(1, 1) * rhs(1, 3) + lhs(1, 2) * rhs(2, 3) + lhs(1, 3) * rhs(3, 3),

		lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0) + lhs(2, 3) * rhs(3, 0),
		lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1) + lhs(2, 3) * rhs(3, 1),
		lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2) + lhs(2, 3) * rhs(3, 2),
		lhs(2, 0) * rhs(0, 3) + lhs(2, 1) * rhs(1, 3) + lhs(2, 2) * rhs(2, 3) + lhs(2, 3) * rhs(3, 3),

		lhs(3, 0) * rhs(0, 0) + lhs(3, 1) * rhs(1, 0) + lhs(3, 2) * rhs(2, 0) + lhs(3, 3) * rhs(3, 0),
		lhs(3, 0) * rhs(0, 1) + lhs(3, 1) * rhs(1, 1) + lhs(3, 2) * rhs(2, 1) + lhs(3, 3) * rhs(3, 1),
		lhs(3, 0) * rhs(0, 2) + lhs(3, 1) * rhs(1, 2) + lhs(3, 2) * rhs(2, 2) + lhs(3, 3) * rhs(3, 2),
		lhs(3, 0) * rhs(0, 3) + lhs(3, 1) * rhs(1, 3) + lhs(3, 2) * rhs(2, 3) + lhs(3, 3) * rhs(3, 3)
	);
}

inline mat4 operator+(mat4 lhs, double rhs) noexcept { lhs += rhs;	return lhs; }
inline mat4 operator-(mat4 lhs, double rhs) noexcept { lhs -= rhs;	return lhs; }
inline mat4 operator*(mat4 lhs, double rhs) noexcept { lhs *= rhs;	return lhs; }
inline mat4 operator/(mat4 lhs, double rhs) noexcept { lhs /= rhs;	return lhs; }
inline mat4 operator+(double lhs, mat4 rhs) noexcept { return rhs + lhs; }
inline mat4 operator-(double lhs, mat4 rhs) noexcept { return -rhs + lhs; }
inline mat4 operator*(double lhs, mat4 rhs) noexcept { return rhs * lhs; }

//Multiply a 4x4 matrix. Access elements using (row, col) operator
inline vec4<F> operator * (const mat4& m, const vec4<F>& v) noexcept {
	return vec4<F>(
		m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z + m(0, 3) * v.w,
		m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z + m(1, 3) * v.w,
		m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z + m(2, 3) * v.w,
		m(3, 0) * v.x + m(3, 1) * v.y + m(3, 2) * v.z + m(3, 3) * v.w
	);
}

//Multiply mat4 by vec3. An
//An optimised version assuming w entry is 0, discards w in return value.
inline vec3 operator * (const mat4& m, const vec3& v) noexcept {
	return vec3(
		m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z,
		m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z,
		m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z
	);
}
*/

/**************************************************************************************************
 * Reflect function.  Normal must be normalised.
 * Type should be vec2, vec3, vec4 
 * https://registry.khronos.org/OpenGL-Refpages/gl4/html/reflect.xhtml
 * ************************************************************************************************/
template <class T>
inline static T reflect(const T & incident, const T & normal) noexcept {
	return incident - normal * 2.0f * dot(normal, incident);
}

/**************************************************************************************************
 * Refract function.  Incident and Normal must be normalised.
 * T Type should be vec2, vec3, vec4.
 * F should be a floating point type. 
 * https://registry.khronos.org/OpenGL-Refpages/gl4/html/refract.xhtml
 * ************************************************************************************************/
template <class T, typename F>
inline static T refract(const T & incident, const T & normal, F eta) noexcept {
	using std::sqrt;
	const auto n_dot_i = dot(incident, normal); 
    const auto k = 1.0f - eta * eta (1.0f - n_dot_i * n_dot_i);
    if (k<0.0f) return T{};
    return eta* incident - (eta * n_dot_i + sqrt(k)) * normal;
    
}

/**************************************************************************************************
 * Rescales a value in the range 0..1 to new range (no clamping)
 * ************************************************************************************************/
template <typename F>
constexpr inline F rescale_from_01(F value, F newMin, F newMax) noexcept {
	return value * (newMax - newMin) + newMin;
}

/**************************************************************************************************
 * Rescale a value to the range 0..1 (no clamping)
 * ************************************************************************************************/
template <typename F>
constexpr inline F rescale_to_01(F value, F oldMin, F oldMax) noexcept {
	return (value - oldMin) / (oldMax - oldMin);
}

/**************************************************************************************************
 * Rescale a value (no clamping)
 * ************************************************************************************************/
template <typename F>
constexpr inline F rescale(F value, F oldMin, F oldMax, F newMin, F newMax) noexcept {
	return rescale_to_01(value, oldMin, oldMax) * (newMax - newMin) + newMin;

}

/**************************************************************************************************
 * Clamp a value
 * ************************************************************************************************/
template <typename F>
constexpr inline F clamp(F value, F min, F max) requires(! mt::Simd<F>) {
	return (value < min) ? min : ((value > max) ? max : value);
}

/**************************************************************************************************
 * Clamp a value to the range 0..1
 * ************************************************************************************************/
template <mt::SimdFloat S>
constexpr inline S clamp_01(S value) requires(mt::Simd<S>) {
	//return mt::if_greater(value, S{ 1.0 }, S{ 1.0 }, mt::if_less(value, S{}, S{}, value));
	return max(0.0f, min(1.0f, value));

}

constexpr inline float clamp_01(float value) {
	return (value < 0.0f) ? 0.0f : ((value > 1.0f) ? 1.0f : value);
}

template <typename F>
constexpr inline F clamp_01(F value) requires(!mt::Simd<F>) {
	return (value < 0.0) ? 0.0 : ((value > 1.0) ? 1.0 : value);
}

/**************************************************************************************************
 * Convert to int and clamp.
 * ************************************************************************************************/
template <typename F>
inline int clamp_to_int(F value, int min, int max) noexcept {
	using std::round;
	const int v = static_cast<int>(round(value));
	return (v < min) ? min : ((v > max) ? max : v);
}


/**************************************************************************************************
 * 
 * ************************************************************************************************/
template <typename F>
constexpr inline F smoothstep(F edge0, F edge1, F value) {
	const auto t = clamp_01((value - edge0) / (edge1 - edge0));
	return t * t * (3.0 - 2.0 * t);
}
template <typename F>
constexpr inline F smoothstep(float edge0f, float edge1f, F value) {
	F edge0{ edge0f };
	F edge1{ edge1f };

	const auto t = clamp_01((value - edge0) / (edge1 - edge0));
	return t * t * (3.0 - 2.0 * t);
}



/**************************************************************************************************
 * 
 * ************************************************************************************************/
template <typename F>
constexpr inline F step(F edge, F value) {
	return (value < edge) ? 0.0 : 1.0;
}


/**************************************************************************************************
 * 
 * ************************************************************************************************/
template <class T, typename F>
constexpr inline T mix(T v1, T v2, F weight) {
	return (v2 * weight) + ((1.0f-weight)* v1);
	
}

