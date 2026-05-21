/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar DisplayP3Linear<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/display_p3_reference.h"

void test_display_p3_linear_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::DisplayP3Linear<float> default_value;
	test.check_equal(default_value.red, 0.0f, "DisplayP3Linear<float> default r");
	test.check_equal(default_value.green, 0.0f, "DisplayP3Linear<float> default g");
	test.check_equal(default_value.blue, 0.0f, "DisplayP3Linear<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "DisplayP3Linear<float> default a");

	// Grey constructor copies one value into every colour channel.
	mt::DisplayP3Linear<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "DisplayP3Linear<float> grey r");
	test.check_equal(grey.green, 0.25f, "DisplayP3Linear<float> grey g");
	test.check_equal(grey.blue, 0.25f, "DisplayP3Linear<float> grey b");
	test.check_equal(grey.alpha, 1.0f, "DisplayP3Linear<float> grey a");

	// RGB constructor fills alpha with one.
	mt::DisplayP3Linear<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "DisplayP3Linear<float> rgb r");
	test.check_equal(rgb.green, 0.2f, "DisplayP3Linear<float> rgb g");
	test.check_equal(rgb.blue, 0.3f, "DisplayP3Linear<float> rgb b");
	test.check_equal(rgb.alpha, 1.0f, "DisplayP3Linear<float> rgb a");

	// RGBA constructor stores every component directly.
	mt::DisplayP3Linear<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "DisplayP3Linear<float> rgba r");
	test.check_equal(rgba.green, 0.2f, "DisplayP3Linear<float> rgba g");
	test.check_equal(rgba.blue, 0.3f, "DisplayP3Linear<float> rgba b");
	test.check_equal(rgba.alpha, 0.4f, "DisplayP3Linear<float> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::DisplayP3Linear<float> premultiplied = mt::DisplayP3Linear<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "DisplayP3Linear<float> premultiply r");
	test.check_equal(premultiplied.green, 0.25f, "DisplayP3Linear<float> premultiply g");
	test.check_equal(premultiplied.blue, 0.375f, "DisplayP3Linear<float> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5f, "DisplayP3Linear<float> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::DisplayP3Linear<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "DisplayP3Linear<float> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5f, "DisplayP3Linear<float> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75f, "DisplayP3Linear<float> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5f, "DisplayP3Linear<float> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::DisplayP3Linear<float> zero_alpha = mt::DisplayP3Linear<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "DisplayP3Linear<float> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5f, "DisplayP3Linear<float> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75f, "DisplayP3Linear<float> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0f, "DisplayP3Linear<float> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::DisplayP3Linear<float> clamped = mt::DisplayP3Linear<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "DisplayP3Linear<float> clamp r");
	test.check_equal(clamped.green, 0.5f, "DisplayP3Linear<float> clamp g");
	test.check_equal(clamped.blue, 1.0f, "DisplayP3Linear<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "DisplayP3Linear<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::DisplayP3Linear<float> no_negatives = mt::DisplayP3Linear<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "DisplayP3Linear<float> no_negatives r");
	test.check_equal(no_negatives.green, 0.5f, "DisplayP3Linear<float> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25f, "DisplayP3Linear<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "DisplayP3Linear<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::DisplayP3Linear<float> sanitised = mt::DisplayP3Linear<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "DisplayP3Linear<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "DisplayP3Linear<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "DisplayP3Linear<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "DisplayP3Linear<float> sanitise negative infinity");

	// Reference data checks linear Display P3 to gamma-2.4 encoded Display P3.
	for (const auto& colour_case : colour_reference::display_p3_display_p3_linear_cases) {
		mt::DisplayP3Linear<float> colour(static_cast<float>(colour_case.expected[0]), static_cast<float>(colour_case.expected[1]), static_cast<float>(colour_case.expected[2]), static_cast<float>(colour_case.expected[3]));
		mt::DisplayP3<float> actual = colour.to_display_p3();
		mt::DisplayP3<float> transfer = colour.to_display_p3();
		std::string message = std::string("DisplayP3Linear<float> to_display_p3 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 5.0e-6f, message + " r");
		test.check_close(actual.green, colour_case.source[1], 5.0e-6f, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 5.0e-6f, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 5.0e-6f, message + " a");
		message = std::string("DisplayP3Linear<float> to_display_p3 ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 5.0e-6f, message + " r");
		test.check_close(transfer.green, colour_case.source[1], 5.0e-6f, message + " g");
		test.check_close(transfer.blue, colour_case.source[2], 5.0e-6f, message + " b");
		test.check_close(transfer.alpha, colour_case.source[3], 5.0e-6f, message + " a");
	}

	// Reference data checks linear Display P3 to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::display_p3_linear_xyz_d65_cases) {
		mt::DisplayP3Linear<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("DisplayP3Linear<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}
}

int main() {
	TestContext test;

	test_display_p3_linear_float(test);

	return test.result();
}
