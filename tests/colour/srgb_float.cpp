/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Scalar Srgb<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "colour/colour_conversions.h"
#include "colour/gamut.h"
#include "reference/srgb_reference.h"

namespace {

template <typename C>
void check_rgb_gamut_api(TestContext& test, const std::string& name) {
	C clipped = C(-0.25f, 0.5f, 1.25f, 2.0f).clip();
	test.check_equal(clipped.red, 0.0f, name + " clip red");
	test.check_equal(clipped.green, 0.5f, name + " clip green");
	test.check_equal(clipped.blue, 1.0f, name + " clip blue");
	test.check_equal(clipped.alpha, 1.0f, name + " clip alpha");

	C clipped_rgb = C(-0.25f, 0.5f, 1.25f, 2.0f).clip_rgb();
	test.check_equal(clipped_rgb.red, 0.0f, name + " clip_rgb red");
	test.check_equal(clipped_rgb.green, 0.5f, name + " clip_rgb green");
	test.check_equal(clipped_rgb.blue, 1.0f, name + " clip_rgb blue");
	test.check_equal(clipped_rgb.alpha, 2.0f, name + " clip_rgb preserves high alpha");

	C clipped_rgb_negative_alpha = C(0.25f, 0.5f, 0.75f, -0.5f).clip_rgb();
	test.check_equal(clipped_rgb_negative_alpha.alpha, -0.5f, name + " clip_rgb preserves negative alpha");

	C fitted = C(1.35f, -0.2f, 0.45f, 1.5f).fit_gamut();
	test.check(fitted.red >= 0.0f && fitted.red <= 1.0f, name + " fit_gamut red in gamut");
	test.check(fitted.green >= 0.0f && fitted.green <= 1.0f, name + " fit_gamut green in gamut");
	test.check(fitted.blue >= 0.0f && fitted.blue <= 1.0f, name + " fit_gamut blue in gamut");
	test.check_equal(fitted.alpha, 1.5f, name + " fit_gamut preserves alpha");
}

} // namespace

