/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar AdobeRgbLinear<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/adobe_rgb_reference.h"

void test_adobe_rgb_linear_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::AdobeRgbLinear<double> default_value;
	test.check_equal(default_value.red, 0.0, "AdobeRgbLinear<double> default r");
	test.check_equal(default_value.green, 0.0, "AdobeRgbLinear<double> default g");
	test.check_equal(default_value.blue, 0.0, "AdobeRgbLinear<double> default b");
	test.check_equal(default_value.alpha, 1.0, "AdobeRgbLinear<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::AdobeRgbLinear<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "AdobeRgbLinear<double> grey r");
	test.check_equal(grey.green, 0.25, "AdobeRgbLinear<double> grey g");
	test.check_equal(grey.blue, 0.25, "AdobeRgbLinear<double> grey b");
	test.check_equal(grey.alpha, 1.0, "AdobeRgbLinear<double> grey a");

	// RGB constructor fills alpha with one.
	mt::AdobeRgbLinear<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "AdobeRgbLinear<double> rgb r");
	test.check_equal(rgb.green, 0.2, "AdobeRgbLinear<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "AdobeRgbLinear<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "AdobeRgbLinear<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::AdobeRgbLinear<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "AdobeRgbLinear<double> rgba r");
	test.check_equal(rgba.green, 0.2, "AdobeRgbLinear<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "AdobeRgbLinear<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "AdobeRgbLinear<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::AdobeRgbLinear<double> premultiplied = mt::AdobeRgbLinear<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "AdobeRgbLinear<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "AdobeRgbLinear<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "AdobeRgbLinear<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "AdobeRgbLinear<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::AdobeRgbLinear<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "AdobeRgbLinear<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "AdobeRgbLinear<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "AdobeRgbLinear<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "AdobeRgbLinear<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::AdobeRgbLinear<double> zero_alpha = mt::AdobeRgbLinear<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "AdobeRgbLinear<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "AdobeRgbLinear<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "AdobeRgbLinear<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "AdobeRgbLinear<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::AdobeRgbLinear<double> clamped = mt::AdobeRgbLinear<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "AdobeRgbLinear<double> clamp r");
	test.check_equal(clamped.green, 0.5, "AdobeRgbLinear<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "AdobeRgbLinear<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "AdobeRgbLinear<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::AdobeRgbLinear<double> no_negatives = mt::AdobeRgbLinear<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "AdobeRgbLinear<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "AdobeRgbLinear<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "AdobeRgbLinear<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "AdobeRgbLinear<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::AdobeRgbLinear<double> sanitised = mt::AdobeRgbLinear<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "AdobeRgbLinear<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "AdobeRgbLinear<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "AdobeRgbLinear<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "AdobeRgbLinear<double> sanitise negative infinity");

	// Reference data checks linear Adobe RGB (1998) to gamma-2.4 encoded Adobe RGB (1998).
	for (const auto& colour_case : colour_reference::adobe_rgb_adobe_rgb_linear_cases) {
		mt::AdobeRgbLinear<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::AdobeRgb<double> actual = colour.to_adobe_rgb();
		mt::AdobeRgb<double> transfer = colour.to_adobe_rgb();
		std::string message = std::string("AdobeRgbLinear<double> to_adobe_rgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
		message = std::string("AdobeRgbLinear<double> to_adobe_rgb ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(transfer.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(transfer.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(transfer.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear Adobe RGB (1998) to HSV.
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_hsv_cases) {
		mt::AdobeRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::AdobeRgbLinearHsv<double> actual = colour.to_hsv();
		std::string message = std::string("AdobeRgbLinear<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear Adobe RGB (1998) to HSL.
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_hsl_cases) {
		mt::AdobeRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::AdobeRgbLinearHsl<double> actual = colour.to_hsl();
		std::string message = std::string("AdobeRgbLinear<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear Adobe RGB (1998) to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_xyz_d65_cases) {
		mt::AdobeRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("AdobeRgbLinear<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_adobe_rgb_linear_double(test);

	return test.result();
}
