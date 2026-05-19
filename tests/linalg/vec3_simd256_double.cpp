/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 vec3<Simd256Float64> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

#include "simd-uint32.h"
#include "simd-f64.h"

#include "linalg/vec3.h"

namespace {

double random_double(std::mt19937& rng, double min_value, double max_value) {
	std::uniform_real_distribution<double> distribution(min_value, max_value);
	return distribution(rng);
}

double safe_divisor(double value) {
	if (std::fabs(value) < 0.25) {
		return value < 0.0 ? -0.25 : 0.25;
	}
	return value;
}

} // namespace

void test_vec3_simd256_double(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float64::compiler_supported(), "vec3 Simd256Float64 compiler support");
	test.check(mt::Simd256Float64::cpu_supported(cpu), "vec3 Simd256Float64 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float64 ax(0.0);
	mt::Simd256Float64 ay(0.0);
	mt::Simd256Float64 az(0.0);
	mt::Simd256Float64 bx(0.0);
	mt::Simd256Float64 by(0.0);
	mt::Simd256Float64 bz(0.0);

	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		ax.set_element(lane, double(lane + 1));
		ay.set_element(lane, double(lane + 1) * -2.0);
		az.set_element(lane, double(lane + 1) * 3.0);
		bx.set_element(lane, double(lane + 1) * 0.5);
		by.set_element(lane, double(lane + 1) * 4.0);
		bz.set_element(lane, double(lane + 1) * 5.0);
	}

	mt::vec3<mt::Simd256Float64> a(ax, ay, az);
	mt::vec3<mt::Simd256Float64> b(bx, by, bz);

	// SIMD vector plus vector.
	mt::vec3<mt::Simd256Float64> actual = a + b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec3<Simd256Float64> add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec3<Simd256Float64> add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec3<Simd256Float64> add z lane " + std::to_string(lane));
	}

	// SIMD vector divided by vector.
	actual = a / b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec3<Simd256Float64> divide x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec3<Simd256Float64> divide y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec3<Simd256Float64> divide z lane " + std::to_string(lane));
	}

	// SIMD swizzles reduce vec3 to vec2 lane-wise.
	mt::vec2<mt::Simd256Float64> swizzle = a.xy();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle.x.element(lane), ax.element(lane), "vec3<Simd256Float64> xy x lane " + std::to_string(lane));
		test.check_equal(swizzle.y.element(lane), ay.element(lane), "vec3<Simd256Float64> xy y lane " + std::to_string(lane));
	}
	swizzle = a.yz();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle.x.element(lane), ay.element(lane), "vec3<Simd256Float64> yz x lane " + std::to_string(lane));
		test.check_equal(swizzle.y.element(lane), az.element(lane), "vec3<Simd256Float64> yz y lane " + std::to_string(lane));
	}
	swizzle = a.xz();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle.x.element(lane), ax.element(lane), "vec3<Simd256Float64> xz x lane " + std::to_string(lane));
		test.check_equal(swizzle.y.element(lane), az.element(lane), "vec3<Simd256Float64> xz y lane " + std::to_string(lane));
	}

	// SIMD dot product reduces vec3 components lane-wise.
	mt::Simd256Float64 dot_actual = mt::dot(a, b);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		double expected = ax.element(lane) * bx.element(lane) + ay.element(lane) * by.element(lane) + az.element(lane) * bz.element(lane);
		test.check_equal(dot_actual.element(lane), expected, "vec3<Simd256Float64> dot lane " + std::to_string(lane));
	}

	// SIMD cross product is computed lane-wise.
	actual = mt::cross(mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(1.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0)), mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0), mt::Simd256Float64(0.0)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec3<Simd256Float64> cross x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0, "vec3<Simd256Float64> cross y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 1.0, "vec3<Simd256Float64> cross z lane " + std::to_string(lane));
	}

	// SIMD normalization returns a new unit-length vector per lane.
	mt::vec3<mt::Simd256Float64> normalized = mt::normalize(a);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		double length = std::sqrt(ax.element(lane) * ax.element(lane) + ay.element(lane) * ay.element(lane) + az.element(lane) * az.element(lane));
		test.check_close(normalized.x.element(lane), ax.element(lane) / length, "vec3<Simd256Float64> normalize x lane " + std::to_string(lane));
		test.check_close(normalized.y.element(lane), ay.element(lane) / length, "vec3<Simd256Float64> normalize y lane " + std::to_string(lane));
		test.check_close(normalized.z.element(lane), az.element(lane) / length, "vec3<Simd256Float64> normalize z lane " + std::to_string(lane));
	}

	// SIMD component-wise floor.
	actual = mt::floor(mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25), mt::Simd256Float64(3.5)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0, "vec3<Simd256Float64> floor x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec3<Simd256Float64> floor y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 3.0, "vec3<Simd256Float64> floor z lane " + std::to_string(lane));
	}

	// SIMD wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(-1.0), mt::Simd256Float64(7.0), mt::Simd256Float64(12.0)), 0.0, 5.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 4.0, "vec3<Simd256Float64> wrap x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec3<Simd256Float64> wrap y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 2.0, "vec3<Simd256Float64> wrap z lane " + std::to_string(lane));
	}

	// SIMD refract uses per-lane masks when total internal reflection happens.
	actual = mt::refract(mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(-1.0), mt::Simd256Float64(0.0)), mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0), mt::Simd256Float64(0.0)), 1.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec3<Simd256Float64> refract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -1.0, "vec3<Simd256Float64> refract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0, "vec3<Simd256Float64> refract z lane " + std::to_string(lane));
	}

	actual = mt::refract(mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(1.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0)), mt::vec3<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0), mt::Simd256Float64(0.0)), 2.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec3<Simd256Float64> refract total internal x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0, "vec3<Simd256Float64> refract total internal y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0, "vec3<Simd256Float64> refract total internal z lane " + std::to_string(lane));
	}

	// Randomized SIMD arithmetic checks each lane through element().
	std::mt19937 rng(0x20260519u);
	for (int i = 0; i < 250; ++i) {
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			ax.set_element(lane, random_double(rng, -1000.0, 1000.0));
			ay.set_element(lane, random_double(rng, -1000.0, 1000.0));
			az.set_element(lane, random_double(rng, -1000.0, 1000.0));
			bx.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
			by.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
			bz.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
		}

		a = mt::vec3<mt::Simd256Float64>(ax, ay, az);
		b = mt::vec3<mt::Simd256Float64>(bx, by, bz);

		actual = a + b;
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec3<Simd256Float64> random add x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec3<Simd256Float64> random add y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec3<Simd256Float64> random add z lane " + std::to_string(lane));
		}

		actual = a / b;
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec3<Simd256Float64> random divide x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec3<Simd256Float64> random divide y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec3<Simd256Float64> random divide z lane " + std::to_string(lane));
		}
	}
}

int main() {
	TestContext test;

	test_vec3_simd256_double(test);

	return test.result();
}
