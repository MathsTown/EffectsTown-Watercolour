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

Type:           Mat2
                (aliased to mat2, for easy glsl porting)

Description:    A column-major 2x2 matrix that supports floating point types and types implementing the
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

#include "vec2.h"

namespace mt {

/**************************************************************************************************
 * A 2x2 column-major matrix.
 * Constructor elements and operator() use logical row, column order.
 * ************************************************************************************************/
template <typename F> struct Mat2 {
	using ComponentType = F;
	using column_type = Vec2<F>;

private:
	Vec2<F> columns[2]{};

public:
	Mat2() = default;

	Mat2(F diagonal) noexcept : columns{ Vec2<F>(diagonal, F(0)), Vec2<F>(F(0), diagonal) } { }

	Mat2(F n00, F n01, F n10, F n11) noexcept : columns{ Vec2<F>(n00, n10), Vec2<F>(n01, n11) } { }

	Mat2(const Vec2<F>& column0, const Vec2<F>& column1) noexcept : columns{ column0, column1 } { }

	[[nodiscard]]
	static Mat2 identity() noexcept {
		return Mat2(F(1));
	}

	[[nodiscard]]
	static Mat2 zero() noexcept {
		return Mat2();
	}

	[[nodiscard]]
	bool operator==(const Mat2& rhs) const noexcept
	    requires linalg_detail::BoolEqualityComparable<F>
	{
		return (columns[0] == rhs.columns[0]) && (columns[1] == rhs.columns[1]);
	}

	F& operator()(int row, int col) noexcept {
		return row == 0 ? columns[col].x : columns[col].y;
	}

	[[nodiscard]]
	const F& operator()(int row, int col) const noexcept {
		return row == 0 ? columns[col].x : columns[col].y;
	}

	Vec2<F>& column(int col) noexcept {
		return columns[col];
	}

	[[nodiscard]]
	const Vec2<F>& column(int col) const noexcept {
		return columns[col];
	}

	[[nodiscard]]
	Vec2<F> row(int row) const noexcept {
		return Vec2<F>((*this)(row, 0), (*this)(row, 1));
	}

	[[nodiscard]]
	Mat2 operator-() const noexcept {
		return Mat2(-columns[0], -columns[1]);
	}

	Mat2& operator+=(const Mat2& rhs) noexcept {
		columns[0] += rhs.columns[0];
		columns[1] += rhs.columns[1];
		return *this;
	}

	Mat2& operator-=(const Mat2& rhs) noexcept {
		columns[0] -= rhs.columns[0];
		columns[1] -= rhs.columns[1];
		return *this;
	}

	Mat2& operator+=(F rhs) noexcept {
		columns[0] += rhs;
		columns[1] += rhs;
		return *this;
	}

	Mat2& operator-=(F rhs) noexcept {
		columns[0] -= rhs;
		columns[1] -= rhs;
		return *this;
	}

	Mat2& operator*=(F rhs) noexcept {
		columns[0] *= rhs;
		columns[1] *= rhs;
		return *this;
	}

	Mat2& operator/=(F rhs) noexcept {
		columns[0] /= rhs;
		columns[1] /= rhs;
		return *this;
	}

	Mat2& operator*=(const Mat2& rhs) noexcept {
		const Mat2 lhs = *this;

		columns[0] = Vec2<F>(lhs.columns[0].x * rhs.columns[0].x + lhs.columns[1].x * rhs.columns[0].y, lhs.columns[0].y * rhs.columns[0].x + lhs.columns[1].y * rhs.columns[0].y);
		columns[1] = Vec2<F>(lhs.columns[0].x * rhs.columns[1].x + lhs.columns[1].x * rhs.columns[1].y, lhs.columns[0].y * rhs.columns[1].x + lhs.columns[1].y * rhs.columns[1].y);

		return *this;
	}

	[[nodiscard]]
	F determinant() const noexcept {
		return columns[0].x * columns[1].y - columns[1].x * columns[0].y;
	}

	[[nodiscard]]
	Mat2 transpose() const noexcept {
		return Mat2((*this)(0, 0), (*this)(1, 0), (*this)(0, 1), (*this)(1, 1));
	}

	[[nodiscard]]
	Mat2 inverse() const noexcept {
		const F inv_det = F(1) / determinant();
		return Mat2(columns[1].y * inv_det, -columns[1].x * inv_det, -columns[0].y * inv_det, columns[0].x * inv_det);
	}
};

template <typename F> using mat2 = Mat2<F>;

template <typename F>
[[nodiscard]]
inline Mat2<F> operator+(Mat2<F> lhs, const Mat2<F>& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline Mat2<F> operator-(Mat2<F> lhs, const Mat2<F>& rhs) noexcept {
	lhs -= rhs;
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator+(Mat2<F> lhs, F2 rhs) noexcept {
	lhs += static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator-(Mat2<F> lhs, F2 rhs) noexcept {
	lhs -= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator*(Mat2<F> lhs, F2 rhs) noexcept {
	lhs *= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator/(Mat2<F> lhs, F2 rhs) noexcept {
	lhs /= static_cast<F>(rhs);
	return lhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator+(F2 lhs, Mat2<F> rhs) noexcept {
	rhs += static_cast<F>(lhs);
	return rhs;
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator-(F2 lhs, Mat2<F> rhs) noexcept {
	return -rhs + static_cast<F>(lhs);
}

template <typename F, typename F2>
[[nodiscard]]
inline Mat2<F> operator*(F2 lhs, Mat2<F> rhs) noexcept {
	rhs *= static_cast<F>(lhs);
	return rhs;
}

template <typename F>
[[nodiscard]]
inline Vec2<F> operator*(const Mat2<F>& m, const Vec2<F>& v) noexcept {
	return Vec2<F>(m(0, 0) * v.x + m(0, 1) * v.y, m(1, 0) * v.x + m(1, 1) * v.y);
}

template <typename F>
[[nodiscard]]
inline Vec2<F> operator*(const Vec2<F>& v, const Mat2<F>& m) noexcept {
	return Vec2<F>(v.x * m(0, 0) + v.y * m(1, 0), v.x * m(0, 1) + v.y * m(1, 1));
}

template <typename F>
[[nodiscard]]
inline Mat2<F> operator*(Mat2<F> lhs, const Mat2<F>& rhs) noexcept {
	lhs *= rhs;
	return lhs;
}

template <typename F>
[[nodiscard]]
inline F determinant(const Mat2<F>& m) noexcept {
	return m.determinant();
}

template <typename F>
[[nodiscard]]
inline Mat2<F> transpose(const Mat2<F>& m) noexcept {
	return m.transpose();
}

template <typename F>
[[nodiscard]]
inline Mat2<F> inverse(const Mat2<F>& m) noexcept {
	return m.inverse();
}

} // namespace mt
