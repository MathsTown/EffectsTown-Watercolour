/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Cielab<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/cielab_reference.h"

void test_cielab_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Cielab<double> default_value;
	test.check_equal(default_value.lightness, 0.0, "Cielab<double> default l");
	test.check_equal(default_value.a, 0.0, "Cielab<double> default a");
	test.check_equal(default_value.b, 0.0, "Cielab<double> default b");
	test.check_equal(default_value.alpha, 1.0, "Cielab<double> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Cielab<double> lab(50.0, -20.0, 30.0);
	test.check_equal(lab.lightness, 50.0, "Cielab<double> lab l");
	test.check_equal(lab.a, -20.0, "Cielab<double> lab a");
	test.check_equal(lab.b, 30.0, "Cielab<double> lab b");
	test.check_equal(lab.alpha, 1.0, "Cielab<double> lab alpha");

	// Four-component constructor stores every component directly.
	mt::Cielab<double> laba(50.0, -20.0, 30.0, 0.4);
	test.check_equal(laba.lightness, 50.0, "Cielab<double> laba l");
	test.check_equal(laba.a, -20.0, "Cielab<double> laba a");
	test.check_equal(laba.b, 30.0, "Cielab<double> laba b");
	test.check_equal(laba.alpha, 0.4, "Cielab<double> laba alpha");

	// Clamp restricts bounded components to their valid ranges.
	mt::Cielab<double> clamped = mt::Cielab<double>(125.0, -50.0, 125.0, 2.0).clamp();
	test.check_equal(clamped.lightness, 100.0, "Cielab<double> clamp l");
	test.check_equal(clamped.a, -50.0, "Cielab<double> clamp a");
	test.check_equal(clamped.b, 125.0, "Cielab<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "Cielab<double> clamp alpha");

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Cielab<double> no_negatives = mt::Cielab<double>(-25.0, -50.0, 125.0, -0.5).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0, "Cielab<double> no_negatives l");
	test.check_equal(no_negatives.a, -50.0, "Cielab<double> no_negatives a");
	test.check_equal(no_negatives.b, 125.0, "Cielab<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "Cielab<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Cielab<double> sanitised = mt::Cielab<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0, "Cielab<double> sanitise negative zero");
	test.check_equal(sanitised.a, 0.0, "Cielab<double> sanitise nan");
	test.check_equal(sanitised.b, std::numeric_limits<double>::max(), "Cielab<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Cielab<double> sanitise negative infinity");

	// Reference data checks CIE XYZ D50 to CIELAB D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielab_d50_cases) {
		mt::XyzD50<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Cielab<double> actual = colour.to_cielab();
		std::string message = std::string("Cielab<double> from XyzD50 ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.a, colour_case.expected[1], 5.0e-10, message + " a");
		test.check_close(actual.b, colour_case.expected[2], 5.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-10, message + " alpha");
	}

	// Reference data checks CIELAB D50 back to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielab_d50_cases) {
		mt::Cielab<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::XyzD50<double> actual = colour.to_xyz_d50();
		std::string message = std::string("Cielab<double> to XyzD50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 5.0e-10, message + " x");
		test.check_close(actual.y, colour_case.source[1], 5.0e-10, message + " y");
		test.check_close(actual.z, colour_case.source[2], 5.0e-10, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-10, message + " alpha");
	}

	// Reference data checks CIELAB D50 to CIELCH D50.
	const int case_count = int(sizeof(colour_reference::xyz_d50_cielab_d50_cases) / sizeof(colour_reference::xyz_d50_cielab_d50_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& lab_case = colour_reference::xyz_d50_cielab_d50_cases[index];
		const auto& lch_case = colour_reference::xyz_d50_cielch_d50_cases[index];
		mt::Cielab<double> colour(lab_case.expected[0], lab_case.expected[1], lab_case.expected[2], lab_case.expected[3]);
		mt::Cielch<double> actual = colour.to_cielch();
		std::string message = std::string("Cielab<double> to Cielch ") + lab_case.name;
		test.check_close(actual.lightness, lch_case.expected[0], 5.0e-10, message + " l");
		test.check_close(actual.chroma, lch_case.expected[1], 5.0e-10, message + " c");
		if (lch_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, lch_case.expected[2], 1.0e-8, message + " h");
		}
		test.check_close(actual.alpha, lch_case.expected[3], 5.0e-10, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_cielab_double(test);

	return test.result();
}
