/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Oklab<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/oklab_reference.h"

void test_oklab_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Oklab<double> default_value;
	test.check_equal(default_value.lightness, 0.0, "Oklab<double> default l");
	test.check_equal(default_value.a, 0.0, "Oklab<double> default a");
	test.check_equal(default_value.b, 0.0, "Oklab<double> default b");
	test.check_equal(default_value.alpha, 1.0, "Oklab<double> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Oklab<double> lab(0.1, -0.2, 0.3);
	test.check_equal(lab.lightness, 0.1, "Oklab<double> lab l");
	test.check_equal(lab.a, -0.2, "Oklab<double> lab a");
	test.check_equal(lab.b, 0.3, "Oklab<double> lab b");
	test.check_equal(lab.alpha, 1.0, "Oklab<double> lab alpha");

	// Four-component constructor stores every component directly.
	mt::Oklab<double> laba(0.1, -0.2, 0.3, 0.4);
	test.check_equal(laba.lightness, 0.1, "Oklab<double> laba l");
	test.check_equal(laba.a, -0.2, "Oklab<double> laba a");
	test.check_equal(laba.b, 0.3, "Oklab<double> laba b");
	test.check_equal(laba.alpha, 0.4, "Oklab<double> laba alpha");

	// Clamp restricts bounded components to the 0..1 range.
	mt::Oklab<double> clamped = mt::Oklab<double>(-0.25, -0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.lightness, 0.0, "Oklab<double> clamp l");
	test.check_equal(clamped.a, -0.5, "Oklab<double> clamp a");
	test.check_equal(clamped.b, 1.25, "Oklab<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "Oklab<double> clamp alpha");

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Oklab<double> no_negatives = mt::Oklab<double>(-0.25, -0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0, "Oklab<double> no_negatives l");
	test.check_equal(no_negatives.a, -0.5, "Oklab<double> no_negatives a");
	test.check_equal(no_negatives.b, 1.25, "Oklab<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "Oklab<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Oklab<double> sanitised = mt::Oklab<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0, "Oklab<double> sanitise negative zero");
	test.check_equal(sanitised.a, 0.0, "Oklab<double> sanitise nan");
	test.check_equal(sanitised.b, std::numeric_limits<double>::max(), "Oklab<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Oklab<double> sanitise negative infinity");

	// Reference data checks CIE XYZ D65 to Oklab.
	for (const auto& colour_case : colour_reference::xyz_d65_oklab_cases) {
		mt::Xyz<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Oklab<double> actual = colour.to_oklab();
		std::string message = std::string("Oklab<double> from Xyz ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.a, colour_case.expected[1], 5.0e-10, message + " a");
		test.check_close(actual.b, colour_case.expected[2], 5.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-10, message + " alpha");
	}

	// Reference data checks Oklab back to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::xyz_d65_oklab_cases) {
		mt::Oklab<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("Oklab<double> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 1.0e-9, message + " x");
		test.check_close(actual.y, colour_case.source[1], 1.0e-9, message + " y");
		test.check_close(actual.z, colour_case.source[2], 1.0e-9, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 1.0e-9, message + " a");
	}

	// Reference data checks Oklab to Oklch.
	const int case_count = int(sizeof(colour_reference::xyz_d65_oklab_cases) / sizeof(colour_reference::xyz_d65_oklab_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& oklab_case = colour_reference::xyz_d65_oklab_cases[index];
		const auto& oklch_case = colour_reference::xyz_d65_oklch_cases[index];
		mt::Oklab<double> colour(oklab_case.expected[0], oklab_case.expected[1], oklab_case.expected[2], oklab_case.expected[3]);
		mt::Oklch<double> actual = colour.to_oklch();
		std::string message = std::string("Oklab<double> to Oklch ") + oklab_case.name;
		test.check_close(actual.lightness, oklch_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.chroma, oklch_case.expected[1], 5.0e-10, message + " c");
		if (oklch_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, oklch_case.expected[2], 1.0e-8, message + " h");
		}
		test.check_close(actual.alpha, oklch_case.expected[3], 5.0e-10, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_oklab_double(test);

	return test.result();
}
