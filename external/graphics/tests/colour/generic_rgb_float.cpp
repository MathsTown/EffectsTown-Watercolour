/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
GenericRgb<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <concepts>
#include <limits>
#include <string>
#include <type_traits>

#include "colour.h"

namespace {

mt::GenericRgb<float> render_srgb_as_generic() {
	return mt::Srgb<float>(0.125f, 0.25f, 0.5f, 0.75f);
}

template <typename C> void check_rgb_type_generic_conversion(TestContext& test, const std::string& name) {
	C source(0.125f, 0.25f, 0.5f, 0.75f);

	mt::GenericRgb<float> implicit = source;
	test.check_equal(implicit.red, 0.125f, name + " implicit generic red");
	test.check_equal(implicit.green, 0.25f, name + " implicit generic green");
	test.check_equal(implicit.blue, 0.5f, name + " implicit generic blue");
	test.check_equal(implicit.alpha, 0.75f, name + " implicit generic alpha");

	mt::GenericRgb<float> member = source.to_generic_rgb();
	test.check_equal(member.red, 0.125f, name + " to_generic_rgb red");
	test.check_equal(member.green, 0.25f, name + " to_generic_rgb green");
	test.check_equal(member.blue, 0.5f, name + " to_generic_rgb blue");
	test.check_equal(member.alpha, 0.75f, name + " to_generic_rgb alpha");

	C interpreted(implicit);
	test.check_equal(interpreted.red, 0.125f, name + " explicit from generic red");
	test.check_equal(interpreted.green, 0.25f, name + " explicit from generic green");
	test.check_equal(interpreted.blue, 0.5f, name + " explicit from generic blue");
	test.check_equal(interpreted.alpha, 0.75f, name + " explicit from generic alpha");
}

} // namespace

