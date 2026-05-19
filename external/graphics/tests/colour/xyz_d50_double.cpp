/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar XyzD50<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/xyz_d50_reference.h"

void test_xyz_d50_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::XyzD50<double> default_value;
	test.check_equal(default_value.x, 0.0, "XyzD50<double> default x");
	test.check_equal(default_value.y, 0.0, "XyzD50<double> default y");
	test.check_equal(default_value.z, 0.0, "XyzD50<double> default z");
	test.check_equal(default_value.alpha, 1.0, "XyzD50<double> default alpha");

	// XYZ constructor fills alpha with one.
	mt::XyzD50<double> xyz(0.9642956764295677, 1.0, 0.8251046025104604);
	test.check_equal(xyz.x, 0.9642956764295677, "XyzD50<double> xyz x");
	test.check_equal(xyz.y, 1.0, "XyzD50<double> xyz y");
	test.check_equal(xyz.z, 0.8251046025104604, "XyzD50<double> xyz z");
	test.check_equal(xyz.alpha, 1.0, "XyzD50<double> xyz alpha");

	// XYZ alpha constructor stores every component directly.
	mt::XyzD50<double> xyza(0.1, 0.2, 0.3, 0.4);
	test.check_equal(xyza.x, 0.1, "XyzD50<double> xyza x");
	test.check_equal(xyza.y, 0.2, "XyzD50<double> xyza y");
	test.check_equal(xyza.z, 0.3, "XyzD50<double> xyza z");
	test.check_equal(xyza.alpha, 0.4, "XyzD50<double> xyza alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::XyzD50<double> clamped = mt::XyzD50<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.x, 0.0, "XyzD50<double> clamp x");
	test.check_equal(clamped.y, 0.5, "XyzD50<double> clamp y");
	test.check_equal(clamped.z, 1.0, "XyzD50<double> clamp z");
	test.check_equal(clamped.alpha, 1.0, "XyzD50<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::XyzD50<double> no_negatives = mt::XyzD50<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.x, 0.0, "XyzD50<double> no_negatives x");
	test.check_equal(no_negatives.y, 0.5, "XyzD50<double> no_negatives y");
	test.check_equal(no_negatives.z, 1.25, "XyzD50<double> no_negatives z");
	test.check_equal(no_negatives.alpha, 0.0, "XyzD50<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::XyzD50<double> sanitised = mt::XyzD50<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.x, 0.0, "XyzD50<double> sanitise negative zero");
	test.check_equal(sanitised.y, 0.0, "XyzD50<double> sanitise nan");
	test.check_equal(sanitised.z, std::numeric_limits<double>::max(), "XyzD50<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "XyzD50<double> sanitise alpha negative infinity");

	// Reference data checks CIE XYZ D65 to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cases) {
		mt::Xyz<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::XyzD50<double> actual = colour.to_xyz_d50();
		std::string message = std::string("XyzD50<double> from Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks CIE XYZ D50 back to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cases) {
		mt::XyzD50<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("XyzD50<double> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks CIE XYZ D65 to CIE XYZ D50 with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cat02_cases) {
		mt::Xyz<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::XyzD50<double> actual = colour.to_xyz_d50_with_cat02();
		std::string message = std::string("XyzD50<double> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks CIE XYZ D50 back to CIE XYZ D65 with CAT02.
	for (const auto& colour_case : colour_reference::xyz_d65_xyz_d50_cat02_cases) {
		mt::XyzD50<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::Xyz<double> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("XyzD50<double> to Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.source[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.source[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_xyz_d50_double(test);

	return test.result();
}
