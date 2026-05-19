/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 mat2<Simd256Float64> tests.
*********************************************************************************************************/

#include "../test.h"

#include "simd-uint32.h"
#include "simd-f64.h"

#include "linalg/mat2.h"

void test_mat2_simd256_double(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float64::compiler_supported(), "mat2 Simd256Float64 compiler support");
	test.check(mt::Simd256Float64::cpu_supported(cpu), "mat2 Simd256Float64 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float64 a00(0.0);
	mt::Simd256Float64 a01(0.0);
	mt::Simd256Float64 a10(0.0);
	mt::Simd256Float64 a11(0.0);
	mt::Simd256Float64 b00(0.0);
	mt::Simd256Float64 b01(0.0);
	mt::Simd256Float64 b10(0.0);
	mt::Simd256Float64 b11(0.0);

	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		a00.set_element(lane, double(lane + 1));
		a01.set_element(lane, double(lane + 2));
		a10.set_element(lane, double(lane + 3));
		a11.set_element(lane, double(lane + 4));
		b00.set_element(lane, double(lane + 5));
		b01.set_element(lane, double(lane + 6));
		b10.set_element(lane, double(lane + 7));
		b11.set_element(lane, double(lane + 8));
	}

	mt::mat2<mt::Simd256Float64> a(a00, a01, a10, a11);
	mt::mat2<mt::Simd256Float64> b(b00, b01, b10, b11);

	// Constructor and element access use logical row-major order.
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(a(0, 0).element(lane), a00.element(lane), "mat2<Simd256Float64> constructor 00 lane " + std::to_string(lane));
		test.check_equal(a(0, 1).element(lane), a01.element(lane), "mat2<Simd256Float64> constructor 01 lane " + std::to_string(lane));
		test.check_equal(a(1, 0).element(lane), a10.element(lane), "mat2<Simd256Float64> constructor 10 lane " + std::to_string(lane));
		test.check_equal(a(1, 1).element(lane), a11.element(lane), "mat2<Simd256Float64> constructor 11 lane " + std::to_string(lane));
	}

	// Column and row access expose the expected lane-wise vectors.
	mt::vec2<mt::Simd256Float64> column = a.column(1);
	mt::vec2<mt::Simd256Float64> row = a.row(1);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(column.x.element(lane), a01.element(lane), "mat2<Simd256Float64> column x lane " + std::to_string(lane));
		test.check_equal(column.y.element(lane), a11.element(lane), "mat2<Simd256Float64> column y lane " + std::to_string(lane));
		test.check_equal(row.x.element(lane), a10.element(lane), "mat2<Simd256Float64> row x lane " + std::to_string(lane));
		test.check_equal(row.y.element(lane), a11.element(lane), "mat2<Simd256Float64> row y lane " + std::to_string(lane));
	}

	// Scalar multiplication works on both sides.
	mt::mat2<mt::Simd256Float64> actual = a * 2.0;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0, "mat2<Simd256Float64> matrix times scalar 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a01.element(lane) * 2.0, "mat2<Simd256Float64> matrix times scalar 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a10.element(lane) * 2.0, "mat2<Simd256Float64> matrix times scalar 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0, "mat2<Simd256Float64> matrix times scalar 11 lane " + std::to_string(lane));
	}

	actual = 2.0 * a;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0, "mat2<Simd256Float64> scalar times matrix 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a01.element(lane) * 2.0, "mat2<Simd256Float64> scalar times matrix 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a10.element(lane) * 2.0, "mat2<Simd256Float64> scalar times matrix 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0, "mat2<Simd256Float64> scalar times matrix 11 lane " + std::to_string(lane));
	}

	// Matrix multiplication is lane-wise standard matrix multiplication.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		const double expected00 = a00.element(lane) * b00.element(lane) + a01.element(lane) * b10.element(lane);
		const double expected01 = a00.element(lane) * b01.element(lane) + a01.element(lane) * b11.element(lane);
		const double expected10 = a10.element(lane) * b00.element(lane) + a11.element(lane) * b10.element(lane);
		const double expected11 = a10.element(lane) * b01.element(lane) + a11.element(lane) * b11.element(lane);
		test.check_equal(actual(0, 0).element(lane), expected00, "mat2<Simd256Float64> matrix multiply 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), expected01, "mat2<Simd256Float64> matrix multiply 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), expected10, "mat2<Simd256Float64> matrix multiply 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), expected11, "mat2<Simd256Float64> matrix multiply 11 lane " + std::to_string(lane));
	}

	// Matrix-vector multiplication supports column and row vector forms.
	mt::vec2<mt::Simd256Float64> vector_actual = a * mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(2.0), mt::Simd256Float64(3.0));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), a00.element(lane) * 2.0 + a01.element(lane) * 3.0, "mat2<Simd256Float64> matrix vector x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), a10.element(lane) * 2.0 + a11.element(lane) * 3.0, "mat2<Simd256Float64> matrix vector y lane " + std::to_string(lane));
	}

	vector_actual = mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(2.0), mt::Simd256Float64(3.0)) * a;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), 2.0 * a00.element(lane) + 3.0 * a10.element(lane), "mat2<Simd256Float64> vector matrix x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), 2.0 * a01.element(lane) + 3.0 * a11.element(lane), "mat2<Simd256Float64> vector matrix y lane " + std::to_string(lane));
	}

	// Determinant, transpose, and inverse operate lane-wise.
	mt::Simd256Float64 determinant_actual = mt::determinant(a);
	actual = mt::transpose(a);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		const double expected_determinant = a00.element(lane) * a11.element(lane) - a01.element(lane) * a10.element(lane);
		test.check_equal(determinant_actual.element(lane), expected_determinant, "mat2<Simd256Float64> determinant lane " + std::to_string(lane));
		test.check_equal(actual(0, 0).element(lane), a00.element(lane), "mat2<Simd256Float64> transpose 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a10.element(lane), "mat2<Simd256Float64> transpose 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a01.element(lane), "mat2<Simd256Float64> transpose 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane), "mat2<Simd256Float64> transpose 11 lane " + std::to_string(lane));
	}

	mt::mat2<mt::Simd256Float64> invertible(mt::Simd256Float64(4.0), mt::Simd256Float64(7.0), mt::Simd256Float64(2.0), mt::Simd256Float64(6.0));
	actual = mt::inverse(invertible);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_close(actual(0, 0).element(lane), 0.6, "mat2<Simd256Float64> inverse 00 lane " + std::to_string(lane));
		test.check_close(actual(0, 1).element(lane), -0.7, "mat2<Simd256Float64> inverse 01 lane " + std::to_string(lane));
		test.check_close(actual(1, 0).element(lane), -0.2, "mat2<Simd256Float64> inverse 10 lane " + std::to_string(lane));
		test.check_close(actual(1, 1).element(lane), 0.4, "mat2<Simd256Float64> inverse 11 lane " + std::to_string(lane));
	}
}

int main() {
	TestContext test;

	test_mat2_simd256_double(test);

	return test.result();
}
