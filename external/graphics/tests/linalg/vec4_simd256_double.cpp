/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 vec4<Simd256Float64> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

#include "simd-uint32.h"
#include "simd-f64.h"

#include "linalg/vec4.h"

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

void test_vec4_simd256_double(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float64::compiler_supported(), "vec4 Simd256Float64 compiler support");
	test.check(mt::Simd256Float64::cpu_supported(cpu), "vec4 Simd256Float64 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float64 ax(0.0);
	mt::Simd256Float64 ay(0.0);
	mt::Simd256Float64 az(0.0);
	mt::Simd256Float64 aw(0.0);
	mt::Simd256Float64 bx(0.0);
	mt::Simd256Float64 by(0.0);
	mt::Simd256Float64 bz(0.0);
	mt::Simd256Float64 bw(0.0);

	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		ax.set_element(lane, double(lane + 1));
		ay.set_element(lane, double(lane + 1) * -2.0);
		az.set_element(lane, double(lane + 1) * 3.0);
		aw.set_element(lane, double(lane + 1) * -4.0);
		bx.set_element(lane, double(lane + 1) * 0.5);
		by.set_element(lane, double(lane + 1) * 4.0);
		bz.set_element(lane, double(lane + 1) * 5.0);
		bw.set_element(lane, double(lane + 1) * 6.0);
	}

	mt::vec4<mt::Simd256Float64> a(ax, ay, az, aw);
	mt::vec4<mt::Simd256Float64> b(bx, by, bz, bw);

	// SIMD vector plus vector.
	mt::vec4<mt::Simd256Float64> actual = a + b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec4<Simd256Float64> add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec4<Simd256Float64> add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec4<Simd256Float64> add z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) + bw.element(lane), "vec4<Simd256Float64> add w lane " + std::to_string(lane));
	}

	// SIMD vector divided by vector.
	actual = a / b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec4<Simd256Float64> divide x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec4<Simd256Float64> divide y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec4<Simd256Float64> divide z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) / bw.element(lane), "vec4<Simd256Float64> divide w lane " + std::to_string(lane));
	}

	// SIMD vec2 swizzles reduce vec4 to vec2 lane-wise.
	mt::vec2<mt::Simd256Float64> swizzle2 = a.xy();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle2.x.element(lane), ax.element(lane), "vec4<Simd256Float64> xy x lane " + std::to_string(lane));
		test.check_equal(swizzle2.y.element(lane), ay.element(lane), "vec4<Simd256Float64> xy y lane " + std::to_string(lane));
	}
	swizzle2 = a.xw();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle2.x.element(lane), ax.element(lane), "vec4<Simd256Float64> xw x lane " + std::to_string(lane));
		test.check_equal(swizzle2.y.element(lane), aw.element(lane), "vec4<Simd256Float64> xw y lane " + std::to_string(lane));
	}
	swizzle2 = a.zw();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle2.x.element(lane), az.element(lane), "vec4<Simd256Float64> zw x lane " + std::to_string(lane));
		test.check_equal(swizzle2.y.element(lane), aw.element(lane), "vec4<Simd256Float64> zw y lane " + std::to_string(lane));
	}

	// SIMD vec3 swizzles reduce vec4 to vec3 lane-wise.
	mt::vec3<mt::Simd256Float64> swizzle3 = a.xyz();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle3.x.element(lane), ax.element(lane), "vec4<Simd256Float64> xyz x lane " + std::to_string(lane));
		test.check_equal(swizzle3.y.element(lane), ay.element(lane), "vec4<Simd256Float64> xyz y lane " + std::to_string(lane));
		test.check_equal(swizzle3.z.element(lane), az.element(lane), "vec4<Simd256Float64> xyz z lane " + std::to_string(lane));
	}
	swizzle3 = a.xzw();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle3.x.element(lane), ax.element(lane), "vec4<Simd256Float64> xzw x lane " + std::to_string(lane));
		test.check_equal(swizzle3.y.element(lane), az.element(lane), "vec4<Simd256Float64> xzw y lane " + std::to_string(lane));
		test.check_equal(swizzle3.z.element(lane), aw.element(lane), "vec4<Simd256Float64> xzw z lane " + std::to_string(lane));
	}
	swizzle3 = a.yzw();
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(swizzle3.x.element(lane), ay.element(lane), "vec4<Simd256Float64> yzw x lane " + std::to_string(lane));
		test.check_equal(swizzle3.y.element(lane), az.element(lane), "vec4<Simd256Float64> yzw y lane " + std::to_string(lane));
		test.check_equal(swizzle3.z.element(lane), aw.element(lane), "vec4<Simd256Float64> yzw z lane " + std::to_string(lane));
	}

	// SIMD dot product reduces vec4 components lane-wise.
	mt::Simd256Float64 dot_actual = mt::dot(a, b);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		double expected = ax.element(lane) * bx.element(lane) + ay.element(lane) * by.element(lane) + az.element(lane) * bz.element(lane) + aw.element(lane) * bw.element(lane);
		test.check_equal(dot_actual.element(lane), expected, "vec4<Simd256Float64> dot lane " + std::to_string(lane));
	}

	// SIMD normalization returns a new unit-length vector per lane.
	mt::vec4<mt::Simd256Float64> normalized = mt::normalize(a);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		double length = std::sqrt(ax.element(lane) * ax.element(lane) + ay.element(lane) * ay.element(lane) + az.element(lane) * az.element(lane) + aw.element(lane) * aw.element(lane));
		test.check_close(normalized.x.element(lane), ax.element(lane) / length, "vec4<Simd256Float64> normalize x lane " + std::to_string(lane));
		test.check_close(normalized.y.element(lane), ay.element(lane) / length, "vec4<Simd256Float64> normalize y lane " + std::to_string(lane));
		test.check_close(normalized.z.element(lane), az.element(lane) / length, "vec4<Simd256Float64> normalize z lane " + std::to_string(lane));
		test.check_close(normalized.w.element(lane), aw.element(lane) / length, "vec4<Simd256Float64> normalize w lane " + std::to_string(lane));
	}

	// SIMD component-wise floor.
	actual = mt::floor(mt::vec4<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25), mt::Simd256Float64(3.5), mt::Simd256Float64(-4.5)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0, "vec4<Simd256Float64> floor x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec4<Simd256Float64> floor y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 3.0, "vec4<Simd256Float64> floor z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), -5.0, "vec4<Simd256Float64> floor w lane " + std::to_string(lane));
	}

	// SIMD wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec4<mt::Simd256Float64>(mt::Simd256Float64(-1.0), mt::Simd256Float64(7.0), mt::Simd256Float64(12.0), mt::Simd256Float64(5.0)), 0.0, 5.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 4.0, "vec4<Simd256Float64> wrap x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec4<Simd256Float64> wrap y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 2.0, "vec4<Simd256Float64> wrap z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 0.0, "vec4<Simd256Float64> wrap w lane " + std::to_string(lane));
	}

	// SIMD refract uses per-lane masks when total internal reflection happens.
	actual = mt::refract(mt::vec4<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(-1.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0)), mt::vec4<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0)), 1.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec4<Simd256Float64> refract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -1.0, "vec4<Simd256Float64> refract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0, "vec4<Simd256Float64> refract z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 0.0, "vec4<Simd256Float64> refract w lane " + std::to_string(lane));
	}

	actual = mt::refract(mt::vec4<mt::Simd256Float64>(mt::Simd256Float64(1.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0)), mt::vec4<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0), mt::Simd256Float64(0.0), mt::Simd256Float64(0.0)), 2.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec4<Simd256Float64> refract total internal x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0, "vec4<Simd256Float64> refract total internal y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0, "vec4<Simd256Float64> refract total internal z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 0.0, "vec4<Simd256Float64> refract total internal w lane " + std::to_string(lane));
	}

	// Randomized SIMD arithmetic checks each lane through element().
	std::mt19937 rng(0x20260523u);
	for (int i = 0; i < 250; ++i) {
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			ax.set_element(lane, random_double(rng, -1000.0, 1000.0));
			ay.set_element(lane, random_double(rng, -1000.0, 1000.0));
			az.set_element(lane, random_double(rng, -1000.0, 1000.0));
			aw.set_element(lane, random_double(rng, -1000.0, 1000.0));
			bx.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
			by.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
			bz.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
			bw.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
		}

		a = mt::vec4<mt::Simd256Float64>(ax, ay, az, aw);
		b = mt::vec4<mt::Simd256Float64>(bx, by, bz, bw);

		actual = a + b;
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec4<Simd256Float64> random add x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec4<Simd256Float64> random add y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec4<Simd256Float64> random add z lane " + std::to_string(lane));
			test.check_equal(actual.w.element(lane), aw.element(lane) + bw.element(lane), "vec4<Simd256Float64> random add w lane " + std::to_string(lane));
		}

		actual = a / b;
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec4<Simd256Float64> random divide x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec4<Simd256Float64> random divide y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec4<Simd256Float64> random divide z lane " + std::to_string(lane));
			test.check_equal(actual.w.element(lane), aw.element(lane) / bw.element(lane), "vec4<Simd256Float64> random divide w lane " + std::to_string(lane));
		}
	}
}

int main() {
	TestContext test;

	test_vec4_simd256_double(test);

	return test.result();
}
