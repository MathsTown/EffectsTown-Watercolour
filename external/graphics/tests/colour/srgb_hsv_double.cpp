/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbHsv<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_hsv_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbHsv<double> default_value;
	test.check_equal(default_value.hue, 0.0, "SrgbHsv<double> default hue");
	test.check_equal(default_value.saturation, 0.0, "SrgbHsv<double> default saturation");
	test.check_equal(default_value.value, 0.0, "SrgbHsv<double> default value");
	test.check_equal(default_value.alpha, 1.0, "SrgbHsv<double> default alpha");

	// Three-component constructor fills alpha with one.
	mt::SrgbHsv<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "SrgbHsv<double> HSV hue");
	test.check_equal(hsa.saturation, 0.2, "SrgbHsv<double> HSV saturation");
	test.check_equal(hsa.value, 0.3, "SrgbHsv<double> HSV value");
	test.check_equal(hsa.alpha, 1.0, "SrgbHsv<double> HSV alpha");

	// Four-component constructor stores every component directly.
	mt::SrgbHsv<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "SrgbHsv<double> HSVA hue");
	test.check_equal(hsla.saturation, 0.2, "SrgbHsv<double> HSVA saturation");
	test.check_equal(hsla.value, 0.3, "SrgbHsv<double> HSVA value");
	test.check_equal(hsla.alpha, 0.4, "SrgbHsv<double> HSVA alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbHsv<double> clamped = mt::SrgbHsv<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "SrgbHsv<double> clamp hue");
	test.check_equal(clamped.saturation, 0.5, "SrgbHsv<double> clamp saturation");
	test.check_equal(clamped.value, 1.0, "SrgbHsv<double> clamp value");
	test.check_equal(clamped.alpha, 1.0, "SrgbHsv<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbHsv<double> no_negatives = mt::SrgbHsv<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "SrgbHsv<double> no_negatives hue");
	test.check_equal(no_negatives.saturation, 0.5, "SrgbHsv<double> no_negatives saturation");
	test.check_equal(no_negatives.value, 1.25, "SrgbHsv<double> no_negatives value");
	test.check_equal(no_negatives.alpha, 0.0, "SrgbHsv<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbHsv<double> sanitised = mt::SrgbHsv<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "SrgbHsv<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "SrgbHsv<double> sanitise nan");
	test.check_equal(sanitised.value, std::numeric_limits<double>::max(), "SrgbHsv<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "SrgbHsv<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbHsv<double> wrapped_negative = mt::SrgbHsv<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "SrgbHsv<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "SrgbHsv<double> wrap negative saturation");
	test.check_equal(wrapped_negative.value, 0.75, "SrgbHsv<double> wrap negative value");
	test.check_equal(wrapped_negative.alpha, 1.0, "SrgbHsv<double> wrap negative alpha");

	mt::SrgbHsv<double> wrapped_over_one = mt::SrgbHsv<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "SrgbHsv<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "SrgbHsv<double> wrap over-one saturation");
	test.check_equal(wrapped_over_one.value, 0.75, "SrgbHsv<double> wrap over-one value");
	test.check_equal(wrapped_over_one.alpha, 1.0, "SrgbHsv<double> wrap over-one alpha");

	// Reference data checks transfer-encoded sRGB HSV to transfer-encoded sRGB.
	for (const auto& colour_case : colour_reference::srgb_hsv_cases) {
		mt::SrgbHsv<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::Srgb<double> actual = colour.to_srgb();
		std::string message = std::string("SrgbHsv<double> to_srgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_hsv_double(test);

	return test.result();
}
