/********************************************************************************************************

Authors:        (c) 2026 Maths Town

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
********************************************************************************************************

Type:           Mat3
                (aliased to mat3, for easy glsl porting)

Description:    A column-major 3x3 matrix that supports floating point types and types implementing the
                concepts from the Maths Town SIMD library.

Operators:      == - + - * / += -= *= /= with matrix and scalar forms

Member Functions:
                Access:               operator(row,col) column row
                Matrix:               determinant transpose inverse

Free Functions:
                Operators:            + - * /
                Matrix:               determinant transpose inverse

*********************************************************************************************************/
#pragma once

#include "vec3.h"

namespace mt {

/**************************************************************************************************
 * A 3x3 column-major matrix.
 * Constructor elements and operator() use logical row, column order.
 * ************************************************************************************************/
template <typename F> struct Mat3 {
	using ComponentType = F;
	using column_type = Vec3<F>;

private:
	Vec3<F> columns[3]{};

public:
	Mat3() = default;

	Mat3(F diagonal) noexcept : columns{ Vec3<F>(diagonal, F(0), F(0)), Vec3<F>(F(0), diagonal, F(0)), Vec3<F>(F(0), F(0), diagonal) } { }

	Mat3(F n00, F n01, F n02, F n10, F n11, F n12, F n20, F n21, F n22) noexcept : columns{ Vec3<F>(n00, n10, n20), Vec3<F>(n01, n11, n21), Vec3<F>(n02, n12, n22) } { }

	Mat3(const Vec3<F>& column0, const Vec3<F>& column1, const Vec3<F>& column2) noexcept : columns{ column0, column1, column2 } { }

	[[nodiscard]]
	static Mat3 identity() noexcept {
		return Mat3(F(1));
	}

	[[nodiscard]]
	static Mat3 zero() noexcept {
		return Mat3();
	}

	[[nodiscard]]
	bool operator==(const Mat3& rhs) const noexcept
	    requires linalg_detail::BoolEqualityComparable<F>
	{
		return (columns[0] == rhs.columns[0]) && (columns[1] == rhs.columns[1]) && (columns[2] == rhs.columns[2]);
	}

	F& operator()(int row, int col) noexcept {
		return row == 0 ? columns[col].x : (row == 1 ? columns[col].y : columns[col].z);
	}

	[[nodiscard]]
	const F& operator()(int row, int col) const noexcept {
		return row == 0 ? columns[col].x : (row == 1 ? columns[col].y : columns[col].z);
	}

	Vec3<F>& column(int col) noexcept {
		return columns[col];
	}

	[[nodiscard]]
	const Vec3<F>& column(int col) const noexcept {
		return columns[col];
	}

	[[nodiscard]]
	Vec3<F> row(int row) const noexcept {
		return Vec3<F>((*this)(row, 0), (*this)(row, 1), (*this)(row, 2));
	}

	[[nodiscard]]
	Mat3 operator-() const noexcept {
		return Mat3(-columns[0], -columns[1], -columns[2]);
	}

	Mat3& operator+=(const Mat3& rhs) noexcept {
		columns[0] += rhs.columns[0];
		columns[1] += rhs.columns[1];
		columns[2] += rhs.columns[2];
		return *this;
	}

	Mat3& operator-=(const Mat3& rhs) noexcept {
		columns[0] -= rhs.columns[0];
		columns[1] -= rhs.columns[1];
		columns[2] -= rhs.columns[2];
		return *this;
	}

	Mat3& operator+=(F rhs) noexcept {
		columns[0] += rhs;
		columns[1] += rhs;
		columns[2] += rhs;
		return *this;
	}

	Mat3& operator-=(F rhs) noexcept {
		columns[0] -= rhs;
		columns[1] -= rhs;
		columns[2] -= rhs;
		return *this;
	}

	Mat3& operator*=(F rhs) noexcept {
		columns[0] *= rhs;
		columns[1] *= rhs;
		columns[2] *= rhs;
		return *this;
	}

	Mat3& operator/=(F rhs) noexcept {
		columns[0] /= rhs;
		columns[1] /= rhs;
		columns[2] /= rhs;
		return *this;
	}

	Mat3& operator*=(const Mat3& rhs) noexcept {
		const Mat3 lhs = *this;

		*this = Mat3(lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0), lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1), lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2), lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0), lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1), lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2), lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0), lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1), lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2));

		return *this;
	}

	[[nodiscard]]
	F determinant() const noexcept {
		return (*this)(0, 0) * ((*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1)) - (*this)(0, 1) * ((*this)(1, 0) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 0)) + (*this)(0, 2) * ((*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0));
	}

	[[nodiscard]]
	Mat3 transpose() const noexcept {
		return Mat3((*this)(0, 0), (*this)(1, 0), (*this)(2, 0), (*this)(0, 1), (*this)(1, 1), (*this)(2, 1), (*this)(0, 2), (*this)(1, 2), (*this)(2, 2));
	}

	[[nodiscard]]
	Mat3 inverse() const noexcept {
		const F inv_det = F(1) / determinant();

		const F a = (*this)(0, 0);
		const F b = (*this)(0, 1);
		const F c = (*this)(0, 2);
		const F d = (*this)(1, 0);
		const F e = (*this)(1, 1);
		const F f = (*this)(1, 2);
		const F g = (*this)(2, 0);
		const F h = (*this)(2, 1);
		const F i = (*this)(2, 2);

		return Mat3((e * i - f * h) * inv_det, (c * h - b * i) * inv_det, (b * f - c * e) * inv_det, (f * g - d * i) * inv_det, (a * i - c * g) * inv_det, (c * d - a * f) * inv_det, (d * h - e * g) * inv_det, (b * g - a * h) * inv_det, (a * e - b * d) * inv_det);
	}
};

