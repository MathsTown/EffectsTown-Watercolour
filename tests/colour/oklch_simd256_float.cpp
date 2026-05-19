/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Oklch<Simd256Float32> tests.
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

mt::Oklch<mt::Simd256Float32> oklch_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 l(0.0f);
	mt::Simd256Float32 c(0.0f);
	mt::Simd256Float32 h(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		l.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		c.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		h.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::Oklch<mt::Simd256Float32>(l, c, h, alpha);
}

} // namespace

void test_oklch_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Oklch<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Oklch<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Oklch<mt::Simd256Float32> lcha(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(lcha.lightness.element(lane), 0.1f, "Oklch<Simd256Float32> lcha l lane " + std::to_string(lane));
		test.check_equal(lcha.chroma.element(lane), 0.2f, "Oklch<Simd256Float32> lcha c lane " + std::to_string(lane));
		test.check_equal(lcha.hue.element(lane), 0.3f, "Oklch<Simd256Float32> lcha h lane " + std::to_string(lane));
		test.check_equal(lcha.alpha.element(lane), 0.4f, "Oklch<Simd256Float32> lcha alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::Oklch<mt::Simd256Float32> clamped = mt::Oklch<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.lightness.element(lane), 0.0f, "Oklch<Simd256Float32> clamp l lane " + std::to_string(lane));
		test.check_equal(clamped.chroma.element(lane), 0.5f, "Oklch<Simd256Float32> clamp c lane " + std::to_string(lane));
		test.check_equal(clamped.hue.element(lane), 1.0f, "Oklch<Simd256Float32> clamp h lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Oklch<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Oklch<mt::Simd256Float32> no_negatives = mt::Oklch<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(-0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.lightness.element(lane), 0.0f, "Oklch<Simd256Float32> no_negatives l lane " + std::to_string(lane));
		test.check_equal(no_negatives.chroma.element(lane), 0.0f, "Oklch<Simd256Float32> no_negatives c lane " + std::to_string(lane));
		test.check_equal(no_negatives.hue.element(lane), 1.25f, "Oklch<Simd256Float32> no_negatives h lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Oklch<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Oklch<mt::Simd256Float32> sanitised = mt::Oklch<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.lightness.element(lane), 0.0f, "Oklch<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.chroma.element(lane), 0.0f, "Oklch<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.hue.element(lane), std::numeric_limits<float>::max(), "Oklch<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Oklch<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Oklch<mt::Simd256Float32> wrapped_negative = mt::Oklch<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(-0.25f), mt::Simd256Float32(1.0f)).wrap_hue();
	mt::Oklch<mt::Simd256Float32> wrapped_over_one = mt::Oklch<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(1.0f)).wrap_hue();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(wrapped_negative.lightness.element(lane), 0.25f, "Oklch<Simd256Float32> wrap negative l lane " + std::to_string(lane));
		test.check_equal(wrapped_negative.chroma.element(lane), 0.5f, "Oklch<Simd256Float32> wrap negative c lane " + std::to_string(lane));
		test.check_equal(wrapped_negative.hue.element(lane), 0.75f, "Oklch<Simd256Float32> wrap negative h lane " + std::to_string(lane));
		test.check_equal(wrapped_negative.alpha.element(lane), 1.0f, "Oklch<Simd256Float32> wrap negative alpha lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.lightness.element(lane), 0.25f, "Oklch<Simd256Float32> wrap over-one l lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.chroma.element(lane), 0.5f, "Oklch<Simd256Float32> wrap over-one c lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.hue.element(lane), 0.25f, "Oklch<Simd256Float32> wrap over-one h lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.alpha.element(lane), 1.0f, "Oklch<Simd256Float32> wrap over-one alpha lane " + std::to_string(lane));
	}

	// Reference data checks CIE XYZ D65 to Oklch.
	const int oklch_case_count = int(sizeof(colour_reference::xyz_d65_oklch_cases) / sizeof(colour_reference::xyz_d65_oklch_cases[0]));
	for (int first_case = 0; first_case < oklch_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Oklch<mt::Simd256Float32> actual = xyz_batch(colour_reference::xyz_d65_oklch_cases, first_case, oklch_case_count).to_oklch();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < oklch_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_oklch_cases[first_case + lane];
			std::string message = std::string("Oklch<Simd256Float32> from Xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 5.0e-5f, message + " l");
			test.check_close(actual.chroma.element(lane), colour_case.expected[1], 5.0e-5f, message + " c");
			if (colour_case.expected[1] > 1.0e-4) {
				test.check_close_hue(actual.hue.element(lane), colour_case.expected[2], 5.0e-4f, message + " h");
			}
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 5.0e-5f, message + " alpha");
		}
	}

	// Reference data checks Oklch back to CIE XYZ D65.
	for (int first_case = 0; first_case < oklch_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Xyz<mt::Simd256Float32> actual = oklch_batch(colour_reference::xyz_d65_oklch_cases, first_case, oklch_case_count).to_xyz();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < oklch_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_oklch_cases[first_case + lane];
			std::string message = std::string("Oklch<Simd256Float32> to Xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.source[0], 8.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.source[1], 8.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.source[2], 8.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 8.0e-5f, message + " a");
		}
	}

	// Reference data checks Oklch to Oklab.
	for (int first_case = 0; first_case < oklch_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Oklab<mt::Simd256Float32> actual = oklch_batch(colour_reference::xyz_d65_oklch_cases, first_case, oklch_case_count).to_oklab();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < oklch_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_oklab_cases[first_case + lane];
			std::string message = std::string("Oklch<Simd256Float32> to Oklab ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 5.0e-5f, message + " l");
			test.check_close(actual.a.element(lane), colour_case.expected[1], 5.0e-5f, message + " a");
			test.check_close(actual.b.element(lane), colour_case.expected[2], 5.0e-5f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 5.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_oklch_simd256_float(test);

	return test.result();
}
