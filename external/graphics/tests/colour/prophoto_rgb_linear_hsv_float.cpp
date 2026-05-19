/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar ProPhotoRgbLinearHsv<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/prophoto_rgb_reference.h"

void test_prophoto_rgb_linear_hsv_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::ProPhotoRgbLinearHsv<float> default_value;
	test.check_equal(default_value.hue, 0.0f, "ProPhotoRgbLinearHsv<float> default h");
	test.check_equal(default_value.saturation, 0.0f, "ProPhotoRgbLinearHsv<float> default s");
	test.check_equal(default_value.value, 0.0f, "ProPhotoRgbLinearHsv<float> default v");
	test.check_equal(default_value.alpha, 1.0f, "ProPhotoRgbLinearHsv<float> default a");

	// Three-component constructor fills alpha with one.
	mt::ProPhotoRgbLinearHsv<float> hsa(0.1f, 0.2f, 0.3f);
	test.check_equal(hsa.hue, 0.1f, "ProPhotoRgbLinearHsv<float> HSV h");
	test.check_equal(hsa.saturation, 0.2f, "ProPhotoRgbLinearHsv<float> HSV s");
	test.check_equal(hsa.value, 0.3f, "ProPhotoRgbLinearHsv<float> HSV v");
	test.check_equal(hsa.alpha, 1.0f, "ProPhotoRgbLinearHsv<float> HSV a");

	// Four-component constructor stores every component directly.
	mt::ProPhotoRgbLinearHsv<float> hsla(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(hsla.hue, 0.1f, "ProPhotoRgbLinearHsv<float> HSVA h");
	test.check_equal(hsla.saturation, 0.2f, "ProPhotoRgbLinearHsv<float> HSVA s");
	test.check_equal(hsla.value, 0.3f, "ProPhotoRgbLinearHsv<float> HSVA v");
	test.check_equal(hsla.alpha, 0.4f, "ProPhotoRgbLinearHsv<float> HSVA a");

	// Clamp restricts every component to the 0..1 range.
	mt::ProPhotoRgbLinearHsv<float> clamped = mt::ProPhotoRgbLinearHsv<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.hue, 0.0f, "ProPhotoRgbLinearHsv<float> clamp h");
	test.check_equal(clamped.saturation, 0.5f, "ProPhotoRgbLinearHsv<float> clamp s");
	test.check_equal(clamped.value, 1.0f, "ProPhotoRgbLinearHsv<float> clamp v");
	test.check_equal(clamped.alpha, 1.0f, "ProPhotoRgbLinearHsv<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::ProPhotoRgbLinearHsv<float> no_negatives = mt::ProPhotoRgbLinearHsv<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.hue, 0.0f, "ProPhotoRgbLinearHsv<float> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5f, "ProPhotoRgbLinearHsv<float> no_negatives s");
	test.check_equal(no_negatives.value, 1.25f, "ProPhotoRgbLinearHsv<float> no_negatives v");
	test.check_equal(no_negatives.alpha, 0.0f, "ProPhotoRgbLinearHsv<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::ProPhotoRgbLinearHsv<float> sanitised = mt::ProPhotoRgbLinearHsv<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0f, "ProPhotoRgbLinearHsv<float> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0f, "ProPhotoRgbLinearHsv<float> sanitise nan");
	test.check_equal(sanitised.value, std::numeric_limits<float>::max(), "ProPhotoRgbLinearHsv<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "ProPhotoRgbLinearHsv<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::ProPhotoRgbLinearHsv<float> wrapped_negative = mt::ProPhotoRgbLinearHsv<float>(-0.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75f, "ProPhotoRgbLinearHsv<float> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5f, "ProPhotoRgbLinearHsv<float> wrap negative s");
	test.check_equal(wrapped_negative.value, 0.75f, "ProPhotoRgbLinearHsv<float> wrap negative v");
	test.check_equal(wrapped_negative.alpha, 1.0f, "ProPhotoRgbLinearHsv<float> wrap negative a");

	mt::ProPhotoRgbLinearHsv<float> wrapped_over_one = mt::ProPhotoRgbLinearHsv<float>(1.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25f, "ProPhotoRgbLinearHsv<float> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5f, "ProPhotoRgbLinearHsv<float> wrap over-one s");
	test.check_equal(wrapped_over_one.value, 0.75f, "ProPhotoRgbLinearHsv<float> wrap over-one v");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "ProPhotoRgbLinearHsv<float> wrap over-one a");

	// Reference data checks linear ProPhoto RGB HSV to linear ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_hsv_cases) {
		mt::ProPhotoRgbLinearHsv<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgbLinear<float> actual = colour.to_prophoto_rgb_linear();
		std::string message = std::string("ProPhotoRgbLinearHsv<float> to_prophoto_rgb_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 3.0e-5f, message + " a");
	}
}

int main() {
	TestContext test;

	test_prophoto_rgb_linear_hsv_float(test);

	return test.result();
}
