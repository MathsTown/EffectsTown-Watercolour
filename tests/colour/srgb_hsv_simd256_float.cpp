/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 SrgbHsv<Simd256Float32> tests.
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

mt::SrgbHsv<mt::Simd256Float32> srgb_hsv_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 hue(0.0f);
	mt::Simd256Float32 saturation(0.0f);
	mt::Simd256Float32 value(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		hue.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		saturation.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		value.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::SrgbHsv<mt::Simd256Float32>(hue, saturation, value, alpha);
}

} // namespace

void test_srgb_hsv_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "SrgbHsv<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "SrgbHsv<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::SrgbHsv<mt::Simd256Float32> hsla(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(hsla.hue.element(lane), 0.1f, "SrgbHsv<Simd256Float32> HSVA hue lane " + std::to_string(lane));
		test.check_equal(hsla.saturation.element(lane), 0.2f, "SrgbHsv<Simd256Float32> HSVA saturation lane " + std::to_string(lane));
		test.check_equal(hsla.value.element(lane), 0.3f, "SrgbHsv<Simd256Float32> HSVA value lane " + std::to_string(lane));
		test.check_equal(hsla.alpha.element(lane), 0.4f, "SrgbHsv<Simd256Float32> HSVA alpha lane " + std::to_string(lane));
	}

	// mix_longest_path accepts a scalar amount and broadcasts it across SIMD lanes.
	mt::SrgbHsv<mt::Simd256Float32> longest = mt::mix_longest_path(mt::SrgbHsv<mt::Simd256Float32>(mt::Simd256Float32(0.05f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.4f), mt::Simd256Float32(0.25f)), mt::SrgbHsv<mt::Simd256Float32>(mt::Simd256Float32(0.95f), mt::Simd256Float32(0.6f), mt::Simd256Float32(0.8f), mt::Simd256Float32(0.75f)), 0.5f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close_hue(longest.hue.element(lane), 0.5f, 1.0e-6f, "SrgbHsv<Simd256Float32> mix_longest_path hue lane " + std::to_string(lane));
		test.check_close(longest.saturation.element(lane), 0.4f, "SrgbHsv<Simd256Float32> mix_longest_path saturation lane " + std::to_string(lane));
		test.check_close(longest.value.element(lane), 0.6f, "SrgbHsv<Simd256Float32> mix_longest_path value lane " + std::to_string(lane));
		test.check_close(longest.alpha.element(lane), 0.5f, "SrgbHsv<Simd256Float32> mix_longest_path alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbHsv<mt::Simd256Float32> clamped = mt::SrgbHsv<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.hue.element(lane), 0.0f, "SrgbHsv<Simd256Float32> clamp hue lane " + std::to_string(lane));
		test.check_equal(clamped.saturation.element(lane), 0.5f, "SrgbHsv<Simd256Float32> clamp saturation lane " + std::to_string(lane));
		test.check_equal(clamped.value.element(lane), 1.0f, "SrgbHsv<Simd256Float32> clamp value lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "SrgbHsv<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::SrgbHsv<mt::Simd256Float32> wrapped_negative = mt::SrgbHsv<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	mt::SrgbHsv<mt::Simd256Float32> wrapped_over_one = mt::SrgbHsv<mt::Simd256Float32>(mt::Simd256Float32(1.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(wrapped_negative.hue.element(lane), 0.75f, "SrgbHsv<Simd256Float32> wrap negative hue lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.hue.element(lane), 0.25f, "SrgbHsv<Simd256Float32> wrap over-one hue lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbHsv<mt::Simd256Float32> sanitised = mt::SrgbHsv<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.hue.element(lane), 0.0f, "SrgbHsv<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.saturation.element(lane), 0.0f, "SrgbHsv<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.value.element(lane), std::numeric_limits<float>::max(), "SrgbHsv<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "SrgbHsv<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks gamma-2.4 encoded sRGB HSV to gamma-2.4 encoded sRGB.
	const int case_count = int(sizeof(colour_reference::srgb_hsv_cases) / sizeof(colour_reference::srgb_hsv_cases[0]));
	for (int first_case = 0; first_case < case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Srgb<mt::Simd256Float32> actual = srgb_hsv_batch(colour_reference::srgb_hsv_cases, first_case, case_count).to_srgb();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_hsv_cases[first_case + lane];
			std::string message = std::string("SrgbHsv<Simd256Float32> to_srgb ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), colour_case.source[0], 3.0e-5f, message + " r");
			test.check_close(actual.green.element(lane), colour_case.source[1], 3.0e-5f, message + " g");
			test.check_close(actual.blue.element(lane), colour_case.source[2], 3.0e-5f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_srgb_hsv_simd256_float(test);

	return test.result();
}
