/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Rec709Gamma24<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/rec709_reference.h"

void test_rec709_gamma24_double(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Rec709Gamma24<double> default_value;
	test.check_equal(default_value.red, 0.0, "Rec709Gamma24<double> default r");
	test.check_equal(default_value.green, 0.0, "Rec709Gamma24<double> default g");
	test.check_equal(default_value.blue, 0.0, "Rec709Gamma24<double> default b");
	test.check_equal(default_value.alpha, 1.0, "Rec709Gamma24<double> default a");

	// Grey constructor copies one value into every colour channel.
	mt::Rec709Gamma24<double> grey(0.25);
	test.check_equal(grey.red, 0.25, "Rec709Gamma24<double> grey r");
	test.check_equal(grey.green, 0.25, "Rec709Gamma24<double> grey g");
	test.check_equal(grey.blue, 0.25, "Rec709Gamma24<double> grey b");
	test.check_equal(grey.alpha, 1.0, "Rec709Gamma24<double> grey a");

	// RGB constructor fills alpha with one.
	mt::Rec709Gamma24<double> rgb(0.1, 0.2, 0.3);
	test.check_equal(rgb.red, 0.1, "Rec709Gamma24<double> rgb r");
	test.check_equal(rgb.green, 0.2, "Rec709Gamma24<double> rgb g");
	test.check_equal(rgb.blue, 0.3, "Rec709Gamma24<double> rgb b");
	test.check_equal(rgb.alpha, 1.0, "Rec709Gamma24<double> rgb a");

	// RGBA constructor stores every component directly.
	mt::Rec709Gamma24<double> rgba(0.1, 0.2, 0.3, 0.4);
	test.check_equal(rgba.red, 0.1, "Rec709Gamma24<double> rgba r");
	test.check_equal(rgba.green, 0.2, "Rec709Gamma24<double> rgba g");
	test.check_equal(rgba.blue, 0.3, "Rec709Gamma24<double> rgba b");
	test.check_equal(rgba.alpha, 0.4, "Rec709Gamma24<double> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::Rec709Gamma24<double> premultiplied = mt::Rec709Gamma24<double>(0.25, 0.5, 0.75, 0.5).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125, "Rec709Gamma24<double> premultiply r");
	test.check_equal(premultiplied.green, 0.25, "Rec709Gamma24<double> premultiply g");
	test.check_equal(premultiplied.blue, 0.375, "Rec709Gamma24<double> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5, "Rec709Gamma24<double> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::Rec709Gamma24<double> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25, "Rec709Gamma24<double> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5, "Rec709Gamma24<double> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75, "Rec709Gamma24<double> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5, "Rec709Gamma24<double> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::Rec709Gamma24<double> zero_alpha = mt::Rec709Gamma24<double>(0.25, 0.5, 0.75, 0.0).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25, "Rec709Gamma24<double> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5, "Rec709Gamma24<double> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75, "Rec709Gamma24<double> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0, "Rec709Gamma24<double> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::Rec709Gamma24<double> clamped = mt::Rec709Gamma24<double>(-0.25, 0.5, 1.25, 2.0).clamp();
	test.check_equal(clamped.red, 0.0, "Rec709Gamma24<double> clamp r");
	test.check_equal(clamped.green, 0.5, "Rec709Gamma24<double> clamp g");
	test.check_equal(clamped.blue, 1.0, "Rec709Gamma24<double> clamp b");
	test.check_equal(clamped.alpha, 1.0, "Rec709Gamma24<double> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Rec709Gamma24<double> no_negatives = mt::Rec709Gamma24<double>(-0.25, 0.5, 1.25, -0.5).no_negatives();
	test.check_equal(no_negatives.red, 0.0, "Rec709Gamma24<double> no_negatives r");
	test.check_equal(no_negatives.green, 0.5, "Rec709Gamma24<double> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25, "Rec709Gamma24<double> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0, "Rec709Gamma24<double> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec709Gamma24<double> sanitised = mt::Rec709Gamma24<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "Rec709Gamma24<double> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0, "Rec709Gamma24<double> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "Rec709Gamma24<double> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "Rec709Gamma24<double> sanitise negative infinity");

	// Gamma 2.4 transfer checks include a negative value to exercise the reflected extension.
	const double gamma_cases[][4] = {
		{ 0.0, 0.18, 0.5, 1.0 },
		{ 1.0, 0.75, 0.25, 0.4 },
		{ -0.25, 0.25, 1.25, 0.8 },
	};
	for (const auto& colour_case : gamma_cases) {
		const auto decode = [](double value) { return value < 0.0 ? -std::pow(-value, 2.4) : std::pow(value, 2.4); };

		mt::Rec709Gamma24<double> colour(colour_case[0], colour_case[1], colour_case[2], colour_case[3]);
		mt::Rec709Linear<double> actual = colour.to_linear();
		test.check_close(actual.red, decode(colour_case[0]), 1.0e-12, "Rec709Gamma24<double> to_linear r");
		test.check_close(actual.green, decode(colour_case[1]), 1.0e-12, "Rec709Gamma24<double> to_linear g");
		test.check_close(actual.blue, decode(colour_case[2]), 1.0e-12, "Rec709Gamma24<double> to_linear b");
		test.check_close(actual.alpha, colour_case[3], 1.0e-12, "Rec709Gamma24<double> to_linear a");

		mt::Rec709Gamma24<double> encoded = actual.to_rec709_gamma24();
		test.check_close(encoded.red, colour_case[0], 1.0e-12, "Rec709Linear<double> to_rec709_gamma24 r");
		test.check_close(encoded.green, colour_case[1], 1.0e-12, "Rec709Linear<double> to_rec709_gamma24 g");
		test.check_close(encoded.blue, colour_case[2], 1.0e-12, "Rec709Linear<double> to_rec709_gamma24 b");
		test.check_close(encoded.alpha, colour_case[3], 1.0e-12, "Rec709Linear<double> to_rec709_gamma24 a");
	}

	// Reference data checks gamma-2.4 encoded Rec.709 to HSV.
	for (const auto& colour_case : colour_reference::rec709_oetf_hsv_cases) {
		mt::Rec709Gamma24<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Rec709Gamma24Hsv<double> actual = colour.to_hsv();
		std::string message = std::string("Rec709Gamma24<double> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 2.0e-11, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// Reference data checks gamma-2.4 encoded Rec.709 to HSL.
	for (const auto& colour_case : colour_reference::rec709_oetf_hsl_cases) {
		mt::Rec709Gamma24<double> colour(colour_case.source[0], colour_case.source[1], colour_case.source[2], colour_case.source[3]);
		mt::Rec709Gamma24Hsl<double> actual = colour.to_hsl();
		std::string message = std::string("Rec709Gamma24<double> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 2.0e-11, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 2.0e-11, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 2.0e-11, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 2.0e-11, message + " a");
	}

	// XYZ conversion uses the shared Rec.709 linear matrix after gamma decoding.
	mt::Rec709Gamma24<double> xyz_colour(0.5, 0.25, 0.75, 0.4);
	mt::Xyz<double> expected_xyz = xyz_colour.to_linear().to_xyz();
	mt::Xyz<double> actual_xyz = xyz_colour.to_xyz();
	test.check_close(actual_xyz.x, expected_xyz.x, 1.0e-12, "Rec709Gamma24<double> to_xyz x");
	test.check_close(actual_xyz.y, expected_xyz.y, 1.0e-12, "Rec709Gamma24<double> to_xyz y");
	test.check_close(actual_xyz.z, expected_xyz.z, 1.0e-12, "Rec709Gamma24<double> to_xyz z");
	test.check_close(actual_xyz.alpha, expected_xyz.alpha, 1.0e-12, "Rec709Gamma24<double> to_xyz a");

	mt::Rec709Gamma24<double> xyz_roundtrip = expected_xyz.to_rec709_gamma24();
	test.check_close(xyz_roundtrip.red, xyz_colour.red, 1.0e-10, "Xyz<double> to_rec709_gamma24 r");
	test.check_close(xyz_roundtrip.green, xyz_colour.green, 1.0e-10, "Xyz<double> to_rec709_gamma24 g");
	test.check_close(xyz_roundtrip.blue, xyz_colour.blue, 1.0e-10, "Xyz<double> to_rec709_gamma24 b");
	test.check_close(xyz_roundtrip.alpha, xyz_colour.alpha, 1.0e-12, "Xyz<double> to_rec709_gamma24 a");
}

int main() {
	TestContext test;

	test_rec709_gamma24_double(test);

	return test.result();
}
