/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
AVX2 XyzD50<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "simd-f32.h"

#include "colour/colour_conversions.h"
#include "reference/xyz_d50_reference.h"

namespace {

mt::Xyz<mt::Simd256Float32> xyz_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 x(0.0f);
	mt::Simd256Float32 y(0.0f);
	mt::Simd256Float32 z(0.0f);
	mt::Simd256Float32 a(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		x.set_element(lane, static_cast<float>(cases[case_index].source[0]));
		y.set_element(lane, static_cast<float>(cases[case_index].source[1]));
		z.set_element(lane, static_cast<float>(cases[case_index].source[2]));
		a.set_element(lane, static_cast<float>(cases[case_index].source[3]));
	}

	return mt::Xyz<mt::Simd256Float32>(x, y, z, a);
}

mt::XyzD50<mt::Simd256Float32> xyz_d50_batch(const colour_reference::ColourReferenceCase* cases, int first_case, int case_count) {
	mt::Simd256Float32 x(0.0f);
	mt::Simd256Float32 y(0.0f);
	mt::Simd256Float32 z(0.0f);
	mt::Simd256Float32 a(1.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		int case_index = first_case + lane;
		if (case_index >= case_count) {
			case_index = case_count - 1;
		}

		x.set_element(lane, static_cast<float>(cases[case_index].expected[0]));
		y.set_element(lane, static_cast<float>(cases[case_index].expected[1]));
		z.set_element(lane, static_cast<float>(cases[case_index].expected[2]));
		a.set_element(lane, static_cast<float>(cases[case_index].expected[3]));
	}

	return mt::XyzD50<mt::Simd256Float32>(x, y, z, a);
}

} // namespace

