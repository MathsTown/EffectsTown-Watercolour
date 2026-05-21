/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Rec2020Gamma24<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "reference/rec2020_reference.h"

void test_rec2020_gamma24_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Rec2020Gamma24<float> default_value;
	test.check_equal(default_value.red, 0.0f, "Rec2020Gamma24<float> default r");
	test.check_equal(default_value.green, 0.0f, "Rec2020Gamma24<float> default g");
	test.check_equal(default_value.blue, 0.0f, "Rec2020Gamma24<float> default b");
	test.check_equal(default_value.alpha, 1.0f, "Rec2020Gamma24<float> default a");

	// Grey constructor copies one value into every colour channel.
	mt::Rec2020Gamma24<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "Rec2020Gamma24<float> grey r");
	test.check_equal(grey.green, 0.25f, "Rec2020Gamma24<float> grey g");
	test.check_equal(grey.blue, 0.25f, "Rec2020Gamma24<float> grey b");
	test.check_equal(grey.alpha, 1.0f, "Rec2020Gamma24<float> grey a");

	// RGB constructor fills alpha with one.
	mt::Rec2020Gamma24<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "Rec2020Gamma24<float> rgb r");
	test.check_equal(rgb.green, 0.2f, "Rec2020Gamma24<float> rgb g");
	test.check_equal(rgb.blue, 0.3f, "Rec2020Gamma24<float> rgb b");
	test.check_equal(rgb.alpha, 1.0f, "Rec2020Gamma24<float> rgb a");

	// RGBA constructor stores every component directly.
	mt::Rec2020Gamma24<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "Rec2020Gamma24<float> rgba r");
	test.check_equal(rgba.green, 0.2f, "Rec2020Gamma24<float> rgba g");
	test.check_equal(rgba.blue, 0.3f, "Rec2020Gamma24<float> rgba b");
	test.check_equal(rgba.alpha, 0.4f, "Rec2020Gamma24<float> rgba a");

	// Premultiply alpha multiplies only the colour channels.
	mt::Rec2020Gamma24<float> premultiplied = mt::Rec2020Gamma24<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "Rec2020Gamma24<float> premultiply r");
	test.check_equal(premultiplied.green, 0.25f, "Rec2020Gamma24<float> premultiply g");
	test.check_equal(premultiplied.blue, 0.375f, "Rec2020Gamma24<float> premultiply b");
	test.check_equal(premultiplied.alpha, 0.5f, "Rec2020Gamma24<float> premultiply a");

	// Un-premultiply alpha divides only the colour channels.
	mt::Rec2020Gamma24<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "Rec2020Gamma24<float> unpremultiply r");
	test.check_equal(unpremultiplied.green, 0.5f, "Rec2020Gamma24<float> unpremultiply g");
	test.check_equal(unpremultiplied.blue, 0.75f, "Rec2020Gamma24<float> unpremultiply b");
	test.check_equal(unpremultiplied.alpha, 0.5f, "Rec2020Gamma24<float> unpremultiply a");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::Rec2020Gamma24<float> zero_alpha = mt::Rec2020Gamma24<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "Rec2020Gamma24<float> zero alpha unpremultiply r");
	test.check_equal(zero_alpha.green, 0.5f, "Rec2020Gamma24<float> zero alpha unpremultiply g");
	test.check_equal(zero_alpha.blue, 0.75f, "Rec2020Gamma24<float> zero alpha unpremultiply b");
	test.check_equal(zero_alpha.alpha, 0.0f, "Rec2020Gamma24<float> zero alpha unpremultiply a");

	// Clamp restricts every component to the 0..1 range.
	mt::Rec2020Gamma24<float> clamped = mt::Rec2020Gamma24<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "Rec2020Gamma24<float> clamp r");
	test.check_equal(clamped.green, 0.5f, "Rec2020Gamma24<float> clamp g");
	test.check_equal(clamped.blue, 1.0f, "Rec2020Gamma24<float> clamp b");
	test.check_equal(clamped.alpha, 1.0f, "Rec2020Gamma24<float> clamp a");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Rec2020Gamma24<float> no_negatives = mt::Rec2020Gamma24<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "Rec2020Gamma24<float> no_negatives r");
	test.check_equal(no_negatives.green, 0.5f, "Rec2020Gamma24<float> no_negatives g");
	test.check_equal(no_negatives.blue, 1.25f, "Rec2020Gamma24<float> no_negatives b");
	test.check_equal(no_negatives.alpha, 0.0f, "Rec2020Gamma24<float> no_negatives a");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec2020Gamma24<float> sanitised = mt::Rec2020Gamma24<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "Rec2020Gamma24<float> sanitise negative zero");
	test.check_equal(sanitised.green, 0.0f, "Rec2020Gamma24<float> sanitise nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "Rec2020Gamma24<float> sanitise infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Rec2020Gamma24<float> sanitise negative infinity");

	// Gamma 2.4 transfer checks include a negative value to exercise the reflected extension.
	const float gamma_cases[][4] = {
		{ 0.0f, 0.18f, 0.5f, 1.0f },
		{ 1.0f, 0.75f, 0.25f, 0.4f },
		{ -0.25f, 0.25f, 1.25f, 0.8f },
	};
	for (const auto& colour_case : gamma_cases) {
		const auto decode = [](float value) { return value < 0.0f ? -std::pow(-value, 2.4f) : std::pow(value, 2.4f); };

		mt::Rec2020Gamma24<float> colour(colour_case[0], colour_case[1], colour_case[2], colour_case[3]);
		mt::Rec2020Linear<float> actual = colour.to_linear();
		test.check_close(actual.red, decode(colour_case[0]), 5.0e-6f, "Rec2020Gamma24<float> to_linear r");
		test.check_close(actual.green, decode(colour_case[1]), 5.0e-6f, "Rec2020Gamma24<float> to_linear g");
		test.check_close(actual.blue, decode(colour_case[2]), 5.0e-6f, "Rec2020Gamma24<float> to_linear b");
		test.check_close(actual.alpha, colour_case[3], 5.0e-6f, "Rec2020Gamma24<float> to_linear a");

		mt::Rec2020Gamma24<float> encoded = actual.to_rec2020_gamma24();
		test.check_close(encoded.red, colour_case[0], 5.0e-6f, "Rec2020Linear<float> to_rec2020_gamma24 r");
		test.check_close(encoded.green, colour_case[1], 5.0e-6f, "Rec2020Linear<float> to_rec2020_gamma24 g");
		test.check_close(encoded.blue, colour_case[2], 5.0e-6f, "Rec2020Linear<float> to_rec2020_gamma24 b");
		test.check_close(encoded.alpha, colour_case[3], 5.0e-6f, "Rec2020Linear<float> to_rec2020_gamma24 a");
	}

	// Reference data checks gamma-2.4 encoded Rec.2020 to HSV.
	for (const auto& colour_case : colour_reference::rec2020_oetf_hsv_cases) {
		mt::Rec2020Gamma24<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Rec2020Gamma24Hsv<float> actual = colour.to_hsv();
		std::string message = std::string("Rec2020Gamma24<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " v");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// Reference data checks gamma-2.4 encoded Rec.2020 to HSL.
	for (const auto& colour_case : colour_reference::rec2020_oetf_hsl_cases) {
		mt::Rec2020Gamma24<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Rec2020Gamma24Hsl<float> actual = colour.to_hsl();
		std::string message = std::string("Rec2020Gamma24<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " h");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " s");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " l");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " a");
	}

	// XYZ conversion uses the shared Rec.2020 linear matrix after gamma decoding.
	mt::Rec2020Gamma24<float> xyz_colour(0.5f, 0.25f, 0.75f, 0.4f);
	mt::Xyz<float> expected_xyz = xyz_colour.to_linear().to_xyz();
	mt::Xyz<float> actual_xyz = xyz_colour.to_xyz();
	test.check_close(actual_xyz.x, expected_xyz.x, 5.0e-6f, "Rec2020Gamma24<float> to_xyz x");
	test.check_close(actual_xyz.y, expected_xyz.y, 5.0e-6f, "Rec2020Gamma24<float> to_xyz y");
	test.check_close(actual_xyz.z, expected_xyz.z, 5.0e-6f, "Rec2020Gamma24<float> to_xyz z");
	test.check_close(actual_xyz.alpha, expected_xyz.alpha, 5.0e-6f, "Rec2020Gamma24<float> to_xyz a");

	mt::Rec2020Gamma24<float> xyz_roundtrip = expected_xyz.to_rec2020_gamma24();
	test.check_close(xyz_roundtrip.red, xyz_colour.red, 5.0e-5f, "Xyz<float> to_rec2020_gamma24 r");
	test.check_close(xyz_roundtrip.green, xyz_colour.green, 5.0e-5f, "Xyz<float> to_rec2020_gamma24 g");
	test.check_close(xyz_roundtrip.blue, xyz_colour.blue, 5.0e-5f, "Xyz<float> to_rec2020_gamma24 b");
	test.check_close(xyz_roundtrip.alpha, xyz_colour.alpha, 5.0e-6f, "Xyz<float> to_rec2020_gamma24 a");
}

int main() {
	TestContext test;

	test_rec2020_gamma24_float(test);

	return test.result();
}
