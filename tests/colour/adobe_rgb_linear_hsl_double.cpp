/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar AdobeRgbLinearHsl<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/adobe_rgb_reference.h"

void test_adobe_rgb_linear_hsl_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::AdobeRgbLinearHsl<double> default_value;
	test.check_equal(default_value.hue, 0.0, "AdobeRgbLinearHsl<double> default h");
	test.check_equal(default_value.saturation, 0.0, "AdobeRgbLinearHsl<double> default s");
	test.check_equal(default_value.lightness, 0.0, "AdobeRgbLinearHsl<double> default l");
	test.check_equal(default_value.alpha, 1.0, "AdobeRgbLinearHsl<double> default a");

	// Three-component constructor fills alpha with one.
	mt::AdobeRgbLinearHsl<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "AdobeRgbLinearHsl<double> HSL h");
	test.check_equal(hsa.saturation, 0.2, "AdobeRgbLinearHsl<double> HSL s");
	test.check_equal(hsa.lightness, 0.3, "AdobeRgbLinearHsl<double> HSL l");
	test.check_equal(hsa.alpha, 1.0, "AdobeRgbLinearHsl<double> HSL a");

	// Four-component constructor stores every component directly.
	mt::AdobeRgbLinearHsl<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "AdobeRgbLinearHsl<double> HSLA h");
	test.check_equal(hsla.saturation, 0.2, "AdobeRgbLinearHsl<double> HSLA s");
	test.check_equal(hsla.lightness, 0.3, "AdobeRgbLinearHsl<double> HSLA l");
	test.check_equal(hsla.alpha, 0.4, "AdobeRgbLinearHsl<double> HSLA a");

	// Clamp restricts every component to the 0..1 range.
	mt::AdobeRgbLinearHsl<double> clamped = mt::AdobeRgbLinearHsl<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "AdobeRgbLinearHsl<double> clamp h");
	test.check_equal(clamped.saturation, 0.5, "AdobeRgbLinearHsl<double> clamp s");
	test.check_equal(clamped.lightness, 1.0, "AdobeRgbLinearHsl<double> clamp l");
	test.check_equal(clamped.alpha, 1.0, "AdobeRgbLinearHsl<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::AdobeRgbLinearHsl<double> no_negatives = mt::AdobeRgbLinearHsl<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "AdobeRgbLinearHsl<double> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5, "AdobeRgbLinearHsl<double> no_negatives s");
	test.check_equal(no_negatives.lightness, 1.25, "AdobeRgbLinearHsl<double> no_negatives l");
	test.check_equal(no_negatives.alpha, 0.0, "AdobeRgbLinearHsl<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::AdobeRgbLinearHsl<double> sanitised = mt::AdobeRgbLinearHsl<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "AdobeRgbLinearHsl<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "AdobeRgbLinearHsl<double> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<double>::max(), "AdobeRgbLinearHsl<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "AdobeRgbLinearHsl<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::AdobeRgbLinearHsl<double> wrapped_negative = mt::AdobeRgbLinearHsl<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "AdobeRgbLinearHsl<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "AdobeRgbLinearHsl<double> wrap negative s");
	test.check_equal(wrapped_negative.lightness, 0.75, "AdobeRgbLinearHsl<double> wrap negative l");
	test.check_equal(wrapped_negative.alpha, 1.0, "AdobeRgbLinearHsl<double> wrap negative a");

	mt::AdobeRgbLinearHsl<double> wrapped_over_one = mt::AdobeRgbLinearHsl<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "AdobeRgbLinearHsl<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "AdobeRgbLinearHsl<double> wrap over-one s");
	test.check_equal(wrapped_over_one.lightness, 0.75, "AdobeRgbLinearHsl<double> wrap over-one l");
	test.check_equal(wrapped_over_one.alpha, 1.0, "AdobeRgbLinearHsl<double> wrap over-one a");

	// Reference data checks linear Adobe RGB (1998) HSL to linear Adobe RGB (1998).
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_hsl_cases) {
		mt::AdobeRgbLinearHsl<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::AdobeRgbLinear<double> actual = colour.to_adobe_rgb_linear();
		std::string message = std::string("AdobeRgbLinearHsl<double> to_adobe_rgb_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_adobe_rgb_linear_hsl_double(test);

	return test.result();
}
