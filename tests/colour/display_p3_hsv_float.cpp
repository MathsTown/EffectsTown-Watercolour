/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar DisplayP3Hsv<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/display_p3_reference.h"

void test_display_p3_hsv_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::DisplayP3Hsv<float> default_value;
	test.check_equal(default_value.hue, 0.0f, "DisplayP3Hsv<float> default h");
	test.check_equal(default_value.saturation, 0.0f, "DisplayP3Hsv<float> default s");
	test.check_equal(default_value.value, 0.0f, "DisplayP3Hsv<float> default v");
	test.check_equal(default_value.alpha, 1.0f, "DisplayP3Hsv<float> default a");

	// Three-component constructor fills alpha with one.
	mt::DisplayP3Hsv<float> hsa(0.1f, 0.2f, 0.3f);
	test.check_equal(hsa.hue, 0.1f, "DisplayP3Hsv<float> HSV h");
	test.check_equal(hsa.saturation, 0.2f, "DisplayP3Hsv<float> HSV s");
	test.check_equal(hsa.value, 0.3f, "DisplayP3Hsv<float> HSV v");
	test.check_equal(hsa.alpha, 1.0f, "DisplayP3Hsv<float> HSV a");

	// Four-component constructor stores every component directly.
	mt::DisplayP3Hsv<float> hsla(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(hsla.hue, 0.1f, "DisplayP3Hsv<float> HSVA h");
	test.check_equal(hsla.saturation, 0.2f, "DisplayP3Hsv<float> HSVA s");
	test.check_equal(hsla.value, 0.3f, "DisplayP3Hsv<float> HSVA v");
	test.check_equal(hsla.alpha, 0.4f, "DisplayP3Hsv<float> HSVA a");

	// Clamp restricts every component to the 0..1 range.
	mt::DisplayP3Hsv<float> clamped = mt::DisplayP3Hsv<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.hue, 0.0f, "DisplayP3Hsv<float> clamp h");
	test.check_equal(clamped.saturation, 0.5f, "DisplayP3Hsv<float> clamp s");
	test.check_equal(clamped.value, 1.0f, "DisplayP3Hsv<float> clamp v");
	test.check_equal(clamped.alpha, 1.0f, "DisplayP3Hsv<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::DisplayP3Hsv<float> no_negatives = mt::DisplayP3Hsv<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.hue, 0.0f, "DisplayP3Hsv<float> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5f, "DisplayP3Hsv<float> no_negatives s");
	test.check_equal(no_negatives.value, 1.25f, "DisplayP3Hsv<float> no_negatives v");
	test.check_equal(no_negatives.alpha, 0.0f, "DisplayP3Hsv<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::DisplayP3Hsv<float> sanitised = mt::DisplayP3Hsv<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0f, "DisplayP3Hsv<float> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0f, "DisplayP3Hsv<float> sanitise nan");
	test.check_equal(sanitised.value, std::numeric_limits<float>::max(), "DisplayP3Hsv<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "DisplayP3Hsv<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::DisplayP3Hsv<float> wrapped_negative = mt::DisplayP3Hsv<float>(-0.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75f, "DisplayP3Hsv<float> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5f, "DisplayP3Hsv<float> wrap negative s");
	test.check_equal(wrapped_negative.value, 0.75f, "DisplayP3Hsv<float> wrap negative v");
	test.check_equal(wrapped_negative.alpha, 1.0f, "DisplayP3Hsv<float> wrap negative a");

	mt::DisplayP3Hsv<float> wrapped_over_one = mt::DisplayP3Hsv<float>(1.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25f, "DisplayP3Hsv<float> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5f, "DisplayP3Hsv<float> wrap over-one s");
	test.check_equal(wrapped_over_one.value, 0.75f, "DisplayP3Hsv<float> wrap over-one v");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "DisplayP3Hsv<float> wrap over-one a");

	// Reference data checks gamma-2.4 encoded Display P3 HSV to gamma-2.4 encoded Display P3.
	for (const auto& colour_case : colour_reference::display_p3_hsv_cases) {
		mt::DisplayP3Hsv<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::DisplayP3<float> actual = colour.to_display_p3();
		std::string message = std::string("DisplayP3Hsv<float> to_display_p3 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 3.0e-5f, message + " a");
	}
}

int main() {
	TestContext test;

	test_display_p3_hsv_float(test);

	return test.result();
}
