/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbHsv<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_hsv_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbHsv<float> default_value;
	test.check_equal(default_value.hue, 0.0f, "SrgbHsv<float> default hue");
	test.check_equal(default_value.saturation, 0.0f, "SrgbHsv<float> default saturation");
	test.check_equal(default_value.value, 0.0f, "SrgbHsv<float> default value");
	test.check_equal(default_value.alpha, 1.0f, "SrgbHsv<float> default alpha");

	// Three-component constructor fills alpha with one.
	mt::SrgbHsv<float> hsa(0.1f, 0.2f, 0.3f);
	test.check_equal(hsa.hue, 0.1f, "SrgbHsv<float> HSV hue");
	test.check_equal(hsa.saturation, 0.2f, "SrgbHsv<float> HSV saturation");
	test.check_equal(hsa.value, 0.3f, "SrgbHsv<float> HSV value");
	test.check_equal(hsa.alpha, 1.0f, "SrgbHsv<float> HSV alpha");

	// Four-component constructor stores every component directly.
	mt::SrgbHsv<float> hsla(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(hsla.hue, 0.1f, "SrgbHsv<float> HSVA hue");
	test.check_equal(hsla.saturation, 0.2f, "SrgbHsv<float> HSVA saturation");
	test.check_equal(hsla.value, 0.3f, "SrgbHsv<float> HSVA value");
	test.check_equal(hsla.alpha, 0.4f, "SrgbHsv<float> HSVA alpha");

	// mix interpolates hue along the shortest circular path and linearly mixes other components.
	mt::SrgbHsv<float> mixed = mt::mix(mt::SrgbHsv<float>(0.05f, 0.2f, 0.4f, 0.25f), mt::SrgbHsv<float>(0.95f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(mixed.hue, 0.0f, 1.0e-6f, "SrgbHsv<float> mix shortest hue");
	test.check_close(mixed.saturation, 0.4f, "SrgbHsv<float> mix saturation");
	test.check_close(mixed.value, 0.6f, "SrgbHsv<float> mix value");
	test.check_close(mixed.alpha, 0.5f, "SrgbHsv<float> mix alpha");

	mt::SrgbHsv<float> lerped = mt::lerp(mt::SrgbHsv<float>(0.05f, 0.2f, 0.4f, 0.25f), mt::SrgbHsv<float>(0.95f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(lerped.hue, mixed.hue, 1.0e-6f, "SrgbHsv<float> lerp alias hue");
	test.check_close(lerped.saturation, mixed.saturation, "SrgbHsv<float> lerp alias saturation");
	test.check_close(lerped.value, mixed.value, "SrgbHsv<float> lerp alias value");
	test.check_close(lerped.alpha, mixed.alpha, "SrgbHsv<float> lerp alias alpha");

	// mix_longest_path interpolates hue along the longest circular path.
	mt::SrgbHsv<float> longest = mt::mix_longest_path(mt::SrgbHsv<float>(0.05f, 0.2f, 0.4f, 0.25f), mt::SrgbHsv<float>(0.95f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(longest.hue, 0.5f, 1.0e-6f, "SrgbHsv<float> mix_longest_path hue");
	test.check_close(longest.saturation, 0.4f, "SrgbHsv<float> mix_longest_path saturation");
	test.check_close(longest.value, 0.6f, "SrgbHsv<float> mix_longest_path value");
	test.check_close(longest.alpha, 0.5f, "SrgbHsv<float> mix_longest_path alpha");

	mt::SrgbHsv<float> same_hue_longest = mt::mix_longest_path(mt::SrgbHsv<float>(0.25f, 0.2f, 0.4f, 0.25f), mt::SrgbHsv<float>(0.25f, 0.6f, 0.8f, 0.75f), 0.5f);
	test.check_close_hue(same_hue_longest.hue, 0.75f, 1.0e-6f, "SrgbHsv<float> mix_longest_path same hue");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbHsv<float> clamped = mt::SrgbHsv<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.hue, 0.0f, "SrgbHsv<float> clamp hue");
	test.check_equal(clamped.saturation, 0.5f, "SrgbHsv<float> clamp saturation");
	test.check_equal(clamped.value, 1.0f, "SrgbHsv<float> clamp value");
	test.check_equal(clamped.alpha, 1.0f, "SrgbHsv<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbHsv<float> no_negatives = mt::SrgbHsv<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.hue, 0.0f, "SrgbHsv<float> no_negatives hue");
	test.check_equal(no_negatives.saturation, 0.5f, "SrgbHsv<float> no_negatives saturation");
	test.check_equal(no_negatives.value, 1.25f, "SrgbHsv<float> no_negatives value");
	test.check_equal(no_negatives.alpha, 0.0f, "SrgbHsv<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbHsv<float> sanitised = mt::SrgbHsv<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0f, "SrgbHsv<float> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0f, "SrgbHsv<float> sanitise nan");
	test.check_equal(sanitised.value, std::numeric_limits<float>::max(), "SrgbHsv<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "SrgbHsv<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbHsv<float> wrapped_negative = mt::SrgbHsv<float>(-0.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75f, "SrgbHsv<float> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5f, "SrgbHsv<float> wrap negative saturation");
	test.check_equal(wrapped_negative.value, 0.75f, "SrgbHsv<float> wrap negative value");
	test.check_equal(wrapped_negative.alpha, 1.0f, "SrgbHsv<float> wrap negative alpha");

	mt::SrgbHsv<float> wrapped_over_one = mt::SrgbHsv<float>(1.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25f, "SrgbHsv<float> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5f, "SrgbHsv<float> wrap over-one saturation");
	test.check_equal(wrapped_over_one.value, 0.75f, "SrgbHsv<float> wrap over-one value");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "SrgbHsv<float> wrap over-one alpha");

	// Reference data checks transfer-encoded sRGB HSV to transfer-encoded sRGB.
	for (const auto& colour_case : colour_reference::srgb_hsv_cases) {
		mt::SrgbHsv<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Srgb<float> actual = colour.to_srgb();
		std::string message = std::string("SrgbHsv<float> to_srgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 3.0e-5f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_hsv_float(test);

	return test.result();
}
