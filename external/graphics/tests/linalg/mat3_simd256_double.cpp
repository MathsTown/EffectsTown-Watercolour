/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 mat3<Simd256Float64> tests.
*********************************************************************************************************/

#include "../test.h"

#include "simd-uint32.h"
#include "simd-f64.h"

#include "linalg/mat3.h"

void test_mat3_simd256_double(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float64::compiler_supported(), "mat3 Simd256Float64 compiler support");
	test.check(mt::Simd256Float64::cpu_supported(cpu), "mat3 Simd256Float64 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float64 a00(0.0);
	mt::Simd256Float64 a01(0.0);
	mt::Simd256Float64 a02(0.0);
	mt::Simd256Float64 a10(0.0);
	mt::Simd256Float64 a11(0.0);
	mt::Simd256Float64 a12(0.0);
	mt::Simd256Float64 a20(0.0);
	mt::Simd256Float64 a21(0.0);
	mt::Simd256Float64 a22(0.0);
	mt::Simd256Float64 b00(0.0);
	mt::Simd256Float64 b01(0.0);
	mt::Simd256Float64 b02(0.0);
	mt::Simd256Float64 b10(0.0);
	mt::Simd256Float64 b11(0.0);
	mt::Simd256Float64 b12(0.0);
	mt::Simd256Float64 b20(0.0);
	mt::Simd256Float64 b21(0.0);
	mt::Simd256Float64 b22(0.0);

	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		a00.set_element(lane, double(lane + 1));
		a01.set_element(lane, double(lane + 2));
		a02.set_element(lane, double(lane + 3));
		a10.set_element(lane, double(lane + 4));
		a11.set_element(lane, double(lane + 5));
		a12.set_element(lane, double(lane + 6));
		a20.set_element(lane, double(lane + 7));
		a21.set_element(lane, double(lane + 8));
		a22.set_element(lane, double(lane + 9));
		b00.set_element(lane, double(lane + 10));
		b01.set_element(lane, double(lane + 11));
		b02.set_element(lane, double(lane + 12));
		b10.set_element(lane, double(lane + 13));
		b11.set_element(lane, double(lane + 14));
		b12.set_element(lane, double(lane + 15));
		b20.set_element(lane, double(lane + 16));
		b21.set_element(lane, double(lane + 17));
		b22.set_element(lane, double(lane + 18));
	}

	mt::mat3<mt::Simd256Float64> a(a00, a01, a02, a10, a11, a12, a20, a21, a22);
	mt::mat3<mt::Simd256Float64> b(b00, b01, b02, b10, b11, b12, b20, b21, b22);

	// Constructor, column, and row access expose the expected lane-wise values.
	mt::vec3<mt::Simd256Float64> column = a.column(2);
	mt::vec3<mt::Simd256Float64> row = a.row(1);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(a(0, 0).element(lane), a00.element(lane), "mat3<Simd256Float64> constructor 00 lane " + std::to_string(lane));
		test.check_equal(a(1, 2).element(lane), a12.element(lane), "mat3<Simd256Float64> constructor 12 lane " + std::to_string(lane));
		test.check_equal(column.x.element(lane), a02.element(lane), "mat3<Simd256Float64> column x lane " + std::to_string(lane));
		test.check_equal(column.y.element(lane), a12.element(lane), "mat3<Simd256Float64> column y lane " + std::to_string(lane));
		test.check_equal(column.z.element(lane), a22.element(lane), "mat3<Simd256Float64> column z lane " + std::to_string(lane));
		test.check_equal(row.x.element(lane), a10.element(lane), "mat3<Simd256Float64> row x lane " + std::to_string(lane));
		test.check_equal(row.y.element(lane), a11.element(lane), "mat3<Simd256Float64> row y lane " + std::to_string(lane));
		test.check_equal(row.z.element(lane), a12.element(lane), "mat3<Simd256Float64> row z lane " + std::to_string(lane));
	}

	// Scalar multiplication works on both sides.
	mt::mat3<mt::Simd256Float64> actual = a * 2.0;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0, "mat3<Simd256Float64> matrix times scalar 00 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0, "mat3<Simd256Float64> matrix times scalar 11 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), a22.element(lane) * 2.0, "mat3<Simd256Float64> matrix times scalar 22 lane " + std::to_string(lane));
	}

	actual = 2.0 * a;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0, "mat3<Simd256Float64> scalar times matrix 00 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0, "mat3<Simd256Float64> scalar times matrix 11 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), a22.element(lane) * 2.0, "mat3<Simd256Float64> scalar times matrix 22 lane " + std::to_string(lane));
	}

	// Matrix multiplication is lane-wise standard matrix multiplication.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		const double expected00 = a00.element(lane) * b00.element(lane) + a01.element(lane) * b10.element(lane) + a02.element(lane) * b20.element(lane);
		const double expected11 = a10.element(lane) * b01.element(lane) + a11.element(lane) * b11.element(lane) + a12.element(lane) * b21.element(lane);
		const double expected22 = a20.element(lane) * b02.element(lane) + a21.element(lane) * b12.element(lane) + a22.element(lane) * b22.element(lane);
		test.check_equal(actual(0, 0).element(lane), expected00, "mat3<Simd256Float64> matrix multiply 00 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), expected11, "mat3<Simd256Float64> matrix multiply 11 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), expected22, "mat3<Simd256Float64> matrix multiply 22 lane " + std::to_string(lane));
	}

	// Matrix-vector multiplication supports column and row vector forms.
	mt::vec3<mt::Simd256Float64> vector_actual = a * mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(2.0), mt::Simd256Float64(3.0), mt::Simd256Float64(4.0));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), a00.element(lane) * 2.0 + a01.element(lane) * 3.0 + a02.element(lane) * 4.0, "mat3<Simd256Float64> matrix vector x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), a10.element(lane) * 2.0 + a11.element(lane) * 3.0 + a12.element(lane) * 4.0, "mat3<Simd256Float64> matrix vector y lane " + std::to_string(lane));
		test.check_equal(vector_actual.z.element(lane), a20.element(lane) * 2.0 + a21.element(lane) * 3.0 + a22.element(lane) * 4.0, "mat3<Simd256Float64> matrix vector z lane " + std::to_string(lane));
	}

	vector_actual = mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(2.0), mt::Simd256Float64(3.0), mt::Simd256Float64(4.0)) * a;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), 2.0 * a00.element(lane) + 3.0 * a10.element(lane) + 4.0 * a20.element(lane), "mat3<Simd256Float64> vector matrix x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), 2.0 * a01.element(lane) + 3.0 * a11.element(lane) + 4.0 * a21.element(lane), "mat3<Simd256Float64> vector matrix y lane " + std::to_string(lane));
		test.check_equal(vector_actual.z.element(lane), 2.0 * a02.element(lane) + 3.0 * a12.element(lane) + 4.0 * a22.element(lane), "mat3<Simd256Float64> vector matrix z lane " + std::to_string(lane));
	}

	// Transpose operates lane-wise.
	actual = mt::transpose(a);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane), "mat3<Simd256Float64> transpose 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a10.element(lane), "mat3<Simd256Float64> transpose 01 lane " + std::to_string(lane));
		test.check_equal(actual(0, 2).element(lane), a20.element(lane), "mat3<Simd256Float64> transpose 02 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a01.element(lane), "mat3<Simd256Float64> transpose 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane), "mat3<Simd256Float64> transpose 11 lane " + std::to_string(lane));
		test.check_equal(actual(1, 2).element(lane), a21.element(lane), "mat3<Simd256Float64> transpose 12 lane " + std::to_string(lane));
		test.check_equal(actual(2, 0).element(lane), a02.element(lane), "mat3<Simd256Float64> transpose 20 lane " + std::to_string(lane));
		test.check_equal(actual(2, 1).element(lane), a12.element(lane), "mat3<Simd256Float64> transpose 21 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), a22.element(lane), "mat3<Simd256Float64> transpose 22 lane " + std::to_string(lane));
	}

	// Determinant and inverse use the standard 3x3 formulas.
	mt::mat3<mt::Simd256Float64> invertible(mt::Simd256Float64(1.0), mt::Simd256Float64(2.0), mt::Simd256Float64(3.0), mt::Simd256Float64(0.0), mt::Simd256Float64(1.0), mt::Simd256Float64(4.0), mt::Simd256Float64(5.0), mt::Simd256Float64(6.0), mt::Simd256Float64(0.0));
	mt::Simd256Float64 determinant_actual = mt::determinant(invertible);
	actual = mt::inverse(invertible);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(determinant_actual.element(lane), 1.0, "mat3<Simd256Float64> determinant lane " + std::to_string(lane));
		test.check_close(actual(0, 0).element(lane), -24.0, "mat3<Simd256Float64> inverse 00 lane " + std::to_string(lane));
		test.check_close(actual(0, 1).element(lane), 18.0, "mat3<Simd256Float64> inverse 01 lane " + std::to_string(lane));
		test.check_close(actual(0, 2).element(lane), 5.0, "mat3<Simd256Float64> inverse 02 lane " + std::to_string(lane));
		test.check_close(actual(1, 0).element(lane), 20.0, "mat3<Simd256Float64> inverse 10 lane " + std::to_string(lane));
		test.check_close(actual(1, 1).element(lane), -15.0, "mat3<Simd256Float64> inverse 11 lane " + std::to_string(lane));
		test.check_close(actual(1, 2).element(lane), -4.0, "mat3<Simd256Float64> inverse 12 lane " + std::to_string(lane));
		test.check_close(actual(2, 0).element(lane), -5.0, "mat3<Simd256Float64> inverse 20 lane " + std::to_string(lane));
		test.check_close(actual(2, 1).element(lane), 4.0, "mat3<Simd256Float64> inverse 21 lane " + std::to_string(lane));
		test.check_close(actual(2, 2).element(lane), 1.0, "mat3<Simd256Float64> inverse 22 lane " + std::to_string(lane));
	}
}

int main() {
	TestContext test;

	test_mat3_simd256_double(test);

	return test.result();
}
