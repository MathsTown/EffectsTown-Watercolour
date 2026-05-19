/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 vec2<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

#include "simd-f32.h"

#include "linalg/vec2.h"

namespace {

float random_float(std::mt19937& rng, float min_value, float max_value) {
	std::uniform_real_distribution<float> distribution(min_value, max_value);
	return distribution(rng);
}

float safe_divisor(float value) {
	if (std::fabs(value) < 0.25f) {
		return value < 0.0f ? -0.25f : 0.25f;
	}
	return value;
}

} // namespace

void test_vec2_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "Simd256Float32 compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "Simd256Float32 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float32 ax(0.0f);
	mt::Simd256Float32 ay(0.0f);
	mt::Simd256Float32 bx(0.0f);
	mt::Simd256Float32 by(0.0f);
	mt::Simd256Float32 scalar(3.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		ax.set_element(lane, float(lane + 1));
		ay.set_element(lane, float(lane + 1) * -2.0f);
		bx.set_element(lane, float(lane + 1) * 0.5f);
		by.set_element(lane, float(lane + 1) * 4.0f);
	}

	mt::vec2<mt::Simd256Float32> a(ax, ay);
	mt::vec2<mt::Simd256Float32> b(bx, by);

	// SIMD vector plus vector.
	mt::vec2<mt::Simd256Float32> actual = a + b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec2<Simd256Float32> add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec2<Simd256Float32> add y lane " + std::to_string(lane));
	}

	// SIMD vector minus vector.
	actual = a - b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) - bx.element(lane), "vec2<Simd256Float32> subtract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) - by.element(lane), "vec2<Simd256Float32> subtract y lane " + std::to_string(lane));
	}

	// SIMD vector times vector.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) * bx.element(lane), "vec2<Simd256Float32> multiply x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) * by.element(lane), "vec2<Simd256Float32> multiply y lane " + std::to_string(lane));
	}

	// SIMD vector divided by vector.
	actual = a / b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec2<Simd256Float32> divide x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec2<Simd256Float32> divide y lane " + std::to_string(lane));
	}

	// SIMD vector plus scalar.
	actual = a + scalar;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + 3.0f, "vec2<Simd256Float32> vector scalar add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + 3.0f, "vec2<Simd256Float32> vector scalar add y lane " + std::to_string(lane));
	}

	// SIMD scalar minus vector.
	actual = scalar - b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 3.0f - bx.element(lane), "vec2<Simd256Float32> scalar vector subtract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 3.0f - by.element(lane), "vec2<Simd256Float32> scalar vector subtract y lane " + std::to_string(lane));
	}

	// SIMD compound vector add mutates in place.
	actual = a;
	actual += b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec2<Simd256Float32> compound add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec2<Simd256Float32> compound add y lane " + std::to_string(lane));
	}

	// SIMD dot product reduces vec2 components lane-wise.
	mt::Simd256Float32 dot_actual = mt::dot(a, b);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		float expected = ax.element(lane) * bx.element(lane) + ay.element(lane) * by.element(lane);
		test.check_equal(dot_actual.element(lane), expected, "vec2<Simd256Float32> dot lane " + std::to_string(lane));
	}

	// SIMD normalization returns a new unit-length vector per lane.
	mt::vec2<mt::Simd256Float32> normalized = mt::normalize(a);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		float length = std::sqrt(ax.element(lane) * ax.element(lane) + ay.element(lane) * ay.element(lane));
		// Normalize uses sqrt and division, so compare with tolerance.
		test.check_close(normalized.x.element(lane), ax.element(lane) / length, "vec2<Simd256Float32> normalize x lane " + std::to_string(lane));
		test.check_close(normalized.y.element(lane), ay.element(lane) / length, "vec2<Simd256Float32> normalize y lane " + std::to_string(lane));
	}

	// SIMD component-wise floor.
	actual = mt::floor(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0f, "vec2<Simd256Float32> floor x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec2<Simd256Float32> floor y lane " + std::to_string(lane));
	}

	// SIMD component-wise ceiling.
	actual = mt::ceil(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -1.0f, "vec2<Simd256Float32> ceil x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 3.0f, "vec2<Simd256Float32> ceil y lane " + std::to_string(lane));
	}

	// SIMD component-wise round.
	actual = mt::round(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0f, "vec2<Simd256Float32> round x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec2<Simd256Float32> round y lane " + std::to_string(lane));
	}

	// SIMD component-wise fractional part.
	actual = mt::fract(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.25f, "vec2<Simd256Float32> fract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.25f, "vec2<Simd256Float32> fract y lane " + std::to_string(lane));
	}

	// SIMD component-wise truncation.
	actual = mt::trunc(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -1.0f, "vec2<Simd256Float32> trunc x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec2<Simd256Float32> trunc y lane " + std::to_string(lane));
	}

	// SIMD component-wise absolute value and square root.
	actual = mt::abs(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(-2.25f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 1.75f, "vec2<Simd256Float32> abs x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.25f, "vec2<Simd256Float32> abs y lane " + std::to_string(lane));
	}

	actual = mt::sqrt(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(9.0f), mt::Simd256Float32(16.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 3.0f, "vec2<Simd256Float32> sqrt x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 4.0f, "vec2<Simd256Float32> sqrt y lane " + std::to_string(lane));
	}

	// SIMD component-wise exp/log and trigonometry.
	actual = mt::exp(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 1.0f, "vec2<Simd256Float32> exp x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 1.0f, "vec2<Simd256Float32> exp y lane " + std::to_string(lane));
	}

	actual = mt::log(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(1.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> log x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 0.0f, "vec2<Simd256Float32> log y lane " + std::to_string(lane));
	}

	actual = mt::radians(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(180.0f), mt::Simd256Float32(90.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 3.14159265358979323846f, "vec2<Simd256Float32> radians x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 1.57079632679489661923f, "vec2<Simd256Float32> radians y lane " + std::to_string(lane));
	}

	actual = mt::degrees(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(3.14159265358979323846f), mt::Simd256Float32(1.57079632679489661923f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 180.0f, "vec2<Simd256Float32> degrees x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 90.0f, "vec2<Simd256Float32> degrees y lane " + std::to_string(lane));
	}

	const mt::vec2<mt::Simd256Float32> member_degrees(mt::Simd256Float32(180.0f), mt::Simd256Float32(90.0f));
	actual = member_degrees.radians();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 3.14159265358979323846f, "vec2<Simd256Float32> member radians x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 1.57079632679489661923f, "vec2<Simd256Float32> member radians y lane " + std::to_string(lane));
	}

	const mt::vec2<mt::Simd256Float32> member_radians(mt::Simd256Float32(3.14159265358979323846f), mt::Simd256Float32(1.57079632679489661923f));
	actual = member_radians.degrees();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 180.0f, "vec2<Simd256Float32> member degrees x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 90.0f, "vec2<Simd256Float32> member degrees y lane " + std::to_string(lane));
	}

	actual = mt::sin(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> sin x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 0.0f, "vec2<Simd256Float32> sin y lane " + std::to_string(lane));
	}

	actual = mt::cos(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 1.0f, "vec2<Simd256Float32> cos x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 1.0f, "vec2<Simd256Float32> cos y lane " + std::to_string(lane));
	}

	// SIMD min, max, clamp, power, and fused operations.
	actual = mt::min(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(5.0f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(4.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 1.0f, "vec2<Simd256Float32> min x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 4.0f, "vec2<Simd256Float32> min y lane " + std::to_string(lane));
	}

	actual = mt::max(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(5.0f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(4.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 2.0f, "vec2<Simd256Float32> max x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 5.0f, "vec2<Simd256Float32> max y lane " + std::to_string(lane));
	}

	actual = mt::clamp(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-0.5f), mt::Simd256Float32(1.5f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> clamp x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 1.0f, "vec2<Simd256Float32> clamp y lane " + std::to_string(lane));
	}

	actual = mt::pow(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(4.0f)), mt::Simd256Float32(2.0f));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_close(actual.x.element(lane), 4.0f, "vec2<Simd256Float32> pow x lane " + std::to_string(lane));
		test.check_close(actual.y.element(lane), 16.0f, "vec2<Simd256Float32> pow y lane " + std::to_string(lane));
	}

	actual = mt::fma(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(2.0f), mt::Simd256Float32(-3.0f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(4.0f), mt::Simd256Float32(5.0f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(2.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 9.0f, "vec2<Simd256Float32> fma x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -13.0f, "vec2<Simd256Float32> fma y lane " + std::to_string(lane));
	}

	// SIMD wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(-1.0f), mt::Simd256Float32(7.0f)), 0.0f, 5.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 4.0f, "vec2<Simd256Float32> wrap x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec2<Simd256Float32> wrap y lane " + std::to_string(lane));
	}

	// SIMD range helpers accept scalar bounds.
	actual = mt::rescale_from_01(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.75f)), 10.0f, 20.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 12.5f, "vec2<Simd256Float32> rescale_from_01 scalar x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 17.5f, "vec2<Simd256Float32> rescale_from_01 scalar y lane " + std::to_string(lane));
	}

	actual = mt::rescale_to_01(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(12.5f), mt::Simd256Float32(17.5f)), 10.0f, 20.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.25f, "vec2<Simd256Float32> rescale_to_01 scalar x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.75f, "vec2<Simd256Float32> rescale_to_01 scalar y lane " + std::to_string(lane));
	}

	actual = mt::step(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.5f), mt::Simd256Float32(1.5f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(2.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> step vector x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 1.0f, "vec2<Simd256Float32> step vector y lane " + std::to_string(lane));
	}

	actual = mt::step(0.5f, mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.25f), mt::Simd256Float32(0.75f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> step scalar edge x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 1.0f, "vec2<Simd256Float32> step scalar edge y lane " + std::to_string(lane));
	}

	// SIMD refract uses per-lane masks when total internal reflection happens.
	actual = mt::refract(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(-1.0f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f)), 1.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> refract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -1.0f, "vec2<Simd256Float32> refract y lane " + std::to_string(lane));
	}

	actual = mt::refract(mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f)), mt::vec2<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f)), 2.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec2<Simd256Float32> refract total internal x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0f, "vec2<Simd256Float32> refract total internal y lane " + std::to_string(lane));
	}

	// Randomized SIMD arithmetic checks each lane through element().
	std::mt19937 rng(0x20260514u);
	for (int i = 0; i < 250; ++i) {
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			ax.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			ay.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			bx.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
			by.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
		}

		a = mt::vec2<mt::Simd256Float32>(ax, ay);
		b = mt::vec2<mt::Simd256Float32>(bx, by);

		actual = a + b;
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec2<Simd256Float32> random add x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec2<Simd256Float32> random add y lane " + std::to_string(lane));
		}

		actual = a / b;
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec2<Simd256Float32> random divide x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec2<Simd256Float32> random divide y lane " + std::to_string(lane));
		}
	}
}

int main() {
	TestContext test;

	test_vec2_simd256_float(test);

	return test.result();
}
