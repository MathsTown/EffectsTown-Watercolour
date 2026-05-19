/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 SrgbHsl<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/srgb_reference.h"

namespace {

mt::SrgbHsl<mt::Simd256Float32> srgb_hsl_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 hue(0.0f);
	mt::Simd256Float32 saturation(0.0f);
	mt::Simd256Float32 lightness(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		hue.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		saturation.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		lightness.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::SrgbHsl<mt::Simd256Float32>(hue, saturation, lightness, alpha);
}

} // namespace

void test_srgb_hsl_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "SrgbHsl<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "SrgbHsl<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::SrgbHsl<mt::Simd256Float32> hsla(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(hsla.hue.element(lane), 0.1f, "SrgbHsl<Simd256Float32> HSLA hue lane " + std::to_string(lane));
		test.check_equal(hsla.saturation.element(lane), 0.2f, "SrgbHsl<Simd256Float32> HSLA saturation lane " + std::to_string(lane));
		test.check_equal(hsla.lightness.element(lane), 0.3f, "SrgbHsl<Simd256Float32> HSLA lightness lane " + std::to_string(lane));
		test.check_equal(hsla.alpha.element(lane), 0.4f, "SrgbHsl<Simd256Float32> HSLA alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbHsl<mt::Simd256Float32> clamped = mt::SrgbHsl<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.hue.element(lane), 0.0f, "SrgbHsl<Simd256Float32> clamp hue lane " + std::to_string(lane));
		test.check_equal(clamped.saturation.element(lane), 0.5f, "SrgbHsl<Simd256Float32> clamp saturation lane " + std::to_string(lane));
		test.check_equal(clamped.lightness.element(lane), 1.0f, "SrgbHsl<Simd256Float32> clamp lightness lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "SrgbHsl<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbHsl<mt::Simd256Float32> wrapped_negative = mt::SrgbHsl<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	mt::SrgbHsl<mt::Simd256Float32> wrapped_over_one = mt::SrgbHsl<mt::Simd256Float32>(mt::Simd256Float32(1.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(wrapped_negative.hue.element(lane), 0.75f, "SrgbHsl<Simd256Float32> wrap negative hue lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.hue.element(lane), 0.25f, "SrgbHsl<Simd256Float32> wrap over-one hue lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbHsl<mt::Simd256Float32> sanitised = mt::SrgbHsl<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.hue.element(lane), 0.0f, "SrgbHsl<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.saturation.element(lane), 0.0f, "SrgbHsl<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.lightness.element(lane), std::numeric_limits<float>::max(), "SrgbHsl<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "SrgbHsl<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks transfer-encoded sRGB HSL to transfer-encoded sRGB.
	const int case_count = int(sizeof(colour_reference::srgb_hsl_cases) / sizeof(colour_reference::srgb_hsl_cases[0]));
	for (int first_case = 0; first_case < case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Srgb<mt::Simd256Float32> actual = srgb_hsl_batch(colour_reference::srgb_hsl_cases, first_case, case_count).to_srgb();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_hsl_cases[first_case + lane];
			std::string message = std::string("SrgbHsl<Simd256Float32> to_srgb ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), colour_case.source[0], 3.0e-5f, message + " red");
			test.check_close(actual.green.element(lane), colour_case.source[1], 3.0e-5f, message + " green");
			test.check_close(actual.blue.element(lane), colour_case.source[2], 3.0e-5f, message + " blue");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_srgb_hsl_simd256_float(test);

	return test.result();
}
