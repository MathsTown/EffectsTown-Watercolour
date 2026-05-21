/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 SrgbLinear<Simd256Float32> tests.
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

mt::SrgbLinear<mt::Simd256Float32> srgb_linear_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count, bool use_expected) {
	mt::Simd256Float32 red(0.0f);
	mt::Simd256Float32 green(0.0f);
	mt::Simd256Float32 blue(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		const double* values = use_expected ? cases[case_index].expected : cases[case_index].source;
		red.set_element(lane, static_cast<float>(values[0]));
		green.set_element(lane, static_cast<float>(values[1]));
		blue.set_element(lane, static_cast<float>(values[2]));
		alpha.set_element(lane, static_cast<float>(values[3]));
	}

	return mt::SrgbLinear<mt::Simd256Float32>(red, green, blue, alpha);
}

} // namespace

void test_srgb_linear_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "SrgbLinear<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "SrgbLinear<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::SrgbLinear<mt::Simd256Float32> rgba(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(rgba.red.element(lane), 0.1f, "SrgbLinear<Simd256Float32> rgba red lane " + std::to_string(lane));
		test.check_equal(rgba.green.element(lane), 0.2f, "SrgbLinear<Simd256Float32> rgba green lane " + std::to_string(lane));
		test.check_equal(rgba.blue.element(lane), 0.3f, "SrgbLinear<Simd256Float32> rgba blue lane " + std::to_string(lane));
		test.check_equal(rgba.alpha.element(lane), 0.4f, "SrgbLinear<Simd256Float32> rgba alpha lane " + std::to_string(lane));
	}

	// mix accepts a scalar amount and broadcasts it across SIMD lanes.
	mt::SrgbLinear<mt::Simd256Float32> mixed = mt::mix(rgba, mt::SrgbLinear<mt::Simd256Float32>(mt::Simd256Float32(0.5f), mt::Simd256Float32(0.6f), mt::Simd256Float32(0.7f), mt::Simd256Float32(0.8f)), 0.5f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(mixed.red.element(lane), 0.3f, "SrgbLinear<Simd256Float32> mix red lane " + std::to_string(lane));
		test.check_close(mixed.green.element(lane), 0.4f, "SrgbLinear<Simd256Float32> mix green lane " + std::to_string(lane));
		test.check_close(mixed.blue.element(lane), 0.5f, "SrgbLinear<Simd256Float32> mix blue lane " + std::to_string(lane));
		test.check_close(mixed.alpha.element(lane), 0.6f, "SrgbLinear<Simd256Float32> mix alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::SrgbLinear<mt::Simd256Float32> clamped = mt::SrgbLinear<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.red.element(lane), 0.0f, "SrgbLinear<Simd256Float32> clamp red lane " + std::to_string(lane));
		test.check_equal(clamped.green.element(lane), 0.5f, "SrgbLinear<Simd256Float32> clamp green lane " + std::to_string(lane));
		test.check_equal(clamped.blue.element(lane), 1.0f, "SrgbLinear<Simd256Float32> clamp blue lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "SrgbLinear<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::SrgbLinear<mt::Simd256Float32> sanitised = mt::SrgbLinear<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.red.element(lane), 0.0f, "SrgbLinear<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.green.element(lane), 0.0f, "SrgbLinear<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.blue.element(lane), std::numeric_limits<float>::max(), "SrgbLinear<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "SrgbLinear<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks linear sRGB to gamma-2.4 encoded sRGB.
	const int transfer_case_count = int(sizeof(colour_reference::srgb_srgb_linear_cases) / sizeof(colour_reference::srgb_srgb_linear_cases[0]));
	for (int first_case = 0; first_case < transfer_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Srgb<mt::Simd256Float32> actual = srgb_linear_batch(colour_reference::srgb_srgb_linear_cases, first_case, transfer_case_count, true).to_srgb();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < transfer_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_srgb_linear_cases[first_case + lane];
			std::string message = std::string("SrgbLinear<Simd256Float32> to_srgb ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), colour_case.source[0], 2.0e-5f, message + " red");
			test.check_close(actual.green.element(lane), colour_case.source[1], 2.0e-5f, message + " green");
			test.check_close(actual.blue.element(lane), colour_case.source[2], 2.0e-5f, message + " blue");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks linear sRGB to HSV.
	const int hsv_case_count = int(sizeof(colour_reference::srgb_linear_hsv_cases) / sizeof(colour_reference::srgb_linear_hsv_cases[0]));
	for (int first_case = 0; first_case < hsv_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::SrgbLinearHsv<mt::Simd256Float32> actual = srgb_linear_batch(colour_reference::srgb_linear_hsv_cases, first_case, hsv_case_count, false).to_hsv();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < hsv_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_linear_hsv_cases[first_case + lane];
			std::string message = std::string("SrgbLinear<Simd256Float32> to_hsv ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close_hue(actual.hue.element(lane), colour_case.expected[0], 3.0e-5f, message + " h");
			test.check_close(actual.saturation.element(lane), colour_case.expected[1], 3.0e-5f, message + " s");
			test.check_close(actual.value.element(lane), colour_case.expected[2], 3.0e-5f, message + " v");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks linear sRGB to HSL.
	const int hsl_case_count = int(sizeof(colour_reference::srgb_linear_hsl_cases) / sizeof(colour_reference::srgb_linear_hsl_cases[0]));
	for (int first_case = 0; first_case < hsl_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::SrgbLinearHsl<mt::Simd256Float32> actual = srgb_linear_batch(colour_reference::srgb_linear_hsl_cases, first_case, hsl_case_count, false).to_hsl();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < hsl_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_linear_hsl_cases[first_case + lane];
			std::string message = std::string("SrgbLinear<Simd256Float32> to_hsl ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close_hue(actual.hue.element(lane), colour_case.expected[0], 3.0e-5f, message + " h");
			test.check_close(actual.saturation.element(lane), colour_case.expected[1], 3.0e-5f, message + " s");
			test.check_close(actual.lightness.element(lane), colour_case.expected[2], 3.0e-5f, message + " l");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks linear sRGB to CIE XYZ D65.
	const int xyz_case_count = int(sizeof(colour_reference::srgb_linear_xyz_d65_cases) / sizeof(colour_reference::srgb_linear_xyz_d65_cases[0]));
	for (int first_case = 0; first_case < xyz_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Xyz<mt::Simd256Float32> actual = srgb_linear_batch(colour_reference::srgb_linear_xyz_d65_cases, first_case, xyz_case_count, false).to_xyz();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_case_count; ++lane) {
			const auto& colour_case = colour_reference::srgb_linear_xyz_d65_cases[first_case + lane];
			std::string message = std::string("SrgbLinear<Simd256Float32> to_xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.expected[0], 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.expected[1], 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.expected[2], 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_srgb_linear_simd256_float(test);

	return test.result();
}