void test_srgb_float(TestContext& test) {
	// Default constructor creates black with full alpha.
	mt::Srgb<float> default_value;
	test.check_equal(default_value.red, 0.0f, "Srgb<float> default red");
	test.check_equal(default_value.green, 0.0f, "Srgb<float> default green");
	test.check_equal(default_value.blue, 0.0f, "Srgb<float> default blue");
	test.check_equal(default_value.alpha, 1.0f, "Srgb<float> default alpha");

	// Grey constructor copies one value into every colour channel.
	mt::Srgb<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "Srgb<float> grey red");
	test.check_equal(grey.green, 0.25f, "Srgb<float> grey green");
	test.check_equal(grey.blue, 0.25f, "Srgb<float> grey blue");
	test.check_equal(grey.alpha, 1.0f, "Srgb<float> grey alpha");

	// Three-component constructor fills alpha with one.
	mt::Srgb<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "Srgb<float> rgb red");
	test.check_equal(rgb.green, 0.2f, "Srgb<float> rgb green");
	test.check_equal(rgb.blue, 0.3f, "Srgb<float> rgb blue");
	test.check_equal(rgb.alpha, 1.0f, "Srgb<float> rgb alpha");

	// Four-component constructor stores every component directly.
	mt::Srgb<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "Srgb<float> rgba red");
	test.check_equal(rgba.green, 0.2f, "Srgb<float> rgba green");
	test.check_equal(rgba.blue, 0.3f, "Srgb<float> rgba blue");
	test.check_equal(rgba.alpha, 0.4f, "Srgb<float> rgba alpha");

	// Premultiply alpha multiplies only the colour channels.
	mt::Srgb<float> premultiplied = mt::Srgb<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "Srgb<float> premultiply red");
	test.check_equal(premultiplied.green, 0.25f, "Srgb<float> premultiply green");
	test.check_equal(premultiplied.blue, 0.375f, "Srgb<float> premultiply blue");
	test.check_equal(premultiplied.alpha, 0.5f, "Srgb<float> premultiply alpha");

	// Un-premultiply alpha divides only the colour channels.
	mt::Srgb<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "Srgb<float> unpremultiply red");
	test.check_equal(unpremultiplied.green, 0.5f, "Srgb<float> unpremultiply green");
	test.check_equal(unpremultiplied.blue, 0.75f, "Srgb<float> unpremultiply blue");
	test.check_equal(unpremultiplied.alpha, 0.5f, "Srgb<float> unpremultiply alpha");

	// Un-premultiply with zero alpha leaves colour channels unchanged.
	mt::Srgb<float> zero_alpha = mt::Srgb<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "Srgb<float> zero alpha unpremultiply red");
	test.check_equal(zero_alpha.green, 0.5f, "Srgb<float> zero alpha unpremultiply green");
	test.check_equal(zero_alpha.blue, 0.75f, "Srgb<float> zero alpha unpremultiply blue");
	test.check_equal(zero_alpha.alpha, 0.0f, "Srgb<float> zero alpha unpremultiply alpha");

	// Clamp restricts every component to the 0..1 range.
	mt::Srgb<float> clamped = mt::Srgb<float>(-0.25f, 0.5f, 1.25f, 2.0f).clamp();
	test.check_equal(clamped.red, 0.0f, "Srgb<float> clamp red");
	test.check_equal(clamped.green, 0.5f, "Srgb<float> clamp green");
	test.check_equal(clamped.blue, 1.0f, "Srgb<float> clamp blue");
	test.check_equal(clamped.alpha, 1.0f, "Srgb<float> clamp alpha");

	check_rgb_gamut_api<mt::Srgb<float>>(test, "Srgb<float>");
	check_rgb_gamut_api<mt::SrgbLinear<float>>(test, "SrgbLinear<float>");
	check_rgb_gamut_api<mt::DisplayP3<float>>(test, "DisplayP3<float>");
	check_rgb_gamut_api<mt::DisplayP3Linear<float>>(test, "DisplayP3Linear<float>");
	check_rgb_gamut_api<mt::AdobeRgb<float>>(test, "AdobeRgb<float>");
	check_rgb_gamut_api<mt::AdobeRgbLinear<float>>(test, "AdobeRgbLinear<float>");
	check_rgb_gamut_api<mt::ProPhotoRgb<float>>(test, "ProPhotoRgb<float>");
	check_rgb_gamut_api<mt::ProPhotoRgbLinear<float>>(test, "ProPhotoRgbLinear<float>");
	check_rgb_gamut_api<mt::Rec709<float>>(test, "Rec709<float>");
	check_rgb_gamut_api<mt::Rec709Linear<float>>(test, "Rec709Linear<float>");
	check_rgb_gamut_api<mt::Rec2020<float>>(test, "Rec2020<float>");
	check_rgb_gamut_api<mt::Rec2020Linear<float>>(test, "Rec2020Linear<float>");
	check_rgb_gamut_api<mt::Aces2065_1<float>>(test, "Aces2065_1<float>");
	check_rgb_gamut_api<mt::Acescg<float>>(test, "Acescg<float>");

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Srgb<float> no_negatives = mt::Srgb<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "Srgb<float> no_negatives red");
	test.check_equal(no_negatives.green, 0.5f, "Srgb<float> no_negatives green");
	test.check_equal(no_negatives.blue, 1.25f, "Srgb<float> no_negatives blue");
	test.check_equal(no_negatives.alpha, 0.0f, "Srgb<float> no_negatives alpha");

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Srgb<float> sanitised = mt::Srgb<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "Srgb<float> sanitise red negative zero");
	test.check_equal(sanitised.green, 0.0f, "Srgb<float> sanitise green nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "Srgb<float> sanitise blue infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "Srgb<float> sanitise alpha negative infinity");

	// blend_normal performs straight-alpha source-over compositing with layer opacity.
	mt::Srgb<float> normal = mt::blend_normal(mt::Srgb<float>(0.2f, 0.4f, 0.6f, 0.5f), mt::Srgb<float>(1.0f, 0.0f, 0.0f, 0.5f), 0.5f);
	test.check_close(normal.red, 0.52f, "Srgb<float> blend_normal red");
	test.check_close(normal.green, 0.24f, "Srgb<float> blend_normal green");
	test.check_close(normal.blue, 0.36f, "Srgb<float> blend_normal blue");
	test.check_close(normal.alpha, 0.625f, "Srgb<float> blend_normal alpha");

	// RGB blend modes use bottom/top notation and include alpha compositing.
	mt::Srgb<float> bottom(0.25f, 0.5f, 0.75f, 1.0f);
	mt::Srgb<float> top(0.8f, 0.2f, 0.6f, 1.0f);

	mt::Srgb<float> multiply = mt::blend_multiply(bottom, top);
	test.check_close(multiply.red, 0.2f, "Srgb<float> blend_multiply red");
	test.check_close(multiply.green, 0.1f, "Srgb<float> blend_multiply green");
	test.check_close(multiply.blue, 0.45f, "Srgb<float> blend_multiply blue");
	test.check_close(multiply.alpha, 1.0f, "Srgb<float> blend_multiply alpha");

	mt::Srgb<float> screen = mt::blend_screen(bottom, top);
	test.check_close(screen.red, 0.85f, "Srgb<float> blend_screen red");
	test.check_close(screen.green, 0.6f, "Srgb<float> blend_screen green");
	test.check_close(screen.blue, 0.9f, "Srgb<float> blend_screen blue");

	mt::Srgb<float> overlay = mt::blend_overlay(bottom, top);
	test.check_close(overlay.red, 0.4f, "Srgb<float> blend_overlay red");
	test.check_close(overlay.green, 0.2f, "Srgb<float> blend_overlay green");
	test.check_close(overlay.blue, 0.8f, "Srgb<float> blend_overlay blue");

	mt::Srgb<float> darken = mt::blend_darken(bottom, top);
	test.check_close(darken.red, 0.25f, "Srgb<float> blend_darken red");
	test.check_close(darken.green, 0.2f, "Srgb<float> blend_darken green");
	test.check_close(darken.blue, 0.6f, "Srgb<float> blend_darken blue");

	mt::Srgb<float> lighten = mt::blend_lighten(bottom, top);
	test.check_close(lighten.red, 0.8f, "Srgb<float> blend_lighten red");
	test.check_close(lighten.green, 0.5f, "Srgb<float> blend_lighten green");
	test.check_close(lighten.blue, 0.75f, "Srgb<float> blend_lighten blue");

	mt::Srgb<float> color_dodge = mt::blend_color_dodge(bottom, top);
	test.check_close(color_dodge.red, 1.0f, "Srgb<float> blend_color_dodge red");
	test.check_close(color_dodge.green, 0.625f, "Srgb<float> blend_color_dodge green");
	test.check_close(color_dodge.blue, 1.0f, "Srgb<float> blend_color_dodge blue");

	mt::Srgb<float> color_burn = mt::blend_color_burn(bottom, top);
	test.check_close(color_burn.red, 0.0625f, "Srgb<float> blend_color_burn red");
	test.check_close(color_burn.green, 0.0f, "Srgb<float> blend_color_burn green");
	test.check_close(color_burn.blue, 0.583333333f, 1.0e-6f, "Srgb<float> blend_color_burn blue");

	mt::Srgb<float> hard_light = mt::blend_hard_light(bottom, top);
	test.check_close(hard_light.red, 0.7f, "Srgb<float> blend_hard_light red");
	test.check_close(hard_light.green, 0.2f, "Srgb<float> blend_hard_light green");
	test.check_close(hard_light.blue, 0.8f, "Srgb<float> blend_hard_light blue");

	mt::Srgb<float> soft_light = mt::blend_soft_light(bottom, top);
	test.check_close(soft_light.red, 0.4f, "Srgb<float> blend_soft_light red");
	test.check_close(soft_light.green, 0.35f, "Srgb<float> blend_soft_light green");
	test.check_close(soft_light.blue, 0.7732051f, 1.0e-5f, "Srgb<float> blend_soft_light blue");

	mt::Srgb<float> difference = mt::blend_difference(bottom, top);
	test.check_close(difference.red, 0.55f, "Srgb<float> blend_difference red");
	test.check_close(difference.green, 0.3f, "Srgb<float> blend_difference green");
	test.check_close(difference.blue, 0.15f, "Srgb<float> blend_difference blue");

	mt::Srgb<float> exclusion = mt::blend_exclusion(bottom, top);
	test.check_close(exclusion.red, 0.65f, "Srgb<float> blend_exclusion red");
	test.check_close(exclusion.green, 0.5f, "Srgb<float> blend_exclusion green");
	test.check_close(exclusion.blue, 0.45f, "Srgb<float> blend_exclusion blue");

	mt::Srgb<float> hue = mt::blend_hue(bottom, top);
	test.check_close(hue.red, 0.765833333f, 1.0e-6f, "Srgb<float> blend_hue red");
	test.check_close(hue.green, 0.265833333f, 1.0e-6f, "Srgb<float> blend_hue green");
	test.check_close(hue.blue, 0.599166667f, 1.0e-6f, "Srgb<float> blend_hue blue");

	mt::Srgb<float> saturation = mt::blend_saturation(bottom, top);
	test.check_close(saturation.red, 0.2095f, "Srgb<float> blend_saturation red");
	test.check_close(saturation.green, 0.5095f, "Srgb<float> blend_saturation green");
	test.check_close(saturation.blue, 0.8095f, "Srgb<float> blend_saturation blue");

	mt::Srgb<float> color = mt::blend_color(bottom, top);
	test.check_close(color.red, 0.8285f, "Srgb<float> blend_color red");
	test.check_close(color.green, 0.2285f, "Srgb<float> blend_color green");
	test.check_close(color.blue, 0.6285f, "Srgb<float> blend_color blue");

	mt::Srgb<float> luminosity = mt::blend_luminosity(bottom, top);
	test.check_close(luminosity.red, 0.2215f, "Srgb<float> blend_luminosity red");
	test.check_close(luminosity.green, 0.4715f, "Srgb<float> blend_luminosity green");
	test.check_close(luminosity.blue, 0.7215f, "Srgb<float> blend_luminosity blue");

	mt::Srgb<float> linear_burn = mt::blend_linear_burn(bottom, top);
	test.check_close(linear_burn.red, 0.05f, "Srgb<float> blend_linear_burn red");
	test.check_close(linear_burn.green, 0.0f, "Srgb<float> blend_linear_burn green");
	test.check_close(linear_burn.blue, 0.35f, "Srgb<float> blend_linear_burn blue");

	mt::Srgb<float> linear_dodge = mt::blend_linear_dodge(bottom, top);
	test.check_close(linear_dodge.red, 1.0f, "Srgb<float> blend_linear_dodge red");
	test.check_close(linear_dodge.green, 0.7f, "Srgb<float> blend_linear_dodge green");
	test.check_close(linear_dodge.blue, 1.0f, "Srgb<float> blend_linear_dodge blue");

	mt::Srgb<float> add = mt::blend_add(bottom, top);
	test.check_close(add.red, linear_dodge.red, "Srgb<float> blend_add alias red");
	test.check_close(add.green, linear_dodge.green, "Srgb<float> blend_add alias green");
	test.check_close(add.blue, linear_dodge.blue, "Srgb<float> blend_add alias blue");

	mt::Srgb<float> linear_light = mt::blend_linear_light(bottom, top);
	test.check_close(linear_light.red, 0.85f, "Srgb<float> blend_linear_light red");
	test.check_close(linear_light.green, 0.0f, "Srgb<float> blend_linear_light green");
	test.check_close(linear_light.blue, 0.95f, "Srgb<float> blend_linear_light blue");

	mt::Srgb<float> vivid_light = mt::blend_vivid_light(bottom, top);
	test.check_close(vivid_light.red, 0.625f, "Srgb<float> blend_vivid_light red");
	test.check_close(vivid_light.green, 0.0f, "Srgb<float> blend_vivid_light green");
	test.check_close(vivid_light.blue, 0.9375f, "Srgb<float> blend_vivid_light blue");

	mt::Srgb<float> pin_light = mt::blend_pin_light(bottom, top);
	test.check_close(pin_light.red, 0.6f, "Srgb<float> blend_pin_light red");
	test.check_close(pin_light.green, 0.4f, "Srgb<float> blend_pin_light green");
	test.check_close(pin_light.blue, 0.75f, "Srgb<float> blend_pin_light blue");

	mt::Srgb<float> hard_mix = mt::blend_hard_mix(bottom, top);
	test.check_close(hard_mix.red, 1.0f, "Srgb<float> blend_hard_mix red");
	test.check_close(hard_mix.green, 0.0f, "Srgb<float> blend_hard_mix green");
	test.check_close(hard_mix.blue, 1.0f, "Srgb<float> blend_hard_mix blue");

	mt::Srgb<float> subtract = mt::blend_subtract(bottom, top);
	test.check_close(subtract.red, 0.0f, "Srgb<float> blend_subtract red");
	test.check_close(subtract.green, 0.3f, "Srgb<float> blend_subtract green");
	test.check_close(subtract.blue, 0.15f, "Srgb<float> blend_subtract blue");

	mt::Srgb<float> divide = mt::blend_divide(bottom, top);
	test.check_close(divide.red, 0.3125f, "Srgb<float> blend_divide red");
	test.check_close(divide.green, 1.0f, "Srgb<float> blend_divide green");
	test.check_close(divide.blue, 1.0f, "Srgb<float> blend_divide blue");

	mt::Srgb<float> divide_by_zero = mt::blend_divide(bottom, mt::Srgb<float>(0.0f, 0.2f, 0.0f, 1.0f));
	test.check_close(divide_by_zero.red, 1.0f, "Srgb<float> blend_divide zero red");
	test.check_close(divide_by_zero.blue, 1.0f, "Srgb<float> blend_divide zero blue");

	// Matte operations preserve bottom RGB and adjust alpha.
	mt::Srgb<float> matte_bottom(0.1f, 0.2f, 0.3f, 0.5f);
	mt::Srgb<float> matte_top(0.8f, 0.2f, 0.6f, 0.25f);
	mt::Srgb<float> opaque_luma_top(0.8f, 0.2f, 0.6f, 1.0f);

	mt::Srgb<float> stencil_alpha = mt::matte_stencil_alpha(matte_bottom, matte_top, 0.5f);
	test.check_close(stencil_alpha.red, 0.1f, "Srgb<float> matte_stencil_alpha red");
	test.check_close(stencil_alpha.alpha, 0.3125f, "Srgb<float> matte_stencil_alpha alpha");

	mt::Srgb<float> silhouette_alpha = mt::matte_silhouette_alpha(matte_bottom, matte_top, 0.5f);
	test.check_close(silhouette_alpha.alpha, 0.4375f, "Srgb<float> matte_silhouette_alpha alpha");

	mt::Srgb<float> stencil_luma = mt::matte_stencil_luma(matte_bottom, opaque_luma_top, 0.5f);
	test.check_close(stencil_luma.alpha, 0.356f, "Srgb<float> matte_stencil_luma alpha");

	mt::Srgb<float> silhouette_luma = mt::matte_silhouette_luma(matte_bottom, opaque_luma_top, 0.5f);
	test.check_close(silhouette_luma.alpha, 0.394f, "Srgb<float> matte_silhouette_luma alpha");

	mt::Srgb<float> alpha_add = mt::matte_alpha_add(matte_bottom, mt::Srgb<float>(0.8f, 0.2f, 0.6f, 0.75f), 0.5f);
	test.check_close(alpha_add.alpha, 0.875f, "Srgb<float> matte_alpha_add alpha");

	// Reference data checks transfer-encoded sRGB to linear sRGB.
	for (const auto& colour_case : colour_reference::srgb_srgb_linear_cases) {
		mt::Srgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::SrgbLinear<float> actual = colour.to_linear();
		std::string message = std::string("Srgb<float> to_linear ") + colour_case.name;
		test.check_close(actual.red, colour_case.expected[0], 5.0e-6f, message + " red");
		test.check_close(actual.green, colour_case.expected[1], 5.0e-6f, message + " green");
		test.check_close(actual.blue, colour_case.expected[2], 5.0e-6f, message + " blue");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}

	// Reference data checks transfer-encoded sRGB to HSV.
	for (const auto& colour_case : colour_reference::srgb_hsv_cases) {
		mt::Srgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::SrgbHsv<float> actual = colour.to_hsv();
		std::string message = std::string("Srgb<float> to_hsv ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " hue");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " saturation");
		test.check_close(actual.value, colour_case.expected[2], 3.0e-5f, message + " value");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}

	// Reference data checks transfer-encoded sRGB to HSL.
	for (const auto& colour_case : colour_reference::srgb_hsl_cases) {
		mt::Srgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::SrgbHsl<float> actual = colour.to_hsl();
		std::string message = std::string("Srgb<float> to_hsl ") + colour_case.name;
		test.check_close_hue(actual.hue, colour_case.expected[0], 3.0e-5f, message + " hue");
		test.check_close(actual.saturation, colour_case.expected[1], 3.0e-5f, message + " saturation");
		test.check_close(actual.lightness, colour_case.expected[2], 3.0e-5f, message + " lightness");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}

	// Reference data checks transfer-encoded sRGB to CIE XYZ D65.
	for (const auto& colour_case : colour_reference::srgb_xyz_d65_cases) {
		mt::Srgb<float> colour(static_cast<float>(colour_case.source[0]), static_cast<float>(colour_case.source[1]), static_cast<float>(colour_case.source[2]), static_cast<float>(colour_case.source[3]));
		mt::Xyz<float> actual = colour.to_xyz();
		std::string message = std::string("Srgb<float> to_xyz ") + colour_case.name;
		test.check_close(actual.x, colour_case.expected[0], 8.0e-6f, message + " x");
		test.check_close(actual.y, colour_case.expected[1], 8.0e-6f, message + " y");
		test.check_close(actual.z, colour_case.expected[2], 8.0e-6f, message + " z");
		test.check_close(actual.alpha, colour_case.expected[3], 5.0e-6f, message + " alpha");
	}
}

int main() {
	TestContext test;

	test_srgb_float(test);

	return test.result();
}
