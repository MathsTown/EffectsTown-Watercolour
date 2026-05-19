/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar mat3<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg/mat3.h"

void test_mat3_double(TestContext& test) {
	// Constructors initialize double matrix elements directly.
	mt::mat3<double> default_value;
	test.check_equal(default_value(0, 0), 0.0, "mat3<double> default 00");
	test.check_equal(default_value(1, 1), 0.0, "mat3<double> default 11");
	test.check_equal(default_value(2, 2), 0.0, "mat3<double> default 22");

	mt::mat3<double> identity = mt::mat3<double>::identity();
	test.check_equal(identity(0, 0), 1.0, "mat3<double> identity 00");
	test.check_equal(identity(1, 1), 1.0, "mat3<double> identity 11");
	test.check_equal(identity(2, 2), 1.0, "mat3<double> identity 22");
	test.check_equal(identity(0, 1), 0.0, "mat3<double> identity off diagonal");

	mt::mat3<double> a(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
	test.check_equal(a(0, 0), 1.0, "mat3<double> constructor 00");
	test.check_equal(a(0, 1), 2.0, "mat3<double> constructor 01");
	test.check_equal(a(0, 2), 3.0, "mat3<double> constructor 02");
	test.check_equal(a(1, 0), 4.0, "mat3<double> constructor 10");
	test.check_equal(a(1, 1), 5.0, "mat3<double> constructor 11");
	test.check_equal(a(1, 2), 6.0, "mat3<double> constructor 12");
	test.check_equal(a(2, 0), 7.0, "mat3<double> constructor 20");
	test.check_equal(a(2, 1), 8.0, "mat3<double> constructor 21");
	test.check_equal(a(2, 2), 9.0, "mat3<double> constructor 22");

	mt::mat3<double> from_columns(mt::vec3<double>(1.0, 4.0, 7.0), mt::vec3<double>(2.0, 5.0, 8.0), mt::vec3<double>(3.0, 6.0, 9.0));
	test.check(from_columns == a, "mat3<double> column constructor");

	mt::vec3<double> row = a.row(1);
	test.check_equal(row.x, 4.0, "mat3<double> row x");
	test.check_equal(row.y, 5.0, "mat3<double> row y");
	test.check_equal(row.z, 6.0, "mat3<double> row z");

	mt::vec3<double> column = a.column(2);
	test.check_equal(column.x, 3.0, "mat3<double> column x");
	test.check_equal(column.y, 6.0, "mat3<double> column y");
	test.check_equal(column.z, 9.0, "mat3<double> column z");

	test.check(a == mt::mat3<double>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0), "mat3<double> equality true");
	test.check(!(a == mt::mat3<double>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 10.0)), "mat3<double> equality false");

	mt::mat3<double> b(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

	// Matrix addition and subtraction are component-wise.
	mt::mat3<double> actual = a + b;
	test.check_equal(actual(0, 0), 10.0, "mat3<double> add 00");
	test.check_equal(actual(1, 1), 10.0, "mat3<double> add 11");
	test.check_equal(actual(2, 2), 10.0, "mat3<double> add 22");

	actual = b - a;
	test.check_equal(actual(0, 0), 8.0, "mat3<double> subtract 00");
	test.check_equal(actual(1, 1), 0.0, "mat3<double> subtract 11");
	test.check_equal(actual(2, 2), -8.0, "mat3<double> subtract 22");

	// Scalar multiplication works on both sides.
	actual = a * 2.0;
	test.check_equal(actual(0, 0), 2.0, "mat3<double> matrix times scalar 00");
	test.check_equal(actual(1, 1), 10.0, "mat3<double> matrix times scalar 11");
	test.check_equal(actual(2, 2), 18.0, "mat3<double> matrix times scalar 22");

	actual = 2.0 * a;
	test.check_equal(actual(0, 0), 2.0, "mat3<double> scalar times matrix 00");
	test.check_equal(actual(1, 1), 10.0, "mat3<double> scalar times matrix 11");
	test.check_equal(actual(2, 2), 18.0, "mat3<double> scalar times matrix 22");

	// Matrix multiplication is standard matrix multiplication.
	actual = a * b;
	test.check_equal(actual(0, 0), 30.0, "mat3<double> matrix multiply 00");
	test.check_equal(actual(0, 1), 24.0, "mat3<double> matrix multiply 01");
	test.check_equal(actual(0, 2), 18.0, "mat3<double> matrix multiply 02");
	test.check_equal(actual(1, 0), 84.0, "mat3<double> matrix multiply 10");
	test.check_equal(actual(1, 1), 69.0, "mat3<double> matrix multiply 11");
	test.check_equal(actual(1, 2), 54.0, "mat3<double> matrix multiply 12");
	test.check_equal(actual(2, 0), 138.0, "mat3<double> matrix multiply 20");
	test.check_equal(actual(2, 1), 114.0, "mat3<double> matrix multiply 21");
	test.check_equal(actual(2, 2), 90.0, "mat3<double> matrix multiply 22");

	actual = a;
	actual *= b;
	test.check_equal(actual(0, 0), 30.0, "mat3<double> compound matrix multiply 00");
	test.check_equal(actual(1, 1), 69.0, "mat3<double> compound matrix multiply 11");
	test.check_equal(actual(2, 2), 90.0, "mat3<double> compound matrix multiply 22");

	// Matrix-vector multiplication supports column and row vector forms.
	mt::vec3<double> vector_actual = a * mt::vec3<double>(2.0, 3.0, 4.0);
	test.check_equal(vector_actual.x, 20.0, "mat3<double> matrix vector x");
	test.check_equal(vector_actual.y, 47.0, "mat3<double> matrix vector y");
	test.check_equal(vector_actual.z, 74.0, "mat3<double> matrix vector z");

	vector_actual = mt::vec3<double>(2.0, 3.0, 4.0) * a;
	test.check_equal(vector_actual.x, 42.0, "mat3<double> vector matrix x");
	test.check_equal(vector_actual.y, 51.0, "mat3<double> vector matrix y");
	test.check_equal(vector_actual.z, 60.0, "mat3<double> vector matrix z");

	// Determinant, transpose, and inverse preserve double precision.
	mt::mat3<double> invertible(1.0, 2.0, 3.0, 0.0, 1.0, 4.0, 5.0, 6.0, 0.0);
	test.check_equal(mt::determinant(invertible), 1.0, "mat3<double> determinant");

	actual = mt::transpose(a);
	test.check_equal(actual(0, 0), 1.0, "mat3<double> transpose 00");
	test.check_equal(actual(0, 1), 4.0, "mat3<double> transpose 01");
	test.check_equal(actual(0, 2), 7.0, "mat3<double> transpose 02");
	test.check_equal(actual(1, 0), 2.0, "mat3<double> transpose 10");
	test.check_equal(actual(1, 1), 5.0, "mat3<double> transpose 11");
	test.check_equal(actual(1, 2), 8.0, "mat3<double> transpose 12");
	test.check_equal(actual(2, 0), 3.0, "mat3<double> transpose 20");
	test.check_equal(actual(2, 1), 6.0, "mat3<double> transpose 21");
	test.check_equal(actual(2, 2), 9.0, "mat3<double> transpose 22");

	actual = mt::inverse(invertible);
	test.check_close(actual(0, 0), -24.0, "mat3<double> inverse 00");
	test.check_close(actual(0, 1), 18.0, "mat3<double> inverse 01");
	test.check_close(actual(0, 2), 5.0, "mat3<double> inverse 02");
	test.check_close(actual(1, 0), 20.0, "mat3<double> inverse 10");
	test.check_close(actual(1, 1), -15.0, "mat3<double> inverse 11");
	test.check_close(actual(1, 2), -4.0, "mat3<double> inverse 12");
	test.check_close(actual(2, 0), -5.0, "mat3<double> inverse 20");
	test.check_close(actual(2, 1), 4.0, "mat3<double> inverse 21");
	test.check_close(actual(2, 2), 1.0, "mat3<double> inverse 22");
}

int main() {
	TestContext test;

	test_mat3_double(test);

	return test.result();
}
