/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Rec709<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/rec709_reference.h"

void test_rec709_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Rec709<double> default_value;
	test.check_equal(default_value.red, 0.0, "Rec709<double> default r");
	test.check_equal(default_value.green, 0.0, "Rec709<double> default g");
	test.check_equal(default_value.blue, 0.0, "Rec709<double> default b");
	test.check_equal(default_value.alpha, 1.0, "Rec709<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::Rec709<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "Rec709<double> grey r");
	test.check_equal(grey.green, 0.25, "Rec709<double> grey g");
	test.check_equal(grey.blue, 0.25, "Rec709<double> grey b");
	test.check_equal(grey.alpha, 1.0, "Rec709<double> grey a");

	// RGB constructor fills alpha with one.
	mt::Rec709<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "Rec709<double> rgb r");
	test.check_equal(rgb.green, 0.2, "Rec709<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "Rec709<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "Rec709<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::Rec709<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "Rec709<double> rgba r");
	test.check_equal(rgba.green, 0.2, "Rec709<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "Rec709<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "Rec709<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::Rec709<double> premultiplied = mt::Rec709<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "Rec709<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "Rec709<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "Rec709<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "Rec709<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::Rec709<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "Rec709<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "Rec709<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "Rec709<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "Rec709<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::Rec709<double> zero_alpha = mt::Rec709<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "Rec709<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "Rec709<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "Rec709<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "Rec709<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::Rec709<double> clamped = mt::Rec709<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "Rec709<double> clamp r");
	test.check_equal(clamped.green, 0.5, "Rec709<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "Rec709<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "Rec709<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Rec709<double> no_negatives = mt::Rec709<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "Rec709<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "Rec709<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "Rec709<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "Rec709<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec709<double> sanitised = mt::Rec709<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "Rec709<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "Rec709<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "Rec709<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Rec709<double> sanitise negative infinity");

	// Reference data checks transfer-encoded Rec.709 to linear Rec.709.
	for (const auto& colour_case : colour_reference::rec709_rec709_linear_cases) {
		mt::Rec709<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Rec709Linear<double> actual = colour.to_linear();
		std::string message = std::string("Rec709<double> to_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.expected[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.expected[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.expected[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks transfer-encoded Rec.709 to HSV.
	for (const auto& colour_case : colour_reference::rec709_hsv_cases) {
		mt::Rec709<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Rec709Hsv<double> actual = colour.to_hsv();
		std::string message = std::string("Rec709<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks transfer-encoded Rec.709 to HSL.
	for (const auto& colour_case : colour_reference::rec709_hsl_cases) {
		mt::Rec709<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Rec709Hsl<double> actual = colour.to_hsl();
		std::string message = std::string("Rec709<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks transfer-encoded Rec.709 to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::rec709_xyz_d65_cases) {
		mt::Rec709<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("Rec709<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_rec709_double(test);

	return test.result();
}
