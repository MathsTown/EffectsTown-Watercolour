/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 Cielab<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/cielab_reference.h"

namespace {

mt::XyzD50<mt::Simd256Float32> xyz_d50_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 x(0.0f);
	mt::Simd256Float32 y(0.0f);
	mt::Simd256Float32 z(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		x.set_element(lane, static_cast<float>(cases[case_index].source[0]));
		y.set_element(lane, static_cast<float>(cases[case_index].source[1]));
		z.set_element(lane, static_cast<float>(cases[case_index].source[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].source[3]));
	}

	return mt::XyzD50<mt::Simd256Float32>(x, y, z, alpha);
}

mt::Cielab<mt::Simd256Float32> cielab_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 l(0.0f);
	mt::Simd256Float32 a(0.0f);
	mt::Simd256Float32 b(0.0f);
	mt::Simd256Float32 alpha(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		l.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		a.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		b.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		alpha.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::Cielab<mt::Simd256Float32>(l, a, b, alpha);
}

} // namespace

void test_cielab_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Cielab<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Cielab<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::Cielab<mt::Simd256Float32> laba(mt::Simd256Float32(50.0f), mt::Simd256Float32(-20.0f), mt::Simd256Float32(30.0f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(laba.lightness.element(lane), 50.0f, "Cielab<Simd256Float32> laba l lane " + std::to_string(lane));
		test.check_equal(laba.a.element(lane), -20.0f, "Cielab<Simd256Float32> laba a lane " + std::to_string(lane));
		test.check_equal(laba.b.element(lane), 30.0f, "Cielab<Simd256Float32> laba b lane " + std::to_string(lane));
		test.check_equal(laba.alpha.element(lane), 0.4f, "Cielab<Simd256Float32> laba alpha lane " + std::to_string(lane));
	}

	// Clamp restricts bounded components to their valid ranges.
	mt::Cielab<mt::Simd256Float32> clamped = mt::Cielab<mt::Simd256Float32>(mt::Simd256Float32(125.0f), mt::Simd256Float32(-50.0f), mt::Simd256Float32(125.0f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.lightness.element(lane), 100.0f, "Cielab<Simd256Float32> clamp l lane " + std::to_string(lane));
		test.check_equal(clamped.a.element(lane), -50.0f, "Cielab<Simd256Float32> clamp a lane " + std::to_string(lane));
		test.check_equal(clamped.b.element(lane), 125.0f, "Cielab<Simd256Float32> clamp b lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "Cielab<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative bounded components to zero without clamping over-whites.
	mt::Cielab<mt::Simd256Float32> no_negatives = mt::Cielab<mt::Simd256Float32>(mt::Simd256Float32(-25.0f), mt::Simd256Float32(-50.0f), mt::Simd256Float32(125.0f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.lightness.element(lane), 0.0f, "Cielab<Simd256Float32> no_negatives l lane " + std::to_string(lane));
		test.check_equal(no_negatives.a.element(lane), -50.0f, "Cielab<Simd256Float32> no_negatives a lane " + std::to_string(lane));
		test.check_equal(no_negatives.b.element(lane), 125.0f, "Cielab<Simd256Float32> no_negatives b lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "Cielab<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::Cielab<mt::Simd256Float32> sanitised = mt::Cielab<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.lightness.element(lane), 0.0f, "Cielab<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.a.element(lane), 0.0f, "Cielab<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.b.element(lane), std::numeric_limits<float>::max(), "Cielab<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "Cielab<Simd256Float32> sanitise negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks CIE XYZ D50 to CIELAB D50.
	const int lab_case_count = int(sizeof(colour_reference::xyz_d50_cielab_d50_cases) / sizeof(colour_reference::xyz_d50_cielab_d50_cases[0]));
	for (int first_case = 0; first_case < lab_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Cielab<mt::Simd256Float32> actual = xyz_d50_batch(colour_reference::xyz_d50_cielab_d50_cases, first_case, lab_case_count).to_cielab();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < lab_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d50_cielab_d50_cases[first_case + lane];
			std::string message = std::string("Cielab<Simd256Float32> from XyzD50 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 2.0e-4f, message + " l");
			test.check_close(actual.a.element(lane), colour_case.expected[1], 2.0e-4f, message + " a");
			test.check_close(actual.b.element(lane), colour_case.expected[2], 2.0e-4f, message + " b");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-4f, message + " alpha");
		}
	}

	// Reference data checks CIELAB D50 back to CIE XYZ D50.
	for (int first_case = 0; first_case < lab_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::XyzD50<mt::Simd256Float32> actual = cielab_batch(colour_reference::xyz_d50_cielab_d50_cases, first_case, lab_case_count).to_xyz_d50();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < lab_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d50_cielab_d50_cases[first_case + lane];
			std::string message = std::string("Cielab<Simd256Float32> to XyzD50 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.source[0], 2.0e-4f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.source[1], 2.0e-4f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.source[2], 2.0e-4f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-4f, message + " alpha");
		}
	}

	// Reference data checks CIELAB D50 to CIELCH D50.
	for (int first_case = 0; first_case < lab_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Cielch<mt::Simd256Float32> actual = cielab_batch(colour_reference::xyz_d50_cielab_d50_cases, first_case, lab_case_count).to_cielch();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < lab_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d50_cielch_d50_cases[first_case + lane];
			std::string message = std::string("Cielab<Simd256Float32> to Cielch ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.lightness.element(lane), colour_case.expected[0], 2.0e-4f, message + " l");
			test.check_close(actual.chroma.element(lane), colour_case.expected[1], 2.0e-4f, message + " c");
			if (colour_case.expected[1] > 1.0e-4) {
				test.check_close_hue(actual.hue.element(lane), colour_case.expected[2], 5.0e-4f, message + " h");
			}
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-4f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_cielab_simd256_float(test);

	return test.result();
}
