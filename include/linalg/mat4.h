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

Type:           Mat4
                (aliased to mat4, for easy glsl porting)

Description:    A column-major 4x4 matrix that supports floating point types and types implementing the
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

#include "vec4.h"

namespace mt {

/**************************************************************************************************
 * A 4x4 column-major matrix.
 * Constructor elements and operator() use logical row, column order.
 * ************************************************************************************************/
template <typename F> struct Mat4 {
	using ComponentType = F;
	using column_type = Vec4<F>;

private:
	Vec4<F> columns[4]{};

	[[nodiscard]]
	static F det3(F n00, F n01, F n02, F n10, F n11, F n12, F n20, F n21, F n22) noexcept {
		return n00 * (n11 * n22 - n12 * n21) - n01 * (n10 * n22 - n12 * n20) + n02 * (n10 * n21 - n11 * n20);
	}

	[[nodiscard]]
	F minor_determinant(int row, int col) const noexcept {
		F values[9]{};
		int index = 0;

		for (int r = 0; r < 4; ++r) {
			if (r == row) {
				continue;
			}

			for (int c = 0; c < 4; ++c) {
				if (c == col) {
					continue;
				}

				values[index] = (*this)(r, c);
				++index;
			}
		}

		return det3(values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8]);
	}

	[[nodiscard]]
	F cofactor(int row, int col) const noexcept {
		const F minor_value = minor_determinant(row, col);
		return ((row + col) % 2) == 0 ? minor_value : -minor_value;
	}

public:
	Mat4() = default;

	Mat4(F diagonal) noexcept : columns{ Vec4<F>(diagonal, F(0), F(0), F(0)), Vec4<F>(F(0), diagonal, F(0), F(0)), Vec4<F>(F(0), F(0), diagonal, F(0)), Vec4<F>(F(0), F(0), F(0), diagonal) } { }

	Mat4(F n00, F n01, F n02, F n03, F n10, F n11, F n12, F n13, F n20, F n21, F n22, F n23, F n30, F n31, F n32, F n33) noexcept : columns{ Vec4<F>(n00, n10, n20, n30), Vec4<F>(n01, n11, n21, n31), Vec4<F>(n02, n12, n22, n32), Vec4<F>(n03, n13, n23, n33) } { }

	Mat4(const Vec4<F>& column0, const Vec4<F>& column1, const Vec4<F>& column2, const Vec4<F>& column3) noexcept : columns{ column0, column1, column2, column3 } { }

	[[nodiscard]]
	static Mat4 identity() noexcept {
		return Mat4(F(1));
	}

	[[nodiscard]]
	static Mat4 zero() noexcept {
		return Mat4();
	}

	[[nodiscard]]
	bool operator==(const Mat4& rhs) const noexcept
	    requires linalg_detail::BoolEqualityComparable<F>
	{
		return (columns[0] == rhs.columns[0]) && (columns[1] == rhs.columns[1]) && (columns[2] == rhs.columns[2]) && (columns[3] == rhs.columns[3]);
	}

	F& operator()(int row, int col) noexcept {
		return row == 0 ? columns[col].x : (row == 1 ? columns[col].y : (row == 2 ? columns[col].z : columns[col].w));
	}

	[[nodiscard]]
	const F& operator()(int row, int col) const noexcept {
		return row == 0 ? columns[col].x : (row == 1 ? columns[col].y : (row == 2 ? columns[col].z : columns[col].w));
	}

	Vec4<F>& column(int col) noexcept {
		return columns[col];
	}

	[[nodiscard]]
	const Vec4<F>& column(int col) const noexcept {
		return columns[col];
	}

	[[nodiscard]]
	Vec4<F> row(int row) const noexcept {
		return Vec4<F>((*this)(row, 0), (*this)(row, 1), (*this)(row, 2), (*this)(row, 3));
	}

	[[nodiscard]]
	Mat4 operator-() const noexcept {
		return Mat4(-columns[0], -columns[1], -columns[2], -columns[3]);
	}

	Mat4& operator+=(const Mat4& rhs) noexcept {
		columns[0] += rhs.columns[0];
		columns[1] += rhs.columns[1];
		columns[2] += rhs.columns[2];
		columns[3] += rhs.columns[3];
		return *this;
	}

	Mat4& operator-=(const Mat4& rhs) noexcept {
		columns[0] -= rhs.columns[0];
		columns[1] -= rhs.columns[1];
		columns[2] -= rhs.columns[2];
		columns[3] -= rhs.columns[3];
		return *this;
	}

	Mat4& operator+=(F rhs) noexcept {
		columns[0] += rhs;
		columns[1] += rhs;
		columns[2] += rhs;
		columns[3] += rhs;
		return *this;
	}

	Mat4& operator-=(F rhs) noexcept {
		columns[0] -= rhs;
		columns[1] -= rhs;
		columns[2] -= rhs;
		columns[3] -= rhs;
		return *this;
	}

