/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Rec709Gamma24<Simd256Float32> tests.
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

mt::Rec709Gamma24<mt::Simd256Float32> rec709_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 r(0.0f);
	mt::Simd256Float32 g(0.0f);
	mt::Simd256Float32 b(0.0f);
	mt::Simd256Float32 a(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		r.set_element(lane, static_cast<float>(cases[case_index].source[0]));
		g.set_element(lane, static_cast<float>(cases[case_index].source[1]));
		b.set_element(lane, static_cast<float>(cases[case_index].source[2]));
		a.set_element(lane, static_cast<float>(cases[case_index].source[3]));
	}

	return mt::Rec709Gamma24<mt::Simd256Float32>(r, g, b, a);
}

} // namespace

void test_rec709_gamma24_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Rec709Gamma24<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Rec709Gamma24<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Rec709Gamma24<mt::Simd256Float32> rgba(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(rgba.red.element(lane), 0.1f, "Rec709Gamma24<Simd256Float32> rgba r lane " + std::to_string(lane));
		test.check_equal(rgba.green.element(lane), 0.2f, "Rec709Gamma24<Simd256Float32> rgba g lane " + std::to_string(lane));
		test.check_equal(rgba.blue.element(lane), 0.3f, "Rec709Gamma24<Simd256Float32> rgba b lane " + std::to_string(lane));
		test.check_equal(rgba.alpha.element(lane), 0.4f, "Rec709Gamma24<Simd256Float32> rgba a lane " + std::to_string(lane));
	}

	// Premultiply alpha multiplies only the colour channels.
	mt::Rec709Gamma24<mt::Simd256Float32> premultiplied = mt::Rec709Gamma24<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(0.75f), mt::Simd256Float32(0.5f)).premultiply_alpha();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(premultiplied.red.element(lane), 0.125f, "Rec709Gamma24<Simd256Float32> premultiply r lane " + std::to_string(lane));
		test.check_equal(premultiplied.green.element(lane), 0.25f, "Rec709Gamma24<Simd256Float32> premultiply g lane " + std::to_string(lane));
		test.check_equal(premultiplied.blue.element(lane), 0.375f, "Rec709Gamma24<Simd256Float32> premultiply b lane " + std::to_string(lane));
		test.check_equal(premultiplied.alpha.element(lane), 0.5f, "Rec709Gamma24<Simd256Float32> premultiply a lane " + std::to_string(lane));
	}

	// Un-premultiply alpha divides only the colour channels.
	mt::Rec709Gamma24<mt::Simd256Float32> unpremultiplied = premultiplied.un_premultiply_alpha();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(unpremultiplied.red.element(lane), 0.25f, "Rec709Gamma24<Simd256Float32> unpremultiply r lane " + std::to_string(lane));
		test.check_equal(unpremultiplied.green.element(lane), 0.5f, "Rec709Gamma24<Simd256Float32> unpremultiply g lane " + std::to_string(lane));
		test.check_equal(unpremultiplied.blue.element(lane), 0.75f, "Rec709Gamma24<Simd256Float32> unpremultiply b lane " + std::to_string(lane));
		test.check_equal(unpremultiplied.alpha.element(lane), 0.5f, "Rec709Gamma24<Simd256Float32> unpremultiply a lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::Rec709Gamma24<mt::Simd256Float32> clamped = mt::Rec709Gamma24<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.red.element(lane), 0.0f, "Rec709Gamma24<Simd256Float32> clamp r lane " + std::to_string(lane));
		test.check_equal(clamped.green.element(lane), 0.5f, "Rec709Gamma24<Simd256Float32> clamp g lane " + std::to_string(lane));
		test.check_equal(clamped.blue.element(lane), 1.0f, "Rec709Gamma24<Simd256Float32> clamp b lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Rec709Gamma24<Simd256Float32> clamp a lane " + std::to_string(lane));
	}

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::Rec709Gamma24<mt::Simd256Float32> no_negatives = mt::Rec709Gamma24<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.red.element(lane), 0.0f, "Rec709Gamma24<Simd256Float32> no_negatives r lane " + std::to_string(lane));
		test.check_equal(no_negatives.green.element(lane), 0.5f, "Rec709Gamma24<Simd256Float32> no_negatives g lane " + std::to_string(lane));
		test.check_equal(no_negatives.blue.element(lane), 1.25f, "Rec709Gamma24<Simd256Float32> no_negatives b lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Rec709Gamma24<Simd256Float32> no_negatives a lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Rec709Gamma24<mt::Simd256Float32> sanitised = mt::Rec709Gamma24<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.red.element(lane), 0.0f, "Rec709Gamma24<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.green.element(lane), 0.0f, "Rec709Gamma24<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.blue.element(lane), std::numeric_limits<float>::max(), "Rec709Gamma24<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Rec709Gamma24<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Gamma 2.4 transfer checks use the reference source values with expected values computed directly.
	const int linear_case_count = int(sizeof(colour_reference::rec709_oetf_rec709_linear_cases) / sizeof(colour_reference::rec709_oetf_rec709_linear_cases[0]));
	for (int first_case = 0; first_case < linear_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Rec709Linear<mt::Simd256Float32> actual = rec709_batch(colour_reference::rec709_oetf_rec709_linear_cases, first_case, linear_case_count).to_linear();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < linear_case_count; ++lane) {
			const auto& colour_case = colour_reference::rec709_oetf_rec709_linear_cases[first_case + lane];
			const auto decode = [](float value) { return value < 0.0f ? -std::pow(-value, 2.4f) : std::pow(value, 2.4f); };
			std::string message = std::string("Rec709Gamma24<Simd256Float32> to_linear ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.red.element(lane), decode(static_cast<float>(colour_case.source[0])), 2.0e-5f, message + " r");
			test.check_close(actual.green.element(lane), decode(static_cast<float>(colour_case.source[1])), 2.0e-5f, message + " g");
			test.check_close(actual.blue.element(lane), decode(static_cast<float>(colour_case.source[2])), 2.0e-5f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " a");
		}
	}

	// Reference data checks gamma-2.4 encoded Rec.709 to HSV.
	const int hsv_case_count = int(sizeof(colour_reference::rec709_oetf_hsv_cases) / sizeof(colour_reference::rec709_oetf_hsv_cases[0]));
	for (int first_case = 0; first_case < hsv_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Rec709Gamma24Hsv<mt::Simd256Float32> actual = rec709_batch(colour_reference::rec709_oetf_hsv_cases, first_case, hsv_case_count).to_hsv();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < hsv_case_count; ++lane) {
			const auto& colour_case = colour_reference::rec709_oetf_hsv_cases[first_case + lane];
			std::string message = std::string("Rec709Gamma24<Simd256Float32> to_hsv ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close_hue(actual.hue.element(lane), colour_case.expected[0], 3.0e-5f, message + " h");
			test.check_close(actual.saturation.element(lane), colour_case.expected[1], 3.0e-5f, message + " s");
			test.check_close(actual.value.element(lane), colour_case.expected[2], 3.0e-5f, message + " v");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " a");
		}
	}

	// Reference data checks gamma-2.4 encoded Rec.709 to HSL.
	const int hsl_case_count = int(sizeof(colour_reference::rec709_oetf_hsl_cases) / sizeof(colour_reference::rec709_oetf_hsl_cases[0]));
	for (int first_case = 0; first_case < hsl_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Rec709Gamma24Hsl<mt::Simd256Float32> actual = rec709_batch(colour_reference::rec709_oetf_hsl_cases, first_case, hsl_case_count).to_hsl();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < hsl_case_count; ++lane) {
			const auto& colour_case = colour_reference::rec709_oetf_hsl_cases[first_case + lane];
			std::string message = std::string("Rec709Gamma24<Simd256Float32> to_hsl ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close_hue(actual.hue.element(lane), colour_case.expected[0], 3.0e-5f, message + " h");
			test.check_close(actual.saturation.element(lane), colour_case.expected[1], 3.0e-5f, message + " s");
			test.check_close(actual.lightness.element(lane), colour_case.expected[2], 3.0e-5f, message + " l");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " a");
		}
	}

	// XYZ conversion uses the shared Rec.709 linear matrix after gamma decoding.
	const int xyz_case_count = int(sizeof(colour_reference::rec709_oetf_xyz_d65_cases) / sizeof(colour_reference::rec709_oetf_xyz_d65_cases[0]));
	for (int first_case = 0; first_case < xyz_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Rec709Gamma24<mt::Simd256Float32> colour = rec709_batch(colour_reference::rec709_oetf_xyz_d65_cases, first_case, xyz_case_count);
		mt::Xyz<mt::Simd256Float32> actual = colour.to_xyz();
		mt::Xyz<mt::Simd256Float32> expected = colour.to_linear().to_xyz();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_case_count; ++lane) {
			const auto& colour_case = colour_reference::rec709_oetf_xyz_d65_cases[first_case + lane];
			std::string message = std::string("Rec709Gamma24<Simd256Float32> to_xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), expected.x.element(lane), 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), expected.y.element(lane), 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), expected.z.element(lane), 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), expected.alpha.element(lane), 2.0e-5f, message + " a");
		}
	}
}

int main() {
	TestContext test;

	test_rec709_gamma24_simd256_float(test);

	return test.result();
}
