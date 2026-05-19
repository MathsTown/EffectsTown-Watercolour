/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar XyzAces<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/aces_reference.h"
#include "reference/xyz_aces_reference.h"

void test_xyz_aces_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::XyzAces<float> default_value;
	test.check_equal(default_value.x, 0.0f, "XyzAces<float> default x");
	test.check_equal(default_value.y, 0.0f, "XyzAces<float> default y");
	test.check_equal(default_value.z, 0.0f, "XyzAces<float> default z");
	test.check_equal(default_value.alpha, 1.0f, "XyzAces<float> default alpha");

	// XYZ constructor fills alpha with one.
	mt::XyzAces<float> xyz(0.9526460745698463f, 1.0f, 1.008825184351586f);
	test.check_equal(xyz.x, 0.9526460745698463f, "XyzAces<float> xyz x");
	test.check_equal(xyz.y, 1.0f, "XyzAces<float> xyz y");
	test.check_equal(xyz.z, 1.008825184351586f, "XyzAces<float> xyz z");
	test.check_equal(xyz.alpha, 1.0f, "XyzAces<float> xyz alpha");

	// XYZA constructor stores every component directly.
	mt::XyzAces<float> xyza(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(xyza.x, 0.1f, "XyzAces<float> xyza x");
	test.check_equal(xyza.y, 0.2f, "XyzAces<float> xyza y");
	test.check_equal(xyza.z, 0.3f, "XyzAces<float> xyza z");
	test.check_equal(xyza.alpha, 0.4f, "XyzAces<float> xyza alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::XyzAces<float> clamped = mt::XyzAces<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.x, 0.0f, "XyzAces<float> clamp x");
	test.check_equal(clamped.y, 0.5f, "XyzAces<float> clamp y");
	test.check_equal(clamped.z, 1.0f, "XyzAces<float> clamp z");
	test.check_equal(clamped.alpha, 1.0f, "XyzAces<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::XyzAces<float> no_negatives = mt::XyzAces<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.x, 0.0f, "XyzAces<float> no_negatives x");
	test.check_equal(no_negatives.y, 0.5f, "XyzAces<float> no_negatives y");
	test.check_equal(no_negatives.z, 1.25f, "XyzAces<float> no_negatives z");
	test.check_equal(no_negatives.alpha, 0.0f, "XyzAces<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::XyzAces<float> sanitised = mt::XyzAces<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.x, 0.0f, "XyzAces<float> sanitise negative zero");
	test.check_equal(sanitised.y, 0.0f, "XyzAces<float> sanitise nan");
	test.check_equal(sanitised.z, std::numeric_limits<float>::max(), "XyzAces<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "XyzAces<float> sanitise alpha negative infinity");

	// Reference data checks CIE XYZ D65 to ACES-white XYZ with Bradford.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::XyzAces<float> actual = colour.to_xyz_aces();
		std::string message = std::string("XyzAces<float> from Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks ACES-white XYZ back to CIE XYZ D65 with Bradford.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cases) {
		mt::XyzAces<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("XyzAces<float> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks CIE XYZ D65 to ACES-white XYZ with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cat02_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::XyzAces<float> actual = colour.to_xyz_aces_with_cat02();
		std::string message = std::string("XyzAces<float> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks ACES-white XYZ back to CIE XYZ D65 with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cat02_cases) {
		mt::XyzAces<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Xyz<float> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("XyzAces<float> to Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-5f, message + " alpha");
	}

	// Reference data checks exact ACES-white XYZ to ACES2065-1.
	for (const auto& colour_case : colour_reference::aces2065_1_xyz_aces_cases) {
		mt::XyzAces<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Aces2065_1<float> actual = colour.to_aces2065_1();
		std::string message = std::string("XyzAces<float> to_aces2065_1 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-5f, message + " a");
	}

	// Reference data checks exact ACES-white XYZ to ACEScg.
	for (const auto& colour_case : colour_reference::acescg_xyz_aces_cases) {
		mt::XyzAces<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Acescg<float> actual = colour.to_acescg();
		std::string message = std::string("XyzAces<float> to_acescg ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-5f, message + " a");
	}
}

int main() {
	TestContext test;

	test_xyz_aces_float(test);

	return test.result();
}
