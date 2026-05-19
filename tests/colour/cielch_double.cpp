/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Cielch<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/cielab_reference.h"

void test_cielch_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Cielch<double> default_value;
	test.check_equal(default_value.lightness, 0.0, "Cielch<double> default l");
	test.check_equal(default_value.chroma, 0.0, "Cielch<double> default c");
	test.check_equal(default_value.hue, 0.0, "Cielch<double> default h");
	test.check_equal(default_value.alpha, 1.0, "Cielch<double> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Cielch<double> lch(50.0, 20.0, 0.3);
	test.check_equal(lch.lightness, 50.0, "Cielch<double> lch l");
	test.check_equal(lch.chroma, 20.0, "Cielch<double> lch c");
	test.check_equal(lch.hue, 0.3, "Cielch<double> lch h");
	test.check_equal(lch.alpha, 1.0, "Cielch<double> lch alpha");

	// Four-component constructor stores every component directly.
	mt::Cielch<double> lcha(50.0, 20.0, 0.3, 0.4);
	test.check_equal(lcha.lightness, 50.0, "Cielch<double> lcha l");
	test.check_equal(lcha.chroma, 20.0, "Cielch<double> lcha c");
	test.check_equal(lcha.hue, 0.3, "Cielch<double> lcha h");
	test.check_equal(lcha.alpha, 0.4, "Cielch<double> lcha alpha");

	// Clamp restricts bounded components to their valid ranges.
	mt::Cielch<double> clamped = mt::Cielch<double>(125.0, -50.0, 1.25, 2.0).clamp();
	test.check_equal(clamped.lightness, 100.0, "Cielch<double> clamp l");
	test.check_equal(clamped.chroma, 0.0, "Cielch<double> clamp c");
	test.check_equal(clamped.hue, 1.0, "Cielch<double> clamp h");
	test.check_equal(clamped.alpha, 1.0, "Cielch<double> clamp alpha");

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Cielch<double> no_negatives = mt::Cielch<double>(-25.0, -50.0, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0, "Cielch<double> no_negatives l");
	test.check_equal(no_negatives.chroma, 0.0, "Cielch<double> no_negatives c");
	test.check_equal(no_negatives.hue, 1.25, "Cielch<double> no_negatives h");
	test.check_equal(no_negatives.alpha, 0.0, "Cielch<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Cielch<double> sanitised = mt::Cielch<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0, "Cielch<double> sanitise negative zero");
	test.check_equal(sanitised.chroma, 0.0, "Cielch<double> sanitise nan");
	test.check_equal(sanitised.hue, std::numeric_limits<double>::max(), "Cielch<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Cielch<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Cielch<double> wrapped_negative = mt::Cielch<double>(50.0, 20.0, -0.25, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.lightness, 50.0, "Cielch<double> wrap negative l");
	test.check_equal(wrapped_negative.chroma, 20.0, "Cielch<double> wrap negative c");
	test.check_equal(wrapped_negative.hue, 0.75, "Cielch<double> wrap negative h");
	test.check_equal(wrapped_negative.alpha, 1.0, "Cielch<double> wrap negative alpha");

	mt::Cielch<double> wrapped_over_one = mt::Cielch<double>(50.0, 20.0, 1.25, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.lightness, 50.0, "Cielch<double> wrap over-one l");
	test.check_equal(wrapped_over_one.chroma, 20.0, "Cielch<double> wrap over-one c");
	test.check_equal(wrapped_over_one.hue, 0.25, "Cielch<double> wrap over-one h");
	test.check_equal(wrapped_over_one.alpha, 1.0, "Cielch<double> wrap over-one alpha");

	// Reference data checks CIE XYZ D50 to CIELCH D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielch_d50_cases) {
		mt::XyzD50<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Cielch<double> actual = colour.to_cielch();
		std::string message = std::string("Cielch<double> from XyzD50 ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.chroma, colour_case.expected[1], 5.0e-10, message + " c");
		if (colour_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, colour_case.expected[2], 1.0e-8, message + " h");
		}
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-10, message + " alpha");
	}

	// Reference data checks CIELCH D50 back to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielch_d50_cases) {
		mt::Cielch<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::XyzD50<double> actual = colour.to_xyz_d50();
		std::string message = std::string("Cielch<double> to XyzD50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 5.0e-10, message + " x");
		test.check_close(actual.y, colour_case.source[1], 5.0e-10, message + " y");
		test.check_close(actual.z, colour_case.source[2], 5.0e-10, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-10, message + " alpha");
	}

	// Reference data checks CIELCH D50 to CIELAB D50.
	const int case_count = int(sizeof(colour_reference::xyz_d50_cielch_d50_cases) / sizeof(colour_reference::xyz_d50_cielch_d50_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& lch_case = colour_reference::xyz_d50_cielch_d50_cases[index];
		const auto& lab_case = colour_reference::xyz_d50_cielab_d50_cases[index];
		mt::Cielch<double> colour(lch_case.expected[0], lch_case.expected[1], lch_case.expected[2], lch_case.expected[3]);
		mt::Cielab<double> actual = colour.to_cielab();
		std::string message = std::string("Cielch<double> to Cielab ") + lch_case.name;
		test.check_close(actual.lightness, lab_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.a, lab_case.expected[1], 5.0e-10, message + " a");
		test.check_close(actual.b, lab_case.expected[2], 5.0e-10, message + " b");
		test.check_close(actual.alpha, lab_case.expected[3], 5.0e-10, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_cielch_double(test);

	return test.result();
}
