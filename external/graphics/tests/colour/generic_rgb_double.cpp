/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
GenericRgb<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <concepts>
#include <limits>
#include <string>
#include <type_traits>

#include "colour.h"

namespace {

mt::GenericRgb<double> render_srgb_as_generic() {
	return mt::Srgb<double>(0.125, 0.25, 0.5, 0.75);
}

template <typename C> void check_rgb_type_generic_conversion(TestContext& test, const std::string& name) {
	C source(0.125, 0.25, 0.5, 0.75);

	mt::GenericRgb<double> implicit = source;
	test.check_equal(implicit.red, 0.125, name + " implicit generic red");
	test.check_equal(implicit.green, 0.25, name + " implicit generic green");
	test.check_equal(implicit.blue, 0.5, name + " implicit generic blue");
	test.check_equal(implicit.alpha, 0.75, name + " implicit generic alpha");

	mt::GenericRgb<double> member = source.to_generic_rgb();
	test.check_equal(member.red, 0.125, name + " to_generic_rgb red");
	test.check_equal(member.green, 0.25, name + " to_generic_rgb green");
	test.check_equal(member.blue, 0.5, name + " to_generic_rgb blue");
	test.check_equal(member.alpha, 0.75, name + " to_generic_rgb alpha");

	C interpreted(implicit);
	test.check_equal(interpreted.red, 0.125, name + " explicit from generic red");
	test.check_equal(interpreted.green, 0.25, name + " explicit from generic green");
	test.check_equal(interpreted.blue, 0.5, name + " explicit from generic blue");
	test.check_equal(interpreted.alpha, 0.75, name + " explicit from generic alpha");
}

} // namespace

void test_generic_rgb_double(TestContext& test) {
	static_assert(mt::RgbaColour<mt::GenericRgb<double>>);
	static_assert(std::is_trivially_copyable_v<mt::GenericRgb<double>>);
	static_assert(std::is_standard_layout_v<mt::GenericRgb<double>>);
	static_assert(sizeof(mt::GenericRgb<double>) == sizeof(double) * 4);
	static_assert(std::is_convertible_v<mt::Srgb<double>, mt::GenericRgb<double>>);
	static_assert(!std::is_convertible_v<mt::GenericRgb<double>, mt::Srgb<double>>);
	static_assert(!std::constructible_from<mt::GenericRgb<double>, mt::Srgb<float>>);
	static_assert(!std::constructible_from<mt::Srgb<double>, mt::GenericRgb<float>>);

	mt::GenericRgb<double> rendered = render_srgb_as_generic();
	test.check_equal(rendered.red, 0.125, "GenericRgb<double> renderer implicit red");
	test.check_equal(rendered.green, 0.25, "GenericRgb<double> renderer implicit green");
	test.check_equal(rendered.blue, 0.5, "GenericRgb<double> renderer implicit blue");
	test.check_equal(rendered.alpha, 0.75, "GenericRgb<double> renderer implicit alpha");

	mt::GenericRgb<double> clipped_rgb = mt::GenericRgb<double>(-0.25, 0.5, 1.25, 2.0).clip_rgb();
	test.check_equal(clipped_rgb.red, 0.0, "GenericRgb<double> clip_rgb red");
	test.check_equal(clipped_rgb.green, 0.5, "GenericRgb<double> clip_rgb green");
	test.check_equal(clipped_rgb.blue, 1.0, "GenericRgb<double> clip_rgb blue");
	test.check_equal(clipped_rgb.alpha, 2.0, "GenericRgb<double> clip_rgb preserves alpha");

	mt::GenericRgb<double> fitted = mt::GenericRgb<double>(-0.25, 0.5, 1.25, 2.0).fit_gamut();
	test.check_equal(fitted.red, 0.0, "GenericRgb<double> fit_gamut red");
	test.check_equal(fitted.green, 0.5, "GenericRgb<double> fit_gamut green");
	test.check_equal(fitted.blue, 1.0, "GenericRgb<double> fit_gamut blue");
	test.check_equal(fitted.alpha, 2.0, "GenericRgb<double> fit_gamut preserves alpha");

	mt::GenericRgb<double> sanitised = mt::GenericRgb<double>(-0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()).sanitise_floats();
	test.check_equal(sanitised.red, 0.0, "GenericRgb<double> sanitise red negative zero");
	test.check_equal(sanitised.green, 0.0, "GenericRgb<double> sanitise green nan");
	test.check_equal(sanitised.blue, std::numeric_limits<double>::max(), "GenericRgb<double> sanitise blue infinity");
	test.check_equal(sanitised.alpha, std::numeric_limits<double>::lowest(), "GenericRgb<double> sanitise alpha negative infinity");

	check_rgb_type_generic_conversion<mt::Srgb<double>>(test, "Srgb<double>");
	check_rgb_type_generic_conversion<mt::SrgbLinear<double>>(test, "SrgbLinear<double>");
	check_rgb_type_generic_conversion<mt::DisplayP3<double>>(test, "DisplayP3<double>");
	check_rgb_type_generic_conversion<mt::DisplayP3Linear<double>>(test, "DisplayP3Linear<double>");
	check_rgb_type_generic_conversion<mt::AdobeRgb<double>>(test, "AdobeRgb<double>");
	check_rgb_type_generic_conversion<mt::AdobeRgbLinear<double>>(test, "AdobeRgbLinear<double>");
	check_rgb_type_generic_conversion<mt::ProPhotoRgb<double>>(test, "ProPhotoRgb<double>");
	check_rgb_type_generic_conversion<mt::ProPhotoRgbLinear<double>>(test, "ProPhotoRgbLinear<double>");
	check_rgb_type_generic_conversion<mt::Rec709Oetf<double>>(test, "Rec709Oetf<double>");
	check_rgb_type_generic_conversion<mt::Rec709Gamma24<double>>(test, "Rec709Gamma24<double>");
	check_rgb_type_generic_conversion<mt::Rec709Linear<double>>(test, "Rec709Linear<double>");
	check_rgb_type_generic_conversion<mt::Rec2020Oetf<double>>(test, "Rec2020Oetf<double>");
	check_rgb_type_generic_conversion<mt::Rec2020Gamma24<double>>(test, "Rec2020Gamma24<double>");
	check_rgb_type_generic_conversion<mt::Rec2020Linear<double>>(test, "Rec2020Linear<double>");
	check_rgb_type_generic_conversion<mt::Aces2065_1<double>>(test, "Aces2065_1<double>");
	check_rgb_type_generic_conversion<mt::Acescg<double>>(test, "Acescg<double>");
}

int main() {
	TestContext test;

	test_generic_rgb_double(test);

	return test.result();
}
