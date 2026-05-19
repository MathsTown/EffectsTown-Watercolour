/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Cielch<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/cielab_reference.h"

void test_cielch_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Cielch<float> default_value;
	test.check_equal(default_value.lightness, 0.0f, "Cielch<float> default l");
	test.check_equal(default_value.chroma, 0.0f, "Cielch<float> default c");
	test.check_equal(default_value.hue, 0.0f, "Cielch<float> default h");
	test.check_equal(default_value.alpha, 1.0f, "Cielch<float> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Cielch<float> lch(50.0f, 20.0f, 0.3f);
	test.check_equal(lch.lightness, 50.0f, "Cielch<float> lch l");
	test.check_equal(lch.chroma, 20.0f, "Cielch<float> lch c");
	test.check_equal(lch.hue, 0.3f, "Cielch<float> lch h");
	test.check_equal(lch.alpha, 1.0f, "Cielch<float> lch alpha");

	// Four-component constructor stores every component directly.
	mt::Cielch<float> lcha(50.0f, 20.0f, 0.3f, 0.4f);
	test.check_equal(lcha.lightness, 50.0f, "Cielch<float> lcha l");
	test.check_equal(lcha.chroma, 20.0f, "Cielch<float> lcha c");
	test.check_equal(lcha.hue, 0.3f, "Cielch<float> lcha h");
	test.check_equal(lcha.alpha, 0.4f, "Cielch<float> lcha alpha");

	// Clamp restricts bounded components to their valid ranges.
	mt::Cielch<float> clamped = mt::Cielch<float>(125.0f, -50.0f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.lightness, 100.0f, "Cielch<float> clamp l");
	test.check_equal(clamped.chroma, 0.0f, "Cielch<float> clamp c");
	test.check_equal(clamped.hue, 1.0f, "Cielch<float> clamp h");
	test.check_equal(clamped.alpha, 1.0f, "Cielch<float> clamp alpha");

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Cielch<float> no_negatives = mt::Cielch<float>(-25.0f, -50.0f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0f, "Cielch<float> no_negatives l");
	test.check_equal(no_negatives.chroma, 0.0f, "Cielch<float> no_negatives c");
	test.check_equal(no_negatives.hue, 1.25f, "Cielch<float> no_negatives h");
	test.check_equal(no_negatives.alpha, 0.0f, "Cielch<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Cielch<float> sanitised = mt::Cielch<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0f, "Cielch<float> sanitise negative zero");
	test.check_equal(sanitised.chroma, 0.0f, "Cielch<float> sanitise nan");
	test.check_equal(sanitised.hue, std::numeric_limits<float>::max(), "Cielch<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Cielch<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Cielch<float> wrapped_negative = mt::Cielch<float>(50.0f, 20.0f, -0.25f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.lightness, 50.0f, "Cielch<float> wrap negative l");
	test.check_equal(wrapped_negative.chroma, 20.0f, "Cielch<float> wrap negative c");
	test.check_equal(wrapped_negative.hue, 0.75f, "Cielch<float> wrap negative h");
	test.check_equal(wrapped_negative.alpha, 1.0f, "Cielch<float> wrap negative alpha");

	mt::Cielch<float> wrapped_over_one = mt::Cielch<float>(50.0f, 20.0f, 1.25f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.lightness, 50.0f, "Cielch<float> wrap over-one l");
	test.check_equal(wrapped_over_one.chroma, 20.0f, "Cielch<float> wrap over-one c");
	test.check_equal(wrapped_over_one.hue, 0.25f, "Cielch<float> wrap over-one h");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "Cielch<float> wrap over-one alpha");

	// Reference data checks CIE XYZ D50 to CIELCH D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielch_d50_cases) {
		mt::XyzD50<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Cielch<float> actual = colour.to_cielch();
		std::string message = std::string("Cielch<float> from XyzD50 ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 1.0e-4f, message + " l");
		test.check_close(actual.chroma, colour_case.expected[1], 1.0e-4f, message + " c");
		if (colour_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, colour_case.expected[2], 2.0e-4f, message + " h");
		}
		test.check_close(actual.alpha, colour_case.expected[3], 1.0e-4f, message + " alpha");
	}

	// Reference data checks CIELCH D50 back to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielch_d50_cases) {
		mt::Cielch<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::XyzD50<float> actual = colour.to_xyz_d50();
		std::string message = std::string("Cielch<float> to XyzD50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 1.0e-4f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 1.0e-4f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 1.0e-4f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 1.0e-4f, message + " alpha");
	}

	// Reference data checks CIELCH D50 to CIELAB D50.
	const int case_count = int(sizeof(colour_reference::xyz_d50_cielch_d50_cases) / sizeof(colour_reference::xyz_d50_cielch_d50_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& lch_case = colour_reference::xyz_d50_cielch_d50_cases[index];
		const auto& lab_case = colour_reference::xyz_d50_cielab_d50_cases[index];
		mt::Cielch<float> colour(static_cast<float>(lch_case.expected[0]), static_cast<float>(lch_case.expected[1]), static_cast<float>(lch_case.expected[2]), static_cast<float>(lch_case.expected[3]));
		mt::Cielab<float> actual = colour.to_cielab();
		std::string message = std::string("Cielch<float> to Cielab ") + lch_case.name;
		test.check_close(actual.lightness, lab_case.expected[0], 1.0e-4f, message + " l");
		test.check_close(actual.a, lab_case.expected[1], 1.0e-4f, message + " a");
		test.check_close(actual.b, lab_case.expected[2], 1.0e-4f, message + " b");
		test.check_close(actual.alpha, lab_case.expected[3], 1.0e-4f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_cielch_float(test);

	return test.result();
}
