/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar mat3<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg/mat3.h"

void test_mat3_float(TestContext& test) {
	// Default constructor zero-initializes all matrix elements.
	mt::mat3<float> default_value;
	test.check_equal(default_value(0, 0), 0.0f, "mat3<float> default 00");
	test.check_equal(default_value(0, 1), 0.0f, "mat3<float> default 01");
	test.check_equal(default_value(0, 2), 0.0f, "mat3<float> default 02");
	test.check_equal(default_value(1, 0), 0.0f, "mat3<float> default 10");
	test.check_equal(default_value(1, 1), 0.0f, "mat3<float> default 11");
	test.check_equal(default_value(1, 2), 0.0f, "mat3<float> default 12");
	test.check_equal(default_value(2, 0), 0.0f, "mat3<float> default 20");
	test.check_equal(default_value(2, 1), 0.0f, "mat3<float> default 21");
	test.check_equal(default_value(2, 2), 0.0f, "mat3<float> default 22");

	// Static zero returns a zero matrix.
	mt::mat3<float> zero = mt::mat3<float>::zero();
	test.check(zero == default_value, "mat3<float> zero");

	// Single-value constructor creates a diagonal matrix.
	mt::mat3<float> diagonal(2.0f);
	test.check_equal(diagonal(0, 0), 2.0f, "mat3<float> diagonal 00");
	test.check_equal(diagonal(0, 1), 0.0f, "mat3<float> diagonal 01");
	test.check_equal(diagonal(0, 2), 0.0f, "mat3<float> diagonal 02");
	test.check_equal(diagonal(1, 0), 0.0f, "mat3<float> diagonal 10");
	test.check_equal(diagonal(1, 1), 2.0f, "mat3<float> diagonal 11");
	test.check_equal(diagonal(1, 2), 0.0f, "mat3<float> diagonal 12");
	test.check_equal(diagonal(2, 0), 0.0f, "mat3<float> diagonal 20");
	test.check_equal(diagonal(2, 1), 0.0f, "mat3<float> diagonal 21");
	test.check_equal(diagonal(2, 2), 2.0f, "mat3<float> diagonal 22");

	// Identity is a diagonal matrix with ones.
	mt::mat3<float> identity = mt::mat3<float>::identity();
	test.check_equal(identity(0, 0), 1.0f, "mat3<float> identity 00");
	test.check_equal(identity(1, 1), 1.0f, "mat3<float> identity 11");
	test.check_equal(identity(2, 2), 1.0f, "mat3<float> identity 22");
	test.check_equal(identity(0, 1), 0.0f, "mat3<float> identity off diagonal");

	// Nine-value constructor uses logical row-major order.
	mt::mat3<float> a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
	test.check_equal(a(0, 0), 1.0f, "mat3<float> constructor 00");
	test.check_equal(a(0, 1), 2.0f, "mat3<float> constructor 01");
	test.check_equal(a(0, 2), 3.0f, "mat3<float> constructor 02");
	test.check_equal(a(1, 0), 4.0f, "mat3<float> constructor 10");
	test.check_equal(a(1, 1), 5.0f, "mat3<float> constructor 11");
	test.check_equal(a(1, 2), 6.0f, "mat3<float> constructor 12");
	test.check_equal(a(2, 0), 7.0f, "mat3<float> constructor 20");
	test.check_equal(a(2, 1), 8.0f, "mat3<float> constructor 21");
	test.check_equal(a(2, 2), 9.0f, "mat3<float> constructor 22");

	// Column constructor stores the vectors as columns.
	mt::mat3<float> from_columns(mt::vec3<float>(1.0f, 4.0f, 7.0f), mt::vec3<float>(2.0f, 5.0f, 8.0f), mt::vec3<float>(3.0f, 6.0f, 9.0f));
	test.check(from_columns == a, "mat3<float> column constructor");

	// Column access returns stored columns.
	mt::vec3<float> column = a.column(2);
	test.check_equal(column.x, 3.0f, "mat3<float> column x");
	test.check_equal(column.y, 6.0f, "mat3<float> column y");
	test.check_equal(column.z, 9.0f, "mat3<float> column z");

	// Row access returns rows by value.
	mt::vec3<float> row = a.row(1);
	test.check_equal(row.x, 4.0f, "mat3<float> row x");
	test.check_equal(row.y, 5.0f, "mat3<float> row y");
	test.check_equal(row.z, 6.0f, "mat3<float> row z");

	// Mutable element and column access update the stored matrix.
	mt::mat3<float> mutable_value = a;
	mutable_value(2, 0) = 70.0f;
	mutable_value.column(2).z = 90.0f;
	test.check_equal(mutable_value(2, 0), 70.0f, "mat3<float> mutable element");
	test.check_equal(mutable_value(2, 2), 90.0f, "mat3<float> mutable column");

	// Equality is component-wise for scalar component types.
	test.check(a == mt::mat3<float>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f), "mat3<float> equality true");
	test.check(!(a == mt::mat3<float>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 10.0f)), "mat3<float> equality false");

	// Unary minus negates each element.
	mt::mat3<float> actual = -a;
	test.check_equal(actual(0, 0), -1.0f, "mat3<float> unary minus 00");
	test.check_equal(actual(1, 1), -5.0f, "mat3<float> unary minus 11");
	test.check_equal(actual(2, 2), -9.0f, "mat3<float> unary minus 22");

	mt::mat3<float> b(9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);

	// Matrix addition and subtraction are component-wise.
	actual = a + b;
	test.check_equal(actual(0, 0), 10.0f, "mat3<float> add 00");
	test.check_equal(actual(0, 1), 10.0f, "mat3<float> add 01");
	test.check_equal(actual(1, 2), 10.0f, "mat3<float> add 12");
	test.check_equal(actual(2, 2), 10.0f, "mat3<float> add 22");

	actual = b - a;
	test.check_equal(actual(0, 0), 8.0f, "mat3<float> subtract 00");
	test.check_equal(actual(0, 1), 6.0f, "mat3<float> subtract 01");
	test.check_equal(actual(1, 1), 0.0f, "mat3<float> subtract 11");
	test.check_equal(actual(2, 2), -8.0f, "mat3<float> subtract 22");

	// Scalar arithmetic works on every matrix element.
	actual = a + 3.0f;
	test.check_equal(actual(0, 0), 4.0f, "mat3<float> matrix plus scalar 00");
	test.check_equal(actual(1, 1), 8.0f, "mat3<float> matrix plus scalar 11");
	test.check_equal(actual(2, 2), 12.0f, "mat3<float> matrix plus scalar 22");

	actual = 3.0f + a;
	test.check_equal(actual(0, 0), 4.0f, "mat3<float> scalar plus matrix 00");
	test.check_equal(actual(1, 1), 8.0f, "mat3<float> scalar plus matrix 11");
	test.check_equal(actual(2, 2), 12.0f, "mat3<float> scalar plus matrix 22");

	actual = a - 1.0f;
	test.check_equal(actual(0, 0), 0.0f, "mat3<float> matrix minus scalar 00");
	test.check_equal(actual(1, 1), 4.0f, "mat3<float> matrix minus scalar 11");
	test.check_equal(actual(2, 2), 8.0f, "mat3<float> matrix minus scalar 22");

	actual = 10.0f - a;
	test.check_equal(actual(0, 0), 9.0f, "mat3<float> scalar minus matrix 00");
	test.check_equal(actual(1, 1), 5.0f, "mat3<float> scalar minus matrix 11");
	test.check_equal(actual(2, 2), 1.0f, "mat3<float> scalar minus matrix 22");

	actual = a * 2.0f;
	test.check_equal(actual(0, 0), 2.0f, "mat3<float> matrix times scalar 00");
	test.check_equal(actual(1, 1), 10.0f, "mat3<float> matrix times scalar 11");
	test.check_equal(actual(2, 2), 18.0f, "mat3<float> matrix times scalar 22");

	actual = 2.0f * a;
	test.check_equal(actual(0, 0), 2.0f, "mat3<float> scalar times matrix 00");
	test.check_equal(actual(1, 1), 10.0f, "mat3<float> scalar times matrix 11");
	test.check_equal(actual(2, 2), 18.0f, "mat3<float> scalar times matrix 22");

	actual = a / 2.0f;
	test.check_equal(actual(0, 0), 0.5f, "mat3<float> matrix divide scalar 00");
	test.check_equal(actual(1, 1), 2.5f, "mat3<float> matrix divide scalar 11");
	test.check_equal(actual(2, 2), 4.5f, "mat3<float> matrix divide scalar 22");

	// Compound scalar operations update in place.
	actual = a;
	actual += 1.0f;
	actual -= 1.0f;
	actual *= 2.0f;
	actual /= 2.0f;
	test.check(actual == a, "mat3<float> compound scalar round trip");

	// Matrix multiplication is standard matrix multiplication.
	actual = a * b;
	test.check_equal(actual(0, 0), 30.0f, "mat3<float> matrix multiply 00");
	test.check_equal(actual(0, 1), 24.0f, "mat3<float> matrix multiply 01");
	test.check_equal(actual(0, 2), 18.0f, "mat3<float> matrix multiply 02");
	test.check_equal(actual(1, 0), 84.0f, "mat3<float> matrix multiply 10");
	test.check_equal(actual(1, 1), 69.0f, "mat3<float> matrix multiply 11");
	test.check_equal(actual(1, 2), 54.0f, "mat3<float> matrix multiply 12");
	test.check_equal(actual(2, 0), 138.0f, "mat3<float> matrix multiply 20");
	test.check_equal(actual(2, 1), 114.0f, "mat3<float> matrix multiply 21");
	test.check_equal(actual(2, 2), 90.0f, "mat3<float> matrix multiply 22");

	actual = a;
	actual *= b;
	test.check_equal(actual(0, 0), 30.0f, "mat3<float> compound matrix multiply 00");
	test.check_equal(actual(1, 1), 69.0f, "mat3<float> compound matrix multiply 11");
	test.check_equal(actual(2, 2), 90.0f, "mat3<float> compound matrix multiply 22");

	// Matrix times vector treats the vector as a column vector.
	mt::vec3<float> vector_actual = a * mt::vec3<float>(2.0f, 3.0f, 4.0f);
	test.check_equal(vector_actual.x, 20.0f, "mat3<float> matrix vector x");
	test.check_equal(vector_actual.y, 47.0f, "mat3<float> matrix vector y");
	test.check_equal(vector_actual.z, 74.0f, "mat3<float> matrix vector z");

	// Vector times matrix treats the vector as a row vector.
	vector_actual = mt::vec3<float>(2.0f, 3.0f, 4.0f) * a;
	test.check_equal(vector_actual.x, 42.0f, "mat3<float> vector matrix x");
	test.check_equal(vector_actual.y, 51.0f, "mat3<float> vector matrix y");
	test.check_equal(vector_actual.z, 60.0f, "mat3<float> vector matrix z");

	// Determinant, transpose, and inverse are available as members and free functions.
	mt::mat3<float> invertible(1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 4.0f, 5.0f, 6.0f, 0.0f);
	test.check_equal(invertible.determinant(), 1.0f, "mat3<float> member determinant");
	test.check_equal(mt::determinant(invertible), 1.0f, "mat3<float> free determinant");

	actual = a.transpose();
	test.check_equal(actual(0, 0), 1.0f, "mat3<float> member transpose 00");
	test.check_equal(actual(0, 1), 4.0f, "mat3<float> member transpose 01");
	test.check_equal(actual(0, 2), 7.0f, "mat3<float> member transpose 02");
	test.check_equal(actual(1, 0), 2.0f, "mat3<float> member transpose 10");
	test.check_equal(actual(1, 1), 5.0f, "mat3<float> member transpose 11");
	test.check_equal(actual(1, 2), 8.0f, "mat3<float> member transpose 12");
	test.check_equal(actual(2, 0), 3.0f, "mat3<float> member transpose 20");
	test.check_equal(actual(2, 1), 6.0f, "mat3<float> member transpose 21");
	test.check_equal(actual(2, 2), 9.0f, "mat3<float> member transpose 22");

	actual = mt::transpose(a);
	test.check_equal(actual(0, 0), 1.0f, "mat3<float> free transpose 00");
	test.check_equal(actual(1, 1), 5.0f, "mat3<float> free transpose 11");
	test.check_equal(actual(2, 2), 9.0f, "mat3<float> free transpose 22");

	actual = invertible.inverse();
	test.check_close(actual(0, 0), -24.0f, "mat3<float> member inverse 00");
	test.check_close(actual(0, 1), 18.0f, "mat3<float> member inverse 01");
	test.check_close(actual(0, 2), 5.0f, "mat3<float> member inverse 02");
	test.check_close(actual(1, 0), 20.0f, "mat3<float> member inverse 10");
	test.check_close(actual(1, 1), -15.0f, "mat3<float> member inverse 11");
	test.check_close(actual(1, 2), -4.0f, "mat3<float> member inverse 12");
	test.check_close(actual(2, 0), -5.0f, "mat3<float> member inverse 20");
	test.check_close(actual(2, 1), 4.0f, "mat3<float> member inverse 21");
	test.check_close(actual(2, 2), 1.0f, "mat3<float> member inverse 22");

	actual = mt::inverse(invertible);
	test.check_close(actual(0, 0), -24.0f, "mat3<float> free inverse 00");
	test.check_close(actual(1, 1), -15.0f, "mat3<float> free inverse 11");
	test.check_close(actual(2, 2), 1.0f, "mat3<float> free inverse 22");
}

int main() {
	TestContext test;

	test_mat3_float(test);

	return test.result();
}