void test_xyz_d50_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "XyzD50<Simd256Float32> compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "XyzD50<Simd256Float32> CPU support");

	// Constructors store each SIMD component directly.
	mt::XyzD50<mt::Simd256Float32> xyza(mt::Simd256Float32(0.1f), mt::Simd256Float32(0.2f), mt::Simd256Float32(0.3f), mt::Simd256Float32(0.4f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(xyza.x.element(lane), 0.1f, "XyzD50<Simd256Float32> xyza x lane " + std::to_string(lane));
		test.check_equal(xyza.y.element(lane), 0.2f, "XyzD50<Simd256Float32> xyza y lane " + std::to_string(lane));
		test.check_equal(xyza.z.element(lane), 0.3f, "XyzD50<Simd256Float32> xyza z lane " + std::to_string(lane));
		test.check_equal(xyza.alpha.element(lane), 0.4f, "XyzD50<Simd256Float32> xyza alpha lane " + std::to_string(lane));
	}

	// Clamp restricts every component to the 0..1 range.
	mt::XyzD50<mt::Simd256Float32> clamped = mt::XyzD50<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(2.0f)).clamp();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(clamped.x.element(lane), 0.0f, "XyzD50<Simd256Float32> clamp x lane " + std::to_string(lane));
		test.check_equal(clamped.y.element(lane), 0.5f, "XyzD50<Simd256Float32> clamp y lane " + std::to_string(lane));
		test.check_equal(clamped.z.element(lane), 1.0f, "XyzD50<Simd256Float32> clamp z lane " + std::to_string(lane));
		test.check_equal(clamped.alpha.element(lane), 1.0f, "XyzD50<Simd256Float32> clamp alpha lane " + std::to_string(lane));
	}

	// no_negatives clamps negative components to zero without clamping over-whites.
	mt::XyzD50<mt::Simd256Float32> no_negatives = mt::XyzD50<mt::Simd256Float32>(mt::Simd256Float32(-0.25f), mt::Simd256Float32(0.5f), mt::Simd256Float32(1.25f), mt::Simd256Float32(-0.5f)).no_negatives();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(no_negatives.x.element(lane), 0.0f, "XyzD50<Simd256Float32> no_negatives x lane " + std::to_string(lane));
		test.check_equal(no_negatives.y.element(lane), 0.5f, "XyzD50<Simd256Float32> no_negatives y lane " + std::to_string(lane));
		test.check_equal(no_negatives.z.element(lane), 1.25f, "XyzD50<Simd256Float32> no_negatives z lane " + std::to_string(lane));
		test.check_equal(no_negatives.alpha.element(lane), 0.0f, "XyzD50<Simd256Float32> no_negatives alpha lane " + std::to_string(lane));
	}

	// sanitise_floats replaces invalid floating point values with finite canonical values.
	mt::XyzD50<mt::Simd256Float32> sanitised = mt::XyzD50<mt::Simd256Float32>(mt::Simd256Float32(-0.0f), mt::Simd256Float32(std::numeric_limits<float>::quiet_NaN()), mt::Simd256Float32(std::numeric_limits<float>::infinity()), mt::Simd256Float32(-std::numeric_limits<float>::infinity())).sanitise_floats();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(sanitised.x.element(lane), 0.0f, "XyzD50<Simd256Float32> sanitise negative zero lane " + std::to_string(lane));
		test.check_equal(sanitised.y.element(lane), 0.0f, "XyzD50<Simd256Float32> sanitise nan lane " + std::to_string(lane));
		test.check_equal(sanitised.z.element(lane), std::numeric_limits<float>::max(), "XyzD50<Simd256Float32> sanitise infinity lane " + std::to_string(lane));
		test.check_equal(sanitised.alpha.element(lane), std::numeric_limits<float>::lowest(), "XyzD50<Simd256Float32> sanitise alpha negative infinity lane " + std::to_string(lane));
	}

	// Reference data checks CIE XYZ D65 to CIE XYZ D50.
	const int xyz_case_count = int(sizeof(colour_reference::xyz_d65_xyz_d50_cases) / sizeof(colour_reference::xyz_d65_xyz_d50_cases[0]));
	for (int first_case = 0; first_case < xyz_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::XyzD50<mt::Simd256Float32> actual = xyz_batch(colour_reference::xyz_d65_xyz_d50_cases, first_case, xyz_case_count).to_xyz_d50();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_xyz_d50_cases[first_case + lane];
			std::string message = std::string("XyzD50<Simd256Float32> from Xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.expected[0], 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.expected[1], 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.expected[2], 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks CIE XYZ D50 back to CIE XYZ D65.
	for (int first_case = 0; first_case < xyz_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Xyz<mt::Simd256Float32> actual = xyz_d50_batch(colour_reference::xyz_d65_xyz_d50_cases, first_case, xyz_case_count).to_xyz();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_xyz_d50_cases[first_case + lane];
			std::string message = std::string("XyzD50<Simd256Float32> to Xyz ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.source[0], 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.source[1], 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.source[2], 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks CIE XYZ D65 to CIE XYZ D50 with CAT02.
	const int xyz_cat02_case_count = int(sizeof(colour_reference::xyz_d65_xyz_d50_cat02_cases) / sizeof(colour_reference::xyz_d65_xyz_d50_cat02_cases[0]));
	for (int first_case = 0; first_case < xyz_cat02_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::XyzD50<mt::Simd256Float32> actual = xyz_batch(colour_reference::xyz_d65_xyz_d50_cat02_cases, first_case, xyz_cat02_case_count).to_xyz_d50_with_cat02();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_cat02_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_xyz_d50_cat02_cases[first_case + lane];
			std::string message = std::string("XyzD50<Simd256Float32> from Xyz CAT02 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.expected[0], 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.expected[1], 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.expected[2], 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.expected[3], 2.0e-5f, message + " alpha");
		}
	}

	// Reference data checks CIE XYZ D50 back to CIE XYZ D65 with CAT02.
	for (int first_case = 0; first_case < xyz_cat02_case_count; first_case += mt::Simd256Float32::number_of_elements()) {
		mt::Xyz<mt::Simd256Float32> actual = xyz_d50_batch(colour_reference::xyz_d65_xyz_d50_cat02_cases, first_case, xyz_cat02_case_count).to_xyz_with_cat02();
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements() && first_case + lane < xyz_cat02_case_count; ++lane) {
			const auto& colour_case = colour_reference::xyz_d65_xyz_d50_cat02_cases[first_case + lane];
			std::string message = std::string("XyzD50<Simd256Float32> to Xyz CAT02 ") + colour_case.name + " lane " + std::to_string(lane);
			test.check_close(actual.x.element(lane), colour_case.source[0], 2.0e-5f, message + " x");
			test.check_close(actual.y.element(lane), colour_case.source[1], 2.0e-5f, message + " y");
			test.check_close(actual.z.element(lane), colour_case.source[2], 2.0e-5f, message + " z");
			test.check_close(actual.alpha.element(lane), colour_case.source[3], 2.0e-5f, message + " alpha");
		}
	}
}

int main() {
	TestContext test;

	test_xyz_d50_simd256_float(test);

	return test.result();
}
