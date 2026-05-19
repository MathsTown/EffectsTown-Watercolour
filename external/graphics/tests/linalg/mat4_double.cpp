/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar mat4<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg/mat4.h"

void test_mat4_double(TestContext& test) {
	// Constructors initialize double matrix elements directly.
	mt::mat4<double> default_value;
	test.check_equal(default_value(0, 0), 0.0, "mat4<double> default 00");
	test.check_equal(default_value(1, 1), 0.0, "mat4<double> default 11");
	test.check_equal(default_value(2, 2), 0.0, "mat4<double> default 22");
	test.check_equal(default_value(3, 3), 0.0, "mat4<double> default 33");

	mt::mat4<double> identity = mt::mat4<double>::identity();
	test.check_equal(identity(0, 0), 1.0, "mat4<double> identity 00");
	test.check_equal(identity(1, 1), 1.0, "mat4<double> identity 11");
	test.check_equal(identity(2, 2), 1.0, "mat4<double> identity 22");
	test.check_equal(identity(3, 3), 1.0, "mat4<double> identity 33");
	test.check_equal(identity(0, 1), 0.0, "mat4<double> identity off diagonal");

	mt::mat4<double> a(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
	test.check_equal(a(0, 0), 1.0, "mat4<double> constructor 00");
	test.check_equal(a(0, 3), 4.0, "mat4<double> constructor 03");
	test.check_equal(a(1, 1), 6.0, "mat4<double> constructor 11");
	test.check_equal(a(2, 2), 11.0, "mat4<double> constructor 22");
	test.check_equal(a(3, 3), 16.0, "mat4<double> constructor 33");

	mt::mat4<double> from_columns(mt::vec4<double>(1.0, 5.0, 9.0, 13.0), mt::vec4<double>(2.0, 6.0, 10.0, 14.0), mt::vec4<double>(3.0, 7.0, 11.0, 15.0), mt::vec4<double>(4.0, 8.0, 12.0, 16.0));
	test.check(from_columns == a, "mat4<double> column constructor");

	mt::vec4<double> row = a.row(2);
	test.check_equal(row.x, 9.0, "mat4<double> row x");
	test.check_equal(row.y, 10.0, "mat4<double> row y");
	test.check_equal(row.z, 11.0, "mat4<double> row z");
	test.check_equal(row.w, 12.0, "mat4<double> row w");

	mt::vec4<double> column = a.column(3);
	test.check_equal(column.x, 4.0, "mat4<double> column x");
	test.check_equal(column.y, 8.0, "mat4<double> column y");
	test.check_equal(column.z, 12.0, "mat4<double> column z");
	test.check_equal(column.w, 16.0, "mat4<double> column w");

	test.check(a == from_columns, "mat4<double> equality true");
	test.check(!(a == mt::mat4<double>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 17.0)), "mat4<double> equality false");

	mt::mat4<double> b(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

	// Matrix addition and scalar multiplication use component-wise arithmetic.
	mt::mat4<double> actual = a + b;
	test.check_equal(actual(0, 0), 17.0, "mat4<double> add 00");
	test.check_equal(actual(1, 1), 17.0, "mat4<double> add 11");
	test.check_equal(actual(2, 2), 17.0, "mat4<double> add 22");
	test.check_equal(actual(3, 3), 17.0, "mat4<double> add 33");

	actual = a * 2.0;
	test.check_equal(actual(0, 0), 2.0, "mat4<double> matrix times scalar 00");
	test.check_equal(actual(1, 1), 12.0, "mat4<double> matrix times scalar 11");
	test.check_equal(actual(2, 2), 22.0, "mat4<double> matrix times scalar 22");
	test.check_equal(actual(3, 3), 32.0, "mat4<double> matrix times scalar 33");

	actual = 2.0 * a;
	test.check_equal(actual(0, 0), 2.0, "mat4<double> scalar times matrix 00");
	test.check_equal(actual(1, 1), 12.0, "mat4<double> scalar times matrix 11");
	test.check_equal(actual(2, 2), 22.0, "mat4<double> scalar times matrix 22");
	test.check_equal(actual(3, 3), 32.0, "mat4<double> scalar times matrix 33");

	// Matrix multiplication is standard matrix multiplication.
	actual = a * b;
	test.check_equal(actual(0, 0), 80.0, "mat4<double> matrix multiply 00");
	test.check_equal(actual(0, 1), 70.0, "mat4<double> matrix multiply 01");
	test.check_equal(actual(0, 2), 60.0, "mat4<double> matrix multiply 02");
	test.check_equal(actual(0, 3), 50.0, "mat4<double> matrix multiply 03");
	test.check_equal(actual(1, 0), 240.0, "mat4<double> matrix multiply 10");
	test.check_equal(actual(1, 1), 214.0, "mat4<double> matrix multiply 11");
	test.check_equal(actual(1, 2), 188.0, "mat4<double> matrix multiply 12");
	test.check_equal(actual(1, 3), 162.0, "mat4<double> matrix multiply 13");
	test.check_equal(actual(2, 0), 400.0, "mat4<double> matrix multiply 20");
	test.check_equal(actual(2, 1), 358.0, "mat4<double> matrix multiply 21");
	test.check_equal(actual(2, 2), 316.0, "mat4<double> matrix multiply 22");
	test.check_equal(actual(2, 3), 274.0, "mat4<double> matrix multiply 23");
	test.check_equal(actual(3, 0), 560.0, "mat4<double> matrix multiply 30");
	test.check_equal(actual(3, 1), 502.0, "mat4<double> matrix multiply 31");
	test.check_equal(actual(3, 2), 444.0, "mat4<double> matrix multiply 32");
	test.check_equal(actual(3, 3), 386.0, "mat4<double> matrix multiply 33");

	actual = a;
	actual *= b;
	test.check_equal(actual(0, 0), 80.0, "mat4<double> compound matrix multiply 00");
	test.check_equal(actual(1, 1), 214.0, "mat4<double> compound matrix multiply 11");
	test.check_equal(actual(2, 2), 316.0, "mat4<double> compound matrix multiply 22");
	test.check_equal(actual(3, 3), 386.0, "mat4<double> compound matrix multiply 33");

	// Matrix-vector multiplication supports column and row vector forms.
	mt::vec4<double> vector_actual = a * mt::vec4<double>(2.0, 3.0, 4.0, 5.0);
	test.check_equal(vector_actual.x, 40.0, "mat4<double> matrix vector x");
	test.check_equal(vector_actual.y, 96.0, "mat4<double> matrix vector y");
	test.check_equal(vector_actual.z, 152.0, "mat4<double> matrix vector z");
	test.check_equal(vector_actual.w, 208.0, "mat4<double> matrix vector w");

	vector_actual = mt::vec4<double>(2.0, 3.0, 4.0, 5.0) * a;
	test.check_equal(vector_actual.x, 118.0, "mat4<double> vector matrix x");
	test.check_equal(vector_actual.y, 132.0, "mat4<double> vector matrix y");
	test.check_equal(vector_actual.z, 146.0, "mat4<double> vector matrix z");
	test.check_equal(vector_actual.w, 160.0, "mat4<double> vector matrix w");

	// Determinant, transpose, and inverse preserve double precision.
	mt::mat4<double> invertible(1.0, 2.0, 3.0, 4.0, 0.0, 1.0, 4.0, 5.0, 0.0, 0.0, 1.0, 6.0, 0.0, 0.0, 0.0, 1.0);
	test.check_equal(mt::determinant(invertible), 1.0, "mat4<double> determinant");

	actual = mt::transpose(a);
	test.check_equal(actual(0, 0), 1.0, "mat4<double> transpose 00");
	test.check_equal(actual(0, 1), 5.0, "mat4<double> transpose 01");
	test.check_equal(actual(0, 2), 9.0, "mat4<double> transpose 02");
	test.check_equal(actual(0, 3), 13.0, "mat4<double> transpose 03");
	test.check_equal(actual(1, 0), 2.0, "mat4<double> transpose 10");
	test.check_equal(actual(1, 1), 6.0, "mat4<double> transpose 11");
	test.check_equal(actual(1, 2), 10.0, "mat4<double> transpose 12");
	test.check_equal(actual(1, 3), 14.0, "mat4<double> transpose 13");
	test.check_equal(actual(2, 0), 3.0, "mat4<double> transpose 20");
	test.check_equal(actual(2, 1), 7.0, "mat4<double> transpose 21");
	test.check_equal(actual(2, 2), 11.0, "mat4<double> transpose 22");
	test.check_equal(actual(2, 3), 15.0, "mat4<double> transpose 23");
	test.check_equal(actual(3, 0), 4.0, "mat4<double> transpose 30");
	test.check_equal(actual(3, 1), 8.0, "mat4<double> transpose 31");
	test.check_equal(actual(3, 2), 12.0, "mat4<double> transpose 32");
	test.check_equal(actual(3, 3), 16.0, "mat4<double> transpose 33");

	actual = mt::inverse(invertible);
	test.check_close(actual(0, 0), 1.0, "mat4<double> inverse 00");
	test.check_close(actual(0, 1), -2.0, "mat4<double> inverse 01");
	test.check_close(actual(0, 2), 5.0, "mat4<double> inverse 02");
	test.check_close(actual(0, 3), -24.0, "mat4<double> inverse 03");
	test.check_close(actual(1, 0), 0.0, "mat4<double> inverse 10");
	test.check_close(actual(1, 1), 1.0, "mat4<double> inverse 11");
	test.check_close(actual(1, 2), -4.0, "mat4<double> inverse 12");
	test.check_close(actual(1, 3), 19.0, "mat4<double> inverse 13");
	test.check_close(actual(2, 0), 0.0, "mat4<double> inverse 20");
	test.check_close(actual(2, 1), 0.0, "mat4<double> inverse 21");
	test.check_close(actual(2, 2), 1.0, "mat4<double> inverse 22");
	test.check_close(actual(2, 3), -6.0, "mat4<double> inverse 23");
	test.check_close(actual(3, 0), 0.0, "mat4<double> inverse 30");
	test.check_close(actual(3, 1), 0.0, "mat4<double> inverse 31");
	test.check_close(actual(3, 2), 0.0, "mat4<double> inverse 32");
	test.check_close(actual(3, 3), 1.0, "mat4<double> inverse 33");
}

int main() {
	TestContext test;

	test_mat4_double(test);

	return test.result();
}
