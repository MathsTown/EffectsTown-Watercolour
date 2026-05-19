/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbLinear<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_linear_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbLinear<double> default_value;
	test.check_equal(default_value.red, 0.0, "SrgbLinear<double> default red");
	test.check_equal(default_value.green, 0.0, "SrgbLinear<double> default green");
	test.check_equal(default_value.blue, 0.0, "SrgbLinear<double> default blue");
	test.check_equal(default_value.alpha, 1.0, "SrgbLinear<double> default alpha");

	// Grey constructor copies one value into every colour channel.
	mt::SrgbLinear<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "SrgbLinear<double> grey red");
	test.check_equal(grey.green, 0.25, "SrgbLinear<double> grey green");
	test.check_equal(grey.blue, 0.25, "SrgbLinear<double> grey blue");
	test.check_equal(grey.alpha, 1.0, "SrgbLinear<double> grey alpha");

	// Three-component constructor fills alpha with one.
	mt::SrgbLinear<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "SrgbLinear<double> rgb red");
	test.check_equal(rgb.green, 0.2, "SrgbLinear<double> rgb green");
	test.check_equal(rgb.blue, 0.3, "SrgbLinear<double> rgb blue");
	test.check_equal(rgb.alpha, 1.0, "SrgbLinear<double> rgb alpha");

	// Four-component constructor stores every component directly.
	mt::SrgbLinear<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "SrgbLinear<double> rgba red");
	test.check_equal(rgba.green, 0.2, "SrgbLinear<double> rgba green");
	test.check_equal(rgba.blue, 0.3, "SrgbLinear<double> rgba blue");
	test.check_equal(rgba.alpha, 0.4, "SrgbLinear<double> rgba alpha");

	// Premultiply alpha multiplies only the colour channels.
	mt::SrgbLinear<double> premultiplied = mt::SrgbLinear<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "SrgbLinear<double> premultiply red");
	test.check_equal(premultiplied.green, 0.25, "SrgbLinear<double> premultiply green");
	test.check_equal(premultiplied.blue, 0.375, "SrgbLinear<double> premultiply blue");
	test.check_equal(premultiplied.alpha, 0.5, "SrgbLinear<double> premultiply alpha");

	// Un-premultiply alpha divides only the colour channels.
	mt::SrgbLinear<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "SrgbLinear<double> unpremultiply red");
	test.check_equal(unpremultiplied.green, 0.5, "SrgbLinear<double> unpremultiply green");
	test.check_equal(unpremultiplied.blue, 0.75, "SrgbLinear<double> unpremultiply blue");
	test.check_equal(unpremultiplied.alpha, 0.5, "SrgbLinear<double> unpremultiply alpha");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::SrgbLinear<double> zero_alpha = mt::SrgbLinear<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "SrgbLinear<double> zero alpha unpremultiply red");
	test.check_equal(zero_alpha.green, 0.5, "SrgbLinear<double> zero alpha unpremultiply green");
	test.check_equal(zero_alpha.blue, 0.75, "SrgbLinear<double> zero alpha unpremultiply blue");
	test.check_equal(zero_alpha.alpha, 0.0, "SrgbLinear<double> zero alpha unpremultiply alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbLinear<double> clamped = mt::SrgbLinear<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "SrgbLinear<double> clamp red");
	test.check_equal(clamped.green, 0.5, "SrgbLinear<double> clamp green");
	test.check_equal(clamped.blue, 1.0, "SrgbLinear<double> clamp blue");
	test.check_equal(clamped.alpha, 1.0, "SrgbLinear<double> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbLinear<double> no_negatives = mt::SrgbLinear<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "SrgbLinear<double> no_negatives red");
	test.check_equal(no_negatives.green, 0.5, "SrgbLinear<double> no_negatives green");
	test.check_equal(no_negatives.blue, 1.25, "SrgbLinear<double> no_negatives blue");
	test.check_equal(no_negatives.alpha, 0.0, "SrgbLinear<double> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbLinear<double> sanitised = mt::SrgbLinear<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "SrgbLinear<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "SrgbLinear<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "SrgbLinear<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "SrgbLinear<double> sanitise negative infinity");

	// Reference data checks linear sRGB to transfer-encoded sRGB.
	for (const auto& colour_case : colour_reference::srgb_srgb_linear_cases) {
		mt::SrgbLinear<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::Srgb<double> actual = colour.to_srgb();
		mt::Srgb<double> transfer = colour.to_transfer();
		std::string message = std::string("SrgbLinear<double> to_srgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " red");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " green");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " blue");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
		message = std::string("SrgbLinear<double> to_transfer ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 2.0e-11, message + " red");
		test.check_close(transfer.green, colour_case.source[1], 2.0e-11, message + " green");
		test.check_close(transfer.blue, colour_case.source[2], 2.0e-11, message + " blue");
		test.check_close(transfer.alpha, colour_case.source[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks linear sRGB to HSV.
	for (const auto& colour_case : colour_reference::srgb_linear_hsv_cases) {
		mt::SrgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::SrgbLinearHsv<double> actual = colour.to_hsv();
		std::string message = std::string("SrgbLinear<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks linear sRGB to HSL.
	for (const auto& colour_case : colour_reference::srgb_linear_hsl_cases) {
		mt::SrgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::SrgbLinearHsl<double> actual = colour.to_hsl();
		std::string message = std::string("SrgbLinear<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}

	// Reference data checks linear sRGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::srgb_linear_xyz_d65_cases) {
		mt::SrgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("SrgbLinear<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_linear_double(test);

	return test.result();
}
