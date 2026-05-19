/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Cielch<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/cielab_reference.h"

namespace {

mt::XyzD50<mt::Simd256Float32> xyz_d50_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 x(0.0f);
	mt::Simd256Float32 y(0.0f);
	mt::Simd256Float32 z(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		x.set_element(lane, static_cast<float>(cases[case_index].source[0]));
		y.set_element(lane, static_cast<float>(cases[case_index].source[1]));
		z.set_element(lane, static_cast<float>(cases[case_index].source[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].source[3]));
	}

	return mt::XyzD50<mt::Simd256Float32>(x, y, z, alpha);
}

mt::Cielch<mt::Simd256Float32> cielch_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
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

	return mt::Cielch<mt::Simd256Float32>(l, c, h, alpha);
}

} // namespace

void test_cielch_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Cielch<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Cielch<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Cielch<mt::Simd256Float32> lcha(mt::Simd256Float32(50.0f), mt::Simd256Float32(20.0f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(lcha.lightness.element(lane), 50.0f, "Cielch<Simd256Float32> lcha l lane " + std::to_string(lane));
		test.check_equal(lcha.chroma.element(lane), 20.0f, "Cielch<Simd256Float32> lcha c lane " + std::to_string(lane));
		test.check_equal(lcha.hue.element(lane), 0.3f, "Cielch<Simd256Float32> lcha h lane " + std::to_string(lane));
		test.check_equal(lcha.alpha.element(lane), 0.4f, "Cielch<Simd256Float32> lcha alpha lane " + std::to_string(lane));
	}

	// Clamp restricts bounded components to their valid ranges.
	mt::Cielch<mt::Simd256Float32> clamped = mt::Cielch<mt::Simd256Float32>(mt::Simd256Float32(125.0f), mt::Simd256Float32(-50.0f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.lightness.element(lane), 100.0f, "Cielch<Simd256Float32> clamp l lane " + std::to_string(lane));
		test.check_equal(clamped.chroma.element(lane), 0.0f, "Cielch<Simd256Float32> clamp c lane " + std::to_string(lane));
		test.check_equal(clamped.hue.element(lane), 1.0f, "Cielch<Simd256Float32> clamp h lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Cielch<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Cielch<mt::Simd256Float32> no_negatives = mt::Cielch<mt::Simd256Float32>(mt::Simd256Float32(-25.0f), mt::Simd256Float32(-50.0f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.lightness.element(lane), 0.0f, "Cielch<Simd256Float32> no_negatives l lane " + std::to_string(lane));
		test.check_equal(no_negatives.chroma.element(lane), 0.0f, "Cielch<Simd256Float32> no_negatives c lane " + std::to_string(lane));
		test.check_equal(no_negatives.hue.element(lane), 1.25f, "Cielch<Simd256Float32> no_negatives h lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Cielch<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Cielch<mt::Simd256Float32> sanitised = mt::Cielch<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.lightness.element(lane), 0.0f, "Cielch<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.chroma.element(lane), 0.0f, "Cielch<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.hue.element(lane), std::numeric_limits<float>::max(), "Cielch<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Cielch<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Cielch<mt::Simd256Float32> wrapped_negative = mt::Cielch<mt::Simd256Float32>(mt::Simd256Float32(50.0f), mt::Simd256Float32(20.0f), mt::Simd256Float32(-0.25f), mt::Simd256Float32(1.0f)).wrap_hue();
	mt::Cielch<mt::Simd256Float32> wrapped_over_one = mt::Cielch<mt::Simd256Float32>(mt::Simd256Float32(50.0f), mt::Simd256Float32(20.0f), mt::Simd256Float32(1.25f), mt::Simd256Float32(1.0f)).wrap_hue();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(wrapped_negative.lightness.element(lane), 50.0f, "Cielch<Simd256Float32> wrap negative l lane " + std::to_string(lane));
		test.check_equal(wrapped_negative.chroma.element(lane), 20.0f, "Cielch<Simd256Float32> wrap negative c lane " + std::to_string(lane));
		test.check_equal(wrapped_negative.hue.element(lane), 0.75f, "Cielch<Simd256Float32> wrap negative h lane " + std::to_string(lane));
		test.check_equal(wrapped_negative.alpha.element(lane), 1.0f, "Cielch<Simd256Float32> wrap negative alpha lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.lightness.element(lane), 50.0f, "Cielch<Simd256Float32> wrap over-one l lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.chroma.element(lane), 20.0f, "Cielch<Simd256Float32> wrap over-one c lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.hue.element(lane), 0.25f, "Cielch<Simd256Float32> wrap over-one h lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.alpha.element(lane), 1.0f, "Cielch<Simd256Float32> wrap over-one alpha lane " + std::to_string(lane));
	}

	// Reference data checks CIE XYZ D50 to CIELCH D50.
	const int lch_case_count = int(sizeof(colour_reference::xyz_d50_cielch_d50_cases) / sizeof(colour_reference::xyz_d50_cielch_d50_cases[0]));
	for (int first_case = 0; first_case < lch_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Cielch<mt::Simd256Float32> actual = xyz_d50_batch(colour_reference::xyz_d50_cielch_d50_cases, first_case, lch_case_count).to_cielch();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < lch_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d50_cielch_d50_cases[first_case + lane];
			std::string message = std::string("Cielch<Simd256Float32> from XyzD50 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 2.0e-4f, message + " l");
			test.check_close(actual.chroma.element(lane), colour_case.expected[1], 2.0e-4f, message + " c");
			if (colour_case.expected[1] > 1.0e-4) {
				test.check_close_hue(actual.hue.element(lane), colour_case.expected[2], 5.0e-4f, message + " h");
			}
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-4f, message + " alpha");
		}
	}

	// Reference data checks CIELCH D50 back to CIE XYZ D50.
	for (int first_case = 0; first_case < lch_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::XyzD50<mt::Simd256Float32> actual = cielch_batch(colour_reference::xyz_d50_cielch_d50_cases, first_case, lch_case_count).to_xyz_d50();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < lch_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d50_cielch_d50_cases[first_case + lane];
			std::string message = std::string("Cielch<Simd256Float32> to XyzD50 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.source[0], 2.0e-4f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.source[1], 2.0e-4f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.source[2], 2.0e-4f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-4f, message + " alpha");
		}
	}

	// Reference data checks CIELCH D50 to CIELAB D50.
	for (int first_case = 0; first_case < lch_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Cielab<mt::Simd256Float32> actual = cielch_batch(colour_reference::xyz_d50_cielch_d50_cases, first_case, lch_case_count).to_cielab();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < lch_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d50_cielab_d50_cases[first_case + lane];
			std::string message = std::string("Cielch<Simd256Float32> to Cielab ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 2.0e-4f, message + " l");
			test.check_close(actual.a.element(lane), colour_case.expected[1], 2.0e-4f, message + " a");
			test.check_close(actual.b.element(lane), colour_case.expected[2], 2.0e-4f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-4f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_cielch_simd256_float(test);

	return test.result();
}
