/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar DisplayP3Linear<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/display_p3_reference.h"

void test_display_p3_linear_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::DisplayP3Linear<double> default_value;
	test.check_equal(default_value.red, 0.0, "DisplayP3Linear<double> default r");
	test.check_equal(default_value.green, 0.0, "DisplayP3Linear<double> default g");
	test.check_equal(default_value.blue, 0.0, "DisplayP3Linear<double> default b");
	test.check_equal(default_value.alpha, 1.0, "DisplayP3Linear<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::DisplayP3Linear<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "DisplayP3Linear<double> grey r");
	test.check_equal(grey.green, 0.25, "DisplayP3Linear<double> grey g");
	test.check_equal(grey.blue, 0.25, "DisplayP3Linear<double> grey b");
	test.check_equal(grey.alpha, 1.0, "DisplayP3Linear<double> grey a");

	// RGB constructor fills alpha with one.
	mt::DisplayP3Linear<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "DisplayP3Linear<double> rgb r");
	test.check_equal(rgb.green, 0.2, "DisplayP3Linear<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "DisplayP3Linear<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "DisplayP3Linear<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::DisplayP3Linear<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "DisplayP3Linear<double> rgba r");
	test.check_equal(rgba.green, 0.2, "DisplayP3Linear<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "DisplayP3Linear<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "DisplayP3Linear<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::DisplayP3Linear<double> premultiplied = mt::DisplayP3Linear<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "DisplayP3Linear<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "DisplayP3Linear<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "DisplayP3Linear<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "DisplayP3Linear<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::DisplayP3Linear<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "DisplayP3Linear<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "DisplayP3Linear<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "DisplayP3Linear<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "DisplayP3Linear<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::DisplayP3Linear<double> zero_alpha = mt::DisplayP3Linear<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "DisplayP3Linear<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "DisplayP3Linear<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "DisplayP3Linear<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "DisplayP3Linear<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::DisplayP3Linear<double> clamped = mt::DisplayP3Linear<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "DisplayP3Linear<double> clamp r");
	test.check_equal(clamped.green, 0.5, "DisplayP3Linear<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "DisplayP3Linear<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "DisplayP3Linear<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::DisplayP3Linear<double> no_negatives = mt::DisplayP3Linear<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "DisplayP3Linear<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "DisplayP3Linear<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "DisplayP3Linear<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "DisplayP3Linear<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::DisplayP3Linear<double> sanitised = mt::DisplayP3Linear<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "DisplayP3Linear<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "DisplayP3Linear<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "DisplayP3Linear<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "DisplayP3Linear<double> sanitise negative infinity");

	// Reference data checks linear Display P3 to transfer-encoded Display P3.
	for (const auto& colour_case : colour_reference::display_p3_display_p3_linear_cases) {
		mt::DisplayP3Linear<double> colour(static_cast<double>(colour_case.expected[0]), static_cast<double>(colour_case.expected[1]), static_cast<double>(colour_case.expected[2]), static_cast<double>(colour_case.expected[3]));
		mt::DisplayP3<double> actual = colour.to_display_p3();
		mt::DisplayP3<double> transfer = colour.to_transfer();
		std::string message = std::string("DisplayP3Linear<double> to_display_p3 ") + colour_case.name;
		test.check_close(actual.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(actual.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(actual.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(actual.alpha, colour_case.source[3], 2.0e-11, message + " a");
		message = std::string("DisplayP3Linear<double> to_transfer ") + colour_case.name;
		test.check_close(transfer.red, colour_case.source[0], 2.0e-11, message + " r");
		test.check_close(transfer.green, colour_case.source[1], 2.0e-11, message + " g");
		test.check_close(transfer.blue, colour_case.source[2], 2.0e-11, message + " b");
		test.check_close(transfer.alpha, colour_case.source[3], 2.0e-11, message + " a");
	}

	// Reference data checks linear Display P3 to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::display_p3_linear_xyz_d65_cases) {
		mt::DisplayP3Linear<double> colour(static_cast<double>(colour_case.source[0]), static_cast<double>(colour_case.source[1]), static_cast<double>(colour_case.source[2]), static_cast<double>(colour_case.source[3]));
		mt::Xyz<double> actual = colour.to_xyz();
		std::string message = std::string("DisplayP3Linear<double> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 2.0e-11, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 2.0e-11, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 2.0e-11, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}
}

int main() {
	TestContext test;

	test_display_p3_linear_double(test);

	return test.result();
}
