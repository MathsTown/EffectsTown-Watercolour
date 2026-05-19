/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar AcescgHsv<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/aces_reference.h"

void test_acescg_hsv_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::AcescgHsv<double> default_value;
	test.check_equal(default_value.hue, 0.0, "AcescgHsv<double> default h");
	test.check_equal(default_value.saturation, 0.0, "AcescgHsv<double> default s");
	test.check_equal(default_value.value, 0.0, "AcescgHsv<double> default v");
	test.check_equal(default_value.alpha, 1.0, "AcescgHsv<double> default a");

	// Three-component constructor fills alpha with one.
	mt::AcescgHsv<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "AcescgHsv<double> HSV h");
	test.check_equal(hsa.saturation, 0.2, "AcescgHsv<double> HSV s");
	test.check_equal(hsa.value, 0.3, "AcescgHsv<double> HSV v");
	test.check_equal(hsa.alpha, 1.0, "AcescgHsv<double> HSV a");

	// Four-component constructor stores every component directly.
	mt::AcescgHsv<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "AcescgHsv<double> HSVA h");
	test.check_equal(hsla.saturation, 0.2, "AcescgHsv<double> HSVA s");
	test.check_equal(hsla.value, 0.3, "AcescgHsv<double> HSVA v");
	test.check_equal(hsla.alpha, 0.4, "AcescgHsv<double> HSVA a");

	// Clamp restricts every component to the 0..1 range.
	mt::AcescgHsv<double> clamped = mt::AcescgHsv<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "AcescgHsv<double> clamp h");
	test.check_equal(clamped.saturation, 0.5, "AcescgHsv<double> clamp s");
	test.check_equal(clamped.value, 1.0, "AcescgHsv<double> clamp v");
	test.check_equal(clamped.alpha, 1.0, "AcescgHsv<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::AcescgHsv<double> no_negatives = mt::AcescgHsv<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "AcescgHsv<double> no_negatives h");
	test.check_equal(no_negatives.saturation, 0.5, "AcescgHsv<double> no_negatives s");
	test.check_equal(no_negatives.value, 1.25, "AcescgHsv<double> no_negatives v");
	test.check_equal(no_negatives.alpha, 0.0, "AcescgHsv<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::AcescgHsv<double> sanitised = mt::AcescgHsv<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "AcescgHsv<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "AcescgHsv<double> sanitise nan");
	test.check_equal(sanitised.value, std::numeric_limits<double>::max(), "AcescgHsv<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "AcescgHsv<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::AcescgHsv<double> wrapped_negative = mt::AcescgHsv<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "AcescgHsv<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "AcescgHsv<double> wrap negative s");
	test.check_equal(wrapped_negative.value, 0.75, "AcescgHsv<double> wrap negative v");
	test.check_equal(wrapped_negative.alpha, 1.0, "AcescgHsv<double> wrap negative a");

	mt::AcescgHsv<double> wrapped_over_one = mt::AcescgHsv<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "AcescgHsv<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "AcescgHsv<double> wrap over-one s");
	test.check_equal(wrapped_over_one.value, 0.75, "AcescgHsv<double> wrap over-one v");
	test.check_equal(wrapped_over_one.alpha, 1.0, "AcescgHsv<double> wrap over-one a");

	// Reference data checks linear ACEScg HSV to linear ACEScg.
	for (const auto& colour_case : colour_reference::acescg_hsv_cases) {
		mt::AcescgHsv<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::Acescg<double> actual = colour.to_acescg();
		std::string message = std::string("AcescgHsv<double> to_acescg ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_acescg_hsv_double(test);

	return test.result();
}
