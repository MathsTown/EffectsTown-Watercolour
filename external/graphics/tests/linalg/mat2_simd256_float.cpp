/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 mat2<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include "simd-f32.h"

#include "linalg/mat2.h"

void test_mat2_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "mat2 Simd256Float32 compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "mat2 Simd256Float32 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float32 a00(0.0f);
	mt::Simd256Float32 a01(0.0f);
	mt::Simd256Float32 a10(0.0f);
	mt::Simd256Float32 a11(0.0f);
	mt::Simd256Float32 b00(0.0f);
	mt::Simd256Float32 b01(0.0f);
	mt::Simd256Float32 b10(0.0f);
	mt::Simd256Float32 b11(0.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		a00.set_element(lane, float(lane + 1));
		a01.set_element(lane, float(lane + 2));
		a10.set_element(lane, float(lane + 3));
		a11.set_element(lane, float(lane + 4));
		b00.set_element(lane, float(lane + 5));
		b01.set_element(lane, float(lane + 6));
		b10.set_element(lane, float(lane + 7));
		b11.set_element(lane, float(lane + 8));
	}

	mt::mat2<mt::Simd256Float32> a(a00, a01, a10, a11);
	mt::mat2<mt::Simd256Float32> b(b00, b01, b10, b11);

	// Constructor and element access use logical row-major order.
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(a(0, 0).element(lane), a00.element(lane), "mat2<Simd256Float32> constructor 00 lane " + std::to_string(lane));
		test.check_equal(a(0, 1).element(lane), a01.element(lane), "mat2<Simd256Float32> constructor 01 lane " + std::to_string(lane));
		test.check_equal(a(1, 0).element(lane), a10.element(lane), "mat2<Simd256Float32> constructor 10 lane " + std::to_string(lane));
		test.check_equal(a(1, 1).element(lane), a11.element(lane), "mat2<Simd256Float32> constructor 11 lane " + std::to_string(lane));
	}

	// Diagonal constructor creates a lane-wise diagonal matrix.
	mt::mat2<mt::Simd256Float32> diagonal(mt::Simd256Float32(2.0f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(diagonal(0, 0).element(lane), 2.0f, "mat2<Simd256Float32> diagonal 00 lane " + std::to_string(lane));
		test.check_equal(diagonal(0, 1).element(lane), 0.0f, "mat2<Simd256Float32> diagonal 01 lane " + std::to_string(lane));
		test.check_equal(diagonal(1, 0).element(lane), 0.0f, "mat2<Simd256Float32> diagonal 10 lane " + std::to_string(lane));
		test.check_equal(diagonal(1, 1).element(lane), 2.0f, "mat2<Simd256Float32> diagonal 11 lane " + std::to_string(lane));
	}

	// Column and row access expose the expected lane-wise vectors.
	mt::vec2<mt::Simd256Float32> column = a.column(1);
	mt::vec2<mt::Simd256Float32> row = a.row(1);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(column.x.element(lane), a01.element(lane), "mat2<Simd256Float32> column x lane " + std::to_string(lane));
		test.check_equal(column.y.element(lane), a11.element(lane), "mat2<Simd256Float32> column y lane " + std::to_string(lane));
		test.check_equal(row.x.element(lane), a10.element(lane), "mat2<Simd256Float32> row x lane " + std::to_string(lane));
		test.check_equal(row.y.element(lane), a11.element(lane), "mat2<Simd256Float32> row y lane " + std::to_string(lane));
	}

	// Scalar multiplication works on both sides.
	mt::mat2<mt::Simd256Float32> actual = a * 2.0f;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0f, "mat2<Simd256Float32> matrix times scalar 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a01.element(lane) * 2.0f, "mat2<Simd256Float32> matrix times scalar 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a10.element(lane) * 2.0f, "mat2<Simd256Float32> matrix times scalar 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0f, "mat2<Simd256Float32> matrix times scalar 11 lane " + std::to_string(lane));
	}

	actual = 2.0f * a;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0f, "mat2<Simd256Float32> scalar times matrix 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a01.element(lane) * 2.0f, "mat2<Simd256Float32> scalar times matrix 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a10.element(lane) * 2.0f, "mat2<Simd256Float32> scalar times matrix 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0f, "mat2<Simd256Float32> scalar times matrix 11 lane " + std::to_string(lane));
	}

	// Matrix multiplication is lane-wise standard matrix multiplication.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		const float expected00 = a00.element(lane) * b00.element(lane) + a01.element(lane) * b10.element(lane);
		const float expected01 = a00.element(lane) * b01.element(lane) + a01.element(lane) * b11.element(lane);
		const float expected10 = a10.element(lane) * b00.element(lane) + a11.element(lane) * b10.element(lane);
		const float expected11 = a10.element(lane) * b01.element(lane) + a11.element(lane) * b11.element(lane);
		test.check_equal(actual(0, 0).element(lane), expected00, "mat2<Simd256Float32> matrix multiply 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), expected01, "mat2<Simd256Float32> matrix multiply 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), expected10, "mat2<Simd256Float32> matrix multiply 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), expected11, "mat2<Simd256Float32> matrix multiply 11 lane " + std::to_string(lane));
	}

	// Matrix times vector treats the vector as a column vector.
	mt::vec2<mt::Simd256Float32> vector_actual = a * mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(3.0f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), a00.element(lane) * 2.0f + a01.element(lane) * 3.0f, "mat2<Simd256Float32> matrix vector x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), a10.element(lane) * 2.0f + a11.element(lane) * 3.0f, "mat2<Simd256Float32> matrix vector y lane " + std::to_string(lane));
	}

	// Vector times matrix treats the vector as a row vector.
	vector_actual = mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(3.0f)) * a;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), 2.0f * a00.element(lane) + 3.0f * a10.element(lane), "mat2<Simd256Float32> vector matrix x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), 2.0f * a01.element(lane) + 3.0f * a11.element(lane), "mat2<Simd256Float32> vector matrix y lane " + std::to_string(lane));
	}

	// Determinant and transpose operate lane-wise.
	mt::Simd256Float32 determinant_actual = mt::determinant(a);
	actual = mt::transpose(a);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		const float expected_determinant = a00.element(lane) * a11.element(lane) - a01.element(lane) * a10.element(lane);
		test.check_equal(determinant_actual.element(lane), expected_determinant, "mat2<Simd256Float32> determinant lane " + std::to_string(lane));
		test.check_equal(actual(0, 0).element(lane), a00.element(lane), "mat2<Simd256Float32> transpose 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a10.element(lane), "mat2<Simd256Float32> transpose 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a01.element(lane), "mat2<Simd256Float32> transpose 10 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane), "mat2<Simd256Float32> transpose 11 lane " + std::to_string(lane));
	}

	// Inverse uses the standard 2x2 adjugate divided by determinant.
	mt::mat2<mt::Simd256Float32> invertible(mt::Simd256Float32(4.0f), mt::Simd256Float32(7.0f), mt::Simd256Float32(2.0f), mt::Simd256Float32(6.0f));
	actual = mt::inverse(invertible);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual(0, 0).element(lane), 0.6f, "mat2<Simd256Float32> inverse 00 lane " + std::to_string(lane));
		test.check_close(actual(0, 1).element(lane), -0.7f, "mat2<Simd256Float32> inverse 01 lane " + std::to_string(lane));
		test.check_close(actual(1, 0).element(lane), -0.2f, "mat2<Simd256Float32> inverse 10 lane " + std::to_string(lane));
		test.check_close(actual(1, 1).element(lane), 0.4f, "mat2<Simd256Float32> inverse 11 lane " + std::to_string(lane));
	}
}

int main() {
	TestContext test;

	test_mat2_simd256_float(test);

	return test.result();
}
