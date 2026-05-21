/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar ProPhotoRgb<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/prophoto_rgb_reference.h"

void test_prophoto_rgb_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::ProPhotoRgb<double> default_value;
	test.check_equal(default_value.red, 0.0, "ProPhotoRgb<double> default r");
	test.check_equal(default_value.green, 0.0, "ProPhotoRgb<double> default g");
	test.check_equal(default_value.blue, 0.0, "ProPhotoRgb<double> default b");
	test.check_equal(default_value.alpha, 1.0, "ProPhotoRgb<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::ProPhotoRgb<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "ProPhotoRgb<double> grey r");
	test.check_equal(grey.green, 0.25, "ProPhotoRgb<double> grey g");
	test.check_equal(grey.blue, 0.25, "ProPhotoRgb<double> grey b");
	test.check_equal(grey.alpha, 1.0, "ProPhotoRgb<double> grey a");

	// RGB constructor fills alpha with one.
	mt::ProPhotoRgb<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "ProPhotoRgb<double> rgb r");
	test.check_equal(rgb.green, 0.2, "ProPhotoRgb<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "ProPhotoRgb<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "ProPhotoRgb<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::ProPhotoRgb<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "ProPhotoRgb<double> rgba r");
	test.check_equal(rgba.green, 0.2, "ProPhotoRgb<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "ProPhotoRgb<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "ProPhotoRgb<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::ProPhotoRgb<double> premultiplied = mt::ProPhotoRgb<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "ProPhotoRgb<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "ProPhotoRgb<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "ProPhotoRgb<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "ProPhotoRgb<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::ProPhotoRgb<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "ProPhotoRgb<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "ProPhotoRgb<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "ProPhotoRgb<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "ProPhotoRgb<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::ProPhotoRgb<double> zero_alpha = mt::ProPhotoRgb<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "ProPhotoRgb<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "ProPhotoRgb<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "ProPhotoRgb<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "ProPhotoRgb<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::ProPhotoRgb<double> clamped = mt::ProPhotoRgb<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "ProPhotoRgb<double> clamp r");
	test.check_equal(clamped.green, 0.5, "ProPhotoRgb<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "ProPhotoRgb<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "ProPhotoRgb<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::ProPhotoRgb<double> no_negatives = mt::ProPhotoRgb<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "ProPhotoRgb<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "ProPhotoRgb<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "ProPhotoRgb<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "ProPhotoRgb<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::ProPhotoRgb<double> sanitised = mt::ProPhotoRgb<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "ProPhotoRgb<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "ProPhotoRgb<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "ProPhotoRgb<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "ProPhotoRgb<double> sanitise negative infinity");

	// Reference data checks gamma-2.4 encoded ProPhoto RGB to linear ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_prophoto_rgb_linear_cases) {
		mt::ProPhotoRgb<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::ProPhotoRgbLinear<double> actual = colour.to_linear();
		std::string message = std::string("ProPhotoRgb<double> to_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.expected[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.expected[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.expected[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks gamma-2.4 encoded ProPhoto RGB to HSV.
	for (const auto& colour_case : colour_reference::prophoto_rgb_hsv_cases) {
		mt::ProPhotoRgb<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::ProPhotoRgbHsv<double> actual = colour.to_hsv();
		std::string message = std::string("ProPhotoRgb<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks gamma-2.4 encoded ProPhoto RGB to HSL.
	for (const auto& colour_case : colour_reference::prophoto_rgb_hsl_cases) {
		mt::ProPhotoRgb<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::ProPhotoRgbHsl<double> actual = colour.to_hsl();
		std::string message = std::string("ProPhotoRgb<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks gamma-2.4 encoded ProPhoto RGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cases) {
		mt::ProPhotoRgb<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("ProPhotoRgb<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks gamma-2.4 encoded ProPhoto RGB to CIE XYZ D50.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d50_cases) {
		mt::ProPhotoRgb<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::XyzD50<double> actual = colour.to_xyz_d50();
		std::string message = std::string("ProPhotoRgb<double> to_xyz_d50 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks gamma-2.4 encoded ProPhoto RGB to CIE XYZ D65 using CAT02.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cat02_cases) {
		mt::ProPhotoRgb<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Xyz<double> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("ProPhotoRgb<double> to_xyz_with_cat02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks CIE XYZ D50 to gamma-2.4 encoded ProPhoto RGB.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d50_cases) {
		mt::XyzD50<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::ProPhotoRgb<double> actual = colour.to_prophoto_rgb();
		std::string message = std::string("ProPhotoRgb<double> from XyzD50 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-10, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-10, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks CIE XYZ D65 to gamma-2.4 encoded ProPhoto RGB using Bradford adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cases) {
		mt::Xyz<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::ProPhotoRgb<double> actual = colour.to_prophoto_rgb();
		std::string message = std::string("ProPhotoRgb<double> from Xyz ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-10, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-10, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks CIE XYZ D65 to gamma-2.4 encoded ProPhoto RGB using CAT02 adaptation.
	for (const auto& colour_case : colour_reference::prophoto_rgb_xyz_d65_cat02_cases) {
		mt::Xyz<double> colour(colour_case.expected[0], colour_case.expected[1], colour_case.expected[2], colour_case.expected[3]);
		mt::ProPhotoRgb<double> actual = colour.to_prophoto_rgb_with_cat02();
		std::string message = std::string("ProPhotoRgb<double> from Xyz CAT02 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 3.0e-10, message + " r");
		test.check_close(actual.green, colour_case.source[1], 3.0e-10, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 3.0e-10, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_prophoto_rgb_double(test);

	return test.result();
}