template <typename F> using mat3 = Mat3<F>;

template <typename F>
[[nodiscard]]
inline Mat3<F> operator+(Mat3<F> lhs, const Mat3<F>& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Mat3<F> operator-(Mat3<F> lhs, const Mat3<F>& rhs) noexcept {
	lhs -= rhs;
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator+(Mat3<F> lhs, F2 rhs) noexcept {
	lhs += static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator-(Mat3<F> lhs, F2 rhs) noexcept {
	lhs -= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator*(Mat3<F> lhs, F2 rhs) noexcept {
	lhs *= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator/(Mat3<F> lhs, F2 rhs) noexcept {
	lhs /= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator+(F2 lhs, Mat3<F> rhs) noexcept {
	rhs += static_cast<F>(lhs);
	return rhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator-(F2 lhs, Mat3<F> rhs) noexcept {
	return -rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat3<F> operator*(F2 lhs, Mat3<F> rhs) noexcept {
	rhs *= static_cast<F>(lhs);
	return rhs;
}

template <typename F>
[[nodiscard]]
inline Vec3<F> operator*(const Mat3<F>& m, const Vec3<F>& v) noexcept {
	return Vec3<F>(m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z, m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z, m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z);
}

template <typename F>
[[nodiscard]]
inline Vec3<F> operator*(const Vec3<F>& v, const Mat3<F>& m) noexcept {
	return Vec3<F>(v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0), v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1), v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2));
}

template <typename F>
[[nodiscard]]
inline Mat3<F> operator*(Mat3<F> lhs, const Mat3<F>& rhs) noexcept {
	lhs *= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline F determinant(const Mat3<F>& m) noexcept {
	return m.determinant();
}

template <typename F>
[[nodiscard]]
inline Mat3<F> transpose(const Mat3<F>& m) noexcept {
	return m.transpose();
}

template <typename F>
[[nodiscard]]
inline Mat3<F> inverse(const Mat3<F>& m) noexcept {
	return m.inverse();
}

} // namespace mt
