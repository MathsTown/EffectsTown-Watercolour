/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Oklab<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/oklab_reference.h"

void test_oklab_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Oklab<float> default_value;
	test.check_equal(default_value.lightness, 0.0f, "Oklab<float> default l");
	test.check_equal(default_value.a, 0.0f, "Oklab<float> default a");
	test.check_equal(default_value.b, 0.0f, "Oklab<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "Oklab<float> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Oklab<float> lab(0.1f, -0.2f, 0.3f);
	test.check_equal(lab.lightness, 0.1f, "Oklab<float> lab l");
	test.check_equal(lab.a, -0.2f, "Oklab<float> lab a");
	test.check_equal(lab.b, 0.3f, "Oklab<float> lab b");
	test.check_equal(lab.alpha, 1.0f, "Oklab<float> lab alpha");

	// Four-component constructor stores every component directly.
	mt::Oklab<float> laba(0.1f, -0.2f, 0.3f, 0.4f);
	test.check_equal(laba.lightness, 0.1f, "Oklab<float> laba l");
	test.check_equal(laba.a, -0.2f, "Oklab<float> laba a");
	test.check_equal(laba.b, 0.3f, "Oklab<float> laba b");
	test.check_equal(laba.alpha, 0.4f, "Oklab<float> laba alpha");

	// mix interpolates LAB and alpha components in the current coordinate space.
	mt::Oklab<float> mixed = mt::mix(mt::Oklab<float>(0.0f, -0.5f, 0.25f, 0.25f), mt::Oklab<float>(1.0f, 0.5f, -0.25f, 0.75f), 0.5f);
	test.check_close(mixed.lightness, 0.5f, "Oklab<float> mix l");
	test.check_close(mixed.a, 0.0f, "Oklab<float> mix a");
	test.check_close(mixed.b, 0.0f, "Oklab<float> mix b");
	test.check_close(mixed.alpha, 0.5f, "Oklab<float> mix alpha");

	mt::Oklab<float> lerped = mt::lerp(mt::Oklab<float>(0.0f, -0.5f, 0.25f, 0.25f), mt::Oklab<float>(1.0f, 0.5f, -0.25f, 0.75f), 0.5f);
	test.check_close(lerped.lightness, mixed.lightness, "Oklab<float> lerp alias l");
	test.check_close(lerped.a, mixed.a, "Oklab<float> lerp alias a");
	test.check_close(lerped.b, mixed.b, "Oklab<float> lerp alias b");
	test.check_close(lerped.alpha, mixed.alpha, "Oklab<float> lerp alias alpha");

	// Clamp restricts bounded components to the 0..1 range.
	mt::Oklab<float> clamped = mt::Oklab<float>(-0.25f, -0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.lightness, 0.0f, "Oklab<float> clamp l");
	test.check_equal(clamped.a, -0.5f, "Oklab<float> clamp a");
	test.check_equal(clamped.b, 1.25f, "Oklab<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "Oklab<float> clamp alpha");

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Oklab<float> no_negatives = mt::Oklab<float>(-0.25f, -0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0f, "Oklab<float> no_negatives l");
	test.check_equal(no_negatives.a, -0.5f, "Oklab<float> no_negatives a");
	test.check_equal(no_negatives.b, 1.25f, "Oklab<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "Oklab<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Oklab<float> sanitised = mt::Oklab<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0f, "Oklab<float> sanitise negative zero");
	test.check_equal(sanitised.a, 0.0f, "Oklab<float> sanitise nan");
	test.check_equal(sanitised.b, std::numeric_limits<float>::max(), "Oklab<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Oklab<float> sanitise negative infinity");

	// Reference data checks CIE XYZ D65 to Oklab.
	for (const auto& colour_case : colour_reference::xyz_d65_oklab_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Oklab<float> actual = colour.to_oklab();
		std::string message = std::string("Oklab<float> from Xyz ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 5.0e-5f, message + " l");
		test.check_close(actual.a, colour_case.expected[1], 5.0e-5f, message + " a");
		test.check_close(actual.b, colour_case.expected[2], 5.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-5f, message + " alpha");
	}

	// Reference data checks Oklab back to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::xyz_d65_oklab_cases) {
		mt::Oklab<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("Oklab<float> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 8.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 8.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 8.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 8.0e-5f, message + " a");
	}

	// Reference data checks Oklab to Oklch.
	const int case_count = int(sizeof(colour_reference::xyz_d65_oklab_cases) / sizeof(colour_reference::xyz_d65_oklab_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& oklab_case = colour_reference::xyz_d65_oklab_cases[index];
		const auto& oklch_case = colour_reference::xyz_d65_oklch_cases[index];
		mt::Oklab<float> colour(static_cast<float>(oklab_case.expected[0]), static_cast<float>(oklab_case.expected[1]), static_cast<float>(oklab_case.expected[2]), static_cast<float>(oklab_case.expected[3]));
		mt::Oklch<float> actual = colour.to_oklch();
		std::string message = std::string("Oklab<float> to Oklch ") + oklab_case.name;
		test.check_close(actual.lightness, oklch_case.expected[0], 5.0e-5f, message + " l");
		test.check_close(actual.chroma, oklch_case.expected[1], 5.0e-5f, message + " c");
		if (oklch_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, oklch_case.expected[2], 5.0e-4f, message + " h");
		}
		test.check_close(actual.alpha, oklch_case.expected[3], 5.0e-5f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_oklab_float(test);

	return test.result();
}