void test_generic_rgb_float(TestContext& test) {
	static_assert(mt::RgbaColour<mt::GenericRgb<float>>);
	static_assert(std::is_trivially_copyable_v<mt::GenericRgb<float>>);
	static_assert(std::is_standard_layout_v<mt::GenericRgb<float>>);
	static_assert(sizeof(mt::GenericRgb<float>) == sizeof(float) * 4);
	static_assert(std::is_convertible_v<mt::Srgb<float>, mt::GenericRgb<float>>);
	static_assert(!std::is_convertible_v<mt::GenericRgb<float>, mt::Srgb<float>>);
	static_assert(!std::constructible_from<mt::GenericRgb<float>, mt::Srgb<double>>);
	static_assert(!std::constructible_from<mt::Srgb<float>, mt::GenericRgb<double>>);

	mt::GenericRgb<float> default_value;
	test.check_equal(default_value.red, 0.0f, "GenericRgb<float> default red");
	test.check_equal(default_value.green, 0.0f, "GenericRgb<float> default green");
	test.check_equal(default_value.blue, 0.0f, "GenericRgb<float> default blue");
	test.check_equal(default_value.alpha, 1.0f, "GenericRgb<float> default alpha");

	mt::GenericRgb<float> grey(0.25f);
	test.check_equal(grey.red, 0.25f, "GenericRgb<float> grey red");
	test.check_equal(grey.green, 0.25f, "GenericRgb<float> grey green");
	test.check_equal(grey.blue, 0.25f, "GenericRgb<float> grey blue");
	test.check_equal(grey.alpha, 1.0f, "GenericRgb<float> grey alpha");

	mt::GenericRgb<float> rgb(0.1f, 0.2f, 0.3f);
	test.check_equal(rgb.red, 0.1f, "GenericRgb<float> rgb red");
	test.check_equal(rgb.green, 0.2f, "GenericRgb<float> rgb green");
	test.check_equal(rgb.blue, 0.3f, "GenericRgb<float> rgb blue");
	test.check_equal(rgb.alpha, 1.0f, "GenericRgb<float> rgb alpha");

	mt::GenericRgb<float> rgba(0.1f, 0.2f, 0.3f, 0.4f);
	test.check_equal(rgba.red, 0.1f, "GenericRgb<float> rgba red");
	test.check_equal(rgba.green, 0.2f, "GenericRgb<float> rgba green");
	test.check_equal(rgba.blue, 0.3f, "GenericRgb<float> rgba blue");
	test.check_equal(rgba.alpha, 0.4f, "GenericRgb<float> rgba alpha");

	mt::GenericRgb<float> rendered = render_srgb_as_generic();
	test.check_equal(rendered.red, 0.125f, "GenericRgb<float> renderer implicit red");
	test.check_equal(rendered.green, 0.25f, "GenericRgb<float> renderer implicit green");
	test.check_equal(rendered.blue, 0.5f, "GenericRgb<float> renderer implicit blue");
	test.check_equal(rendered.alpha, 0.75f, "GenericRgb<float> renderer implicit alpha");

	mt::GenericRgb<float> premultiplied = mt::GenericRgb<float>(0.25f, 0.5f, 0.75f, 0.5f).premultiply_alpha();
	test.check_equal(premultiplied.red, 0.125f, "GenericRgb<float> premultiply red");
	test.check_equal(premultiplied.green, 0.25f, "GenericRgb<float> premultiply green");
	test.check_equal(premultiplied.blue, 0.375f, "GenericRgb<float> premultiply blue");
	test.check_equal(premultiplied.alpha, 0.5f, "GenericRgb<float> premultiply alpha");

	mt::GenericRgb<float> unpremultiplied = premultiplied.un_premultiply_alpha();
	test.check_equal(unpremultiplied.red, 0.25f, "GenericRgb<float> unpremultiply red");
	test.check_equal(unpremultiplied.green, 0.5f, "GenericRgb<float> unpremultiply green");
	test.check_equal(unpremultiplied.blue, 0.75f, "GenericRgb<float> unpremultiply blue");
	test.check_equal(unpremultiplied.alpha, 0.5f, "GenericRgb<float> unpremultiply alpha");

	mt::GenericRgb<float> zero_alpha = mt::GenericRgb<float>(0.25f, 0.5f, 0.75f, 0.0f).un_premultiply_alpha();
	test.check_equal(zero_alpha.red, 0.25f, "GenericRgb<float> zero alpha unpremultiply red");
	test.check_equal(zero_alpha.green, 0.5f, "GenericRgb<float> zero alpha unpremultiply green");
	test.check_equal(zero_alpha.blue, 0.75f, "GenericRgb<float> zero alpha unpremultiply blue");
	test.check_equal(zero_alpha.alpha, 0.0f, "GenericRgb<float> zero alpha unpremultiply alpha");

	mt::GenericRgb<float> clipped = mt::GenericRgb<float>(-0.25f, 0.5f, 1.25f, 2.0f).clip();
	test.check_equal(clipped.red, 0.0f, "GenericRgb<float> clip red");
	test.check_equal(clipped.green, 0.5f, "GenericRgb<float> clip green");
	test.check_equal(clipped.blue, 1.0f, "GenericRgb<float> clip blue");
	test.check_equal(clipped.alpha, 1.0f, "GenericRgb<float> clip alpha");

	mt::GenericRgb<float> clipped_rgb = mt::GenericRgb<float>(-0.25f, 0.5f, 1.25f, 2.0f).clip_rgb();
	test.check_equal(clipped_rgb.red, 0.0f, "GenericRgb<float> clip_rgb red");
	test.check_equal(clipped_rgb.green, 0.5f, "GenericRgb<float> clip_rgb green");
	test.check_equal(clipped_rgb.blue, 1.0f, "GenericRgb<float> clip_rgb blue");
	test.check_equal(clipped_rgb.alpha, 2.0f, "GenericRgb<float> clip_rgb preserves alpha");

	mt::GenericRgb<float> fitted = mt::GenericRgb<float>(-0.25f, 0.5f, 1.25f, 2.0f).fit_gamut();
	test.check_equal(fitted.red, 0.0f, "GenericRgb<float> fit_gamut red");
	test.check_equal(fitted.green, 0.5f, "GenericRgb<float> fit_gamut green");
	test.check_equal(fitted.blue, 1.0f, "GenericRgb<float> fit_gamut blue");
	test.check_equal(fitted.alpha, 2.0f, "GenericRgb<float> fit_gamut preserves alpha");

	mt::GenericRgb<float> no_negatives = mt::GenericRgb<float>(-0.25f, 0.5f, 1.25f, -0.5f).no_negatives();
	test.check_equal(no_negatives.red, 0.0f, "GenericRgb<float> no_negatives red");
	test.check_equal(no_negatives.green, 0.5f, "GenericRgb<float> no_negatives green");
	test.check_equal(no_negatives.blue, 1.25f, "GenericRgb<float> no_negatives blue");
	test.check_equal(no_negatives.alpha, 0.0f, "GenericRgb<float> no_negatives alpha");

	mt::GenericRgb<float> sanitised = mt::GenericRgb<float>(-0.0f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0f, "GenericRgb<float> sanitise red negative zero");
	test.check_equal(sanitised.green, 0.0f, "GenericRgb<float> sanitise green nan");
	test.check_equal(sanitised.blue, std::numeric_limits<float>::max(), "GenericRgb<float> sanitise blue infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<float>::lowest(), "GenericRgb<float> sanitise alpha negative infinity");

	check_rgb_type_generic_conversion<mt::Srgb<float>>(test, "Srgb<float>");
	check_rgb_type_generic_conversion<mt::SrgbLinear<float>>(test, "SrgbLinear<float>");
	check_rgb_type_generic_conversion<mt::DisplayP3<float>>(test, "DisplayP3<float>");
	check_rgb_type_generic_conversion<mt::DisplayP3Linear<float>>(test, "DisplayP3Linear<float>");
	check_rgb_type_generic_conversion<mt::AdobeRgb<float>>(test, "AdobeRgb<float>");
	check_rgb_type_generic_conversion<mt::AdobeRgbLinear<float>>(test, "AdobeRgbLinear<float>");
	check_rgb_type_generic_conversion<mt::ProPhotoRgb<float>>(test, "ProPhotoRgb<float>");
	check_rgb_type_generic_conversion<mt::ProPhotoRgbLinear<float>>(test, "ProPhotoRgbLinear<float>");
	check_rgb_type_generic_conversion<mt::Rec709Oetf<float>>(test, "Rec709Oetf<float>");
	check_rgb_type_generic_conversion<mt::Rec709Gamma24<float>>(test, "Rec709Gamma24<float>");
	check_rgb_type_generic_conversion<mt::Rec709Linear<float>>(test, "Rec709Linear<float>");
	check_rgb_type_generic_conversion<mt::Rec2020Oetf<float>>(test, "Rec2020Oetf<float>");
	check_rgb_type_generic_conversion<mt::Rec2020Gamma24<float>>(test, "Rec2020Gamma24<float>");
	check_rgb_type_generic_conversion<mt::Rec2020Linear<float>>(test, "Rec2020Linear<float>");
	check_rgb_type_generic_conversion<mt::Aces2065_1<float>>(test, "Aces2065_1<float>");
	check_rgb_type_generic_conversion<mt::Acescg<float>>(test, "Acescg<float>");
}

int main() {
	TestContext test;

	test_generic_rgb_float(test);

	return test.result();
}
