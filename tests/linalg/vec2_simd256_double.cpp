/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 vec2<Simd256Float64> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

#include "simd-uint32.h"
#include "simd-f64.h"

#include "linalg/vec2.h"

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

void test_vec2_simd256_double(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float64::compiler_supported(), "Simd256Float64 compiler support");
	test.check(mt::Simd256Float64::cpu_supported(cpu), "Simd256Float64 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float64 ax(0.0);
	mt::Simd256Float64 ay(0.0);
	mt::Simd256Float64 bx(0.0);
	mt::Simd256Float64 by(0.0);

	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		ax.set_element(lane, double(lane + 1));
		ay.set_element(lane, double(lane + 1) * -2.0);
		bx.set_element(lane, double(lane + 1) * 0.5);
		by.set_element(lane, double(lane + 1) * 4.0);
	}

	mt::vec2<mt::Simd256Float64> a(ax, ay);
	mt::vec2<mt::Simd256Float64> b(bx, by);

	// SIMD vector plus vector.
	mt::vec2<mt::Simd256Float64> actual = a + b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec2<Simd256Float64> add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec2<Simd256Float64> add y lane " + std::to_string(lane));
	}

	// SIMD vector divided by vector.
	actual = a / b;
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec2<Simd256Float64> divide x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec2<Simd256Float64> divide y lane " + std::to_string(lane));
	}

	// SIMD dot product reduces vec2 components lane-wise.
	mt::Simd256Float64 dot_actual = mt::dot(a, b);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		double expected = ax.element(lane) * bx.element(lane) + ay.element(lane) * by.element(lane);
		test.check_equal(dot_actual.element(lane), expected, "vec2<Simd256Float64> dot lane " + std::to_string(lane));
	}

	// SIMD normalization returns a new unit-length vector per lane.
	mt::vec2<mt::Simd256Float64> normalized = mt::normalize(a);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		double length = std::sqrt(ax.element(lane) * ax.element(lane) + ay.element(lane) * ay.element(lane));
		test.check_close(normalized.x.element(lane), ax.element(lane) / length, "vec2<Simd256Float64> normalize x lane " + std::to_string(lane));
		test.check_close(normalized.y.element(lane), ay.element(lane) / length, "vec2<Simd256Float64> normalize y lane " + std::to_string(lane));
	}

	// SIMD component-wise math.
	actual = mt::floor(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0, "vec2<Simd256Float64> floor x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec2<Simd256Float64> floor y lane " + std::to_string(lane));
	}

	actual = mt::ceil(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -1.0, "vec2<Simd256Float64> ceil x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 3.0, "vec2<Simd256Float64> ceil y lane " + std::to_string(lane));
	}

	actual = mt::round(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0, "vec2<Simd256Float64> round x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec2<Simd256Float64> round y lane " + std::to_string(lane));
	}

	actual = mt::fract(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.25, "vec2<Simd256Float64> fract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.25, "vec2<Simd256Float64> fract y lane " + std::to_string(lane));
	}

	actual = mt::trunc(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(2.25)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -1.0, "vec2<Simd256Float64> trunc x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec2<Simd256Float64> trunc y lane " + std::to_string(lane));
	}

	actual = mt::abs(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.75), mt::Simd256Float64(-2.25)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 1.75, "vec2<Simd256Float64> abs x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.25, "vec2<Simd256Float64> abs y lane " + std::to_string(lane));
	}

	actual = mt::sqrt(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(9.0), mt::Simd256Float64(16.0)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 3.0, "vec2<Simd256Float64> sqrt x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 4.0, "vec2<Simd256Float64> sqrt y lane " + std::to_string(lane));
	}

	actual = mt::clamp(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-0.5), mt::Simd256Float64(1.5)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec2<Simd256Float64> clamp x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 1.0, "vec2<Simd256Float64> clamp y lane " + std::to_string(lane));
	}

	// SIMD wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(-1.0), mt::Simd256Float64(7.0)), 0.0, 5.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 4.0, "vec2<Simd256Float64> wrap x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0, "vec2<Simd256Float64> wrap y lane " + std::to_string(lane));
	}

	// SIMD range helpers accept scalar bounds.
	actual = mt::rescale_from_01(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.25), mt::Simd256Float64(0.75)), 10.0, 20.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 12.5, "vec2<Simd256Float64> rescale_from_01 scalar x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 17.5, "vec2<Simd256Float64> rescale_from_01 scalar y lane " + std::to_string(lane));
	}

	actual = mt::rescale_to_01(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(12.5), mt::Simd256Float64(17.5)), 10.0, 20.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.25, "vec2<Simd256Float64> rescale_to_01 scalar x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.75, "vec2<Simd256Float64> rescale_to_01 scalar y lane " + std::to_string(lane));
	}

	actual = mt::step(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.5), mt::Simd256Float64(1.5)), mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.25), mt::Simd256Float64(2.0)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec2<Simd256Float64> step vector x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 1.0, "vec2<Simd256Float64> step vector y lane " + std::to_string(lane));
	}

	actual = mt::step(0.5, mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.25), mt::Simd256Float64(0.75)));
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec2<Simd256Float64> step scalar edge x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 1.0, "vec2<Simd256Float64> step scalar edge y lane " + std::to_string(lane));
	}

	// SIMD refract uses per-lane masks when total internal reflection happens.
	actual = mt::refract(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(-1.0)), mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0)), 1.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec2<Simd256Float64> refract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -1.0, "vec2<Simd256Float64> refract y lane " + std::to_string(lane));
	}

	actual = mt::refract(mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(1.0), mt::Simd256Float64(0.0)), mt::vec2<mt::Simd256Float64>(mt::Simd256Float64(0.0), mt::Simd256Float64(1.0)), 2.0);
	for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0, "vec2<Simd256Float64> refract total internal x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0, "vec2<Simd256Float64> refract total internal y lane " + std::to_string(lane));
	}

	// Randomized SIMD arithmetic checks each lane through element().
	std::mt19937 rng(0x20260515u);
	for (int i = 0; i < 250; ++i) {
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			ax.set_element(lane, random_double(rng, -1000.0, 1000.0));
			ay.set_element(lane, random_double(rng, -1000.0, 1000.0));
			bx.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
			by.set_element(lane, safe_divisor(random_double(rng, -1000.0, 1000.0)));
		}

		a = mt::vec2<mt::Simd256Float64>(ax, ay);
		b = mt::vec2<mt::Simd256Float64>(bx, by);

		actual = a + b;
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec2<Simd256Float64> random add x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec2<Simd256Float64> random add y lane " + std::to_string(lane));
		}

		actual = a / b;
		for (int lane = 0; lane < mt::Simd256Float64::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec2<Simd256Float64> random divide x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec2<Simd256Float64> random divide y lane " + std::to_string(lane));
		}
	}
}

int main() {
	TestContext test;

	test_vec2_simd256_double(test);

	return test.result();
}
