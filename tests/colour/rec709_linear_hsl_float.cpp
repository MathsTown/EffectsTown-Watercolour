/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Rec709LinearHsl<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/rec709_reference.h"

void test_rec709_linear_hsl_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Rec709LinearHsl<float> default_value;
	test.check_equal(default_value.hue, 0.0f, "Rec709LinearHsl<float> default h");
	test.check_equal(default_value.saturation, 0.0f, "Rec709LinearHsl<float> default s");
	test.check_equal(default_value.lightness, 0.0f, "Rec709LinearHsl<float> default l");
	test.check_equal(default_value.alpha, 1.0f, "Rec709LinearHsl<float> default a");

	// Three-component constructor fills alpha with one.
	mt::Rec709LinearHsl<float> hsa(0.1f, 0.2f, 0.3f);
	test.check_equal(hsa.hue, 0.1f, "Rec709LinearHsl<float> HSL h");
	test.check_equal(hsa.saturation, 0.2f, "Rec709LinearHsl<float> HSL s");
	test.check_equal(hsa.lightness, 0.3f, "Rec709LinearHsl<float> HSL l");
	test.check_equal(hsa.alpha, 1.0f, "Rec709LinearHsl<float> HSL a");

	// Four-component constructor stores every component directly.
	mt::Rec709LinearHsl<float> hsla(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(hsla.hue, 0.1f, "Rec709LinearHsl<float> HSLA h");
	test.check_equal(hsla.saturation, 0.2f, "Rec709LinearHsl<float> HSLA s");
	test.check_equal(hsla.lightness, 0.3f, "Rec709LinearHsl<float> HSLA l");
	test.check_equal(hsla.alpha, 0.4f, "Rec709LinearHsl<float> HSLA a");

	// Clamp restricts every component to the 0..1 range.
	mt::Rec709LinearHsl<float> clamped = mt::Rec709LinearHsl<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.hue, 0.0f, "Rec709LinearHsl<float> clamp h");
	test.check_equal(clamped.saturation, 0.5f, "Rec709LinearHsl<float> clamp s");
	test.check_equal(clamped.lightness, 1.0f, "Rec709LinearHsl<float> clamp l");
	test.check_equal(clamped.alpha, 1.0f, "Rec709LinearHsl<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Rec709LinearHsl<float> no_negatives = mt::Rec709LinearHsl<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.hue, 0.0f, "Rec709LinearHsl<float> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5f, "Rec709LinearHsl<float> no_negatives s");
	test.check_equal(no_negatives.lightness, 1.25f, "Rec709LinearHsl<float> no_negatives l");
	test.check_equal(no_negatives.alpha, 0.0f, "Rec709LinearHsl<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec709LinearHsl<float> sanitised = mt::Rec709LinearHsl<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0f, "Rec709LinearHsl<float> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0f, "Rec709LinearHsl<float> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<float>::max(), "Rec709LinearHsl<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Rec709LinearHsl<float> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Rec709LinearHsl<float> wrapped_negative = mt::Rec709LinearHsl<float>(-0.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75f, "Rec709LinearHsl<float> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5f, "Rec709LinearHsl<float> wrap negative s");
	test.check_equal(wrapped_negative.lightness, 0.75f, "Rec709LinearHsl<float> wrap negative l");
	test.check_equal(wrapped_negative.alpha, 1.0f, "Rec709LinearHsl<float> wrap negative a");

	mt::Rec709LinearHsl<float> wrapped_over_one = mt::Rec709LinearHsl<float>(1.25f, 0.5f, 0.75f, 1.0f).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25f, "Rec709LinearHsl<float> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5f, "Rec709LinearHsl<float> wrap over-one s");
	test.check_equal(wrapped_over_one.lightness, 0.75f, "Rec709LinearHsl<float> wrap over-one l");
	test.check_equal(wrapped_over_one.alpha, 1.0f, "Rec709LinearHsl<float> wrap over-one a");

	// Reference data checks linear Rec.709 HSL to linear Rec.709.
	for (const auto& colour_case : colour_reference::rec709_linear_hsl_cases) {
		mt::Rec709LinearHsl<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Rec709Linear<float> actual = colour.to_rec709_linear();
		std::string message = std::string("Rec709LinearHsl<float> to_rec709_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 3.0e-5f, message + " a");
	}
}

int main() {
	TestContext test;

	test_rec709_linear_hsl_float(test);

	return test.result();
}
