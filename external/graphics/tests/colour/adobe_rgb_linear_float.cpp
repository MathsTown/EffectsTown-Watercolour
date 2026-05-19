/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar AdobeRgbLinear<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/adobe_rgb_reference.h"

void test_adobe_rgb_linear_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::AdobeRgbLinear<float> default_value;
	test.check_equal(default_value.red, 0.0f, "AdobeRgbLinear<float> default r");
	test.check_equal(default_value.green, 0.0f, "AdobeRgbLinear<float> default g");
	test.check_equal(default_value.blue, 0.0f, "AdobeRgbLinear<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "AdobeRgbLinear<float> default a");

	// Grey constructor copies one value into every colour channel.
	mt::AdobeRgbLinear<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "AdobeRgbLinear<float> grey r");
	test.check_equal(grey.green, 0.25f, "AdobeRgbLinear<float> grey g");
	test.check_equal(grey.blue, 0.25f, "AdobeRgbLinear<float> grey b");
	test.check_equal(grey.alpha, 1.0f, "AdobeRgbLinear<float> grey a");

	// RGB constructor fills alpha with one.
	mt::AdobeRgbLinear<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "AdobeRgbLinear<float> rgb r");
	test.check_equal(rgb.green, 0.2f, "AdobeRgbLinear<float> rgb g");
	test.check_equal(rgb.blue, 0.3f, "AdobeRgbLinear<float> rgb b");
	test.check_equal(rgb.alpha, 1.0f, "AdobeRgbLinear<float> rgb a");

	// RGBA constructor stores every component directly.
	mt::AdobeRgbLinear<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "AdobeRgbLinear<float> rgba r");
	test.check_equal(rgba.green, 0.2f, "AdobeRgbLinear<float> rgba g");
	test.check_equal(rgba.blue, 0.3f, "AdobeRgbLinear<float> rgba b");
	test.check_equal(rgba.alpha, 0.4f, "AdobeRgbLinear<float> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::AdobeRgbLinear<float> premultiplied = mt::AdobeRgbLinear<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "AdobeRgbLinear<float> premultiply r");
	test.check_equal(premultiplied.green, 0.25f, "AdobeRgbLinear<float> premultiply g");
	test.check_equal(premultiplied.blue, 0.375f, "AdobeRgbLinear<float> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5f, "AdobeRgbLinear<float> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::AdobeRgbLinear<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "AdobeRgbLinear<float> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5f, "AdobeRgbLinear<float> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75f, "AdobeRgbLinear<float> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5f, "AdobeRgbLinear<float> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::AdobeRgbLinear<float> zero_alpha = mt::AdobeRgbLinear<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "AdobeRgbLinear<float> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5f, "AdobeRgbLinear<float> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75f, "AdobeRgbLinear<float> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0f, "AdobeRgbLinear<float> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::AdobeRgbLinear<float> clamped = mt::AdobeRgbLinear<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "AdobeRgbLinear<float> clamp r");
	test.check_equal(clamped.green, 0.5f, "AdobeRgbLinear<float> clamp g");
	test.check_equal(clamped.blue, 1.0f, "AdobeRgbLinear<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "AdobeRgbLinear<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::AdobeRgbLinear<float> no_negatives = mt::AdobeRgbLinear<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "AdobeRgbLinear<float> no_negatives r");
	test.check_equal(no_negatives.green, 0.5f, "AdobeRgbLinear<float> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25f, "AdobeRgbLinear<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "AdobeRgbLinear<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::AdobeRgbLinear<float> sanitised = mt::AdobeRgbLinear<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "AdobeRgbLinear<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "AdobeRgbLinear<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "AdobeRgbLinear<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "AdobeRgbLinear<float> sanitise negative infinity");

	// Reference data checks linear Adobe RGB (1998) to transfer-encoded Adobe RGB (1998).
	for (const auto& colour_case : colour_reference::adobe_rgb_adobe_rgb_linear_cases) {
		mt::AdobeRgbLinear<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::AdobeRgb<float> actual = colour.to_adobe_rgb();
		mt::AdobeRgb<float> transfer = colour.to_transfer();
		std::string message = std::string("AdobeRgbLinear<float> to_adobe_rgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 5.0e-6f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 5.0e-6f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 5.0e-6f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
		message = std::string("AdobeRgbLinear<float> to_transfer ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 5.0e-6f, message + " r");
		test.check_close(transfer.green, colour_case.source[1], 5.0e-6f, message + " g");
		test.check_close(transfer.blue, colour_case.source[2], 5.0e-6f, message + " b");
		test.check_close(transfer.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear Adobe RGB (1998) to HSV.
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_hsv_cases) {
		mt::AdobeRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::AdobeRgbLinearHsv<float> actual = colour.to_hsv();
		std::string message = std::string("AdobeRgbLinear<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear Adobe RGB (1998) to HSL.
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_hsl_cases) {
		mt::AdobeRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::AdobeRgbLinearHsl<float> actual = colour.to_hsl();
		std::string message = std::string("AdobeRgbLinear<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear Adobe RGB (1998) to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::adobe_rgb_linear_xyz_d65_cases) {
		mt::AdobeRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("AdobeRgbLinear<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}
}

int main() {
	TestContext test;

	test_adobe_rgb_linear_float(test);

	return test.result();
}
