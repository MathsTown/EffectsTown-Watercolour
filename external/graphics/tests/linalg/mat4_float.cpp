/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar mat4<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg/mat4.h"

void test_mat4_float(TestContext& test) {
	// Default constructor zero-initializes all matrix elements.
	mt::mat4<float> default_value;
	test.check_equal(default_value(0, 0), 0.0f, "mat4<float> default 00");
	test.check_equal(default_value(0, 1), 0.0f, "mat4<float> default 01");
	test.check_equal(default_value(0, 2), 0.0f, "mat4<float> default 02");
	test.check_equal(default_value(0, 3), 0.0f, "mat4<float> default 03");
	test.check_equal(default_value(1, 0), 0.0f, "mat4<float> default 10");
	test.check_equal(default_value(1, 1), 0.0f, "mat4<float> default 11");
	test.check_equal(default_value(1, 2), 0.0f, "mat4<float> default 12");
	test.check_equal(default_value(1, 3), 0.0f, "mat4<float> default 13");
	test.check_equal(default_value(2, 0), 0.0f, "mat4<float> default 20");
	test.check_equal(default_value(2, 1), 0.0f, "mat4<float> default 21");
	test.check_equal(default_value(2, 2), 0.0f, "mat4<float> default 22");
	test.check_equal(default_value(2, 3), 0.0f, "mat4<float> default 23");
	test.check_equal(default_value(3, 0), 0.0f, "mat4<float> default 30");
	test.check_equal(default_value(3, 1), 0.0f, "mat4<float> default 31");
	test.check_equal(default_value(3, 2), 0.0f, "mat4<float> default 32");
	test.check_equal(default_value(3, 3), 0.0f, "mat4<float> default 33");

	// Static zero returns a zero matrix.
	mt::mat4<float> zero = mt::mat4<float>::zero();
	test.check(zero == default_value, "mat4<float> zero");

	// Single-value constructor creates a diagonal matrix.
	mt::mat4<float> diagonal(2.0f);
	test.check_equal(diagonal(0, 0), 2.0f, "mat4<float> diagonal 00");
	test.check_equal(diagonal(1, 1), 2.0f, "mat4<float> diagonal 11");
	test.check_equal(diagonal(2, 2), 2.0f, "mat4<float> diagonal 22");
	test.check_equal(diagonal(3, 3), 2.0f, "mat4<float> diagonal 33");
	test.check_equal(diagonal(0, 1), 0.0f, "mat4<float> diagonal off diagonal");

	// Identity is a diagonal matrix with ones.
	mt::mat4<float> identity = mt::mat4<float>::identity();
	test.check_equal(identity(0, 0), 1.0f, "mat4<float> identity 00");
	test.check_equal(identity(1, 1), 1.0f, "mat4<float> identity 11");
	test.check_equal(identity(2, 2), 1.0f, "mat4<float> identity 22");
	test.check_equal(identity(3, 3), 1.0f, "mat4<float> identity 33");
	test.check_equal(identity(0, 1), 0.0f, "mat4<float> identity off diagonal");

	// Sixteen-value constructor uses logical row-major order.
	mt::mat4<float> a(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
	test.check_equal(a(0, 0), 1.0f, "mat4<float> constructor 00");
	test.check_equal(a(0, 1), 2.0f, "mat4<float> constructor 01");
	test.check_equal(a(0, 2), 3.0f, "mat4<float> constructor 02");
	test.check_equal(a(0, 3), 4.0f, "mat4<float> constructor 03");
	test.check_equal(a(1, 0), 5.0f, "mat4<float> constructor 10");
	test.check_equal(a(1, 1), 6.0f, "mat4<float> constructor 11");
	test.check_equal(a(1, 2), 7.0f, "mat4<float> constructor 12");
	test.check_equal(a(1, 3), 8.0f, "mat4<float> constructor 13");
	test.check_equal(a(2, 0), 9.0f, "mat4<float> constructor 20");
	test.check_equal(a(2, 1), 10.0f, "mat4<float> constructor 21");
	test.check_equal(a(2, 2), 11.0f, "mat4<float> constructor 22");
	test.check_equal(a(2, 3), 12.0f, "mat4<float> constructor 23");
	test.check_equal(a(3, 0), 13.0f, "mat4<float> constructor 30");
	test.check_equal(a(3, 1), 14.0f, "mat4<float> constructor 31");
	test.check_equal(a(3, 2), 15.0f, "mat4<float> constructor 32");
	test.check_equal(a(3, 3), 16.0f, "mat4<float> constructor 33");

	// Column constructor stores the vectors as columns.
	mt::mat4<float> from_columns(mt::vec4<float>(1.0f, 5.0f, 9.0f, 13.0f), mt::vec4<float>(2.0f, 6.0f, 10.0f, 14.0f), mt::vec4<float>(3.0f, 7.0f, 11.0f, 15.0f), mt::vec4<float>(4.0f, 8.0f, 12.0f, 16.0f));
	test.check(from_columns == a, "mat4<float> column constructor");

	// Column access returns stored columns.
	mt::vec4<float> column = a.column(3);
	test.check_equal(column.x, 4.0f, "mat4<float> column x");
	test.check_equal(column.y, 8.0f, "mat4<float> column y");
	test.check_equal(column.z, 12.0f, "mat4<float> column z");
	test.check_equal(column.w, 16.0f, "mat4<float> column w");

	// Row access returns rows by value.
	mt::vec4<float> row = a.row(2);
	test.check_equal(row.x, 9.0f, "mat4<float> row x");
	test.check_equal(row.y, 10.0f, "mat4<float> row y");
	test.check_equal(row.z, 11.0f, "mat4<float> row z");
	test.check_equal(row.w, 12.0f, "mat4<float> row w");

	// Mutable element and column access update the stored matrix.
	mt::mat4<float> mutable_value = a;
	mutable_value(3, 0) = 130.0f;
	mutable_value.column(3).w = 160.0f;
	test.check_equal(mutable_value(3, 0), 130.0f, "mat4<float> mutable element");
	test.check_equal(mutable_value(3, 3), 160.0f, "mat4<float> mutable column");

	// Equality is component-wise for scalar component types.
	test.check(a == from_columns, "mat4<float> equality true");
	test.check(!(a == mt::mat4<float>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 17.0f)), "mat4<float> equality false");

	// Unary minus negates each element.
	mt::mat4<float> actual = -a;
	test.check_equal(actual(0, 0), -1.0f, "mat4<float> unary minus 00");
	test.check_equal(actual(1, 1), -6.0f, "mat4<float> unary minus 11");
	test.check_equal(actual(2, 2), -11.0f, "mat4<float> unary minus 22");
	test.check_equal(actual(3, 3), -16.0f, "mat4<float> unary minus 33");

	mt::mat4<float> b(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);

	// Matrix addition and subtraction are component-wise.
	actual = a + b;
	test.check_equal(actual(0, 0), 17.0f, "mat4<float> add 00");
	test.check_equal(actual(1, 1), 17.0f, "mat4<float> add 11");
	test.check_equal(actual(2, 2), 17.0f, "mat4<float> add 22");
	test.check_equal(actual(3, 3), 17.0f, "mat4<float> add 33");

	actual = b - a;
	test.check_equal(actual(0, 0), 15.0f, "mat4<float> subtract 00");
	test.check_equal(actual(1, 1), 5.0f, "mat4<float> subtract 11");
	test.check_equal(actual(2, 2), -5.0f, "mat4<float> subtract 22");
	test.check_equal(actual(3, 3), -15.0f, "mat4<float> subtract 33");

	// Scalar arithmetic works on every matrix element.
	actual = a + 3.0f;
	test.check_equal(actual(0, 0), 4.0f, "mat4<float> matrix plus scalar 00");
	test.check_equal(actual(1, 1), 9.0f, "mat4<float> matrix plus scalar 11");
	test.check_equal(actual(2, 2), 14.0f, "mat4<float> matrix plus scalar 22");
	test.check_equal(actual(3, 3), 19.0f, "mat4<float> matrix plus scalar 33");

	actual = 3.0f + a;
	test.check_equal(actual(0, 0), 4.0f, "mat4<float> scalar plus matrix 00");
	test.check_equal(actual(1, 1), 9.0f, "mat4<float> scalar plus matrix 11");
	test.check_equal(actual(2, 2), 14.0f, "mat4<float> scalar plus matrix 22");
	test.check_equal(actual(3, 3), 19.0f, "mat4<float> scalar plus matrix 33");

	actual = a - 1.0f;
	test.check_equal(actual(0, 0), 0.0f, "mat4<float> matrix minus scalar 00");
	test.check_equal(actual(1, 1), 5.0f, "mat4<float> matrix minus scalar 11");
	test.check_equal(actual(2, 2), 10.0f, "mat4<float> matrix minus scalar 22");
	test.check_equal(actual(3, 3), 15.0f, "mat4<float> matrix minus scalar 33");

	actual = 20.0f - a;
	test.check_equal(actual(0, 0), 19.0f, "mat4<float> scalar minus matrix 00");
	test.check_equal(actual(1, 1), 14.0f, "mat4<float> scalar minus matrix 11");
	test.check_equal(actual(2, 2), 9.0f, "mat4<float> scalar minus matrix 22");
	test.check_equal(actual(3, 3), 4.0f, "mat4<float> scalar minus matrix 33");

	actual = a * 2.0f;
	test.check_equal(actual(0, 0), 2.0f, "mat4<float> matrix times scalar 00");
	test.check_equal(actual(1, 1), 12.0f, "mat4<float> matrix times scalar 11");
	test.check_equal(actual(2, 2), 22.0f, "mat4<float> matrix times scalar 22");
	test.check_equal(actual(3, 3), 32.0f, "mat4<float> matrix times scalar 33");

	actual = 2.0f * a;
	test.check_equal(actual(0, 0), 2.0f, "mat4<float> scalar times matrix 00");
	test.check_equal(actual(1, 1), 12.0f, "mat4<float> scalar times matrix 11");
	test.check_equal(actual(2, 2), 22.0f, "mat4<float> scalar times matrix 22");
	test.check_equal(actual(3, 3), 32.0f, "mat4<float> scalar times matrix 33");

	actual = a / 2.0f;
	test.check_equal(actual(0, 0), 0.5f, "mat4<float> matrix divide scalar 00");
	test.check_equal(actual(1, 1), 3.0f, "mat4<float> matrix divide scalar 11");
	test.check_equal(actual(2, 2), 5.5f, "mat4<float> matrix divide scalar 22");
	test.check_equal(actual(3, 3), 8.0f, "mat4<float> matrix divide scalar 33");

	// Compound scalar operations update in place.
	actual = a;
	actual += 1.0f;
	actual -= 1.0f;
	actual *= 2.0f;
	actual /= 2.0f;
	test.check(actual == a, "mat4<float> compound scalar round trip");

	// Matrix multiplication is standard matrix multiplication.
	actual = a * b;
	test.check_equal(actual(0, 0), 80.0f, "mat4<float> matrix multiply 00");
	test.check_equal(actual(0, 1), 70.0f, "mat4<float> matrix multiply 01");
	test.check_equal(actual(0, 2), 60.0f, "mat4<float> matrix multiply 02");
	test.check_equal(actual(0, 3), 50.0f, "mat4<float> matrix multiply 03");
	test.check_equal(actual(1, 0), 240.0f, "mat4<float> matrix multiply 10");
	test.check_equal(actual(1, 1), 214.0f, "mat4<float> matrix multiply 11");
	test.check_equal(actual(1, 2), 188.0f, "mat4<float> matrix multiply 12");
	test.check_equal(actual(1, 3), 162.0f, "mat4<float> matrix multiply 13");
	test.check_equal(actual(2, 0), 400.0f, "mat4<float> matrix multiply 20");
	test.check_equal(actual(2, 1), 358.0f, "mat4<float> matrix multiply 21");
	test.check_equal(actual(2, 2), 316.0f, "mat4<float> matrix multiply 22");
	test.check_equal(actual(2, 3), 274.0f, "mat4<float> matrix multiply 23");
	test.check_equal(actual(3, 0), 560.0f, "mat4<float> matrix multiply 30");
	test.check_equal(actual(3, 1), 502.0f, "mat4<float> matrix multiply 31");
	test.check_equal(actual(3, 2), 444.0f, "mat4<float> matrix multiply 32");
	test.check_equal(actual(3, 3), 386.0f, "mat4<float> matrix multiply 33");

	actual = a;
	actual *= b;
	test.check_equal(actual(0, 0), 80.0f, "mat4<float> compound matrix multiply 00");
	test.check_equal(actual(1, 1), 214.0f, "mat4<float> compound matrix multiply 11");
	test.check_equal(actual(2, 2), 316.0f, "mat4<float> compound matrix multiply 22");
	test.check_equal(actual(3, 3), 386.0f, "mat4<float> compound matrix multiply 33");

	// Matrix times vector treats the vector as a column vector.
	mt::vec4<float> vector_actual = a * mt::vec4<float>(2.0f, 3.0f, 4.0f, 5.0f);
	test.check_equal(vector_actual.x, 40.0f, "mat4<float> matrix vector x");
	test.check_equal(vector_actual.y, 96.0f, "mat4<float> matrix vector y");
	test.check_equal(vector_actual.z, 152.0f, "mat4<float> matrix vector z");
	test.check_equal(vector_actual.w, 208.0f, "mat4<float> matrix vector w");

	// Vector times matrix treats the vector as a row vector.
	vector_actual = mt::vec4<float>(2.0f, 3.0f, 4.0f, 5.0f) * a;
	test.check_equal(vector_actual.x, 118.0f, "mat4<float> vector matrix x");
	test.check_equal(vector_actual.y, 132.0f, "mat4<float> vector matrix y");
	test.check_equal(vector_actual.z, 146.0f, "mat4<float> vector matrix z");
	test.check_equal(vector_actual.w, 160.0f, "mat4<float> vector matrix w");

	// Determinant, transpose, and inverse are available as members and free functions.
	mt::mat4<float> invertible(1.0f, 2.0f, 3.0f, 4.0f, 0.0f, 1.0f, 4.0f, 5.0f, 0.0f, 0.0f, 1.0f, 6.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	test.check_equal(invertible.determinant(), 1.0f, "mat4<float> member determinant");
	test.check_equal(mt::determinant(invertible), 1.0f, "mat4<float> free determinant");

	actual = a.transpose();
	test.check_equal(actual(0, 0), 1.0f, "mat4<float> member transpose 00");
	test.check_equal(actual(0, 1), 5.0f, "mat4<float> member transpose 01");
	test.check_equal(actual(0, 2), 9.0f, "mat4<float> member transpose 02");
	test.check_equal(actual(0, 3), 13.0f, "mat4<float> member transpose 03");
	test.check_equal(actual(1, 0), 2.0f, "mat4<float> member transpose 10");
	test.check_equal(actual(1, 1), 6.0f, "mat4<float> member transpose 11");
	test.check_equal(actual(1, 2), 10.0f, "mat4<float> member transpose 12");
	test.check_equal(actual(1, 3), 14.0f, "mat4<float> member transpose 13");
	test.check_equal(actual(2, 0), 3.0f, "mat4<float> member transpose 20");
	test.check_equal(actual(2, 1), 7.0f, "mat4<float> member transpose 21");
	test.check_equal(actual(2, 2), 11.0f, "mat4<float> member transpose 22");
	test.check_equal(actual(2, 3), 15.0f, "mat4<float> member transpose 23");
	test.check_equal(actual(3, 0), 4.0f, "mat4<float> member transpose 30");
	test.check_equal(actual(3, 1), 8.0f, "mat4<float> member transpose 31");
	test.check_equal(actual(3, 2), 12.0f, "mat4<float> member transpose 32");
	test.check_equal(actual(3, 3), 16.0f, "mat4<float> member transpose 33");

	actual = mt::transpose(a);
	test.check_equal(actual(0, 0), 1.0f, "mat4<float> free transpose 00");
	test.check_equal(actual(1, 1), 6.0f, "mat4<float> free transpose 11");
	test.check_equal(actual(2, 2), 11.0f, "mat4<float> free transpose 22");
	test.check_equal(actual(3, 3), 16.0f, "mat4<float> free transpose 33");

	actual = invertible.inverse();
	test.check_close(actual(0, 0), 1.0f, "mat4<float> member inverse 00");
	test.check_close(actual(0, 1), -2.0f, "mat4<float> member inverse 01");
	test.check_close(actual(0, 2), 5.0f, "mat4<float> member inverse 02");
	test.check_close(actual(0, 3), -24.0f, "mat4<float> member inverse 03");
	test.check_close(actual(1, 0), 0.0f, "mat4<float> member inverse 10");
	test.check_close(actual(1, 1), 1.0f, "mat4<float> member inverse 11");
	test.check_close(actual(1, 2), -4.0f, "mat4<float> member inverse 12");
	test.check_close(actual(1, 3), 19.0f, "mat4<float> member inverse 13");
	test.check_close(actual(2, 0), 0.0f, "mat4<float> member inverse 20");
	test.check_close(actual(2, 1), 0.0f, "mat4<float> member inverse 21");
	test.check_close(actual(2, 2), 1.0f, "mat4<float> member inverse 22");
	test.check_close(actual(2, 3), -6.0f, "mat4<float> member inverse 23");
	test.check_close(actual(3, 0), 0.0f, "mat4<float> member inverse 30");
	test.check_close(actual(3, 1), 0.0f, "mat4<float> member inverse 31");
	test.check_close(actual(3, 2), 0.0f, "mat4<float> member inverse 32");
	test.check_close(actual(3, 3), 1.0f, "mat4<float> member inverse 33");

	actual = mt::inverse(invertible);
	test.check_close(actual(0, 0), 1.0f, "mat4<float> free inverse 00");
	test.check_close(actual(1, 1), 1.0f, "mat4<float> free inverse 11");
	test.check_close(actual(2, 2), 1.0f, "mat4<float> free inverse 22");
	test.check_close(actual(3, 3), 1.0f, "mat4<float> free inverse 33");
}

int main() {
	TestContext test;

	test_mat4_float(test);

	return test.result();
}
