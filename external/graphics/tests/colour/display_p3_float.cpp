/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar DisplayP3<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/display_p3_reference.h"

void test_display_p3_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::DisplayP3<float> default_value;
	test.check_equal(default_value.red, 0.0f, "DisplayP3<float> default r");
	test.check_equal(default_value.green, 0.0f, "DisplayP3<float> default g");
	test.check_equal(default_value.blue, 0.0f, "DisplayP3<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "DisplayP3<float> default a");

	// Grey constructor copies one value into every colour channel.
	mt::DisplayP3<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "DisplayP3<float> grey r");
	test.check_equal(grey.green, 0.25f, "DisplayP3<float> grey g");
	test.check_equal(grey.blue, 0.25f, "DisplayP3<float> grey b");
	test.check_equal(grey.alpha, 1.0f, "DisplayP3<float> grey a");

	// RGB constructor fills alpha with one.
	mt::DisplayP3<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "DisplayP3<float> rgb r");
	test.check_equal(rgb.green, 0.2f, "DisplayP3<float> rgb g");
	test.check_equal(rgb.blue, 0.3f, "DisplayP3<float> rgb b");
	test.check_equal(rgb.alpha, 1.0f, "DisplayP3<float> rgb a");

	// RGBA constructor stores every component directly.
	mt::DisplayP3<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "DisplayP3<float> rgba r");
	test.check_equal(rgba.green, 0.2f, "DisplayP3<float> rgba g");
	test.check_equal(rgba.blue, 0.3f, "DisplayP3<float> rgba b");
	test.check_equal(rgba.alpha, 0.4f, "DisplayP3<float> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::DisplayP3<float> premultiplied = mt::DisplayP3<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "DisplayP3<float> premultiply r");
	test.check_equal(premultiplied.green, 0.25f, "DisplayP3<float> premultiply g");
	test.check_equal(premultiplied.blue, 0.375f, "DisplayP3<float> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5f, "DisplayP3<float> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::DisplayP3<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "DisplayP3<float> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5f, "DisplayP3<float> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75f, "DisplayP3<float> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5f, "DisplayP3<float> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::DisplayP3<float> zero_alpha = mt::DisplayP3<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "DisplayP3<float> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5f, "DisplayP3<float> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75f, "DisplayP3<float> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0f, "DisplayP3<float> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::DisplayP3<float> clamped = mt::DisplayP3<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "DisplayP3<float> clamp r");
	test.check_equal(clamped.green, 0.5f, "DisplayP3<float> clamp g");
	test.check_equal(clamped.blue, 1.0f, "DisplayP3<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "DisplayP3<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::DisplayP3<float> no_negatives = mt::DisplayP3<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "DisplayP3<float> no_negatives r");
	test.check_equal(no_negatives.green, 0.5f, "DisplayP3<float> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25f, "DisplayP3<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "DisplayP3<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::DisplayP3<float> sanitised = mt::DisplayP3<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "DisplayP3<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "DisplayP3<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "DisplayP3<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "DisplayP3<float> sanitise negative infinity");

	// Reference data checks transfer-encoded Display P3 to linear Display P3.
	for (const auto& colour_case : colour_reference::display_p3_display_p3_linear_cases) {
		mt::DisplayP3<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::DisplayP3Linear<float> actual = colour.to_linear();
		std::string message = std::string("DisplayP3<float> to_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.expected[0], 5.0e-6f, message + " r");
		test.check_close(actual.green, colour_case.expected[1], 5.0e-6f, message + " g");
		test.check_close(actual.blue, colour_case.expected[2], 5.0e-6f, message + " b");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded Display P3 to HSV.
	for (const auto& colour_case : colour_reference::display_p3_hsv_cases) {
		mt::DisplayP3<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::DisplayP3Hsv<float> actual = colour.to_hsv();
		std::string message = std::string("DisplayP3<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded Display P3 to HSL.
	for (const auto& colour_case : colour_reference::display_p3_hsl_cases) {
		mt::DisplayP3<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::DisplayP3Hsl<float> actual = colour.to_hsl();
		std::string message = std::string("DisplayP3<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks transfer-encoded Display P3 to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::display_p3_xyz_d65_cases) {
		mt::DisplayP3<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("DisplayP3<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}
}

int main() {
	TestContext test;

	test_display_p3_float(test);

	return test.result();
}
