/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 mat4<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include "simd-f32.h"

#include "linalg/mat4.h"

void test_mat4_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "mat4 Simd256Float32 compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "mat4 Simd256Float32 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float32 a00(0.0f);
	mt::Simd256Float32 a01(0.0f);
	mt::Simd256Float32 a02(0.0f);
	mt::Simd256Float32 a03(0.0f);
	mt::Simd256Float32 a10(0.0f);
	mt::Simd256Float32 a11(0.0f);
	mt::Simd256Float32 a12(0.0f);
	mt::Simd256Float32 a13(0.0f);
	mt::Simd256Float32 a20(0.0f);
	mt::Simd256Float32 a21(0.0f);
	mt::Simd256Float32 a22(0.0f);
	mt::Simd256Float32 a23(0.0f);
	mt::Simd256Float32 a30(0.0f);
	mt::Simd256Float32 a31(0.0f);
	mt::Simd256Float32 a32(0.0f);
	mt::Simd256Float32 a33(0.0f);
	mt::Simd256Float32 b00(0.0f);
	mt::Simd256Float32 b01(0.0f);
	mt::Simd256Float32 b02(0.0f);
	mt::Simd256Float32 b03(0.0f);
	mt::Simd256Float32 b10(0.0f);
	mt::Simd256Float32 b11(0.0f);
	mt::Simd256Float32 b12(0.0f);
	mt::Simd256Float32 b13(0.0f);
	mt::Simd256Float32 b20(0.0f);
	mt::Simd256Float32 b21(0.0f);
	mt::Simd256Float32 b22(0.0f);
	mt::Simd256Float32 b23(0.0f);
	mt::Simd256Float32 b30(0.0f);
	mt::Simd256Float32 b31(0.0f);
	mt::Simd256Float32 b32(0.0f);
	mt::Simd256Float32 b33(0.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		a00.set_element(lane, float(lane + 1));
		a01.set_element(lane, float(lane + 2));
		a02.set_element(lane, float(lane + 3));
		a03.set_element(lane, float(lane + 4));
		a10.set_element(lane, float(lane + 5));
		a11.set_element(lane, float(lane + 6));
		a12.set_element(lane, float(lane + 7));
		a13.set_element(lane, float(lane + 8));
		a20.set_element(lane, float(lane + 9));
		a21.set_element(lane, float(lane + 10));
		a22.set_element(lane, float(lane + 11));
		a23.set_element(lane, float(lane + 12));
		a30.set_element(lane, float(lane + 13));
		a31.set_element(lane, float(lane + 14));
		a32.set_element(lane, float(lane + 15));
		a33.set_element(lane, float(lane + 16));
		b00.set_element(lane, float(lane + 17));
		b01.set_element(lane, float(lane + 18));
		b02.set_element(lane, float(lane + 19));
		b03.set_element(lane, float(lane + 20));
		b10.set_element(lane, float(lane + 21));
		b11.set_element(lane, float(lane + 22));
		b12.set_element(lane, float(lane + 23));
		b13.set_element(lane, float(lane + 24));
		b20.set_element(lane, float(lane + 25));
		b21.set_element(lane, float(lane + 26));
		b22.set_element(lane, float(lane + 27));
		b23.set_element(lane, float(lane + 28));
		b30.set_element(lane, float(lane + 29));
		b31.set_element(lane, float(lane + 30));
		b32.set_element(lane, float(lane + 31));
		b33.set_element(lane, float(lane + 32));
	}

	mt::mat4<mt::Simd256Float32> a(a00, a01, a02, a03, a10, a11, a12, a13, a20, a21, a22, a23, a30, a31, a32, a33);
	mt::mat4<mt::Simd256Float32> b(b00, b01, b02, b03, b10, b11, b12, b13, b20, b21, b22, b23, b30, b31, b32, b33);

	// Constructor and element access use logical row-major order.
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(a(0, 0).element(lane), a00.element(lane), "mat4<Simd256Float32> constructor 00 lane " + std::to_string(lane));
		test.check_equal(a(1, 3).element(lane), a13.element(lane), "mat4<Simd256Float32> constructor 13 lane " + std::to_string(lane));
		test.check_equal(a(3, 3).element(lane), a33.element(lane), "mat4<Simd256Float32> constructor 33 lane " + std::to_string(lane));
	}

	// Diagonal constructor creates a lane-wise diagonal matrix.
	mt::mat4<mt::Simd256Float32> diagonal(mt::Simd256Float32(2.0f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(diagonal(0, 0).element(lane), 2.0f, "mat4<Simd256Float32> diagonal 00 lane " + std::to_string(lane));
		test.check_equal(diagonal(1, 1).element(lane), 2.0f, "mat4<Simd256Float32> diagonal 11 lane " + std::to_string(lane));
		test.check_equal(diagonal(2, 2).element(lane), 2.0f, "mat4<Simd256Float32> diagonal 22 lane " + std::to_string(lane));
		test.check_equal(diagonal(3, 3).element(lane), 2.0f, "mat4<Simd256Float32> diagonal 33 lane " + std::to_string(lane));
		test.check_equal(diagonal(0, 1).element(lane), 0.0f, "mat4<Simd256Float32> diagonal off lane " + std::to_string(lane));
	}

	// Column and row access expose the expected lane-wise vectors.
	mt::vec4<mt::Simd256Float32> column = a.column(3);
	mt::vec4<mt::Simd256Float32> row = a.row(2);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(column.x.element(lane), a03.element(lane), "mat4<Simd256Float32> column x lane " + std::to_string(lane));
		test.check_equal(column.y.element(lane), a13.element(lane), "mat4<Simd256Float32> column y lane " + std::to_string(lane));
		test.check_equal(column.z.element(lane), a23.element(lane), "mat4<Simd256Float32> column z lane " + std::to_string(lane));
		test.check_equal(column.w.element(lane), a33.element(lane), "mat4<Simd256Float32> column w lane " + std::to_string(lane));
		test.check_equal(row.x.element(lane), a20.element(lane), "mat4<Simd256Float32> row x lane " + std::to_string(lane));
		test.check_equal(row.y.element(lane), a21.element(lane), "mat4<Simd256Float32> row y lane " + std::to_string(lane));
		test.check_equal(row.z.element(lane), a22.element(lane), "mat4<Simd256Float32> row z lane " + std::to_string(lane));
		test.check_equal(row.w.element(lane), a23.element(lane), "mat4<Simd256Float32> row w lane " + std::to_string(lane));
	}

	// Scalar multiplication works on both sides.
	mt::mat4<mt::Simd256Float32> actual = a * 2.0f;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0f, "mat4<Simd256Float32> matrix times scalar 00 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0f, "mat4<Simd256Float32> matrix times scalar 11 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), a22.element(lane) * 2.0f, "mat4<Simd256Float32> matrix times scalar 22 lane " + std::to_string(lane));
		test.check_equal(actual(3, 3).element(lane), a33.element(lane) * 2.0f, "mat4<Simd256Float32> matrix times scalar 33 lane " + std::to_string(lane));
	}

	actual = 2.0f * a;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane) * 2.0f, "mat4<Simd256Float32> scalar times matrix 00 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), a11.element(lane) * 2.0f, "mat4<Simd256Float32> scalar times matrix 11 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), a22.element(lane) * 2.0f, "mat4<Simd256Float32> scalar times matrix 22 lane " + std::to_string(lane));
		test.check_equal(actual(3, 3).element(lane), a33.element(lane) * 2.0f, "mat4<Simd256Float32> scalar times matrix 33 lane " + std::to_string(lane));
	}

	// Matrix multiplication is lane-wise standard matrix multiplication.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		const float expected00 = a00.element(lane) * b00.element(lane) + a01.element(lane) * b10.element(lane) + a02.element(lane) * b20.element(lane) + a03.element(lane) * b30.element(lane);
		const float expected11 = a10.element(lane) * b01.element(lane) + a11.element(lane) * b11.element(lane) + a12.element(lane) * b21.element(lane) + a13.element(lane) * b31.element(lane);
		const float expected22 = a20.element(lane) * b02.element(lane) + a21.element(lane) * b12.element(lane) + a22.element(lane) * b22.element(lane) + a23.element(lane) * b32.element(lane);
		const float expected33 = a30.element(lane) * b03.element(lane) + a31.element(lane) * b13.element(lane) + a32.element(lane) * b23.element(lane) + a33.element(lane) * b33.element(lane);
		test.check_equal(actual(0, 0).element(lane), expected00, "mat4<Simd256Float32> matrix multiply 00 lane " + std::to_string(lane));
		test.check_equal(actual(1, 1).element(lane), expected11, "mat4<Simd256Float32> matrix multiply 11 lane " + std::to_string(lane));
		test.check_equal(actual(2, 2).element(lane), expected22, "mat4<Simd256Float32> matrix multiply 22 lane " + std::to_string(lane));
		test.check_equal(actual(3, 3).element(lane), expected33, "mat4<Simd256Float32> matrix multiply 33 lane " + std::to_string(lane));
	}

	// Matrix-vector multiplication supports column and row vector forms.
	mt::vec4<mt::Simd256Float32> vector_actual = a * mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(3.0f), mt::Simd256Float32(4.0f), mt::Simd256Float32(5.0f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), a00.element(lane) * 2.0f + a01.element(lane) * 3.0f + a02.element(lane) * 4.0f + a03.element(lane) * 5.0f, "mat4<Simd256Float32> matrix vector x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), a10.element(lane) * 2.0f + a11.element(lane) * 3.0f + a12.element(lane) * 4.0f + a13.element(lane) * 5.0f, "mat4<Simd256Float32> matrix vector y lane " + std::to_string(lane));
		test.check_equal(vector_actual.z.element(lane), a20.element(lane) * 2.0f + a21.element(lane) * 3.0f + a22.element(lane) * 4.0f + a23.element(lane) * 5.0f, "mat4<Simd256Float32> matrix vector z lane " + std::to_string(lane));
		test.check_equal(vector_actual.w.element(lane), a30.element(lane) * 2.0f + a31.element(lane) * 3.0f + a32.element(lane) * 4.0f + a33.element(lane) * 5.0f, "mat4<Simd256Float32> matrix vector w lane " + std::to_string(lane));
	}

	vector_actual = mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(3.0f), mt::Simd256Float32(4.0f), mt::Simd256Float32(5.0f)) * a;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(vector_actual.x.element(lane), 2.0f * a00.element(lane) + 3.0f * a10.element(lane) + 4.0f * a20.element(lane) + 5.0f * a30.element(lane), "mat4<Simd256Float32> vector matrix x lane " + std::to_string(lane));
		test.check_equal(vector_actual.y.element(lane), 2.0f * a01.element(lane) + 3.0f * a11.element(lane) + 4.0f * a21.element(lane) + 5.0f * a31.element(lane), "mat4<Simd256Float32> vector matrix y lane " + std::to_string(lane));
		test.check_equal(vector_actual.z.element(lane), 2.0f * a02.element(lane) + 3.0f * a12.element(lane) + 4.0f * a22.element(lane) + 5.0f * a32.element(lane), "mat4<Simd256Float32> vector matrix z lane " + std::to_string(lane));
		test.check_equal(vector_actual.w.element(lane), 2.0f * a03.element(lane) + 3.0f * a13.element(lane) + 4.0f * a23.element(lane) + 5.0f * a33.element(lane), "mat4<Simd256Float32> vector matrix w lane " + std::to_string(lane));
	}

	// Transpose operates lane-wise.
	actual = mt::transpose(a);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual(0, 0).element(lane), a00.element(lane), "mat4<Simd256Float32> transpose 00 lane " + std::to_string(lane));
		test.check_equal(actual(0, 1).element(lane), a10.element(lane), "mat4<Simd256Float32> transpose 01 lane " + std::to_string(lane));
		test.check_equal(actual(1, 0).element(lane), a01.element(lane), "mat4<Simd256Float32> transpose 10 lane " + std::to_string(lane));
		test.check_equal(actual(3, 2).element(lane), a23.element(lane), "mat4<Simd256Float32> transpose 32 lane " + std::to_string(lane));
		test.check_equal(actual(3, 3).element(lane), a33.element(lane), "mat4<Simd256Float32> transpose 33 lane " + std::to_string(lane));
	}

	// Determinant and inverse use the standard 4x4 formulas.
	mt::mat4<mt::Simd256Float32> invertible(mt::Simd256Float32(1.0f), mt::Simd256Float32(2.0f), mt::Simd256Float32(3.0f), mt::Simd256Float32(4.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(4.0f), mt::Simd256Float32(5.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(6.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f));
	mt::Simd256Float32 determinant_actual = mt::determinant(invertible);
	actual = mt::inverse(invertible);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(determinant_actual.element(lane), 1.0f, "mat4<Simd256Float32> determinant lane " + std::to_string(lane));
		test.check_close(actual(0, 0).element(lane), 1.0f, "mat4<Simd256Float32> inverse 00 lane " + std::to_string(lane));
		test.check_close(actual(0, 1).element(lane), -2.0f, "mat4<Simd256Float32> inverse 01 lane " + std::to_string(lane));
		test.check_close(actual(0, 2).element(lane), 5.0f, "mat4<Simd256Float32> inverse 02 lane " + std::to_string(lane));
		test.check_close(actual(0, 3).element(lane), -24.0f, "mat4<Simd256Float32> inverse 03 lane " + std::to_string(lane));
		test.check_close(actual(1, 1).element(lane), 1.0f, "mat4<Simd256Float32> inverse 11 lane " + std::to_string(lane));
		test.check_close(actual(1, 2).element(lane), -4.0f, "mat4<Simd256Float32> inverse 12 lane " + std::to_string(lane));
		test.check_close(actual(1, 3).element(lane), 19.0f, "mat4<Simd256Float32> inverse 13 lane " + std::to_string(lane));
		test.check_close(actual(2, 2).element(lane), 1.0f, "mat4<Simd256Float32> inverse 22 lane " + std::to_string(lane));
		test.check_close(actual(2, 3).element(lane), -6.0f, "mat4<Simd256Float32> inverse 23 lane " + std::to_string(lane));
		test.check_close(actual(3, 3).element(lane), 1.0f, "mat4<Simd256Float32> inverse 33 lane " + std::to_string(lane));
	}
}

int main() {
	TestContext test;

	test_mat4_simd256_float(test);

	return test.result();
}
