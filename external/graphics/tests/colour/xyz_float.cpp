/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Xyz<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <limits>

#include "colour/xyz.h"

void test_xyz_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Xyz<float> default_value;
	test.check_equal(default_value.x, 0.0f, "Xyz<float> default x");
	test.check_equal(default_value.y, 0.0f, "Xyz<float> default y");
	test.check_equal(default_value.z, 0.0f, "Xyz<float> default z");
	test.check_equal(default_value.alpha, 1.0f, "Xyz<float> default alpha");

	// XYZ constructor fills alpha with one.
	mt::Xyz<float> xyz(0.95047f, 1.0f, 1.08883f);
	test.check_equal(xyz.x, 0.95047f, "Xyz<float> xyz x");
	test.check_equal(xyz.y, 1.0f, "Xyz<float> xyz y");
	test.check_equal(xyz.z, 1.08883f, "Xyz<float> xyz z");
	test.check_equal(xyz.alpha, 1.0f, "Xyz<float> xyz alpha");

	// XYZA constructor stores every component directly.
	mt::Xyz<float> xyza(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(xyza.x, 0.1f, "Xyz<float> xyza x");
	test.check_equal(xyza.y, 0.2f, "Xyz<float> xyza y");
	test.check_equal(xyza.z, 0.3f, "Xyz<float> xyza z");
	test.check_equal(xyza.alpha, 0.4f, "Xyz<float> xyza alpha");

	// mix interpolates XYZ and alpha components in the current coordinate space.
	mt::Xyz<float> mixed = mt::mix(mt::Xyz<float>(0.0f, 0.25f, 0.5f, 0.25f), mt::Xyz<float>(1.0f, 0.75f, 0.0f, 0.75f), 0.5f);
	test.check_close(mixed.x, 0.5f, "Xyz<float> mix x");
	test.check_close(mixed.y, 0.5f, "Xyz<float> mix y");
	test.check_close(mixed.z, 0.25f, "Xyz<float> mix z");
	test.check_close(mixed.alpha, 0.5f, "Xyz<float> mix alpha");

	mt::Xyz<float> lerped = mt::lerp(mt::Xyz<float>(0.0f, 0.25f, 0.5f, 0.25f), mt::Xyz<float>(1.0f, 0.75f, 0.0f, 0.75f), 0.5f);
	test.check_close(lerped.x, mixed.x, "Xyz<float> lerp alias x");
	test.check_close(lerped.y, mixed.y, "Xyz<float> lerp alias y");
	test.check_close(lerped.z, mixed.z, "Xyz<float> lerp alias z");
	test.check_close(lerped.alpha, mixed.alpha, "Xyz<float> lerp alias alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::Xyz<float> clamped = mt::Xyz<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.x, 0.0f, "Xyz<float> clamp x");
	test.check_equal(clamped.y, 0.5f, "Xyz<float> clamp y");
	test.check_equal(clamped.z, 1.0f, "Xyz<float> clamp z");
	test.check_equal(clamped.alpha, 1.0f, "Xyz<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Xyz<float> no_negatives = mt::Xyz<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.x, 0.0f, "Xyz<float> no_negatives x");
	test.check_equal(no_negatives.y, 0.5f, "Xyz<float> no_negatives y");
	test.check_equal(no_negatives.z, 1.25f, "Xyz<float> no_negatives z");
	test.check_equal(no_negatives.alpha, 0.0f, "Xyz<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Xyz<float> sanitised = mt::Xyz<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.x, 0.0f, "Xyz<float> sanitise negative zero");
	test.check_equal(sanitised.y, 0.0f, "Xyz<float> sanitise nan");
	test.check_equal(sanitised.z, std::numeric_limits<float>::max(), "Xyz<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Xyz<float> sanitise negative infinity");
}

int main() {
	TestContext test;

	test_xyz_float(test);

	return test.result();
}
