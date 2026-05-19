/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Cielab<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/cielab_reference.h"

void test_cielab_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Cielab<float> default_value;
	test.check_equal(default_value.lightness, 0.0f, "Cielab<float> default l");
	test.check_equal(default_value.a, 0.0f, "Cielab<float> default a");
	test.check_equal(default_value.b, 0.0f, "Cielab<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "Cielab<float> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Cielab<float> lab(50.0f, -20.0f, 30.0f);
	test.check_equal(lab.lightness, 50.0f, "Cielab<float> lab l");
	test.check_equal(lab.a, -20.0f, "Cielab<float> lab a");
	test.check_equal(lab.b, 30.0f, "Cielab<float> lab b");
	test.check_equal(lab.alpha, 1.0f, "Cielab<float> lab alpha");

	// Four-component constructor stores every component directly.
	mt::Cielab<float> laba(50.0f, -20.0f, 30.0f, 0.4f);
	test.check_equal(laba.lightness, 50.0f, "Cielab<float> laba l");
	test.check_equal(laba.a, -20.0f, "Cielab<float> laba a");
	test.check_equal(laba.b, 30.0f, "Cielab<float> laba b");
	test.check_equal(laba.alpha, 0.4f, "Cielab<float> laba alpha");

	// Clamp restricts bounded components to their valid ranges.
	mt::Cielab<float> clamped = mt::Cielab<float>(125.0f, -50.0f, 125.0f, 2.0f).clamp();
	test.check_equal(clamped.lightness, 100.0f, "Cielab<float> clamp l");
	test.check_equal(clamped.a, -50.0f, "Cielab<float> clamp a");
	test.check_equal(clamped.b, 125.0f, "Cielab<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "Cielab<float> clamp alpha");

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Cielab<float> no_negatives = mt::Cielab<float>(-25.0f, -50.0f, 125.0f, -0.5f).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0f, "Cielab<float> no_negatives l");
	test.check_equal(no_negatives.a, -50.0f, "Cielab<float> no_negatives a");
	test.check_equal(no_negatives.b, 125.0f, "Cielab<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "Cielab<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Cielab<float> sanitised = mt::Cielab<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0f, "Cielab<float> sanitise negative zero");
	test.check_equal(sanitised.a, 0.0f, "Cielab<float> sanitise nan");
	test.check_equal(sanitised.b, std::numeric_limits<float>::max(), "Cielab<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Cielab<float> sanitise negative infinity");

	// Reference data checks CIE XYZ D50 to CIELAB D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielab_d50_cases) {
		mt::XyzD50<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Cielab<float> actual = colour.to_cielab();
		std::string message = std::string("Cielab<float> from XyzD50 ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 1.0e-4f, message + " l");
		test.check_close(actual.a, colour_case.expected[1], 1.0e-4f, message + " a");
		test.check_close(actual.b, colour_case.expected[2], 1.0e-4f, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 1.0e-4f, message + " alpha");
	}

	// Reference data checks CIELAB D50 back to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::xyz_d50_cielab_d50_cases) {
		mt::Cielab<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::XyzD50<float> actual = colour.to_xyz_d50();
		std::string message = std::string("Cielab<float> to XyzD50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 1.0e-4f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 1.0e-4f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 1.0e-4f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 1.0e-4f, message + " alpha");
	}

	// Reference data checks CIELAB D50 to CIELCH D50.
	const int case_count = int(sizeof(colour_reference::xyz_d50_cielab_d50_cases) / sizeof(colour_reference::xyz_d50_cielab_d50_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& lab_case = colour_reference::xyz_d50_cielab_d50_cases[index];
		const auto& lch_case = colour_reference::xyz_d50_cielch_d50_cases[index];
		mt::Cielab<float> colour(static_cast<float>(lab_case.expected[0]), static_cast<float>(lab_case.expected[1]), static_cast<float>(lab_case.expected[2]), static_cast<float>(lab_case.expected[3]));
		mt::Cielch<float> actual = colour.to_cielch();
		std::string message = std::string("Cielab<float> to Cielch ") + lab_case.name;
		test.check_close(actual.lightness, lch_case.expected[0], 1.0e-4f, message + " l");
		test.check_close(actual.chroma, lch_case.expected[1], 1.0e-4f, message + " c");
		if (lch_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, lch_case.expected[2], 2.0e-4f, message + " h");
		}
		test.check_close(actual.alpha, lch_case.expected[3], 1.0e-4f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_cielab_float(test);

	return test.result();
}
