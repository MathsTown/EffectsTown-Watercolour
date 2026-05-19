/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Rec2020LinearHsl<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/rec2020_reference.h"

void test_rec2020_linear_hsl_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Rec2020LinearHsl<double> default_value;
	test.check_equal(default_value.hue, 0.0, "Rec2020LinearHsl<double> default h");
	test.check_equal(default_value.saturation, 0.0, "Rec2020LinearHsl<double> default s");
	test.check_equal(default_value.lightness, 0.0, "Rec2020LinearHsl<double> default l");
	test.check_equal(default_value.alpha, 1.0, "Rec2020LinearHsl<double> default a");

	// Three-component constructor fills alpha with one.
	mt::Rec2020LinearHsl<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "Rec2020LinearHsl<double> HSL h");
	test.check_equal(hsa.saturation, 0.2, "Rec2020LinearHsl<double> HSL s");
	test.check_equal(hsa.lightness, 0.3, "Rec2020LinearHsl<double> HSL l");
	test.check_equal(hsa.alpha, 1.0, "Rec2020LinearHsl<double> HSL a");

	// Four-component constructor stores every component directly.
	mt::Rec2020LinearHsl<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "Rec2020LinearHsl<double> HSLA h");
	test.check_equal(hsla.saturation, 0.2, "Rec2020LinearHsl<double> HSLA s");
	test.check_equal(hsla.lightness, 0.3, "Rec2020LinearHsl<double> HSLA l");
	test.check_equal(hsla.alpha, 0.4, "Rec2020LinearHsl<double> HSLA a");

	// Clamp restricts every component to the 0..1 range.
	mt::Rec2020LinearHsl<double> clamped = mt::Rec2020LinearHsl<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "Rec2020LinearHsl<double> clamp h");
	test.check_equal(clamped.saturation, 0.5, "Rec2020LinearHsl<double> clamp s");
	test.check_equal(clamped.lightness, 1.0, "Rec2020LinearHsl<double> clamp l");
	test.check_equal(clamped.alpha, 1.0, "Rec2020LinearHsl<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Rec2020LinearHsl<double> no_negatives = mt::Rec2020LinearHsl<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "Rec2020LinearHsl<double> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5, "Rec2020LinearHsl<double> no_negatives s");
	test.check_equal(no_negatives.lightness, 1.25, "Rec2020LinearHsl<double> no_negatives l");
	test.check_equal(no_negatives.alpha, 0.0, "Rec2020LinearHsl<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec2020LinearHsl<double> sanitised = mt::Rec2020LinearHsl<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "Rec2020LinearHsl<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "Rec2020LinearHsl<double> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<double>::max(), "Rec2020LinearHsl<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Rec2020LinearHsl<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Rec2020LinearHsl<double> wrapped_negative = mt::Rec2020LinearHsl<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "Rec2020LinearHsl<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "Rec2020LinearHsl<double> wrap negative s");
	test.check_equal(wrapped_negative.lightness, 0.75, "Rec2020LinearHsl<double> wrap negative l");
	test.check_equal(wrapped_negative.alpha, 1.0, "Rec2020LinearHsl<double> wrap negative a");

	mt::Rec2020LinearHsl<double> wrapped_over_one = mt::Rec2020LinearHsl<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "Rec2020LinearHsl<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "Rec2020LinearHsl<double> wrap over-one s");
	test.check_equal(wrapped_over_one.lightness, 0.75, "Rec2020LinearHsl<double> wrap over-one l");
	test.check_equal(wrapped_over_one.alpha, 1.0, "Rec2020LinearHsl<double> wrap over-one a");

	// Reference data checks linear Rec.2020 HSL to linear Rec.2020.
	for (const auto& colour_case : colour_reference::rec2020_linear_hsl_cases) {
		mt::Rec2020LinearHsl<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::Rec2020Linear<double> actual = colour.to_rec2020_linear();
		std::string message = std::string("Rec2020LinearHsl<double> to_rec2020_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_rec2020_linear_hsl_double(test);

	return test.result();
}
