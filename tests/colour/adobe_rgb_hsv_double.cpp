/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar AdobeRgbHsv<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/adobe_rgb_reference.h"

void test_adobe_rgb_hsv_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::AdobeRgbHsv<double> default_value;
	test.check_equal(default_value.hue, 0.0, "AdobeRgbHsv<double> default h");
	test.check_equal(default_value.saturation, 0.0, "AdobeRgbHsv<double> default s");
	test.check_equal(default_value.value, 0.0, "AdobeRgbHsv<double> default v");
	test.check_equal(default_value.alpha, 1.0, "AdobeRgbHsv<double> default a");

	// Three-component constructor fills alpha with one.
	mt::AdobeRgbHsv<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "AdobeRgbHsv<double> HSV h");
	test.check_equal(hsa.saturation, 0.2, "AdobeRgbHsv<double> HSV s");
	test.check_equal(hsa.value, 0.3, "AdobeRgbHsv<double> HSV v");
	test.check_equal(hsa.alpha, 1.0, "AdobeRgbHsv<double> HSV a");

	// Four-component constructor stores every component directly.
	mt::AdobeRgbHsv<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "AdobeRgbHsv<double> HSVA h");
	test.check_equal(hsla.saturation, 0.2, "AdobeRgbHsv<double> HSVA s");
	test.check_equal(hsla.value, 0.3, "AdobeRgbHsv<double> HSVA v");
	test.check_equal(hsla.alpha, 0.4, "AdobeRgbHsv<double> HSVA a");

	// Clamp restricts every component to the 0..1 range.
	mt::AdobeRgbHsv<double> clamped = mt::AdobeRgbHsv<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "AdobeRgbHsv<double> clamp h");
	test.check_equal(clamped.saturation, 0.5, "AdobeRgbHsv<double> clamp s");
	test.check_equal(clamped.value, 1.0, "AdobeRgbHsv<double> clamp v");
	test.check_equal(clamped.alpha, 1.0, "AdobeRgbHsv<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::AdobeRgbHsv<double> no_negatives = mt::AdobeRgbHsv<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "AdobeRgbHsv<double> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5, "AdobeRgbHsv<double> no_negatives s");
	test.check_equal(no_negatives.value, 1.25, "AdobeRgbHsv<double> no_negatives v");
	test.check_equal(no_negatives.alpha, 0.0, "AdobeRgbHsv<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::AdobeRgbHsv<double> sanitised = mt::AdobeRgbHsv<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "AdobeRgbHsv<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "AdobeRgbHsv<double> sanitise nan");
	test.check_equal(sanitised.value, std::numeric_limits<double>::max(), "AdobeRgbHsv<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "AdobeRgbHsv<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::AdobeRgbHsv<double> wrapped_negative = mt::AdobeRgbHsv<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "AdobeRgbHsv<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "AdobeRgbHsv<double> wrap negative s");
	test.check_equal(wrapped_negative.value, 0.75, "AdobeRgbHsv<double> wrap negative v");
	test.check_equal(wrapped_negative.alpha, 1.0, "AdobeRgbHsv<double> wrap negative a");

	mt::AdobeRgbHsv<double> wrapped_over_one = mt::AdobeRgbHsv<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "AdobeRgbHsv<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "AdobeRgbHsv<double> wrap over-one s");
	test.check_equal(wrapped_over_one.value, 0.75, "AdobeRgbHsv<double> wrap over-one v");
	test.check_equal(wrapped_over_one.alpha, 1.0, "AdobeRgbHsv<double> wrap over-one a");

	// Reference data checks transfer-encoded Adobe RGB (1998) HSV to transfer-encoded Adobe RGB (1998).
	for (const auto& colour_case : colour_reference::adobe_rgb_hsv_cases) {
		mt::AdobeRgbHsv<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::AdobeRgb<double> actual = colour.to_adobe_rgb();
		std::string message = std::string("AdobeRgbHsv<double> to_adobe_rgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_adobe_rgb_hsv_double(test);

	return test.result();
}
