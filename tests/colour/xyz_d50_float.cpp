/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar XyzD50<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/xyz_d50_reference.h"

void test_xyz_d50_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::XyzD50<float> default_value;
	test.check_equal(default_value.x, 0.0f, "XyzD50<float> default x");
	test.check_equal(default_value.y, 0.0f, "XyzD50<float> default y");
	test.check_equal(default_value.z, 0.0f, "XyzD50<float> default z");
	test.check_equal(default_value.alpha, 1.0f, "XyzD50<float> default alpha");

	// XYZ constructor fills alpha with one.
	mt::XyzD50<float> xyz(0.964295676f, 1.0f, 0.825104603f);
	test.check_equal(xyz.x, 0.964295676f, "XyzD50<float> xyz x");
	test.check_equal(xyz.y, 1.0f, "XyzD50<float> xyz y");
	test.check_equal(xyz.z, 0.825104603f, "XyzD50<float> xyz z");
	test.check_equal(xyz.alpha, 1.0f, "XyzD50<float> xyz alpha");

	// XYZ alpha constructor stores every component directly.
	mt::XyzD50<float> xyza(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(xyza.x, 0.1f, "XyzD50<float> xyza x");
	test.check_equal(xyza.y, 0.2f, "XyzD50<float> xyza y");
	test.check_equal(xyza.z, 0.3f, "XyzD50<float> xyza z");
	test.check_equal(xyza.alpha, 0.4f, "XyzD50<float> xyza alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::XyzD50<float> clamped = mt::XyzD50<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.x, 0.0f, "XyzD50<float> clamp x");
	test.check_equal(clamped.y, 0.5f, "XyzD50<float> clamp y");
	test.check_equal(clamped.z, 1.0f, "XyzD50<float> clamp z");
	test.check_equal(clamped.alpha, 1.0f, "XyzD50<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::XyzD50<float> no_negatives = mt::XyzD50<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.x, 0.0f, "XyzD50<float> no_negatives x");
	test.check_equal(no_negatives.y, 0.5f, "XyzD50<float> no_negatives y");
	test.check_equal(no_negatives.z, 1.25f, "XyzD50<float> no_negatives z");
	test.check_equal(no_negatives.alpha, 0.0f, "XyzD50<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::XyzD50<float> sanitised = mt::XyzD50<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.x, 0.0f, "XyzD50<float> sanitise negative zero");
	test.check_equal(sanitised.y, 0.0f, "XyzD50<float> sanitise nan");
	test.check_equal(sanitised.z, std::numeric_limits<float>::max(), "XyzD50<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "XyzD50<float> sanitise alpha negative infinity");

	// Reference data checks CIE XYZ D65 to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::XyzD50<float> actual = colour.to_xyz_d50();
		std::string message = std::string("XyzD50<float> from Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks CIE XYZ D50 back to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cases) {
		mt::XyzD50<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("XyzD50<float> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks CIE XYZ D65 to CIE XYZ D50 with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cat02_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::XyzD50<float> actual = colour.to_xyz_d50_with_cat02();
		std::string message = std::string("XyzD50<float> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks CIE XYZ D50 back to CIE XYZ D65 with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cat02_cases) {
		mt::XyzD50<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Xyz<float> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("XyzD50<float> to Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-5f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_xyz_d50_float(test);

	return test.result();
}
