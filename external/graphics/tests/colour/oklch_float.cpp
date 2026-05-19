/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Oklch<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/oklab_reference.h"

void test_oklch_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Oklch<float> default_value;
	test.check_equal(default_value.lightness, 0.0f, "Oklch<float> default l");
	test.check_equal(default_value.chroma, 0.0f, "Oklch<float> default c");
	test.check_equal(default_value.hue, 0.0f, "Oklch<float> default h");
	test.check_equal(default_value.alpha, 1.0f, "Oklch<float> default alpha");

	// Three-component constructor fills alpha with one.
	mt::Oklch<float> lch(0.1f, 0.2f, 0.3f);
	test.check_equal(lch.lightness, 0.1f, "Oklch<float> lch l");
	test.check_equal(lch.chroma, 0.2f, "Oklch<float> lch c");
	test.check_equal(lch.hue, 0.3f, "Oklch<float> lch h");
	test.check_equal(lch.alpha, 1.0f, "Oklch<float> lch alpha");

	// Four-component constructor stores every component directly.
	mt::Oklch<float> lcha(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(lcha.lightness, 0.1f, "Oklch<float> lcha l");
	test.check_equal(lcha.chroma, 0.2f, "Oklch<float> lcha c");
	test.check_equal(lcha.hue, 0.3f, "Oklch<float> lcha h");
	test.check_equal(lcha.alpha, 0.4f, "Oklch<float> lcha alpha");

	// mix interpolates hue along the shortest circular path and linearly mixes other components.
	mt::Oklch<float> mixed = mt::mix(mt::Oklch<float>(0.0f, 0.2f, 0.95f, 0.25f), mt::Oklch<float>(1.0f, 0.6f, 0.05f, 0.75f), 0.5f);
	test.check_close(mixed.lightness, 0.5f, "Oklch<float> mix l");
	test.check_close(mixed.chroma, 0.4f, "Oklch<float> mix c");
	test.check_close_hue(mixed.hue, 0.0f, 1.0e-6f, "Oklch<float> mix shortest h");
	test.check_close(mixed.alpha, 0.5f, "Oklch<float> mix alpha");

	mt::Oklch<float> lerped = mt::lerp(mt::Oklch<float>(0.0f, 0.2f, 0.95f, 0.25f), mt::Oklch<float>(1.0f, 0.6f, 0.05f, 0.75f), 0.5f);
	test.check_close(lerped.lightness, mixed.lightness, "Oklch<float> lerp alias l");
	test.check_close(lerped.chroma, mixed.chroma, "Oklch<float> lerp alias c");
	test.check_close_hue(lerped.hue, mixed.hue, 1.0e-6f, "Oklch<float> lerp alias h");
	test.check_close(lerped.alpha, mixed.alpha, "Oklch<float> lerp alias alpha");

	// mix_longest_path interpolates hue along the longest circular path.
	mt::Oklch<float> longest = mt::mix_longest_path(mt::Oklch<float>(0.0f, 0.2f, 0.95f, 0.25f), mt::Oklch<float>(1.0f, 0.6f, 0.05f, 0.75f), 0.5f);
	test.check_close(longest.lightness, 0.5f, "Oklch<float> mix_longest_path l");
	test.check_close(longest.chroma, 0.4f, "Oklch<float> mix_longest_path c");
	test.check_close_hue(longest.hue, 0.5f, 1.0e-6f, "Oklch<float> mix_longest_path h");
	test.check_close(longest.alpha, 0.5f, "Oklch<float> mix_longest_path alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::Oklch<float> clamped = mt::Oklch<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.lightness, 0.0f, "Oklch<float> clamp l");
	test.check_equal(clamped.chroma, 0.5f, "Oklch<float> clamp c");
	test.check_equal(clamped.hue, 1.0f, "Oklch<float> clamp h");
	test.check_equal(clamped.alpha, 1.0f, "Oklch<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Oklch<float> no_negatives = mt::Oklch<float>(-0.25f, -0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.lightness, 0.0f, "Oklch<float> no_negatives l");
	test.check_equal(no_negatives.chroma, 0.0f, "Oklch<float> no_negatives c");
	test.check_equal(no_negatives.hue, 1.25f, "Oklch<float> no_negatives h");
	test.check_equal(no_negatives.alpha, 0.0f, "Oklch<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Oklch<float> sanitised = mt::Oklch<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.lightness, 0.0f, "Oklch<float> sanitise negative zero");
	test.check_equal(sanitised.chroma, 0.0f, "Oklch<float> sanitise nan");
	test.check_equal(sanitised.hue, std::numeric_limits<float>::max(), "Oklch<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Oklch<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Oklch<float> wrapped_negative = mt::Oklch<float>(0.25f, 0.5f, -0.25f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.lightness, 0.25f, "Oklch<float> wrap negative l");
	test.check_equal(wrapped_negative.chroma, 0.5f, "Oklch<float> wrap negative c");
	test.check_equal(wrapped_negative.hue, 0.75f, "Oklch<float> wrap negative h");
	test.check_equal(wrapped_negative.alpha, 1.0f, "Oklch<float> wrap negative alpha");

	mt::Oklch<float> wrapped_over_one = mt::Oklch<float>(0.25f, 0.5f, 1.25f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.lightness, 0.25f, "Oklch<float> wrap over-one l");
	test.check_equal(wrapped_over_one.chroma, 0.5f, "Oklch<float> wrap over-one c");
	test.check_equal(wrapped_over_one.hue, 0.25f, "Oklch<float> wrap over-one h");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "Oklch<float> wrap over-one alpha");

	// Reference data checks CIE XYZ D65 to Oklch.
	for (const auto& colour_case : colour_reference::xyz_d65_oklch_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Oklch<float> actual = colour.to_oklch();
		std::string message = std::string("Oklch<float> from Xyz ") + colour_case.name;
		test.check_close(actual.lightness, colour_case.expected[0], 5.0e-5f, message + " l");
		test.check_close(actual.chroma, colour_case.expected[1], 5.0e-5f, message + " c");
		if (colour_case.expected[1] > 1.0e-4) {
			test.check_close_hue(actual.hue, colour_case.expected[2], 5.0e-4f, message + " h");
		}
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-5f, message + " alpha");
	}

	// Reference data checks Oklch back to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::xyz_d65_oklch_cases) {
		mt::Oklch<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("Oklch<float> to Xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.source[0], 8.0e-5f, message + " x");
		test.check_close(actual.y, colour_case.source[1], 8.0e-5f, message + " y");
		test.check_close(actual.z, colour_case.source[2], 8.0e-5f, message + " z");
		test.check_close(actual.alpha, colour_case.source[3], 8.0e-5f, message + " a");
	}

	// Reference data checks Oklch to Oklab.
	const int case_count = int(sizeof(colour_reference::xyz_d65_oklch_cases) / sizeof(colour_reference::xyz_d65_oklch_cases[0]));
	for (int index = 0; index < case_count; ++index) {
		const auto& oklch_case = colour_reference::xyz_d65_oklch_cases[index];
		const auto& oklab_case = colour_reference::xyz_d65_oklab_cases[index];
		mt::Oklch<float> colour(static_cast<float>(oklch_case.expected[0]), static_cast<float>(oklch_case.expected[1]), static_cast<float>(oklch_case.expected[2]), static_cast<float>(oklch_case.expected[3]));
		mt::Oklab<float> actual = colour.to_oklab();
		std::string message = std::string("Oklch<float> to Oklab ") + oklch_case.name;
		test.check_close(actual.lightness, oklab_case.expected[0], 5.0e-5f, message + " l");
		test.check_close(actual.a, oklab_case.expected[1], 5.0e-5f, message + " a");
		test.check_close(actual.b, oklab_case.expected[2], 5.0e-5f, message + " b");
		test.check_close(actual.alpha, oklab_case.expected[3], 5.0e-5f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_oklch_float(test);

	return test.result();
}
