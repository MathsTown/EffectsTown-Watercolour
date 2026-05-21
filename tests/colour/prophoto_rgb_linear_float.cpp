/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar ProPhotoRgbLinear<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/prophoto_rgb_reference.h"

void test_prophoto_rgb_linear_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::ProPhotoRgbLinear<float> default_value;
	test.check_equal(default_value.red, 0.0f, "ProPhotoRgbLinear<float> default r");
	test.check_equal(default_value.green, 0.0f, "ProPhotoRgbLinear<float> default g");
	test.check_equal(default_value.blue, 0.0f, "ProPhotoRgbLinear<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "ProPhotoRgbLinear<float> default a");

	// Grey constructor copies one value into every colour channel.
	mt::ProPhotoRgbLinear<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "ProPhotoRgbLinear<float> grey r");
	test.check_equal(grey.green, 0.25f, "ProPhotoRgbLinear<float> grey g");
	test.check_equal(grey.blue, 0.25f, "ProPhotoRgbLinear<float> grey b");
	test.check_equal(grey.alpha, 1.0f, "ProPhotoRgbLinear<float> grey a");

	// RGB constructor fills alpha with one.
	mt::ProPhotoRgbLinear<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "ProPhotoRgbLinear<float> rgb r");
	test.check_equal(rgb.green, 0.2f, "ProPhotoRgbLinear<float> rgb g");
	test.check_equal(rgb.blue, 0.3f, "ProPhotoRgbLinear<float> rgb b");
	test.check_equal(rgb.alpha, 1.0f, "ProPhotoRgbLinear<float> rgb a");

	// RGBA constructor stores every component directly.
	mt::ProPhotoRgbLinear<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "ProPhotoRgbLinear<float> rgba r");
	test.check_equal(rgba.green, 0.2f, "ProPhotoRgbLinear<float> rgba g");
	test.check_equal(rgba.blue, 0.3f, "ProPhotoRgbLinear<float> rgba b");
	test.check_equal(rgba.alpha, 0.4f, "ProPhotoRgbLinear<float> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::ProPhotoRgbLinear<float> premultiplied = mt::ProPhotoRgbLinear<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "ProPhotoRgbLinear<float> premultiply r");
	test.check_equal(premultiplied.green, 0.25f, "ProPhotoRgbLinear<float> premultiply g");
	test.check_equal(premultiplied.blue, 0.375f, "ProPhotoRgbLinear<float> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5f, "ProPhotoRgbLinear<float> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::ProPhotoRgbLinear<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "ProPhotoRgbLinear<float> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5f, "ProPhotoRgbLinear<float> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75f, "ProPhotoRgbLinear<float> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5f, "ProPhotoRgbLinear<float> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::ProPhotoRgbLinear<float> zero_alpha = mt::ProPhotoRgbLinear<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "ProPhotoRgbLinear<float> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5f, "ProPhotoRgbLinear<float> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75f, "ProPhotoRgbLinear<float> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0f, "ProPhotoRgbLinear<float> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::ProPhotoRgbLinear<float> clamped = mt::ProPhotoRgbLinear<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "ProPhotoRgbLinear<float> clamp r");
	test.check_equal(clamped.green, 0.5f, "ProPhotoRgbLinear<float> clamp g");
	test.check_equal(clamped.blue, 1.0f, "ProPhotoRgbLinear<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "ProPhotoRgbLinear<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::ProPhotoRgbLinear<float> no_negatives = mt::ProPhotoRgbLinear<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "ProPhotoRgbLinear<float> no_negatives r");
	test.check_equal(no_negatives.green, 0.5f, "ProPhotoRgbLinear<float> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25f, "ProPhotoRgbLinear<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "ProPhotoRgbLinear<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::ProPhotoRgbLinear<float> sanitised = mt::ProPhotoRgbLinear<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "ProPhotoRgbLinear<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "ProPhotoRgbLinear<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "ProPhotoRgbLinear<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "ProPhotoRgbLinear<float> sanitise negative infinity");

	// Reference data checks linear ProPhoto RGB to gamma-2.4 encoded ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_prophoto_rgb_linear_cases) {
		mt::ProPhotoRgbLinear<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgb<float> actual = colour.to_prophoto_rgb();
		mt::ProPhotoRgb<float> transfer = colour.to_prophoto_rgb();
		std::string message = std::string("ProPhotoRgbLinear<float> to_prophoto_rgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 5.0e-6f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 5.0e-6f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 5.0e-6f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
		message = std::string("ProPhotoRgbLinear<float> to_prophoto_rgb ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 5.0e-6f, message + " r");
		test.check_close(transfer.green, colour_case.source[1], 5.0e-6f, message + " g");
		test.check_close(transfer.blue, colour_case.source[2], 5.0e-6f, message + " b");
		test.check_close(transfer.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to HSV.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_hsv_cases) {
		mt::ProPhotoRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::ProPhotoRgbLinearHsv<float> actual = colour.to_hsv();
		std::string message = std::string("ProPhotoRgbLinear<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to HSL.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_hsl_cases) {
		mt::ProPhotoRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::ProPhotoRgbLinearHsl<float> actual = colour.to_hsl();
		std::string message = std::string("ProPhotoRgbLinear<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cases) {
		mt::ProPhotoRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("ProPhotoRgbLinear<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d50_cases) {
		mt::ProPhotoRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::XyzD50<float> actual = colour.to_xyz_d50();
		std::string message = std::string("ProPhotoRgbLinear<float> to_xyz_d50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to CIE XYZ D65 using CAT02.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cat02_cases) {
		mt::ProPhotoRgbLinear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("ProPhotoRgbLinear<float> to_xyz_with_cat02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks CIE XYZ D50 to linear ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d50_cases) {
		mt::XyzD50<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgbLinear<float> actual = colour.to_prophoto_rgb_linear();
		std::string message = std::string("ProPhotoRgbLinear<float> from XyzD50 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 4.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 4.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 4.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks CIE XYZ D65 to linear ProPhoto RGB using Bradford adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgbLinear<float> actual = colour.to_prophoto_rgb_linear();
		std::string message = std::string("ProPhotoRgbLinear<float> from Xyz ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 4.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 4.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 4.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks CIE XYZ D65 to linear ProPhoto RGB using CAT02 adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cat02_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgbLinear<float> actual = colour.to_prophoto_rgb_linear_with_cat02();
		std::string message = std::string("ProPhotoRgbLinear<float> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 4.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 4.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 4.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}
}

int main() {
	TestContext test;

	test_prophoto_rgb_linear_float(test);

	return test.result();
}
