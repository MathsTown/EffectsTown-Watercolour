/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Oklab<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/oklab_reference.h"

namespace {

mt::Xyz<mt::Simd256Float32> xyz_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 x(0.0f);
	mt::Simd256Float32 y(0.0f);
	mt::Simd256Float32 z(0.0f);
	mt::Simd256Float32 a(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		x.set_element(lane, static_cast<float>(cases[case_index].source[0]));
		y.set_element(lane, static_cast<float>(cases[case_index].source[1]));
		z.set_element(lane, static_cast<float>(cases[case_index].source[2]));
		a.set_element(lane, static_cast<float>(cases[case_index].source[3]));
	}

	return mt::Xyz<mt::Simd256Float32>(x, y, z, a);
}

mt::Oklab<mt::Simd256Float32> oklab_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 l(0.0f);
	mt::Simd256Float32 a(0.0f);
	mt::Simd256Float32 b(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		l.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		a.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		b.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::Oklab<mt::Simd256Float32>(l, a, b, alpha);
}

} // namespace

void test_oklab_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Oklab<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Oklab<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Oklab<mt::Simd256Float32> laba(mt::Simd256Float32(0.1f), mt::Simd256Float32(-0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(laba.lightness.element(lane), 0.1f, "Oklab<Simd256Float32> laba l lane " + std::to_string(lane));
		test.check_equal(laba.a.element(lane), -0.2f, "Oklab<Simd256Float32> laba a lane " + std::to_string(lane));
		test.check_equal(laba.b.element(lane), 0.3f, "Oklab<Simd256Float32> laba b lane " + std::to_string(lane));
		test.check_equal(laba.alpha.element(lane), 0.4f, "Oklab<Simd256Float32> laba alpha lane " + std::to_string(lane));
	}

	// Clamp restricts bounded components to the 0..1 range.
	mt::Oklab<mt::Simd256Float32> clamped = mt::Oklab<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(-0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.lightness.element(lane), 0.0f, "Oklab<Simd256Float32> clamp l lane " + std::to_string(lane));
		test.check_equal(clamped.a.element(lane), -0.5f, "Oklab<Simd256Float32> clamp a lane " + std::to_string(lane));
		test.check_equal(clamped.b.element(lane), 1.25f, "Oklab<Simd256Float32> clamp b lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Oklab<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Oklab<mt::Simd256Float32> no_negatives = mt::Oklab<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(-0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.lightness.element(lane), 0.0f, "Oklab<Simd256Float32> no_negatives l lane " + std::to_string(lane));
		test.check_equal(no_negatives.a.element(lane), -0.5f, "Oklab<Simd256Float32> no_negatives a lane " + std::to_string(lane));
		test.check_equal(no_negatives.b.element(lane), 1.25f, "Oklab<Simd256Float32> no_negatives b lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Oklab<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Oklab<mt::Simd256Float32> sanitised = mt::Oklab<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.lightness.element(lane), 0.0f, "Oklab<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.a.element(lane), 0.0f, "Oklab<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.b.element(lane), std::numeric_limits<float>::max(), "Oklab<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Oklab<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks CIE XYZ D65 to Oklab.
	const int oklab_case_count = int(sizeof(colour_reference::xyz_d65_oklab_cases) / sizeof(colour_reference::xyz_d65_oklab_cases[0]));
	for (int first_case = 0; first_case < oklab_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Oklab<mt::Simd256Float32> actual = xyz_batch(colour_reference::xyz_d65_oklab_cases, first_case, oklab_case_count).to_oklab();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < oklab_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_oklab_cases[first_case + lane];
			std::string message = std::string("Oklab<Simd256Float32> from Xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 5.0e-5f, message + " l");
			test.check_close(actual.a.element(lane), colour_case.expected[1], 5.0e-5f, message + " a");
			test.check_close(actual.b.element(lane), colour_case.expected[2], 5.0e-5f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 5.0e-5f, message + " alpha");
		}
	}

	// Reference data checks Oklab back to CIE XYZ D65.
	for (int first_case = 0; first_case < oklab_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Xyz<mt::Simd256Float32> actual = oklab_batch(colour_reference::xyz_d65_oklab_cases, first_case, oklab_case_count).to_xyz();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < oklab_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_oklab_cases[first_case + lane];
			std::string message = std::string("Oklab<Simd256Float32> to Xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.source[0], 8.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.source[1], 8.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.source[2], 8.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 8.0e-5f, message + " a");
		}
	}

	// Reference data checks Oklab to Oklch.
	for (int first_case = 0; first_case < oklab_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Oklch<mt::Simd256Float32> actual = oklab_batch(colour_reference::xyz_d65_oklab_cases, first_case, oklab_case_count).to_oklch();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < oklab_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_oklch_cases[first_case + lane];
			std::string message = std::string("Oklab<Simd256Float32> to Oklch ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 5.0e-5f, message + " l");
			test.check_close(actual.chroma.element(lane), colour_case.expected[1], 5.0e-5f, message + " c");
			if (colour_case.expected[1] > 1.0e-4) {
				test.check_close_hue(actual.hue.element(lane), colour_case.expected[2], 5.0e-4f, message + " h");
			}
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 5.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_oklab_simd256_float(test);

	return test.result();
}
