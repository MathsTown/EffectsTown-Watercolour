/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbHsl<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_hsl_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbHsl<double> default_value;
	test.check_equal(default_value.hue, 0.0, "SrgbHsl<double> default hue");
	test.check_equal(default_value.saturation, 0.0, "SrgbHsl<double> default saturation");
	test.check_equal(default_value.lightness, 0.0, "SrgbHsl<double> default lightness");
	test.check_equal(default_value.alpha, 1.0, "SrgbHsl<double> default alpha");

	// Three-component constructor fills alpha with one.
	mt::SrgbHsl<double> hsa(0.1, 0.2, 0.3);
	test.check_equal(hsa.hue, 0.1, "SrgbHsl<double> HSL hue");
	test.check_equal(hsa.saturation, 0.2, "SrgbHsl<double> HSL saturation");
	test.check_equal(hsa.lightness, 0.3, "SrgbHsl<double> HSL lightness");
	test.check_equal(hsa.alpha, 1.0, "SrgbHsl<double> HSL alpha");

	// Four-component constructor stores every component directly.
	mt::SrgbHsl<double> hsla(0.1, 0.2, 0.3, 0.4);
	test.check_equal(hsla.hue, 0.1, "SrgbHsl<double> HSLA hue");
	test.check_equal(hsla.saturation, 0.2, "SrgbHsl<double> HSLA saturation");
	test.check_equal(hsla.lightness, 0.3, "SrgbHsl<double> HSLA lightness");
	test.check_equal(hsla.alpha, 0.4, "SrgbHsl<double> HSLA alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbHsl<double> clamped = mt::SrgbHsl<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.hue, 0.0, "SrgbHsl<double> clamp hue");
	test.check_equal(clamped.saturation, 0.5, "SrgbHsl<double> clamp saturation");
	test.check_equal(clamped.lightness, 1.0, "SrgbHsl<double> clamp lightness");
	test.check_equal(clamped.alpha, 1.0, "SrgbHsl<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbHsl<double> no_negatives = mt::SrgbHsl<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.hue, 0.0, "SrgbHsl<double> no_negatives hue");
	test.check_equal(no_negatives.saturation, 0.5, "SrgbHsl<double> no_negatives saturation");
	test.check_equal(no_negatives.lightness, 1.25, "SrgbHsl<double> no_negatives lightness");
	test.check_equal(no_negatives.alpha, 0.0, "SrgbHsl<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbHsl<double> sanitised = mt::SrgbHsl<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.hue, 0.0, "SrgbHsl<double> sanitise negative zero");
	test.check_equal(sanitised.saturation, 0.0, "SrgbHsl<double> sanitise nan");
	test.check_equal(sanitised.lightness, std::numeric_limits<double>::max(), "SrgbHsl<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "SrgbHsl<double> sanitise negative infinity");

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbHsl<double> wrapped_negative = mt::SrgbHsl<double>(-0.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_negative.hue, 0.75, "SrgbHsl<double> wrap negative hue");
	test.check_equal(wrapped_negative.saturation, 0.5, "SrgbHsl<double> wrap negative saturation");
	test.check_equal(wrapped_negative.lightness, 0.75, "SrgbHsl<double> wrap negative lightness");
	test.check_equal(wrapped_negative.alpha, 1.0, "SrgbHsl<double> wrap negative alpha");

	mt::SrgbHsl<double> wrapped_over_one = mt::SrgbHsl<double>(1.25, 0.5, 0.75, 1.0).wrap_hue();
	test.check_equal(wrapped_over_one.hue, 0.25, "SrgbHsl<double> wrap over-one hue");
	test.check_equal(wrapped_over_one.saturation, 0.5, "SrgbHsl<double> wrap over-one saturation");
	test.check_equal(wrapped_over_one.lightness, 0.75, "SrgbHsl<double> wrap over-one lightness");
	test.check_equal(wrapped_over_one.alpha, 1.0, "SrgbHsl<double> wrap over-one alpha");

	// Reference data checks gamma-2.4 encoded sRGB HSL to gamma-2.4 encoded sRGB.
	for (const auto& colour_case : colour_reference::srgb_hsl_cases) {
		mt::SrgbHsl<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::Srgb<double> actual = colour.to_srgb();
		std::string message = std::string("SrgbHsl<double> to_srgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " red");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " green");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " blue");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_hsl_double(test);

	return test.result();
}
