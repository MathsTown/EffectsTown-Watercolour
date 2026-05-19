/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar ProPhotoRgb<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/prophoto_rgb_reference.h"

void test_prophoto_rgb_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::ProPhotoRgb<float> default_value;
	test.check_equal(default_value.red, 0.0f, "ProPhotoRgb<float> default r");
	test.check_equal(default_value.green, 0.0f, "ProPhotoRgb<float> default g");
	test.check_equal(default_value.blue, 0.0f, "ProPhotoRgb<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "ProPhotoRgb<float> default a");

	// Grey constructor copies one value into every colour channel.
	mt::ProPhotoRgb<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "ProPhotoRgb<float> grey r");
	test.check_equal(grey.green, 0.25f, "ProPhotoRgb<float> grey g");
	test.check_equal(grey.blue, 0.25f, "ProPhotoRgb<float> grey b");
	test.check_equal(grey.alpha, 1.0f, "ProPhotoRgb<float> grey a");

	// RGB constructor fills alpha with one.
	mt::ProPhotoRgb<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "ProPhotoRgb<float> rgb r");
	test.check_equal(rgb.green, 0.2f, "ProPhotoRgb<float> rgb g");
	test.check_equal(rgb.blue, 0.3f, "ProPhotoRgb<float> rgb b");
	test.check_equal(rgb.alpha, 1.0f, "ProPhotoRgb<float> rgb a");

	// RGBA constructor stores every component directly.
	mt::ProPhotoRgb<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "ProPhotoRgb<float> rgba r");
	test.check_equal(rgba.green, 0.2f, "ProPhotoRgb<float> rgba g");
	test.check_equal(rgba.blue, 0.3f, "ProPhotoRgb<float> rgba b");
	test.check_equal(rgba.alpha, 0.4f, "ProPhotoRgb<float> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::ProPhotoRgb<float> premultiplied = mt::ProPhotoRgb<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "ProPhotoRgb<float> premultiply r");
	test.check_equal(premultiplied.green, 0.25f, "ProPhotoRgb<float> premultiply g");
	test.check_equal(premultiplied.blue, 0.375f, "ProPhotoRgb<float> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5f, "ProPhotoRgb<float> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::ProPhotoRgb<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "ProPhotoRgb<float> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5f, "ProPhotoRgb<float> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75f, "ProPhotoRgb<float> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5f, "ProPhotoRgb<float> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::ProPhotoRgb<float> zero_alpha = mt::ProPhotoRgb<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "ProPhotoRgb<float> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5f, "ProPhotoRgb<float> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75f, "ProPhotoRgb<float> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0f, "ProPhotoRgb<float> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::ProPhotoRgb<float> clamped = mt::ProPhotoRgb<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "ProPhotoRgb<float> clamp r");
	test.check_equal(clamped.green, 0.5f, "ProPhotoRgb<float> clamp g");
	test.check_equal(clamped.blue, 1.0f, "ProPhotoRgb<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "ProPhotoRgb<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::ProPhotoRgb<float> no_negatives = mt::ProPhotoRgb<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "ProPhotoRgb<float> no_negatives r");
	test.check_equal(no_negatives.green, 0.5f, "ProPhotoRgb<float> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25f, "ProPhotoRgb<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "ProPhotoRgb<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::ProPhotoRgb<float> sanitised = mt::ProPhotoRgb<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "ProPhotoRgb<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "ProPhotoRgb<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "ProPhotoRgb<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "ProPhotoRgb<float> sanitise negative infinity");

	// Reference data checks transfer-encoded ProPhoto RGB to linear ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_prophoto_rgb_linear_cases) {
		mt::ProPhotoRgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::ProPhotoRgbLinear<float> actual = colour.to_linear();
		std::string message = std::string("ProPhotoRgb<float> to_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.expected[0], 5.0e-6f, message + " r");
		test.check_close(actual.green, colour_case.expected[1], 5.0e-6f, message + " g");
		test.check_close(actual.blue, colour_case.expected[2], 5.0e-6f, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded ProPhoto RGB to HSV.
	for (const auto& colour_case : colour_reference::prophoto_rgb_hsv_cases) {
		mt::ProPhotoRgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::ProPhotoRgbHsv<float> actual = colour.to_hsv();
		std::string message = std::string("ProPhotoRgb<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded ProPhoto RGB to HSL.
	for (const auto& colour_case : colour_reference::prophoto_rgb_hsl_cases) {
		mt::ProPhotoRgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::ProPhotoRgbHsl<float> actual = colour.to_hsl();
		std::string message = std::string("ProPhotoRgb<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded ProPhoto RGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cases) {
		mt::ProPhotoRgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("ProPhotoRgb<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded ProPhoto RGB to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d50_cases) {
		mt::ProPhotoRgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::XyzD50<float> actual = colour.to_xyz_d50();
		std::string message = std::string("ProPhotoRgb<float> to_xyz_d50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded ProPhoto RGB to CIE XYZ D65 using CAT02.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cat02_cases) {
		mt::ProPhotoRgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("ProPhotoRgb<float> to_xyz_with_cat02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks CIE XYZ D50 to transfer-encoded ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d50_cases) {
		mt::XyzD50<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgb<float> actual = colour.to_prophoto_rgb();
		std::string message = std::string("ProPhotoRgb<float> from XyzD50 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 4.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 4.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 4.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks CIE XYZ D65 to transfer-encoded ProPhoto RGB using Bradford adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgb<float> actual = colour.to_prophoto_rgb();
		std::string message = std::string("ProPhotoRgb<float> from Xyz ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 4.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 4.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 4.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks CIE XYZ D65 to transfer-encoded ProPhoto RGB using CAT02 adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cat02_cases) {
		mt::Xyz<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::ProPhotoRgb<float> actual = colour.to_prophoto_rgb_with_cat02();
		std::string message = std::string("ProPhotoRgb<float> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 4.0e-5f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 4.0e-5f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 4.0e-5f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}
}

int main() {
	TestContext test;

	test_prophoto_rgb_float(test);

	return test.result();
}
