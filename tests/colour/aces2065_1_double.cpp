/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Aces2065_1<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/aces_reference.h"

void test_aces2065_1_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Aces2065_1<double> default_value;
	test.check_equal(default_value.red, 0.0, "Aces2065_1<double> default r");
	test.check_equal(default_value.green, 0.0, "Aces2065_1<double> default g");
	test.check_equal(default_value.blue, 0.0, "Aces2065_1<double> default b");
	test.check_equal(default_value.alpha, 1.0, "Aces2065_1<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::Aces2065_1<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "Aces2065_1<double> grey r");
	test.check_equal(grey.green, 0.25, "Aces2065_1<double> grey g");
	test.check_equal(grey.blue, 0.25, "Aces2065_1<double> grey b");
	test.check_equal(grey.alpha, 1.0, "Aces2065_1<double> grey a");

	// RGB constructor fills alpha with one.
	mt::Aces2065_1<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "Aces2065_1<double> rgb r");
	test.check_equal(rgb.green, 0.2, "Aces2065_1<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "Aces2065_1<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "Aces2065_1<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::Aces2065_1<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "Aces2065_1<double> rgba r");
	test.check_equal(rgba.green, 0.2, "Aces2065_1<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "Aces2065_1<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "Aces2065_1<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::Aces2065_1<double> premultiplied = mt::Aces2065_1<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "Aces2065_1<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "Aces2065_1<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "Aces2065_1<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "Aces2065_1<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::Aces2065_1<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "Aces2065_1<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "Aces2065_1<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "Aces2065_1<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "Aces2065_1<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::Aces2065_1<double> zero_alpha = mt::Aces2065_1<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "Aces2065_1<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "Aces2065_1<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "Aces2065_1<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "Aces2065_1<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::Aces2065_1<double> clamped = mt::Aces2065_1<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "Aces2065_1<double> clamp r");
	test.check_equal(clamped.green, 0.5, "Aces2065_1<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "Aces2065_1<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "Aces2065_1<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Aces2065_1<double> no_negatives = mt::Aces2065_1<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "Aces2065_1<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "Aces2065_1<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "Aces2065_1<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "Aces2065_1<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Aces2065_1<double> sanitised = mt::Aces2065_1<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "Aces2065_1<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "Aces2065_1<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "Aces2065_1<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Aces2065_1<double> sanitise negative infinity");

	// Reference data checks ACES2065-1 to ACEScg.
	for (const auto& colour_case : colour_reference::aces2065_1_acescg_cases) {
		mt::Aces2065_1<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Acescg<double> actual = colour.to_acescg();
		std::string message = std::string("Aces2065_1<double> to_acescg ") + colour_case.name;
		test.check_close(actual.red, colour_case.expected[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.expected[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.expected[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ACES2065-1 to HSV.
	for (const auto& colour_case : colour_reference::aces2065_1_hsv_cases) {
		mt::Aces2065_1<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Aces2065_1Hsv<double> actual = colour.to_hsv();
		std::string message = std::string("Aces2065_1<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ACES2065-1 to HSL.
	for (const auto& colour_case : colour_reference::aces2065_1_hsl_cases) {
		mt::Aces2065_1<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Aces2065_1Hsl<double> actual = colour.to_hsl();
		std::string message = std::string("Aces2065_1<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ACES2065-1 to Bradford-adapted CIE XYZ D65.
	for (const auto& colour_case : colour_reference::aces2065_1_xyz_d65_cases) {
		mt::Aces2065_1<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("Aces2065_1<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ACES2065-1 to exact ACES-white XYZ.
	for (const auto& colour_case : colour_reference::aces2065_1_xyz_aces_cases) {
		mt::Aces2065_1<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::XyzAces<double> actual = colour.to_xyz_aces();
		std::string message = std::string("Aces2065_1<double> to_xyz_aces ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear ACES2065-1 to CAT02-adapted CIE XYZ D65.
	for (const auto& colour_case : colour_reference::aces2065_1_xyz_d65_cat02_cases) {
		mt::Aces2065_1<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz_with_cat02();
		std::string message = std::string("Aces2065_1<double> to_xyz_with_cat02 ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_aces2065_1_double(test);

	return test.result();
}
