/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbHsl<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_hsl_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbHsl<float> default_value;
	test.check_equal(default_value.hue, 0.0f, "SrgbHsl<float> default hue");
	test.check_equal(default_value.saturation, 0.0f, "SrgbHsl<float> default saturation");
	test.check_equal(default_value.lightness, 0.0f, "SrgbHsl<float> default lightness");
	test.check_equal(default_value.alpha, 1.0f, "SrgbHsl<float> default alpha");

	// Three-component constructor fills alpha with one.
	mt::SrgbHsl<float> hsa(0.1f, 0.2f, 0.3f);
	test.check_equal(hsa.hue, 0.1f, "SrgbHsl<float> HSL hue");
	test.check_equal(hsa.saturation, 0.2f, "SrgbHsl<float> HSL saturation");
	test.check_equal(hsa.lightness, 0.3f, "SrgbHsl<float> HSL lightness");
	test.check_equal(hsa.alpha, 1.0f, "SrgbHsl<float> HSL alpha");

	// Four-component constructor stores every component directly.
	mt::SrgbHsl<float> hsla(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(hsla.hue, 0.1f, "SrgbHsl<float> HSLA hue");
	test.check_equal(hsla.saturation, 0.2f, "SrgbHsl<float> HSLA saturation");
	test.check_equal(hsla.lightness, 0.3f, "SrgbHsl<float> HSLA lightness");
	test.check_equal(hsla.alpha, 0.4f, "SrgbHsl<float> HSLA alpha");

	// mix interpolates hue along the shortest circular path and linearly mixes other components.
	mt::SrgbHsl<float> mixed = mt::mix(mt::SrgbHsl<float>(0.95f, 0.2f, 0.4f, 0.25f), mt::SrgbHsl<float>(0.05f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(mixed.hue, 0.0f, 1.0e-6f, "SrgbHsl<float> mix shortest hue");
	test.check_close(mixed.saturation, 0.4f, "SrgbHsl<float> mix saturation");
	test.check_close(mixed.lightness, 0.6f, "SrgbHsl<float> mix lightness");
	test.check_close(mixed.alpha, 0.5f, "SrgbHsl<float> mix alpha");

	mt::SrgbHsl<float> lerped = mt::lerp(mt::SrgbHsl<float>(0.95f, 0.2f, 0.4f, 0.25f), mt::SrgbHsl<float>(0.05f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(lerped.hue, mixed.hue, 1.0e-6f, "SrgbHsl<float> lerp alias hue");
	test.check_close(lerped.saturation, mixed.saturation, "SrgbHsl<float> lerp alias saturation");
	test.check_close(lerped.lightness, mixed.lightness, "SrgbHsl<float> lerp alias lightness");
	test.check_close(lerped.alpha, mixed.alpha, "SrgbHsl<float> lerp alias alpha");

	// mix_longest_path interpolates hue along the longest circular path.
	mt::SrgbHsl<float> longest = mt::mix_longest_path(mt::SrgbHsl<float>(0.95f, 0.2f, 0.4f, 0.25f), mt::SrgbHsl<float>(0.05f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(longest.hue, 0.5f, 1.0e-6f, "SrgbHsl<float> mix_longest_path hue");
	test.check_close(longest.saturation, 0.4f, "SrgbHsl<float> mix_longest_path saturation");
	test.check_close(longest.lightness, 0.6f, "SrgbHsl<float> mix_longest_path lightness");
	test.check_close(longest.alpha, 0.5f, "SrgbHsl<float> mix_longest_path alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbHsl<float> clamped = mt::SrgbHsl<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.hue, 0.0f, "SrgbHsl<float> clamp hue");
	test.check_equal(clamped.saturation, 0.5f, "SrgbHsl<float> clamp saturation");
	test.check_equal(clamped.lightness, 1.0f, "SrgbHsl<float> clamp lightness");
	test.check_equal(clamped.alpha, 1.0f, "SrgbHsl<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbHsl<float> no_negatives = mt::SrgbHsl<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.hue, 0.0f, "SrgbHsl<float> no_negatives hue");
	test.check_equal(no_negatives.saturation, 0.5f, "SrgbHsl<float> no_negatives saturation");
	test.check_equal(no_negatives.lightness, 1.25f, "SrgbHsl<float> no_negatives lightness");
	test.check_equal(no_negatives.alpha, 0.0f, "SrgbHsl<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbHsl<float> sanitised = mt::SrgbHsl<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0f, "SrgbHsl<float> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0f, "SrgbHsl<float> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<float>::max(), "SrgbHsl<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "SrgbHsl<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbHsl<float> wrapped_negative = mt::SrgbHsl<float>(-0.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75f, "SrgbHsl<float> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5f, "SrgbHsl<float> wrap negative saturation");
	test.check_equal(wrapped_negative.lightness, 0.75f, "SrgbHsl<float> wrap negative lightness");
	test.check_equal(wrapped_negative.alpha, 1.0f, "SrgbHsl<float> wrap negative alpha");

	mt::SrgbHsl<float> wrapped_over_one = mt::SrgbHsl<float>(1.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25f, "SrgbHsl<float> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5f, "SrgbHsl<float> wrap over-one saturation");
	test.check_equal(wrapped_over_one.lightness, 0.75f, "SrgbHsl<float> wrap over-one lightness");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "SrgbHsl<float> wrap over-one alpha");

	// Reference data checks gamma-2.4 encoded sRGB HSL to gamma-2.4 encoded sRGB.
	for (const auto& colour_case : colour_reference::srgb_hsl_cases) {
		mt::SrgbHsl<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Srgb<float> actual = colour.to_srgb();
		std::string message = std::string("SrgbHsl<float> to_srgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-5f, message + " red");
		test.check_close(actual.green, colour_case.source[1], 3.0e-5f, message + " green");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-5f, message + " blue");
		test.check_close(actual.alpha, colour_case.source[3], 3.0e-5f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_hsl_float(test);

	return test.result();
}
