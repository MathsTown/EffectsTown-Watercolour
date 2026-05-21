/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar ProPhotoRgbLinear<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/prophoto_rgb_reference.h"

void test_prophoto_rgb_linear_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::ProPhotoRgbLinear<double> default_value;
	test.check_equal(default_value.red, 0.0, "ProPhotoRgbLinear<double> default r");
	test.check_equal(default_value.green, 0.0, "ProPhotoRgbLinear<double> default g");
	test.check_equal(default_value.blue, 0.0, "ProPhotoRgbLinear<double> default b");
	test.check_equal(default_value.alpha, 1.0, "ProPhotoRgbLinear<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::ProPhotoRgbLinear<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "ProPhotoRgbLinear<double> grey r");
	test.check_equal(grey.green, 0.25, "ProPhotoRgbLinear<double> grey g");
	test.check_equal(grey.blue, 0.25, "ProPhotoRgbLinear<double> grey b");
	test.check_equal(grey.alpha, 1.0, "ProPhotoRgbLinear<double> grey a");

	// RGB constructor fills alpha with one.
	mt::ProPhotoRgbLinear<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "ProPhotoRgbLinear<double> rgb r");
	test.check_equal(rgb.green, 0.2, "ProPhotoRgbLinear<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "ProPhotoRgbLinear<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "ProPhotoRgbLinear<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::ProPhotoRgbLinear<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "ProPhotoRgbLinear<double> rgba r");
	test.check_equal(rgba.green, 0.2, "ProPhotoRgbLinear<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "ProPhotoRgbLinear<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "ProPhotoRgbLinear<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::ProPhotoRgbLinear<double> premultiplied = mt::ProPhotoRgbLinear<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "ProPhotoRgbLinear<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "ProPhotoRgbLinear<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "ProPhotoRgbLinear<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "ProPhotoRgbLinear<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::ProPhotoRgbLinear<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "ProPhotoRgbLinear<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "ProPhotoRgbLinear<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "ProPhotoRgbLinear<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "ProPhotoRgbLinear<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::ProPhotoRgbLinear<double> zero_alpha = mt::ProPhotoRgbLinear<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "ProPhotoRgbLinear<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "ProPhotoRgbLinear<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "ProPhotoRgbLinear<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "ProPhotoRgbLinear<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::ProPhotoRgbLinear<double> clamped = mt::ProPhotoRgbLinear<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "ProPhotoRgbLinear<double> clamp r");
	test.check_equal(clamped.green, 0.5, "ProPhotoRgbLinear<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "ProPhotoRgbLinear<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "ProPhotoRgbLinear<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::ProPhotoRgbLinear<double> no_negatives = mt::ProPhotoRgbLinear<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "ProPhotoRgbLinear<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "ProPhotoRgbLinear<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "ProPhotoRgbLinear<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "ProPhotoRgbLinear<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::ProPhotoRgbLinear<double> sanitised = mt::ProPhotoRgbLinear<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "ProPhotoRgbLinear<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "ProPhotoRgbLinear<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "ProPhotoRgbLinear<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "ProPhotoRgbLinear<double> sanitise negative infinity");

	// Reference data checks linear ProPhoto RGB to gamma-2.4 encoded ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_prophoto_rgb_linear_cases) {
		mt::ProPhotoRgbLinear<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::ProPhotoRgb<double> actual = colour.to_prophoto_rgb();
		mt::ProPhotoRgb<double> transfer = colour.to_prophoto_rgb();
		std::string message = std::string("ProPhotoRgbLinear<double> to_prophoto_rgb ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
		message = std::string("ProPhotoRgbLinear<double> to_prophoto_rgb ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(transfer.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(transfer.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(transfer.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to HSV.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_hsv_cases) {
		mt::ProPhotoRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::ProPhotoRgbLinearHsv<double> actual = colour.to_hsv();
		std::string message = std::string("ProPhotoRgbLinear<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to HSL.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_hsl_cases) {
		mt::ProPhotoRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::ProPhotoRgbLinearHsl<double> actual = colour.to_hsl();
		std::string message = std::string("ProPhotoRgbLinear<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cases) {
		mt::ProPhotoRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("ProPhotoRgbLinear<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d50_cases) {
		mt::ProPhotoRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::XyzD50<double> actual = colour.to_xyz_d50();
		std::string message = std::string("ProPhotoRgbLinear<double> to_xyz_d50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ProPhoto RGB to CIE XYZ D65 using CAT02.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cat02_cases) {
		mt::ProPhotoRgbLinear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("ProPhotoRgbLinear<double> to_xyz_with_cat02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks CIE XYZ D50 to linear ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d50_cases) {
		mt::XyzD50<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::ProPhotoRgbLinear<double> actual = colour.to_prophoto_rgb_linear();
		std::string message = std::string("ProPhotoRgbLinear<double> from XyzD50 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-10, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-10, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks CIE XYZ D65 to linear ProPhoto RGB using Bradford adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cases) {
		mt::Xyz<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::ProPhotoRgbLinear<double> actual = colour.to_prophoto_rgb_linear();
		std::string message = std::string("ProPhotoRgbLinear<double> from Xyz ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-10, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-10, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks CIE XYZ D65 to linear ProPhoto RGB using CAT02 adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_linear_xyz_d65_cat02_cases) {
		mt::Xyz<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::ProPhotoRgbLinear<double> actual = colour.to_prophoto_rgb_linear_with_cat02();
		std::string message = std::string("ProPhotoRgbLinear<double> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-10, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-10, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_prophoto_rgb_linear_double(test);

	return test.result();
}
