/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Xyz<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <limits>

#include "colour/xyz.h"

void test_xyz_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Xyz<double> default_value;
	test.check_equal(default_value.x, 0.0, "Xyz<double> default x");
	test.check_equal(default_value.y, 0.0, "Xyz<double> default y");
	test.check_equal(default_value.z, 0.0, "Xyz<double> default z");
	test.check_equal(default_value.alpha, 1.0, "Xyz<double> default alpha");

	// XYZ constructor fills alpha with one.
	mt::Xyz<double> xyz(0.95047, 1.0, 1.08883);
	test.check_equal(xyz.x, 0.95047, "Xyz<double> xyz x");
	test.check_equal(xyz.y, 1.0, "Xyz<double> xyz y");
	test.check_equal(xyz.z, 1.08883, "Xyz<double> xyz z");
	test.check_equal(xyz.alpha, 1.0, "Xyz<double> xyz alpha");

	// XYZA constructor stores every component directly.
	mt::Xyz<double> xyza(0.1, 0.2, 0.3, 0.4);
	test.check_equal(xyza.x, 0.1, "Xyz<double> xyza x");
	test.check_equal(xyza.y, 0.2, "Xyz<double> xyza y");
	test.check_equal(xyza.z, 0.3, "Xyz<double> xyza z");
	test.check_equal(xyza.alpha, 0.4, "Xyz<double> xyza alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::Xyz<double> clamped = mt::Xyz<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.x, 0.0, "Xyz<double> clamp x");
	test.check_equal(clamped.y, 0.5, "Xyz<double> clamp y");
	test.check_equal(clamped.z, 1.0, "Xyz<double> clamp z");
	test.check_equal(clamped.alpha, 1.0, "Xyz<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Xyz<double> no_negatives = mt::Xyz<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.x, 0.0, "Xyz<double> no_negatives x");
	test.check_equal(no_negatives.y, 0.5, "Xyz<double> no_negatives y");
	test.check_equal(no_negatives.z, 1.25, "Xyz<double> no_negatives z");
	test.check_equal(no_negatives.alpha, 0.0, "Xyz<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Xyz<double> sanitised = mt::Xyz<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.x, 0.0, "Xyz<double> sanitise negative zero");
	test.check_equal(sanitised.y, 0.0, "Xyz<double> sanitise nan");
	test.check_equal(sanitised.z, std::numeric_limits<double>::max(), "Xyz<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Xyz<double> sanitise negative infinity");
}

int main() {
	TestContext test;

	test_xyz_double(test);

	return test.result();
}
