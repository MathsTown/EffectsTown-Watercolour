/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar mat2<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg/mat2.h"

void test_mat2_double(TestContext& test) {
	// Constructors initialize double matrix elements directly.
	mt::mat2<double> default_value;
	test.check_equal(default_value(0, 0), 0.0, "mat2<double> default 00");
	test.check_equal(default_value(0, 1), 0.0, "mat2<double> default 01");
	test.check_equal(default_value(1, 0), 0.0, "mat2<double> default 10");
	test.check_equal(default_value(1, 1), 0.0, "mat2<double> default 11");

	mt::mat2<double> identity = mt::mat2<double>::identity();
	test.check_equal(identity(0, 0), 1.0, "mat2<double> identity 00");
	test.check_equal(identity(0, 1), 0.0, "mat2<double> identity 01");
	test.check_equal(identity(1, 0), 0.0, "mat2<double> identity 10");
	test.check_equal(identity(1, 1), 1.0, "mat2<double> identity 11");

	mt::mat2<double> a(1.0, 2.0, 3.0, 4.0);
	test.check_equal(a(0, 0), 1.0, "mat2<double> constructor 00");
	test.check_equal(a(0, 1), 2.0, "mat2<double> constructor 01");
	test.check_equal(a(1, 0), 3.0, "mat2<double> constructor 10");
	test.check_equal(a(1, 1), 4.0, "mat2<double> constructor 11");

	mt::mat2<double> from_columns(mt::vec2<double>(5.0, 6.0), mt::vec2<double>(7.0, 8.0));
	test.check_equal(from_columns(0, 0), 5.0, "mat2<double> column constructor 00");
	test.check_equal(from_columns(1, 0), 6.0, "mat2<double> column constructor 10");
	test.check_equal(from_columns(0, 1), 7.0, "mat2<double> column constructor 01");
	test.check_equal(from_columns(1, 1), 8.0, "mat2<double> column constructor 11");

	mt::vec2<double> row = a.row(1);
	test.check_equal(row.x, 3.0, "mat2<double> row x");
	test.check_equal(row.y, 4.0, "mat2<double> row y");

	mt::vec2<double> column = a.column(1);
	test.check_equal(column.x, 2.0, "mat2<double> column x");
	test.check_equal(column.y, 4.0, "mat2<double> column y");

	test.check(a == mt::mat2<double>(1.0, 2.0, 3.0, 4.0), "mat2<double> equality true");
	test.check(!(a == mt::mat2<double>(1.0, 2.0, 3.0, 5.0)), "mat2<double> equality false");

	mt::mat2<double> b(5.0, 6.0, 7.0, 8.0);

	// Matrix addition and subtraction are component-wise.
	mt::mat2<double> actual = a + b;
	test.check_equal(actual(0, 0), 6.0, "mat2<double> add 00");
	test.check_equal(actual(0, 1), 8.0, "mat2<double> add 01");
	test.check_equal(actual(1, 0), 10.0, "mat2<double> add 10");
	test.check_equal(actual(1, 1), 12.0, "mat2<double> add 11");

	actual = b - a;
	test.check_equal(actual(0, 0), 4.0, "mat2<double> subtract 00");
	test.check_equal(actual(0, 1), 4.0, "mat2<double> subtract 01");
	test.check_equal(actual(1, 0), 4.0, "mat2<double> subtract 10");
	test.check_equal(actual(1, 1), 4.0, "mat2<double> subtract 11");

	// Scalar multiplication works on both sides.
	actual = a * 2.0;
	test.check_equal(actual(0, 0), 2.0, "mat2<double> matrix times scalar 00");
	test.check_equal(actual(0, 1), 4.0, "mat2<double> matrix times scalar 01");
	test.check_equal(actual(1, 0), 6.0, "mat2<double> matrix times scalar 10");
	test.check_equal(actual(1, 1), 8.0, "mat2<double> matrix times scalar 11");

	actual = 2.0 * a;
	test.check_equal(actual(0, 0), 2.0, "mat2<double> scalar times matrix 00");
	test.check_equal(actual(0, 1), 4.0, "mat2<double> scalar times matrix 01");
	test.check_equal(actual(1, 0), 6.0, "mat2<double> scalar times matrix 10");
	test.check_equal(actual(1, 1), 8.0, "mat2<double> scalar times matrix 11");

	// Matrix multiplication is standard matrix multiplication.
	actual = a * b;
	test.check_equal(actual(0, 0), 19.0, "mat2<double> matrix multiply 00");
	test.check_equal(actual(0, 1), 22.0, "mat2<double> matrix multiply 01");
	test.check_equal(actual(1, 0), 43.0, "mat2<double> matrix multiply 10");
	test.check_equal(actual(1, 1), 50.0, "mat2<double> matrix multiply 11");

	actual = a;
	actual *= b;
	test.check_equal(actual(0, 0), 19.0, "mat2<double> compound matrix multiply 00");
	test.check_equal(actual(0, 1), 22.0, "mat2<double> compound matrix multiply 01");
	test.check_equal(actual(1, 0), 43.0, "mat2<double> compound matrix multiply 10");
	test.check_equal(actual(1, 1), 50.0, "mat2<double> compound matrix multiply 11");

	// Matrix-vector multiplication supports column and row vector forms.
	mt::vec2<double> vector_actual = a * mt::vec2<double>(5.0, 6.0);
	test.check_equal(vector_actual.x, 17.0, "mat2<double> matrix vector x");
	test.check_equal(vector_actual.y, 39.0, "mat2<double> matrix vector y");

	vector_actual = mt::vec2<double>(5.0, 6.0) * a;
	test.check_equal(vector_actual.x, 23.0, "mat2<double> vector matrix x");
	test.check_equal(vector_actual.y, 34.0, "mat2<double> vector matrix y");

	// Determinant, transpose, and inverse preserve double precision.
	test.check_equal(a.determinant(), -2.0, "mat2<double> member determinant");
	test.check_equal(mt::determinant(a), -2.0, "mat2<double> free determinant");

	actual = mt::transpose(a);
	test.check_equal(actual(0, 0), 1.0, "mat2<double> transpose 00");
	test.check_equal(actual(0, 1), 3.0, "mat2<double> transpose 01");
	test.check_equal(actual(1, 0), 2.0, "mat2<double> transpose 10");
	test.check_equal(actual(1, 1), 4.0, "mat2<double> transpose 11");

	mt::mat2<double> invertible(4.0, 7.0, 2.0, 6.0);
	actual = mt::inverse(invertible);
	test.check_close(actual(0, 0), 0.6, "mat2<double> inverse 00");
	test.check_close(actual(0, 1), -0.7, "mat2<double> inverse 01");
	test.check_close(actual(1, 0), -0.2, "mat2<double> inverse 10");
	test.check_close(actual(1, 1), 0.4, "mat2<double> inverse 11");
}

int main() {
	TestContext test;

	test_mat2_double(test);

	return test.result();
}
