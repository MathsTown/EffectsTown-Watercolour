/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Srgb<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "colour/gamut.h"
#include "reference/srgb_reference.h"

namespace {

mt::Srgb<mt::Simd256Float32> srgb_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 red(0.0f);
	mt::Simd256Float32 green(0.0f);
	mt::Simd256Float32 blue(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		red.set_element(lane, static_cast<float>(cases[case_index].source[0]));
		green.set_element(lane, static_cast<float>(cases[case_index].source[1]));
		blue.set_element(lane, static_cast<float>(cases[case_index].source[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].source[3]));
	}

	return mt::Srgb<mt::Simd256Float32>(red, green, blue, alpha);
}

} // namespace

void test_srgb_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Srgb<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Srgb<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Srgb<mt::Simd256Float32> rgba(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(rgba.red.element(lane), 0.1f, "Srgb<Simd256Float32> rgba red lane " + std::to_string(lane));
		test.check_equal(rgba.green.element(lane), 0.2f, "Srgb<Simd256Float32> rgba green lane " + std::to_string(lane));
		test.check_equal(rgba.blue.element(lane), 0.3f, "Srgb<Simd256Float32> rgba blue lane " + std::to_string(lane));
		test.check_equal(rgba.alpha.element(lane), 0.4f, "Srgb<Simd256Float32> rgba alpha lane " + std::to_string(lane));
	}

	// RGB blend and matte operations accept a scalar opacity and broadcast it across SIMD lanes.
	mt::Srgb<mt::Simd256Float32> blend_bottom(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f));
	mt::Srgb<mt::Simd256Float32> blend_top(mt::Simd256Float32(0.8f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.6f), mt::Simd256Float32(1.0f));
	mt::Srgb<mt::Simd256Float32> normal = mt::blend_normal(blend_bottom, blend_top, 0.5f);
	mt::Srgb<mt::Simd256Float32> multiply = mt::blend_multiply(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> screen = mt::blend_screen(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> overlay = mt::blend_overlay(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> darken = mt::blend_darken(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> lighten = mt::blend_lighten(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> color_dodge = mt::blend_color_dodge(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> color_burn = mt::blend_color_burn(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> hard_light = mt::blend_hard_light(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> soft_light = mt::blend_soft_light(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> difference = mt::blend_difference(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> exclusion = mt::blend_exclusion(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> hue = mt::blend_hue(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> saturation = mt::blend_saturation(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> color = mt::blend_color(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> luminosity = mt::blend_luminosity(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> linear_burn = mt::blend_linear_burn(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> linear_dodge = mt::blend_linear_dodge(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> add = mt::blend_add(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> linear_light = mt::blend_linear_light(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> vivid_light = mt::blend_vivid_light(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> pin_light = mt::blend_pin_light(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> hard_mix = mt::blend_hard_mix(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> subtract = mt::blend_subtract(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> divide = mt::blend_divide(blend_bottom, blend_top);
	mt::Srgb<mt::Simd256Float32> matte = mt::matte_stencil_alpha(mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.5f)), mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(0.8f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.6f), mt::Simd256Float32(0.25f)), 0.5f);
	mt::Srgb<mt::Simd256Float32> stencil_luma = mt::matte_stencil_luma(blend_bottom, blend_top, 0.5f);
	mt::Srgb<mt::Simd256Float32> silhouette_alpha = mt::matte_silhouette_alpha(blend_bottom, blend_top, 0.5f);
	mt::Srgb<mt::Simd256Float32> silhouette_luma = mt::matte_silhouette_luma(blend_bottom, blend_top, 0.5f);
	mt::Srgb<mt::Simd256Float32> alpha_add = mt::matte_alpha_add(blend_bottom, blend_top, 0.5f);
	mt::Simd256Float32 blend_compile_smoke = normal.red + screen.red + overlay.red + darken.red + lighten.red + color_burn.red + hard_light.red + soft_light.red + difference.red + exclusion.red + saturation.red + color.red + luminosity.red + linear_burn.red + linear_dodge.red + add.red + linear_light.red + vivid_light.red + pin_light.red + subtract.red + divide.red + stencil_luma.alpha + silhouette_alpha.alpha + silhouette_luma.alpha + alpha_add.alpha;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check(blend_compile_smoke.element(lane) > 0.0f, "Srgb<Simd256Float32> blend compile smoke lane " + std::to_string(lane));
		test.check_close(multiply.red.element(lane), 0.2f, "Srgb<Simd256Float32> blend_multiply red lane " + std::to_string(lane));
		test.check_close(multiply.green.element(lane), 0.1f, "Srgb<Simd256Float32> blend_multiply green lane " + std::to_string(lane));
		test.check_close(multiply.blue.element(lane), 0.45f, "Srgb<Simd256Float32> blend_multiply blue lane " + std::to_string(lane));
		test.check_close(color_dodge.green.element(lane), 0.625f, "Srgb<Simd256Float32> blend_color_dodge green lane " + std::to_string(lane));
		test.check_close(hue.red.element(lane), 0.765833333f, 1.0e-6f, "Srgb<Simd256Float32> blend_hue red lane " + std::to_string(lane));
		test.check_close(hard_mix.blue.element(lane), 1.0f, "Srgb<Simd256Float32> blend_hard_mix blue lane " + std::to_string(lane));
		test.check_close(matte.alpha.element(lane), 0.3125f, "Srgb<Simd256Float32> matte_stencil_alpha lane " + std::to_string(lane));
	}

	// Premultiply alpha multiplies only the colour channels.
	mt::Srgb<mt::Simd256Float32> premultiplied = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(0.5f)).premultiply_alpha();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(premultiplied.red.element(lane), 0.125f, "Srgb<Simd256Float32> premultiply red lane " + std::to_string(lane));
		test.check_equal(premultiplied.green.element(lane), 0.25f, "Srgb<Simd256Float32> premultiply green lane " + std::to_string(lane));
		test.check_equal(premultiplied.blue.element(lane), 0.375f, "Srgb<Simd256Float32> premultiply blue lane " + std::to_string(lane));
		test.check_equal(premultiplied.alpha.element(lane), 0.5f, "Srgb<Simd256Float32> premultiply alpha lane " + std::to_string(lane));
	}

	// Un-premultiply alpha divides only the colour channels.
	mt::Srgb<mt::Simd256Float32> unpremultiplied = premultiplied.un_premultiply_alpha();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(unpremultiplied.red.element(lane), 0.25f, "Srgb<Simd256Float32> unpremultiply red lane " + std::to_string(lane));
		test.check_equal(unpremultiplied.green.element(lane), 0.5f, "Srgb<Simd256Float32> unpremultiply green lane " + std::to_string(lane));
		test.check_equal(unpremultiplied.blue.element(lane), 0.75f, "Srgb<Simd256Float32> unpremultiply blue lane " + std::to_string(lane));
		test.check_equal(unpremultiplied.alpha.element(lane), 0.5f, "Srgb<Simd256Float32> unpremultiply alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::Srgb<mt::Simd256Float32> clamped = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.red.element(lane), 0.0f, "Srgb<Simd256Float32> clamp red lane " + std::to_string(lane));
		test.check_equal(clamped.green.element(lane), 0.5f, "Srgb<Simd256Float32> clamp green lane " + std::to_string(lane));
		test.check_equal(clamped.blue.element(lane), 1.0f, "Srgb<Simd256Float32> clamp blue lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Srgb<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	mt::Srgb<mt::Simd256Float32> clipped = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clip();
	mt::Srgb<mt::Simd256Float32> clipped_rgb = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clip_rgb();
	mt::Srgb<mt::Simd256Float32> fitted = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(1.35f), mt::Simd256Float32(-0.2f), mt::Simd256Float32(0.45f), mt::Simd256Float32(1.5f)).fit_gamut();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clipped.red.element(lane), 0.0f, "Srgb<Simd256Float32> clip red lane " + std::to_string(lane));
		test.check_equal(clipped.green.element(lane), 0.5f, "Srgb<Simd256Float32> clip green lane " + std::to_string(lane));
		test.check_equal(clipped.blue.element(lane), 1.0f, "Srgb<Simd256Float32> clip blue lane " + std::to_string(lane));
		test.check_equal(clipped.alpha.element(lane), 1.0f, "Srgb<Simd256Float32> clip alpha lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.red.element(lane), 0.0f, "Srgb<Simd256Float32> clip_rgb red lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.green.element(lane), 0.5f, "Srgb<Simd256Float32> clip_rgb green lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.blue.element(lane), 1.0f, "Srgb<Simd256Float32> clip_rgb blue lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.alpha.element(lane), 2.0f, "Srgb<Simd256Float32> clip_rgb preserves alpha lane " + std::to_string(lane));
		test.check(fitted.red.element(lane) >= 0.0f && fitted.red.element(lane) <= 1.0f, "Srgb<Simd256Float32> fit_gamut red in gamut lane " + std::to_string(lane));
		test.check(fitted.green.element(lane) >= 0.0f && fitted.green.element(lane) <= 1.0f, "Srgb<Simd256Float32> fit_gamut green in gamut lane " + std::to_string(lane));
		test.check(fitted.blue.element(lane) >= 0.0f && fitted.blue.element(lane) <= 1.0f, "Srgb<Simd256Float32> fit_gamut blue in gamut lane " + std::to_string(lane));
		test.check_equal(fitted.alpha.element(lane), 1.5f, "Srgb<Simd256Float32> fit_gamut preserves alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Srgb<mt::Simd256Float32> no_negatives = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.red.element(lane), 0.0f, "Srgb<Simd256Float32> no_negatives red lane " + std::to_string(lane));
		test.check_equal(no_negatives.green.element(lane), 0.5f, "Srgb<Simd256Float32> no_negatives green lane " + std::to_string(lane));
		test.check_equal(no_negatives.blue.element(lane), 1.25f, "Srgb<Simd256Float32> no_negatives blue lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Srgb<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Srgb<mt::Simd256Float32> sanitised = mt::Srgb<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.red.element(lane), 0.0f, "Srgb<Simd256Float32> sanitise red negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.green.element(lane), 0.0f, "Srgb<Simd256Float32> sanitise green nan lane " + std::to_string(lane));
		test.check_equal(sanitised.blue.element(lane), std::numeric_limits<float>::max(), "Srgb<Simd256Float32> sanitise blue infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Srgb<Simd256Float32> sanitise alpha negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks gamma-2.4 encoded sRGB to linear sRGB.
	const int linear_case_count = int(sizeof(colour_reference::srgb_srgb_linear_cases) / sizeof(colour_reference::srgb_srgb_linear_cases[0]));
	for (int first_case = 0; first_case < linear_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::SrgbLinear<mt::Simd256Float32> actual = srgb_batch(colour_reference::srgb_srgb_linear_cases, first_case, linear_case_count).to_linear();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < linear_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_srgb_linear_cases[first_case + lane];
			std::string message = std::string("Srgb<Simd256Float32> to_linear ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), colour_case.expected[0], 2.0e-5f, message + " red");
			test.check_close(actual.green.element(lane), colour_case.expected[1], 2.0e-5f, message + " green");
			test.check_close(actual.blue.element(lane), colour_case.expected[2], 2.0e-5f, message + " blue");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks gamma-2.4 encoded sRGB to HSV.
	const int hsv_case_count = int(sizeof(colour_reference::srgb_hsv_cases) / sizeof(colour_reference::srgb_hsv_cases[0]));
	for (int first_case = 0; first_case < hsv_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::SrgbHsv<mt::Simd256Float32> actual = srgb_batch(colour_reference::srgb_hsv_cases, first_case, hsv_case_count).to_hsv();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < hsv_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_hsv_cases[first_case + lane];
			std::string message = std::string("Srgb<Simd256Float32> to_hsv ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close_hue(actual.hue.element(lane), colour_case.expected[0], 3.0e-5f, message + " hue");
			test.check_close(actual.saturation.element(lane), colour_case.expected[1], 3.0e-5f, message + " saturation");
			test.check_close(actual.value.element(lane), colour_case.expected[2], 3.0e-5f, message + " value");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks gamma-2.4 encoded sRGB to HSL.
	const int hsl_case_count = int(sizeof(colour_reference::srgb_hsl_cases) / sizeof(colour_reference::srgb_hsl_cases[0]));
	for (int first_case = 0; first_case < hsl_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::SrgbHsl<mt::Simd256Float32> actual = srgb_batch(colour_reference::srgb_hsl_cases, first_case, hsl_case_count).to_hsl();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < hsl_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_hsl_cases[first_case + lane];
			std::string message = std::string("Srgb<Simd256Float32> to_hsl ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close_hue(actual.hue.element(lane), colour_case.expected[0], 3.0e-5f, message + " hue");
			test.check_close(actual.saturation.element(lane), colour_case.expected[1], 3.0e-5f, message + " saturation");
			test.check_close(actual.lightness.element(lane), colour_case.expected[2], 3.0e-5f, message + " lightness");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks gamma-2.4 encoded sRGB to CIE XYZ D65.
	const int xyz_case_count = int(sizeof(colour_reference::srgb_xyz_d65_cases) / sizeof(colour_reference::srgb_xyz_d65_cases[0]));
	for (int first_case = 0; first_case < xyz_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Xyz<mt::Simd256Float32> actual = srgb_batch(colour_reference::srgb_xyz_d65_cases, first_case, xyz_case_count).to_xyz();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_xyz_d65_cases[first_case + lane];
			std::string message = std::string("Srgb<Simd256Float32> to_xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.expected[0], 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.expected[1], 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.expected[2], 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_srgb_simd256_float(test);

	return test.result();
}
