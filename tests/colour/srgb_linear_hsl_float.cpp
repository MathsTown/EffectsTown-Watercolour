/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbLinearHsl<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_linear_hsl_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbLinearHsl<float> default_value;
	test.check_equal(default_value.hue, 0.0f, "SrgbLinearHsl<float> default h");
	test.check_equal(default_value.saturation, 0.0f, "SrgbLinearHsl<float> default s");
	test.check_equal(default_value.lightness, 0.0f, "SrgbLinearHsl<float> default l");
	test.check_equal(default_value.alpha, 1.0f, "SrgbLinearHsl<float> default a");

	// Three-component constructor fills alpha with one.
	mt::SrgbLinearHsl<float> hsa(0.1f, 0.2f, 0.3f);
	test.check_equal(hsa.hue, 0.1f, "SrgbLinearHsl<float> HSL h");
	test.check_equal(hsa.saturation, 0.2f, "SrgbLinearHsl<float> HSL s");
	test.check_equal(hsa.lightness, 0.3f, "SrgbLinearHsl<float> HSL l");
	test.check_equal(hsa.alpha, 1.0f, "SrgbLinearHsl<float> HSL a");

	// Four-component constructor stores every component directly.
	mt::SrgbLinearHsl<float> hsla(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(hsla.hue, 0.1f, "SrgbLinearHsl<float> HSLA h");
	test.check_equal(hsla.saturation, 0.2f, "SrgbLinearHsl<float> HSLA s");
	test.check_equal(hsla.lightness, 0.3f, "SrgbLinearHsl<float> HSLA l");
	test.check_equal(hsla.alpha, 0.4f, "SrgbLinearHsl<float> HSLA a");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbLinearHsl<float> clamped = mt::SrgbLinearHsl<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.hue, 0.0f, "SrgbLinearHsl<float> clamp h");
	test.check_equal(clamped.saturation, 0.5f, "SrgbLinearHsl<float> clamp s");
	test.check_equal(clamped.lightness, 1.0f, "SrgbLinearHsl<float> clamp l");
	test.check_equal(clamped.alpha, 1.0f, "SrgbLinearHsl<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbLinearHsl<float> no_negatives = mt::SrgbLinearHsl<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.hue, 0.0f, "SrgbLinearHsl<float> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5f, "SrgbLinearHsl<float> no_negatives s");
	test.check_equal(no_negatives.lightness, 1.25f, "SrgbLinearHsl<float> no_negatives l");
	test.check_equal(no_negatives.alpha, 0.0f, "SrgbLinearHsl<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbLinearHsl<float> sanitised = mt::SrgbLinearHsl<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0f, "SrgbLinearHsl<float> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0f, "SrgbLinearHsl<float> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<float>::max(), "SrgbLinearHsl<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "SrgbLinearHsl<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbLinearHsl<float> wrapped_negative = mt::SrgbLinearHsl<float>(-0.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75f, "SrgbLinearHsl<float> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5f, "SrgbLinearHsl<float> wrap negative s");
	test.check_equal(wrapped_negative.lightness, 0.75f, "SrgbLinearHsl<float> wrap negative l");
	test.check_equal(wrapped_negative.alpha, 1.0f, "SrgbLinearHsl<float> wrap negative a");

	mt::SrgbLinearHsl<float> wrapped_over_one = mt::SrgbLinearHsl<float>(1.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25f, "SrgbLinearHsl<float> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5f, "SrgbLinearHsl<float> wrap over-one s");
	test.check_equal(wrapped_over_one.lightness, 0.75f, "SrgbLinearHsl<float> wrap over-one l");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "SrgbLinearHsl<float> wrap over-one a");

	// Reference data checks linear sRGB HSL to linear sRGB.
	for (const auto& colour_case : colour_reference::srgb_linear_hsl_cases) {
		mt::SrgbLinearHsl<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::SrgbLinear<float> actual = colour.to_srgb_linear();
		std::string message = std::string("SrgbLinearHsl<float> to_srgb_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 3.0e-5f, message + " a");
	}
}

int main() {
	TestContext test;

	test_srgb_linear_hsl_float(test);

	return test.result();
}