	Mat4& operator*=(F rhs) noexcept {
		columns[0] *= rhs;
		columns[1] *= rhs;
		columns[2] *= rhs;
		columns[3] *= rhs;
		return *this;
	}

	Mat4& operator/=(F rhs) noexcept {
		columns[0] /= rhs;
		columns[1] /= rhs;
		columns[2] /= rhs;
		columns[3] /= rhs;
		return *this;
	}

	Mat4& operator*=(const Mat4& rhs) noexcept {
		const Mat4 lhs = *this;

		for (int row = 0; row < 4; ++row) {
			for (int col = 0; col < 4; ++col) {
				(*this)(row, col) = lhs(row, 0) * rhs(0, col) + lhs(row, 1) * rhs(1, col) + lhs(row, 2) * rhs(2, col) + lhs(row, 3) * rhs(3, col);
			}
		}

		return *this;
	}

	[[nodiscard]]
	F determinant() const noexcept {
		return (*this)(0, 0) * cofactor(0, 0) + (*this)(0, 1) * cofactor(0, 1) + (*this)(0, 2) * cofactor(0, 2) + (*this)(0, 3) * cofactor(0, 3);
	}

	[[nodiscard]]
	Mat4 transpose() const noexcept {
		return Mat4((*this)(0, 0), (*this)(1, 0), (*this)(2, 0), (*this)(3, 0), (*this)(0, 1), (*this)(1, 1), (*this)(2, 1), (*this)(3, 1), (*this)(0, 2), (*this)(1, 2), (*this)(2, 2), (*this)(3, 2), (*this)(0, 3), (*this)(1, 3), (*this)(2, 3), (*this)(3, 3));
	}

	[[nodiscard]]
	Mat4 inverse() const noexcept {
		const F inv_det = F(1) / determinant();

		return Mat4(cofactor(0, 0) * inv_det, cofactor(1, 0) * inv_det, cofactor(2, 0) * inv_det, cofactor(3, 0) * inv_det, cofactor(0, 1) * inv_det, cofactor(1, 1) * inv_det, cofactor(2, 1) * inv_det, cofactor(3, 1) * inv_det, cofactor(0, 2) * inv_det, cofactor(1, 2) * inv_det, cofactor(2, 2) * inv_det, cofactor(3, 2) * inv_det, cofactor(0, 3) * inv_det, cofactor(1, 3) * inv_det, cofactor(2, 3) * inv_det, cofactor(3, 3) * inv_det);
	}
};

template <typename F> using mat4 = Mat4<F>;

template <typename F>
[[nodiscard]]
inline Mat4<F> operator+(Mat4<F> lhs, const Mat4<F>& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Mat4<F> operator-(Mat4<F> lhs, const Mat4<F>& rhs) noexcept {
	lhs -= rhs;
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator+(Mat4<F> lhs, F2 rhs) noexcept {
	lhs += static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator-(Mat4<F> lhs, F2 rhs) noexcept {
	lhs -= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator*(Mat4<F> lhs, F2 rhs) noexcept {
	lhs *= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator/(Mat4<F> lhs, F2 rhs) noexcept {
	lhs /= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator+(F2 lhs, Mat4<F> rhs) noexcept {
	rhs += static_cast<F>(lhs);
	return rhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator-(F2 lhs, Mat4<F> rhs) noexcept {
	return -rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat4<F> operator*(F2 lhs, Mat4<F> rhs) noexcept {
	rhs *= static_cast<F>(lhs);
	return rhs;
}

template <typename F>
[[nodiscard]]
inline Vec4<F> operator*(const Mat4<F>& m, const Vec4<F>& v) noexcept {
	return Vec4<F>(m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z + m(0, 3) * v.w, m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z + m(1, 3) * v.w, m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z + m(2, 3) * v.w, m(3, 0) * v.x + m(3, 1) * v.y + m(3, 2) * v.z + m(3, 3) * v.w);
}

template <typename F>
[[nodiscard]]
inline Vec4<F> operator*(const Vec4<F>& v, const Mat4<F>& m) noexcept {
	return Vec4<F>(v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0) + v.w * m(3, 0), v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1) + v.w * m(3, 1), v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2) + v.w * m(3, 2), v.x * m(0, 3) + v.y * m(1, 3) + v.z * m(2, 3) + v.w * m(3, 3));
}

template <typename F>
[[nodiscard]]
inline Mat4<F> operator*(Mat4<F> lhs, const Mat4<F>& rhs) noexcept {
	lhs *= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline F determinant(const Mat4<F>& m) noexcept {
	return m.determinant();
}

template <typename F>
[[nodiscard]]
inline Mat4<F> transpose(const Mat4<F>& m) noexcept {
	return m.transpose();
}

template <typename F>
[[nodiscard]]
inline Mat4<F> inverse(const Mat4<F>& m) noexcept {
	return m.inverse();
}

} // namespace mt
