/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
GenericRgb<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <string>
#include <type_traits>

#include "simd-f32.h"
#include "colour.h"

void test_generic_rgb_simd256_float(TestContext& test) {
	using F = mt::Simd256Float32;

	static_assert(mt::RgbaColour<mt::GenericRgb<F>>);
	static_assert(std::is_convertible_v<mt::Srgb<F>, mt::GenericRgb<F>>);
	static_assert(!std::is_convertible_v<mt::GenericRgb<F>, mt::Srgb<F>>);

	F red(0.0f);
	F green(0.0f);
	F blue(0.0f);
	F alpha(0.0f);
	for (int lane = 0; lane < F::number_of_elements(); ++lane) {
		red.set_element(lane, 0.1f * float(lane + 1));
		green.set_element(lane, 0.2f * float(lane + 1));
		blue.set_element(lane, 0.3f * float(lane + 1));
		alpha.set_element(lane, 0.4f + 0.05f * float(lane));
	}

	mt::Srgb<F> source(red, green, blue, alpha);
	mt::GenericRgb<F> implicit = source;
	mt::GenericRgb<F> member = source.to_generic_rgb();
	mt::Srgb<F> interpreted(implicit);

	for (int lane = 0; lane < F::number_of_elements(); ++lane) {
		test.check_equal(implicit.red.element(lane), red.element(lane), "GenericRgb<Simd256Float32> implicit red lane " + std::to_string(lane));
		test.check_equal(implicit.green.element(lane), green.element(lane), "GenericRgb<Simd256Float32> implicit green lane " + std::to_string(lane));
		test.check_equal(implicit.blue.element(lane), blue.element(lane), "GenericRgb<Simd256Float32> implicit blue lane " + std::to_string(lane));
		test.check_equal(implicit.alpha.element(lane), alpha.element(lane), "GenericRgb<Simd256Float32> implicit alpha lane " + std::to_string(lane));

		test.check_equal(member.red.element(lane), red.element(lane), "GenericRgb<Simd256Float32> to_generic_rgb red lane " + std::to_string(lane));
		test.check_equal(member.green.element(lane), green.element(lane), "GenericRgb<Simd256Float32> to_generic_rgb green lane " + std::to_string(lane));
		test.check_equal(member.blue.element(lane), blue.element(lane), "GenericRgb<Simd256Float32> to_generic_rgb blue lane " + std::to_string(lane));
		test.check_equal(member.alpha.element(lane), alpha.element(lane), "GenericRgb<Simd256Float32> to_generic_rgb alpha lane " + std::to_string(lane));

		test.check_equal(interpreted.red.element(lane), red.element(lane), "GenericRgb<Simd256Float32> explicit Srgb red lane " + std::to_string(lane));
		test.check_equal(interpreted.green.element(lane), green.element(lane), "GenericRgb<Simd256Float32> explicit Srgb green lane " + std::to_string(lane));
		test.check_equal(interpreted.blue.element(lane), blue.element(lane), "GenericRgb<Simd256Float32> explicit Srgb blue lane " + std::to_string(lane));
		test.check_equal(interpreted.alpha.element(lane), alpha.element(lane), "GenericRgb<Simd256Float32> explicit Srgb alpha lane " + std::to_string(lane));
	}

	mt::GenericRgb<F> clipped = mt::GenericRgb<F>(F(-0.25f), F(0.5f), F(1.25f), F(2.0f)).clip();
	mt::GenericRgb<F> clipped_rgb = mt::GenericRgb<F>(F(-0.25f), F(0.5f), F(1.25f), F(2.0f)).clip_rgb();
	mt::GenericRgb<F> fitted = mt::GenericRgb<F>(F(-0.25f), F(0.5f), F(1.25f), F(2.0f)).fit_gamut();

	for (int lane = 0; lane < F::number_of_elements(); ++lane) {
		test.check_equal(clipped.red.element(lane), 0.0f, "GenericRgb<Simd256Float32> clip red lane " + std::to_string(lane));
		test.check_equal(clipped.green.element(lane), 0.5f, "GenericRgb<Simd256Float32> clip green lane " + std::to_string(lane));
		test.check_equal(clipped.blue.element(lane), 1.0f, "GenericRgb<Simd256Float32> clip blue lane " + std::to_string(lane));
		test.check_equal(clipped.alpha.element(lane), 1.0f, "GenericRgb<Simd256Float32> clip alpha lane " + std::to_string(lane));

		test.check_equal(clipped_rgb.red.element(lane), 0.0f, "GenericRgb<Simd256Float32> clip_rgb red lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.green.element(lane), 0.5f, "GenericRgb<Simd256Float32> clip_rgb green lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.blue.element(lane), 1.0f, "GenericRgb<Simd256Float32> clip_rgb blue lane " + std::to_string(lane));
		test.check_equal(clipped_rgb.alpha.element(lane), 2.0f, "GenericRgb<Simd256Float32> clip_rgb preserves alpha lane " + std::to_string(lane));

		test.check_equal(fitted.red.element(lane), 0.0f, "GenericRgb<Simd256Float32> fit_gamut red lane " + std::to_string(lane));
		test.check_equal(fitted.green.element(lane), 0.5f, "GenericRgb<Simd256Float32> fit_gamut green lane " + std::to_string(lane));
		test.check_equal(fitted.blue.element(lane), 1.0f, "GenericRgb<Simd256Float32> fit_gamut blue lane " + std::to_string(lane));
		test.check_equal(fitted.alpha.element(lane), 2.0f, "GenericRgb<Simd256Float32> fit_gamut preserves alpha lane " + std::to_string(lane));
	}
}

int main() {
	TestContext test;

	test_generic_rgb_simd256_float(test);

	return test.result();
}
