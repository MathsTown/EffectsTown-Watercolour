/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Oklch<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/oklab_reference.h"

void test_oklch_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Oklch<double> default_value;
	test.check_equal(default_value.lightness, 0.0, "Oklch<double> default l");
	test.check_equal(default_value.chroma, 0.0, "Oklch<double> default c");
	test.check_equal(default_value.hue, 0.0, "Oklch<double> default h");
	test.check_equal(default_value.alpha, 1.0, "Oklch<double> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Oklch<double> lch(0.1, 0.2, 0.3);
	test.check_equal(lch.lightness, 0.1, "Oklch<double> lch l");
	test.check_equal(lch.chroma, 0.2, "Oklch<double> lch c");
	test.check_equal(lch.hue, 0.3, "Oklch<double> lch h");
	test.check_equal(lch.alpha, 1.0, "Oklch<double> lch alpha");

	// Four-component constructor stores every component directly.
	mt::Oklch<double> lcha(0.1, 0.2, 0.3, 0.4);
	test.check_equal(lcha.lightness, 0.1, "Oklch<double> lcha l");
	test.check_equal(lcha.chroma, 0.2, "Oklch<double> lcha c");
	test.check_equal(lcha.hue, 0.3, "Oklch<double> lcha h");
	test.check_equal(lcha.alpha, 0.4, "Oklch<double> lcha alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::Oklch<double> clamped = mt::Oklch<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.lightness, 0.0, "Oklch<double> clamp l");
	test.check_equal(clamped.chroma, 0.5, "Oklch<double> clamp c");
	test.check_equal(clamped.hue, 1.0, "Oklch<double> clamp h");
	test.check_equal(clamped.alpha, 1.0, "Oklch<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Oklch<double> no_negatives = mt::Oklch<double>(-0.25, -0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0, "Oklch<double> no_negatives l");
	test.check_equal(no_negatives.chroma, 0.0, "Oklch<double> no_negatives c");
	test.check_equal(no_negatives.hue, 1.25, "Oklch<double> no_negatives h");
	test.check_equal(no_negatives.alpha, 0.0, "Oklch<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Oklch<double> sanitised = mt::Oklch<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0, "Oklch<double> sanitise negative zero");
	test.check_equal(sanitised.chroma, 0.0, "Oklch<double> sanitise nan");
	test.check_equal(sanitised.hue, std::numeric_limits<double>::max(), "Oklch<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Oklch<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Oklch<double> wrapped_negative = mt::Oklch<double>(0.25, 0.5, -0.25, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.lightness, 0.25, "Oklch<double> wrap negative l");
	test.check_equal(wrapped_negative.chroma, 0.5, "Oklch<double> wrap negative c");
	test.check_equal(wrapped_negative.hue, 0.75, "Oklch<double> wrap negative h");
	test.check_equal(wrapped_negative.alpha, 1.0, "Oklch<double> wrap negative alpha");

	mt::Oklch<double> wrapped_over_one = mt::Oklch<double>(0.25, 0.5, 1.25, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.lightness, 0.25, "Oklch<double> wrap over-one l");
	test.check_equal(wrapped_over_one.chroma, 0.5, "Oklch<double> wrap over-one c");
	test.check_equal(wrapped_over_one.hue, 0.25, "Oklch<double> wrap over-one h");
	test.check_equal(wrapped_over_one.alpha, 1.0, "Oklch<double> wrap over-one alpha");

	// Reference data checks CIE XYZ D65 to Oklch.
	for (const auto& colour_case : colour_reference::xyz_d65_oklch_cases) {
		mt::Xyz<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Oklch<double> actual = colour.to_oklch();
		std::string message = std::string("Oklch<double> from Xyz ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.chroma, colour_case.expected[1], 5.0e-10, message + " c");
		if (colour_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, colour_case.expected[2], 1.0e-8, message + " h");
		}
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-10, message + " alpha");
	}

	// Reference data checks Oklch back to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::xyz_d65_oklch_cases) {
		mt::Oklch<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("Oklch<double> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 1.0e-9, message + " x");
		test.check_close(actual.y, colour_case.source[1], 1.0e-9, message + " y");
		test.check_close(actual.z, colour_case.source[2], 1.0e-9, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 1.0e-9, message + " a");
	}

	// Reference data checks Oklch to Oklab.
	const int case_count = int(sizeof(colour_reference::xyz_d65_oklch_cases) / sizeof(colour_reference::xyz_d65_oklch_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& oklch_case = colour_reference::xyz_d65_oklch_cases[index];
		const auto& oklab_case = colour_reference::xyz_d65_oklab_cases[index];
		mt::Oklch<double> colour(oklch_case.expected[0], oklch_case.expected[1], oklch_case.expected[2], oklch_case.expected[3]);
		mt::Oklab<double> actual = colour.to_oklab();
		std::string message = std::string("Oklch<double> to Oklab ") + oklch_case.name;
		test.check_close(actual.lightness, oklab_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.a, oklab_case.expected[1], 5.0e-10, message + " a");
		test.check_close(actual.b, oklab_case.expected[2], 5.0e-10, message + " b");
		test.check_close(actual.alpha, oklab_case.expected[3], 5.0e-10, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_oklch_double(test);

	return test.result();
}
