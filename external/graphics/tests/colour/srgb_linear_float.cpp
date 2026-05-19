/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar SrgbLinear<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

void test_srgb_linear_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::SrgbLinear<float> default_value;
	test.check_equal(default_value.red, 0.0f, "SrgbLinear<float> default red");
	test.check_equal(default_value.green, 0.0f, "SrgbLinear<float> default green");
	test.check_equal(default_value.blue, 0.0f, "SrgbLinear<float> default blue");
	test.check_equal(default_value.alpha, 1.0f, "SrgbLinear<float> default alpha");

	// Grey constructor copies one value into every colour channel.
	mt::SrgbLinear<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "SrgbLinear<float> grey red");
	test.check_equal(grey.green, 0.25f, "SrgbLinear<float> grey green");
	test.check_equal(grey.blue, 0.25f, "SrgbLinear<float> grey blue");
	test.check_equal(grey.alpha, 1.0f, "SrgbLinear<float> grey alpha");

	// Three-component constructor fills alpha with one.
	mt::SrgbLinear<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "SrgbLinear<float> rgb red");
	test.check_equal(rgb.green, 0.2f, "SrgbLinear<float> rgb green");
	test.check_equal(rgb.blue, 0.3f, "SrgbLinear<float> rgb blue");
	test.check_equal(rgb.alpha, 1.0f, "SrgbLinear<float> rgb alpha");

	// Four-component constructor stores every component directly.
	mt::SrgbLinear<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "SrgbLinear<float> rgba red");
	test.check_equal(rgba.green, 0.2f, "SrgbLinear<float> rgba green");
	test.check_equal(rgba.blue, 0.3f, "SrgbLinear<float> rgba blue");
	test.check_equal(rgba.alpha, 0.4f, "SrgbLinear<float> rgba alpha");

	// mix interpolates colour and alpha components in the current coordinate space.
	mt::SrgbLinear<float> mixed = mt::mix(mt::SrgbLinear<float>(0.0f, 0.25f, 0.5f, 0.25f), mt::SrgbLinear<float>(1.0f, 0.75f, 0.0f, 0.75f), 0.5f);
	test.check_close(mixed.red, 0.5f, "SrgbLinear<float> mix red");
	test.check_close(mixed.green, 0.5f, "SrgbLinear<float> mix green");
	test.check_close(mixed.blue, 0.25f, "SrgbLinear<float> mix blue");
	test.check_close(mixed.alpha, 0.5f, "SrgbLinear<float> mix alpha");

	mt::SrgbLinear<float> lerped = mt::lerp(mt::SrgbLinear<float>(0.0f, 0.25f, 0.5f, 0.25f), mt::SrgbLinear<float>(1.0f, 0.75f, 0.0f, 0.75f), 0.5f);
	test.check_close(lerped.red, mixed.red, "SrgbLinear<float> lerp alias red");
	test.check_close(lerped.green, mixed.green, "SrgbLinear<float> lerp alias green");
	test.check_close(lerped.blue, mixed.blue, "SrgbLinear<float> lerp alias blue");
	test.check_close(lerped.alpha, mixed.alpha, "SrgbLinear<float> lerp alias alpha");

	// Premultiply alpha multiplies only the colour channels.
	mt::SrgbLinear<float> premultiplied = mt::SrgbLinear<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "SrgbLinear<float> premultiply red");
	test.check_equal(premultiplied.green, 0.25f, "SrgbLinear<float> premultiply green");
	test.check_equal(premultiplied.blue, 0.375f, "SrgbLinear<float> premultiply blue");
	test.check_equal(premultiplied.alpha, 0.5f, "SrgbLinear<float> premultiply alpha");

	// Un-premultiply alpha divides only the colour channels.
	mt::SrgbLinear<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "SrgbLinear<float> unpremultiply red");
	test.check_equal(unpremultiplied.green, 0.5f, "SrgbLinear<float> unpremultiply green");
	test.check_equal(unpremultiplied.blue, 0.75f, "SrgbLinear<float> unpremultiply blue");
	test.check_equal(unpremultiplied.alpha, 0.5f, "SrgbLinear<float> unpremultiply alpha");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::SrgbLinear<float> zero_alpha = mt::SrgbLinear<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "SrgbLinear<float> zero alpha unpremultiply red");
	test.check_equal(zero_alpha.green, 0.5f, "SrgbLinear<float> zero alpha unpremultiply green");
	test.check_equal(zero_alpha.blue, 0.75f, "SrgbLinear<float> zero alpha unpremultiply blue");
	test.check_equal(zero_alpha.alpha, 0.0f, "SrgbLinear<float> zero alpha unpremultiply alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbLinear<float> clamped = mt::SrgbLinear<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "SrgbLinear<float> clamp red");
	test.check_equal(clamped.green, 0.5f, "SrgbLinear<float> clamp green");
	test.check_equal(clamped.blue, 1.0f, "SrgbLinear<float> clamp blue");
	test.check_equal(clamped.alpha, 1.0f, "SrgbLinear<float> clamp alpha");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::SrgbLinear<float> no_negatives = mt::SrgbLinear<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "SrgbLinear<float> no_negatives red");
	test.check_equal(no_negatives.green, 0.5f, "SrgbLinear<float> no_negatives green");
	test.check_equal(no_negatives.blue, 1.25f, "SrgbLinear<float> no_negatives blue");
	test.check_equal(no_negatives.alpha, 0.0f, "SrgbLinear<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbLinear<float> sanitised = mt::SrgbLinear<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "SrgbLinear<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "SrgbLinear<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "SrgbLinear<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "SrgbLinear<float> sanitise negative infinity");

	// Reference data checks linear sRGB to transfer-encoded sRGB.
	for (const auto& colour_case : colour_reference::srgb_srgb_linear_cases) {
		mt::SrgbLinear<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::Srgb<float> actual = colour.to_srgb();
		mt::Srgb<float> transfer = colour.to_transfer();
		std::string message = std::string("SrgbLinear<float> to_srgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 5.0e-6f, message + " red");
		test.check_close(actual.green, colour_case.source[1], 5.0e-6f, message + " green");
		test.check_close(actual.blue, colour_case.source[2], 5.0e-6f, message + " blue");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " alpha");
		message = std::string("SrgbLinear<float> to_transfer ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 5.0e-6f, message + " red");
		test.check_close(transfer.green, colour_case.source[1], 5.0e-6f, message + " green");
		test.check_close(transfer.blue, colour_case.source[2], 5.0e-6f, message + " blue");
		test.check_close(transfer.alpha, colour_case.source[3], 5.0e-6f, message + " alpha");
	}

	// Reference data checks linear sRGB to HSV.
	for (const auto& colour_case : colour_reference::srgb_linear_hsv_cases) {
		mt::SrgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::SrgbLinearHsv<float> actual = colour.to_hsv();
		std::string message = std::string("SrgbLinear<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}

	// Reference data checks linear sRGB to HSL.
	for (const auto& colour_case : colour_reference::srgb_linear_hsl_cases) {
		mt::SrgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::SrgbLinearHsl<float> actual = colour.to_hsl();
		std::string message = std::string("SrgbLinear<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}

	// Reference data checks linear sRGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::srgb_linear_xyz_d65_cases) {
		mt::SrgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("SrgbLinear<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_linear_float(test);

	return test.result();
}
