/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Rec2020OetfHsv<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/rec2020_reference.h"

namespace {

mt::Rec2020OetfHsv<mt::Simd256Float32> rec2020_oetf_hsv_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 h(0.0f);
	mt::Simd256Float32 s(0.0f);
	mt::Simd256Float32 v(0.0f);
	mt::Simd256Float32 a(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		h.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		s.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		v.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		a.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::Rec2020OetfHsv<mt::Simd256Float32>(h, s, v, a);
}

} // namespace

void test_rec2020_oetf_hsv_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Rec2020OetfHsv<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Rec2020OetfHsv<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Rec2020OetfHsv<mt::Simd256Float32> hsla(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(hsla.hue.element(lane), 0.1f, "Rec2020OetfHsv<Simd256Float32> HSVA h lane " + std::to_string(lane));
		test.check_equal(hsla.saturation.element(lane), 0.2f, "Rec2020OetfHsv<Simd256Float32> HSVA s lane " + std::to_string(lane));
		test.check_equal(hsla.value.element(lane), 0.3f, "Rec2020OetfHsv<Simd256Float32> HSVA v lane " + std::to_string(lane));
		test.check_equal(hsla.alpha.element(lane), 0.4f, "Rec2020OetfHsv<Simd256Float32> HSVA a lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::Rec2020OetfHsv<mt::Simd256Float32> clamped = mt::Rec2020OetfHsv<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.hue.element(lane), 0.0f, "Rec2020OetfHsv<Simd256Float32> clamp h lane " + std::to_string(lane));
		test.check_equal(clamped.saturation.element(lane), 0.5f, "Rec2020OetfHsv<Simd256Float32> clamp s lane " + std::to_string(lane));
		test.check_equal(clamped.value.element(lane), 1.0f, "Rec2020OetfHsv<Simd256Float32> clamp v lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Rec2020OetfHsv<Simd256Float32> clamp a lane " + std::to_string(lane));
	}

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Rec2020OetfHsv<mt::Simd256Float32> wrapped_negative = mt::Rec2020OetfHsv<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	mt::Rec2020OetfHsv<mt::Simd256Float32> wrapped_over_one = mt::Rec2020OetfHsv<mt::Simd256Float32>(mt::Simd256Float32(1.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(wrapped_negative.hue.element(lane), 0.75f, "Rec2020OetfHsv<Simd256Float32> wrap negative hue lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.hue.element(lane), 0.25f, "Rec2020OetfHsv<Simd256Float32> wrap over-one hue lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec2020OetfHsv<mt::Simd256Float32> sanitised = mt::Rec2020OetfHsv<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.hue.element(lane), 0.0f, "Rec2020OetfHsv<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.saturation.element(lane), 0.0f, "Rec2020OetfHsv<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.value.element(lane), std::numeric_limits<float>::max(), "Rec2020OetfHsv<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Rec2020OetfHsv<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks gamma-2.4 encoded Rec.2020 HSV to gamma-2.4 encoded Rec.2020.
	const int case_count = int(sizeof(colour_reference::rec2020_oetf_hsv_cases) / sizeof(colour_reference::rec2020_oetf_hsv_cases[0]));
	for (int first_case = 0; first_case < case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Rec2020Oetf<mt::Simd256Float32> actual = rec2020_oetf_hsv_batch(colour_reference::rec2020_oetf_hsv_cases, first_case, case_count).to_rec2020_oetf();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < case_count; ++lane) {
			const auto& colour_case = colour_reference::rec2020_oetf_hsv_cases[first_case + lane];
			std::string message = std::string("Rec2020OetfHsv<Simd256Float32> to_rec2020 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), colour_case.source[0], 3.0e-5f, message + " r");
			test.check_close(actual.green.element(lane), colour_case.source[1], 3.0e-5f, message + " g");
			test.check_close(actual.blue.element(lane), colour_case.source[2], 3.0e-5f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " a");
		}
	}
}

int main() {
	TestContext test;

	test_rec2020_oetf_hsv_simd256_float(test);

	return test.result();
}
