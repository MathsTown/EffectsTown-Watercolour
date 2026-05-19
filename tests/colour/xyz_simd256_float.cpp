/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Xyz<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/xyz.h"

void test_xyz_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Xyz<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Xyz<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Xyz<mt::Simd256Float32> xyza(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(xyza.x.element(lane), 0.1f, "Xyz<Simd256Float32> xyza x lane " + std::to_string(lane));
		test.check_equal(xyza.y.element(lane), 0.2f, "Xyz<Simd256Float32> xyza y lane " + std::to_string(lane));
		test.check_equal(xyza.z.element(lane), 0.3f, "Xyz<Simd256Float32> xyza z lane " + std::to_string(lane));
		test.check_equal(xyza.alpha.element(lane), 0.4f, "Xyz<Simd256Float32> xyza alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::Xyz<mt::Simd256Float32> clamped = mt::Xyz<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.x.element(lane), 0.0f, "Xyz<Simd256Float32> clamp x lane " + std::to_string(lane));
		test.check_equal(clamped.y.element(lane), 0.5f, "Xyz<Simd256Float32> clamp y lane " + std::to_string(lane));
		test.check_equal(clamped.z.element(lane), 1.0f, "Xyz<Simd256Float32> clamp z lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Xyz<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Xyz<mt::Simd256Float32> no_negatives = mt::Xyz<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.x.element(lane), 0.0f, "Xyz<Simd256Float32> no_negatives x lane " + std::to_string(lane));
		test.check_equal(no_negatives.y.element(lane), 0.5f, "Xyz<Simd256Float32> no_negatives y lane " + std::to_string(lane));
		test.check_equal(no_negatives.z.element(lane), 1.25f, "Xyz<Simd256Float32> no_negatives z lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Xyz<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Xyz<mt::Simd256Float32> sanitised = mt::Xyz<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.x.element(lane), 0.0f, "Xyz<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.y.element(lane), 0.0f, "Xyz<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.z.element(lane), std::numeric_limits<float>::max(), "Xyz<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Xyz<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}
}

int main() {
	TestContext test;

	test_xyz_simd256_float(test);

	return test.result();
}
