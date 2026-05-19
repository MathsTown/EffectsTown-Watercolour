/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Rec709LinearHsl<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/rec709_reference.h"

namespace {

mt::Rec709LinearHsl<mt::Simd256Float32> rec709_linear_hsl_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 h(0.0f);
	mt::Simd256Float32 s(0.0f);
	mt::Simd256Float32 l(0.0f);
	mt::Simd256Float32 a(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		h.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		s.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		l.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		a.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::Rec709LinearHsl<mt::Simd256Float32>(h, s, l, a);
}

} // namespace

void test_rec709_linear_hsl_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Rec709LinearHsl<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Rec709LinearHsl<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Rec709LinearHsl<mt::Simd256Float32> hsla(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(hsla.hue.element(lane), 0.1f, "Rec709LinearHsl<Simd256Float32> HSLA h lane " + std::to_string(lane));
		test.check_equal(hsla.saturation.element(lane), 0.2f, "Rec709LinearHsl<Simd256Float32> HSLA s lane " + std::to_string(lane));
		test.check_equal(hsla.lightness.element(lane), 0.3f, "Rec709LinearHsl<Simd256Float32> HSLA l lane " + std::to_string(lane));
		test.check_equal(hsla.alpha.element(lane), 0.4f, "Rec709LinearHsl<Simd256Float32> HSLA a lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::Rec709LinearHsl<mt::Simd256Float32> clamped = mt::Rec709LinearHsl<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.hue.element(lane), 0.0f, "Rec709LinearHsl<Simd256Float32> clamp h lane " + std::to_string(lane));
		test.check_equal(clamped.saturation.element(lane), 0.5f, "Rec709LinearHsl<Simd256Float32> clamp s lane " + std::to_string(lane));
		test.check_equal(clamped.lightness.element(lane), 1.0f, "Rec709LinearHsl<Simd256Float32> clamp l lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Rec709LinearHsl<Simd256Float32> clamp a lane " + std::to_string(lane));
	}

	// wrap_hue wraps negative and over-one hue values into a 0..1 turn.
	mt::Rec709LinearHsl<mt::Simd256Float32> wrapped_negative = mt::Rec709LinearHsl<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	mt::Rec709LinearHsl<mt::Simd256Float32> wrapped_over_one = mt::Rec709LinearHsl<mt::Simd256Float32>(mt::Simd256Float32(1.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(1.0f)).wrap_hue();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(wrapped_negative.hue.element(lane), 0.75f, "Rec709LinearHsl<Simd256Float32> wrap negative hue lane " + std::to_string(lane));
		test.check_equal(wrapped_over_one.hue.element(lane), 0.25f, "Rec709LinearHsl<Simd256Float32> wrap over-one hue lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec709LinearHsl<mt::Simd256Float32> sanitised = mt::Rec709LinearHsl<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.hue.element(lane), 0.0f, "Rec709LinearHsl<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.saturation.element(lane), 0.0f, "Rec709LinearHsl<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.lightness.element(lane), std::numeric_limits<float>::max(), "Rec709LinearHsl<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Rec709LinearHsl<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks linear Rec.709 HSL to linear Rec.709.
	const int case_count = int(sizeof(colour_reference::rec709_linear_hsl_cases) / sizeof(colour_reference::rec709_linear_hsl_cases[0]));
	for (int first_case = 0; first_case < case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Rec709Linear<mt::Simd256Float32> actual = rec709_linear_hsl_batch(colour_reference::rec709_linear_hsl_cases, first_case, case_count).to_rec709_linear();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < case_count; ++lane) {
			const auto& colour_case = colour_reference::rec709_linear_hsl_cases[first_case + lane];
			std::string message = std::string("Rec709LinearHsl<Simd256Float32> to_rec709_linear ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), colour_case.source[0], 3.0e-5f, message + " r");
			test.check_close(actual.green.element(lane), colour_case.source[1], 3.0e-5f, message + " g");
			test.check_close(actual.blue.element(lane), colour_case.source[2], 3.0e-5f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " a");
		}
	}
}

int main() {
	TestContext test;

	test_rec709_linear_hsl_simd256_float(test);

	return test.result();
}
