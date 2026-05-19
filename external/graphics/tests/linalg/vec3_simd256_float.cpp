/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 vec3<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

#include "simd-f32.h"

#include "linalg/vec3.h"

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

void test_vec3_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "vec3 Simd256Float32 compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "vec3 Simd256Float32 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float32 ax(0.0f);
	mt::Simd256Float32 ay(0.0f);
	mt::Simd256Float32 az(0.0f);
	mt::Simd256Float32 bx(0.0f);
	mt::Simd256Float32 by(0.0f);
	mt::Simd256Float32 bz(0.0f);
	mt::Simd256Float32 scalar(3.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		ax.set_element(lane, float(lane + 1));
		ay.set_element(lane, float(lane + 1) * -2.0f);
		az.set_element(lane, float(lane + 1) * 3.0f);
		bx.set_element(lane, float(lane + 1) * 0.5f);
		by.set_element(lane, float(lane + 1) * 4.0f);
		bz.set_element(lane, float(lane + 1) * 5.0f);
	}

	mt::vec3<mt::Simd256Float32> a(ax, ay, az);
	mt::vec3<mt::Simd256Float32> b(bx, by, bz);

	// SIMD vector plus vector.
	mt::vec3<mt::Simd256Float32> actual = a + b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec3<Simd256Float32> add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec3<Simd256Float32> add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec3<Simd256Float32> add z lane " + std::to_string(lane));
	}

	// SIMD vector minus vector.
	actual = a - b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) - bx.element(lane), "vec3<Simd256Float32> subtract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) - by.element(lane), "vec3<Simd256Float32> subtract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) - bz.element(lane), "vec3<Simd256Float32> subtract z lane " + std::to_string(lane));
	}

	// SIMD vector times vector.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) * bx.element(lane), "vec3<Simd256Float32> multiply x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) * by.element(lane), "vec3<Simd256Float32> multiply y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) * bz.element(lane), "vec3<Simd256Float32> multiply z lane " + std::to_string(lane));
	}

	// SIMD vector divided by vector.
	actual = a / b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec3<Simd256Float32> divide x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec3<Simd256Float32> divide y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec3<Simd256Float32> divide z lane " + std::to_string(lane));
	}

	// SIMD vector plus scalar.
	actual = a + scalar;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + 3.0f, "vec3<Simd256Float32> vector scalar add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + 3.0f, "vec3<Simd256Float32> vector scalar add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + 3.0f, "vec3<Simd256Float32> vector scalar add z lane " + std::to_string(lane));
	}

	// SIMD scalar minus vector.
	actual = scalar - b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 3.0f - bx.element(lane), "vec3<Simd256Float32> scalar vector subtract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 3.0f - by.element(lane), "vec3<Simd256Float32> scalar vector subtract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 3.0f - bz.element(lane), "vec3<Simd256Float32> scalar vector subtract z lane " + std::to_string(lane));
	}

	// SIMD compound vector add mutates in place.
	actual = a;
	actual += b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec3<Simd256Float32> compound add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec3<Simd256Float32> compound add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec3<Simd256Float32> compound add z lane " + std::to_string(lane));
	}

	// SIMD swizzles reduce vec3 to vec2 lane-wise.
	mt::vec2<mt::Simd256Float32> swizzle = a.xy();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle.x.element(lane), ax.element(lane), "vec3<Simd256Float32> xy x lane " + std::to_string(lane));
		test.check_equal(swizzle.y.element(lane), ay.element(lane), "vec3<Simd256Float32> xy y lane " + std::to_string(lane));
	}
	swizzle = a.yz();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle.x.element(lane), ay.element(lane), "vec3<Simd256Float32> yz x lane " + std::to_string(lane));
		test.check_equal(swizzle.y.element(lane), az.element(lane), "vec3<Simd256Float32> yz y lane " + std::to_string(lane));
	}
	swizzle = a.xz();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle.x.element(lane), ax.element(lane), "vec3<Simd256Float32> xz x lane " + std::to_string(lane));
		test.check_equal(swizzle.y.element(lane), az.element(lane), "vec3<Simd256Float32> xz y lane " + std::to_string(lane));
	}

	// SIMD dot product reduces vec3 components lane-wise.
	mt::Simd256Float32 dot_actual = mt::dot(a, b);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		float expected = ax.element(lane) * bx.element(lane) + ay.element(lane) * by.element(lane) + az.element(lane) * bz.element(lane);
		test.check_equal(dot_actual.element(lane), expected, "vec3<Simd256Float32> dot lane " + std::to_string(lane));
	}

	// SIMD cross product is computed lane-wise.
	actual = mt::cross(mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)), mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec3<Simd256Float32> cross x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0f, "vec3<Simd256Float32> cross y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 1.0f, "vec3<Simd256Float32> cross z lane " + std::to_string(lane));
	}

	// SIMD normalization returns a new unit-length vector per lane.
	mt::vec3<mt::Simd256Float32> normalized = mt::normalize(a);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		float length = std::sqrt(ax.element(lane) * ax.element(lane) + ay.element(lane) * ay.element(lane) + az.element(lane) * az.element(lane));
		// Normalize uses sqrt and division, so compare with tolerance.
		test.check_close(normalized.x.element(lane), ax.element(lane) / length, "vec3<Simd256Float32> normalize x lane " + std::to_string(lane));
		test.check_close(normalized.y.element(lane), ay.element(lane) / length, "vec3<Simd256Float32> normalize y lane " + std::to_string(lane));
		test.check_close(normalized.z.element(lane), az.element(lane) / length, "vec3<Simd256Float32> normalize z lane " + std::to_string(lane));
	}

	// SIMD component-wise floor.
	actual = mt::floor(mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f), mt::Simd256Float32(3.5f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0f, "vec3<Simd256Float32> floor x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec3<Simd256Float32> floor y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 3.0f, "vec3<Simd256Float32> floor z lane " + std::to_string(lane));
	}

	// SIMD wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(-1.0f), mt::Simd256Float32(7.0f), mt::Simd256Float32(12.0f)), 0.0f, 5.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 4.0f, "vec3<Simd256Float32> wrap x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec3<Simd256Float32> wrap y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 2.0f, "vec3<Simd256Float32> wrap z lane " + std::to_string(lane));
	}

	// SIMD refract uses per-lane masks when total internal reflection happens.
	actual = mt::refract(mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(-1.0f), mt::Simd256Float32(0.0f)), mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f)), 1.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec3<Simd256Float32> refract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -1.0f, "vec3<Simd256Float32> refract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0f, "vec3<Simd256Float32> refract z lane " + std::to_string(lane));
	}

	actual = mt::refract(mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)), mt::vec3<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f)), 2.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec3<Simd256Float32> refract total internal x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0f, "vec3<Simd256Float32> refract total internal y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0f, "vec3<Simd256Float32> refract total internal z lane " + std::to_string(lane));
	}

	// Randomized SIMD arithmetic checks each lane through element().
	std::mt19937 rng(0x20260518u);
	for (int i = 0; i < 250; ++i) {
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			ax.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			ay.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			az.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			bx.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
			by.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
			bz.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
		}

		a = mt::vec3<mt::Simd256Float32>(ax, ay, az);
		b = mt::vec3<mt::Simd256Float32>(bx, by, bz);

		actual = a + b;
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec3<Simd256Float32> random add x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec3<Simd256Float32> random add y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec3<Simd256Float32> random add z lane " + std::to_string(lane));
		}

		actual = a / b;
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec3<Simd256Float32> random divide x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec3<Simd256Float32> random divide y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec3<Simd256Float32> random divide z lane " + std::to_string(lane));
		}
	}
}

int main() {
	TestContext test;

	test_vec3_simd256_float(test);

	return test.result();
}
