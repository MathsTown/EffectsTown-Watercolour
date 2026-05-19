/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar mat2<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg/mat2.h"

void test_mat2_float(TestContext& test) {
	// Default constructor zero-initializes all matrix elements.
	mt::mat2<float> default_value;
	test.check_equal(default_value(0, 0), 0.0f, "mat2<float> default 00");
	test.check_equal(default_value(0, 1), 0.0f, "mat2<float> default 01");
	test.check_equal(default_value(1, 0), 0.0f, "mat2<float> default 10");
	test.check_equal(default_value(1, 1), 0.0f, "mat2<float> default 11");

	// Static zero returns a zero matrix.
	mt::mat2<float> zero = mt::mat2<float>::zero();
	test.check(zero == default_value, "mat2<float> zero");

	// Single-value constructor creates a diagonal matrix.
	mt::mat2<float> diagonal(2.0f);
	test.check_equal(diagonal(0, 0), 2.0f, "mat2<float> diagonal 00");
	test.check_equal(diagonal(0, 1), 0.0f, "mat2<float> diagonal 01");
	test.check_equal(diagonal(1, 0), 0.0f, "mat2<float> diagonal 10");
	test.check_equal(diagonal(1, 1), 2.0f, "mat2<float> diagonal 11");

	// Identity is a diagonal matrix with ones.
	mt::mat2<float> identity = mt::mat2<float>::identity();
	test.check_equal(identity(0, 0), 1.0f, "mat2<float> identity 00");
	test.check_equal(identity(0, 1), 0.0f, "mat2<float> identity 01");
	test.check_equal(identity(1, 0), 0.0f, "mat2<float> identity 10");
	test.check_equal(identity(1, 1), 1.0f, "mat2<float> identity 11");

	// Four-value constructor uses logical row-major order.
	mt::mat2<float> a(1.0f, 2.0f, 3.0f, 4.0f);
	test.check_equal(a(0, 0), 1.0f, "mat2<float> constructor 00");
	test.check_equal(a(0, 1), 2.0f, "mat2<float> constructor 01");
	test.check_equal(a(1, 0), 3.0f, "mat2<float> constructor 10");
	test.check_equal(a(1, 1), 4.0f, "mat2<float> constructor 11");

	// Column constructor stores the vectors as columns.
	mt::mat2<float> from_columns(mt::vec2<float>(5.0f, 6.0f), mt::vec2<float>(7.0f, 8.0f));
	test.check_equal(from_columns(0, 0), 5.0f, "mat2<float> column constructor 00");
	test.check_equal(from_columns(1, 0), 6.0f, "mat2<float> column constructor 10");
	test.check_equal(from_columns(0, 1), 7.0f, "mat2<float> column constructor 01");
	test.check_equal(from_columns(1, 1), 8.0f, "mat2<float> column constructor 11");

	// Column access returns stored columns.
	mt::vec2<float> first_column = a.column(0);
	test.check_equal(first_column.x, 1.0f, "mat2<float> column 0 x");
	test.check_equal(first_column.y, 3.0f, "mat2<float> column 0 y");

	mt::vec2<float> second_column = a.column(1);
	test.check_equal(second_column.x, 2.0f, "mat2<float> column 1 x");
	test.check_equal(second_column.y, 4.0f, "mat2<float> column 1 y");

	// Row access returns rows by value.
	mt::vec2<float> first_row = a.row(0);
	test.check_equal(first_row.x, 1.0f, "mat2<float> row 0 x");
	test.check_equal(first_row.y, 2.0f, "mat2<float> row 0 y");

	mt::vec2<float> second_row = a.row(1);
	test.check_equal(second_row.x, 3.0f, "mat2<float> row 1 x");
	test.check_equal(second_row.y, 4.0f, "mat2<float> row 1 y");

	// Mutable element and column access update the stored matrix.
	mt::mat2<float> mutable_value = a;
	mutable_value(1, 0) = 30.0f;
	mutable_value.column(1).y = 40.0f;
	test.check_equal(mutable_value(1, 0), 30.0f, "mat2<float> mutable element");
	test.check_equal(mutable_value(1, 1), 40.0f, "mat2<float> mutable column");

	// Equality is component-wise for scalar component types.
	test.check(a == mt::mat2<float>(1.0f, 2.0f, 3.0f, 4.0f), "mat2<float> equality true");
	test.check(!(a == mt::mat2<float>(1.0f, 2.0f, 3.0f, 5.0f)), "mat2<float> equality false");

	// Unary minus negates each element.
	mt::mat2<float> actual = -a;
	test.check_equal(actual(0, 0), -1.0f, "mat2<float> unary minus 00");
	test.check_equal(actual(0, 1), -2.0f, "mat2<float> unary minus 01");
	test.check_equal(actual(1, 0), -3.0f, "mat2<float> unary minus 10");
	test.check_equal(actual(1, 1), -4.0f, "mat2<float> unary minus 11");

	mt::mat2<float> b(5.0f, 6.0f, 7.0f, 8.0f);

	// Matrix addition and subtraction are component-wise.
	actual = a + b;
	test.check_equal(actual(0, 0), 6.0f, "mat2<float> add 00");
	test.check_equal(actual(0, 1), 8.0f, "mat2<float> add 01");
	test.check_equal(actual(1, 0), 10.0f, "mat2<float> add 10");
	test.check_equal(actual(1, 1), 12.0f, "mat2<float> add 11");

	actual = b - a;
	test.check_equal(actual(0, 0), 4.0f, "mat2<float> subtract 00");
	test.check_equal(actual(0, 1), 4.0f, "mat2<float> subtract 01");
	test.check_equal(actual(1, 0), 4.0f, "mat2<float> subtract 10");
	test.check_equal(actual(1, 1), 4.0f, "mat2<float> subtract 11");

	// Scalar arithmetic works on every matrix element.
	actual = a + 3.0f;
	test.check_equal(actual(0, 0), 4.0f, "mat2<float> matrix plus scalar 00");
	test.check_equal(actual(0, 1), 5.0f, "mat2<float> matrix plus scalar 01");
	test.check_equal(actual(1, 0), 6.0f, "mat2<float> matrix plus scalar 10");
	test.check_equal(actual(1, 1), 7.0f, "mat2<float> matrix plus scalar 11");

	actual = 3.0f + a;
	test.check_equal(actual(0, 0), 4.0f, "mat2<float> scalar plus matrix 00");
	test.check_equal(actual(0, 1), 5.0f, "mat2<float> scalar plus matrix 01");
	test.check_equal(actual(1, 0), 6.0f, "mat2<float> scalar plus matrix 10");
	test.check_equal(actual(1, 1), 7.0f, "mat2<float> scalar plus matrix 11");

	actual = a - 1.0f;
	test.check_equal(actual(0, 0), 0.0f, "mat2<float> matrix minus scalar 00");
	test.check_equal(actual(0, 1), 1.0f, "mat2<float> matrix minus scalar 01");
	test.check_equal(actual(1, 0), 2.0f, "mat2<float> matrix minus scalar 10");
	test.check_equal(actual(1, 1), 3.0f, "mat2<float> matrix minus scalar 11");

	actual = 10.0f - a;
	test.check_equal(actual(0, 0), 9.0f, "mat2<float> scalar minus matrix 00");
	test.check_equal(actual(0, 1), 8.0f, "mat2<float> scalar minus matrix 01");
	test.check_equal(actual(1, 0), 7.0f, "mat2<float> scalar minus matrix 10");
	test.check_equal(actual(1, 1), 6.0f, "mat2<float> scalar minus matrix 11");

	actual = a * 2.0f;
	test.check_equal(actual(0, 0), 2.0f, "mat2<float> matrix times scalar 00");
	test.check_equal(actual(0, 1), 4.0f, "mat2<float> matrix times scalar 01");
	test.check_equal(actual(1, 0), 6.0f, "mat2<float> matrix times scalar 10");
	test.check_equal(actual(1, 1), 8.0f, "mat2<float> matrix times scalar 11");

	actual = 2.0f * a;
	test.check_equal(actual(0, 0), 2.0f, "mat2<float> scalar times matrix 00");
	test.check_equal(actual(0, 1), 4.0f, "mat2<float> scalar times matrix 01");
	test.check_equal(actual(1, 0), 6.0f, "mat2<float> scalar times matrix 10");
	test.check_equal(actual(1, 1), 8.0f, "mat2<float> scalar times matrix 11");

	actual = a / 2.0f;
	test.check_equal(actual(0, 0), 0.5f, "mat2<float> matrix divide scalar 00");
	test.check_equal(actual(0, 1), 1.0f, "mat2<float> matrix divide scalar 01");
	test.check_equal(actual(1, 0), 1.5f, "mat2<float> matrix divide scalar 10");
	test.check_equal(actual(1, 1), 2.0f, "mat2<float> matrix divide scalar 11");

	// Compound scalar operations update in place.
	actual = a;
	actual += 1.0f;
	actual -= 1.0f;
	actual *= 2.0f;
	actual /= 2.0f;
	test.check(actual == a, "mat2<float> compound scalar round trip");

	// Matrix multiplication is standard matrix multiplication.
	actual = a * b;
	test.check_equal(actual(0, 0), 19.0f, "mat2<float> matrix multiply 00");
	test.check_equal(actual(0, 1), 22.0f, "mat2<float> matrix multiply 01");
	test.check_equal(actual(1, 0), 43.0f, "mat2<float> matrix multiply 10");
	test.check_equal(actual(1, 1), 50.0f, "mat2<float> matrix multiply 11");

	actual = a;
	actual *= b;
	test.check_equal(actual(0, 0), 19.0f, "mat2<float> compound matrix multiply 00");
	test.check_equal(actual(0, 1), 22.0f, "mat2<float> compound matrix multiply 01");
	test.check_equal(actual(1, 0), 43.0f, "mat2<float> compound matrix multiply 10");
	test.check_equal(actual(1, 1), 50.0f, "mat2<float> compound matrix multiply 11");

	// Matrix times vector treats the vector as a column vector.
	mt::vec2<float> vector_actual = a * mt::vec2<float>(5.0f, 6.0f);
	test.check_equal(vector_actual.x, 17.0f, "mat2<float> matrix vector x");
	test.check_equal(vector_actual.y, 39.0f, "mat2<float> matrix vector y");

	// Vector times matrix treats the vector as a row vector.
	vector_actual = mt::vec2<float>(5.0f, 6.0f) * a;
	test.check_equal(vector_actual.x, 23.0f, "mat2<float> vector matrix x");
	test.check_equal(vector_actual.y, 34.0f, "mat2<float> vector matrix y");

	// Determinant, transpose, and inverse are available as members and free functions.
	test.check_equal(a.determinant(), -2.0f, "mat2<float> member determinant");
	test.check_equal(mt::determinant(a), -2.0f, "mat2<float> free determinant");

	actual = a.transpose();
	test.check_equal(actual(0, 0), 1.0f, "mat2<float> member transpose 00");
	test.check_equal(actual(0, 1), 3.0f, "mat2<float> member transpose 01");
	test.check_equal(actual(1, 0), 2.0f, "mat2<float> member transpose 10");
	test.check_equal(actual(1, 1), 4.0f, "mat2<float> member transpose 11");

	actual = mt::transpose(a);
	test.check_equal(actual(0, 0), 1.0f, "mat2<float> free transpose 00");
	test.check_equal(actual(0, 1), 3.0f, "mat2<float> free transpose 01");
	test.check_equal(actual(1, 0), 2.0f, "mat2<float> free transpose 10");
	test.check_equal(actual(1, 1), 4.0f, "mat2<float> free transpose 11");

	mt::mat2<float> invertible(4.0f, 7.0f, 2.0f, 6.0f);
	actual = invertible.inverse();
	test.check_close(actual(0, 0), 0.6f, "mat2<float> member inverse 00");
	test.check_close(actual(0, 1), -0.7f, "mat2<float> member inverse 01");
	test.check_close(actual(1, 0), -0.2f, "mat2<float> member inverse 10");
	test.check_close(actual(1, 1), 0.4f, "mat2<float> member inverse 11");

	actual = mt::inverse(invertible);
	test.check_close(actual(0, 0), 0.6f, "mat2<float> free inverse 00");
	test.check_close(actual(0, 1), -0.7f, "mat2<float> free inverse 01");
	test.check_close(actual(1, 0), -0.2f, "mat2<float> free inverse 10");
	test.check_close(actual(1, 1), 0.4f, "mat2<float> free inverse 11");
}

int main() {
	TestContext test;

	test_mat2_float(test);

	return test.result();
}
