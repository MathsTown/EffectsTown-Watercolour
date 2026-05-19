/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
RGB fit_gamut() smoke tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "colour/colour_conversions.h"
#include "colour/gamut.h"

namespace {

struct RgbCase {
	double red;
	double green;
	double blue;
	const char* name;
};

template <typename C>
bool rgb_in_unit_gamut(const C& colour, double tolerance = 1.0e-10) {
	return std::isfinite(colour.red) && std::isfinite(colour.green) && std::isfinite(colour.blue) &&
	       colour.red >= -tolerance && colour.red <= 1.0 + tolerance &&
	       colour.green >= -tolerance && colour.green <= 1.0 + tolerance &&
	       colour.blue >= -tolerance && colour.blue <= 1.0 + tolerance;
}

template <typename C>
bool rgb_close(const C& a, const C& b, double tolerance = 1.0e-8) {
	return std::fabs(a.red - b.red) <= tolerance &&
	       std::fabs(a.green - b.green) <= tolerance &&
	       std::fabs(a.blue - b.blue) <= tolerance;
}

double hue_distance(double a, double b) {
	a = a - std::floor(a);
	b = b - std::floor(b);
	const double direct = std::fabs(a - b);
	return std::min(direct, 1.0 - direct);
}

template <typename C>
void check_fit_gamut_type(TestContext& test, const std::string& type_name) {
	constexpr RgbCase cases[] = {
		{ 1.35, -0.20, 0.45, "red high green negative" },
		{ -0.25, 1.20, 0.30, "red negative green high" },
		{ 0.20, 0.55, 1.15, "blue high" },
	};

	const C in_gamut(0.20, 0.40, 0.60, 1.25);
	const C in_gamut_fitted = in_gamut.fit_gamut();
	test.check_close(in_gamut_fitted.red, in_gamut.red, 1.0e-12, type_name + " in-gamut red unchanged");
	test.check_close(in_gamut_fitted.green, in_gamut.green, 1.0e-12, type_name + " in-gamut green unchanged");
	test.check_close(in_gamut_fitted.blue, in_gamut.blue, 1.0e-12, type_name + " in-gamut blue unchanged");
	test.check_equal(in_gamut_fitted.alpha, in_gamut.alpha, type_name + " in-gamut alpha preserved");

	for (const RgbCase& fit_case : cases) {
		const C colour(fit_case.red, fit_case.green, fit_case.blue, 0.73);
		const C fitted = colour.fit_gamut();
		const C clipped = colour.clip();
		const auto source_oklch = colour.to_xyz().to_oklch();
		const auto fitted_oklch = fitted.to_xyz().to_oklch();
		const std::string case_name = type_name + " " + fit_case.name;

		test.check(rgb_in_unit_gamut(fitted), case_name + " fitted RGB in gamut");
		test.check_equal(fitted.alpha, colour.alpha, case_name + " alpha preserved");
		test.check(!rgb_close(fitted, clipped), case_name + " is not a direct clip");
		test.check(std::isfinite(source_oklch.lightness) && std::isfinite(source_oklch.chroma) && std::isfinite(source_oklch.hue), case_name + " source OkLCh finite");
		test.check_close(fitted_oklch.lightness, source_oklch.lightness, 5.0e-5, case_name + " OkLCh lightness preserved");
		test.check(hue_distance(fitted_oklch.hue, source_oklch.hue) <= 5.0e-4, case_name + " OkLCh hue preserved");
		test.check(fitted_oklch.chroma < source_oklch.chroma, case_name + " OkLCh chroma reduced");
	}
}

} // namespace

void test_gamut_float(TestContext& test) {
	check_fit_gamut_type<mt::Srgb<double>>(test, "Srgb<double>");
	check_fit_gamut_type<mt::SrgbLinear<double>>(test, "SrgbLinear<double>");
	check_fit_gamut_type<mt::DisplayP3<double>>(test, "DisplayP3<double>");
	check_fit_gamut_type<mt::DisplayP3Linear<double>>(test, "DisplayP3Linear<double>");
	check_fit_gamut_type<mt::AdobeRgb<double>>(test, "AdobeRgb<double>");
	check_fit_gamut_type<mt::AdobeRgbLinear<double>>(test, "AdobeRgbLinear<double>");
	check_fit_gamut_type<mt::ProPhotoRgb<double>>(test, "ProPhotoRgb<double>");
	check_fit_gamut_type<mt::ProPhotoRgbLinear<double>>(test, "ProPhotoRgbLinear<double>");
	check_fit_gamut_type<mt::Rec709<double>>(test, "Rec709<double>");
	check_fit_gamut_type<mt::Rec709Linear<double>>(test, "Rec709Linear<double>");
	check_fit_gamut_type<mt::Rec2020<double>>(test, "Rec2020<double>");
	check_fit_gamut_type<mt::Rec2020Linear<double>>(test, "Rec2020Linear<double>");
	check_fit_gamut_type<mt::Aces2065_1<double>>(test, "Aces2065_1<double>");
	check_fit_gamut_type<mt::Acescg<double>>(test, "Acescg<double>");
}

int main() {
	TestContext test;

	test_gamut_float(test);

	return test.result();
}
