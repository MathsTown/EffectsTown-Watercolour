/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar XyzAces<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/aces_reference.h"
#include "reference/xyz_aces_reference.h"

void test_xyz_aces_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::XyzAces<double> default_value;
	test.check_equal(default_value.x, 0.0, "XyzAces<double> default x");
	test.check_equal(default_value.y, 0.0, "XyzAces<double> default y");
	test.check_equal(default_value.z, 0.0, "XyzAces<double> default z");
	test.check_equal(default_value.alpha, 1.0, "XyzAces<double> default alpha");

	// XYZ constructor fills alpha with one.
	mt::XyzAces<double> xyz(0.9526460745698463, 1.0, 1.008825184351586);
	test.check_equal(xyz.x, 0.9526460745698463, "XyzAces<double> xyz x");
	test.check_equal(xyz.y, 1.0, "XyzAces<double> xyz y");
	test.check_equal(xyz.z, 1.008825184351586, "XyzAces<double> xyz z");
	test.check_equal(xyz.alpha, 1.0, "XyzAces<double> xyz alpha");

	// XYZA constructor stores every component directly.
	mt::XyzAces<double> xyza(0.1, 0.2, 0.3, 0.4);
	test.check_equal(xyza.x, 0.1, "XyzAces<double> xyza x");
	test.check_equal(xyza.y, 0.2, "XyzAces<double> xyza y");
	test.check_equal(xyza.z, 0.3, "XyzAces<double> xyza z");
	test.check_equal(xyza.alpha, 0.4, "XyzAces<double> xyza alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::XyzAces<double> clamped = mt::XyzAces<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.x, 0.0, "XyzAces<double> clamp x");
	test.check_equal(clamped.y, 0.5, "XyzAces<double> clamp y");
	test.check_equal(clamped.z, 1.0, "XyzAces<double> clamp z");
	test.check_equal(clamped.alpha, 1.0, "XyzAces<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::XyzAces<double> no_negatives = mt::XyzAces<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.x, 0.0, "XyzAces<double> no_negatives x");
	test.check_equal(no_negatives.y, 0.5, "XyzAces<double> no_negatives y");
	test.check_equal(no_negatives.z, 1.25, "XyzAces<double> no_negatives z");
	test.check_equal(no_negatives.alpha, 0.0, "XyzAces<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::XyzAces<double> sanitised = mt::XyzAces<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.x, 0.0, "XyzAces<double> sanitise negative zero");
	test.check_equal(sanitised.y, 0.0, "XyzAces<double> sanitise nan");
	test.check_equal(sanitised.z, std::numeric_limits<double>::max(), "XyzAces<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "XyzAces<double> sanitise alpha negative infinity");

	// Reference data checks CIE XYZ D65 to ACES-white XYZ with Bradford.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cases) {
		mt::Xyz<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::XyzAces<double> actual = colour.to_xyz_aces();
		std::string message = std::string("XyzAces<double> from Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks ACES-white XYZ back to CIE XYZ D65 with Bradford.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cases) {
		mt::XyzAces<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("XyzAces<double> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks CIE XYZ D65 to ACES-white XYZ with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cat02_cases) {
		mt::Xyz<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::XyzAces<double> actual = colour.to_xyz_aces_with_cat02();
		std::string message = std::string("XyzAces<double> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks ACES-white XYZ back to CIE XYZ D65 with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_aces_cat02_cases) {
		mt::XyzAces<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Xyz<double> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("XyzAces<double> to Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks exact ACES-white XYZ to ACES2065-1.
	for (const auto& colour_case : colour_reference::aces2065_1_xyz_aces_cases) {
		mt::XyzAces<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Aces2065_1<double> actual = colour.to_aces2065_1();
		std::string message = std::string("XyzAces<double> to_aces2065_1 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks exact ACES-white XYZ to ACEScg.
	for (const auto& colour_case : colour_reference::acescg_xyz_aces_cases) {
		mt::XyzAces<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Acescg<double> actual = colour.to_acescg();
		std::string message = std::string("XyzAces<double> to_acescg ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_xyz_aces_double(test);

	return test.result();
}
