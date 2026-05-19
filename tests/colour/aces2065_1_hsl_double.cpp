/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Aces2065_1Hsl<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/aces_reference.h"

void test_aces2065_1_hsl_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Aces2065_1Hsl<double> default_value;
	test.check_equal(default_value.hue, 0.0, "Aces2065_1Hsl<double> default h");
	test.check_equal(default_value.saturation, 0.0, "Aces2065_1Hsl<double> default s");
	test.check_equal(default_value.lightness, 0.0, "Aces2065_1Hsl<double> default l");
	test.check_equal(default_value.alpha, 1.0, "Aces2065_1Hsl<double> default a");

	// Three-component constructor fills alpha with one.
	mt::Aces2065_1Hsl<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "Aces2065_1Hsl<double> HSL h");
	test.check_equal(hsa.saturation, 0.2, "Aces2065_1Hsl<double> HSL s");
	test.check_equal(hsa.lightness, 0.3, "Aces2065_1Hsl<double> HSL l");
	test.check_equal(hsa.alpha, 1.0, "Aces2065_1Hsl<double> HSL a");

	// Four-component constructor stores every component directly.
	mt::Aces2065_1Hsl<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "Aces2065_1Hsl<double> HSLA h");
	test.check_equal(hsla.saturation, 0.2, "Aces2065_1Hsl<double> HSLA s");
	test.check_equal(hsla.lightness, 0.3, "Aces2065_1Hsl<double> HSLA l");
	test.check_equal(hsla.alpha, 0.4, "Aces2065_1Hsl<double> HSLA a");

	// Clamp restricts every component to the 0..1 range.
	mt::Aces2065_1Hsl<double> clamped = mt::Aces2065_1Hsl<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "Aces2065_1Hsl<double> clamp h");
	test.check_equal(clamped.saturation, 0.5, "Aces2065_1Hsl<double> clamp s");
	test.check_equal(clamped.lightness, 1.0, "Aces2065_1Hsl<double> clamp l");
	test.check_equal(clamped.alpha, 1.0, "Aces2065_1Hsl<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Aces2065_1Hsl<double> no_negatives = mt::Aces2065_1Hsl<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "Aces2065_1Hsl<double> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5, "Aces2065_1Hsl<double> no_negatives s");
	test.check_equal(no_negatives.lightness, 1.25, "Aces2065_1Hsl<double> no_negatives l");
	test.check_equal(no_negatives.alpha, 0.0, "Aces2065_1Hsl<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Aces2065_1Hsl<double> sanitised = mt::Aces2065_1Hsl<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "Aces2065_1Hsl<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "Aces2065_1Hsl<double> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<double>::max(), "Aces2065_1Hsl<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Aces2065_1Hsl<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Aces2065_1Hsl<double> wrapped_negative = mt::Aces2065_1Hsl<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "Aces2065_1Hsl<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "Aces2065_1Hsl<double> wrap negative s");
	test.check_equal(wrapped_negative.lightness, 0.75, "Aces2065_1Hsl<double> wrap negative l");
	test.check_equal(wrapped_negative.alpha, 1.0, "Aces2065_1Hsl<double> wrap negative a");

	mt::Aces2065_1Hsl<double> wrapped_over_one = mt::Aces2065_1Hsl<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "Aces2065_1Hsl<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "Aces2065_1Hsl<double> wrap over-one s");
	test.check_equal(wrapped_over_one.lightness, 0.75, "Aces2065_1Hsl<double> wrap over-one l");
	test.check_equal(wrapped_over_one.alpha, 1.0, "Aces2065_1Hsl<double> wrap over-one a");

	// Reference data checks linear ACES2065-1 HSL to linear ACES2065-1.
	for (const auto& colour_case : colour_reference::aces2065_1_hsl_cases) {
		mt::Aces2065_1Hsl<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::Aces2065_1<double> actual = colour.to_aces2065_1();
		std::string message = std::string("Aces2065_1Hsl<double> to_aces2065_1 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_aces2065_1_hsl_double(test);

	return test.result();
}
