/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable AVX2 vec4<Simd256Float32> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

#include "simd-f32.h"

#include "linalg/vec4.h"

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

void test_vec4_simd256_float(TestContext& test) {
	// SIMD tests require both compiler and CPU support.
	mt::CpuInformation cpu;
	test.check(mt::Simd256Float32::compiler_supported(), "vec4 Simd256Float32 compiler support");
	test.check(mt::Simd256Float32::cpu_supported(cpu), "vec4 Simd256Float32 CPU support");

	// Build deterministic lane values through the SIMD lane API.
	mt::Simd256Float32 ax(0.0f);
	mt::Simd256Float32 ay(0.0f);
	mt::Simd256Float32 az(0.0f);
	mt::Simd256Float32 aw(0.0f);
	mt::Simd256Float32 bx(0.0f);
	mt::Simd256Float32 by(0.0f);
	mt::Simd256Float32 bz(0.0f);
	mt::Simd256Float32 bw(0.0f);
	mt::Simd256Float32 scalar(3.0f);

	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		ax.set_element(lane, float(lane + 1));
		ay.set_element(lane, float(lane + 1) * -2.0f);
		az.set_element(lane, float(lane + 1) * 3.0f);
		aw.set_element(lane, float(lane + 1) * -4.0f);
		bx.set_element(lane, float(lane + 1) * 0.5f);
		by.set_element(lane, float(lane + 1) * 4.0f);
		bz.set_element(lane, float(lane + 1) * 5.0f);
		bw.set_element(lane, float(lane + 1) * 6.0f);
	}

	mt::vec4<mt::Simd256Float32> a(ax, ay, az, aw);
	mt::vec4<mt::Simd256Float32> b(bx, by, bz, bw);

	// SIMD vector plus vector.
	mt::vec4<mt::Simd256Float32> actual = a + b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec4<Simd256Float32> add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec4<Simd256Float32> add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec4<Simd256Float32> add z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) + bw.element(lane), "vec4<Simd256Float32> add w lane " + std::to_string(lane));
	}

	// SIMD vector minus vector.
	actual = a - b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) - bx.element(lane), "vec4<Simd256Float32> subtract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) - by.element(lane), "vec4<Simd256Float32> subtract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) - bz.element(lane), "vec4<Simd256Float32> subtract z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) - bw.element(lane), "vec4<Simd256Float32> subtract w lane " + std::to_string(lane));
	}

	// SIMD vector times vector.
	actual = a * b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) * bx.element(lane), "vec4<Simd256Float32> multiply x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) * by.element(lane), "vec4<Simd256Float32> multiply y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) * bz.element(lane), "vec4<Simd256Float32> multiply z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) * bw.element(lane), "vec4<Simd256Float32> multiply w lane " + std::to_string(lane));
	}

	// SIMD vector divided by vector.
	actual = a / b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec4<Simd256Float32> divide x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec4<Simd256Float32> divide y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec4<Simd256Float32> divide z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) / bw.element(lane), "vec4<Simd256Float32> divide w lane " + std::to_string(lane));
	}

	// SIMD vector plus scalar.
	actual = a + scalar;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + 3.0f, "vec4<Simd256Float32> vector scalar add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + 3.0f, "vec4<Simd256Float32> vector scalar add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + 3.0f, "vec4<Simd256Float32> vector scalar add z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) + 3.0f, "vec4<Simd256Float32> vector scalar add w lane " + std::to_string(lane));
	}

	// SIMD scalar minus vector.
	actual = scalar - b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 3.0f - bx.element(lane), "vec4<Simd256Float32> scalar vector subtract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 3.0f - by.element(lane), "vec4<Simd256Float32> scalar vector subtract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 3.0f - bz.element(lane), "vec4<Simd256Float32> scalar vector subtract z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 3.0f - bw.element(lane), "vec4<Simd256Float32> scalar vector subtract w lane " + std::to_string(lane));
	}

	// SIMD compound vector add mutates in place.
	actual = a;
	actual += b;
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec4<Simd256Float32> compound add x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec4<Simd256Float32> compound add y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec4<Simd256Float32> compound add z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), aw.element(lane) + bw.element(lane), "vec4<Simd256Float32> compound add w lane " + std::to_string(lane));
	}

	// SIMD vec2 swizzles reduce vec4 to vec2 lane-wise.
	mt::vec2<mt::Simd256Float32> swizzle2 = a.xy();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle2.x.element(lane), ax.element(lane), "vec4<Simd256Float32> xy x lane " + std::to_string(lane));
		test.check_equal(swizzle2.y.element(lane), ay.element(lane), "vec4<Simd256Float32> xy y lane " + std::to_string(lane));
	}
	swizzle2 = a.xw();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle2.x.element(lane), ax.element(lane), "vec4<Simd256Float32> xw x lane " + std::to_string(lane));
		test.check_equal(swizzle2.y.element(lane), aw.element(lane), "vec4<Simd256Float32> xw y lane " + std::to_string(lane));
	}
	swizzle2 = a.zw();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle2.x.element(lane), az.element(lane), "vec4<Simd256Float32> zw x lane " + std::to_string(lane));
		test.check_equal(swizzle2.y.element(lane), aw.element(lane), "vec4<Simd256Float32> zw y lane " + std::to_string(lane));
	}

	// SIMD vec3 swizzles reduce vec4 to vec3 lane-wise.
	mt::vec3<mt::Simd256Float32> swizzle3 = a.xyz();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle3.x.element(lane), ax.element(lane), "vec4<Simd256Float32> xyz x lane " + std::to_string(lane));
		test.check_equal(swizzle3.y.element(lane), ay.element(lane), "vec4<Simd256Float32> xyz y lane " + std::to_string(lane));
		test.check_equal(swizzle3.z.element(lane), az.element(lane), "vec4<Simd256Float32> xyz z lane " + std::to_string(lane));
	}
	swizzle3 = a.xzw();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle3.x.element(lane), ax.element(lane), "vec4<Simd256Float32> xzw x lane " + std::to_string(lane));
		test.check_equal(swizzle3.y.element(lane), az.element(lane), "vec4<Simd256Float32> xzw y lane " + std::to_string(lane));
		test.check_equal(swizzle3.z.element(lane), aw.element(lane), "vec4<Simd256Float32> xzw z lane " + std::to_string(lane));
	}
	swizzle3 = a.yzw();
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(swizzle3.x.element(lane), ay.element(lane), "vec4<Simd256Float32> yzw x lane " + std::to_string(lane));
		test.check_equal(swizzle3.y.element(lane), az.element(lane), "vec4<Simd256Float32> yzw y lane " + std::to_string(lane));
		test.check_equal(swizzle3.z.element(lane), aw.element(lane), "vec4<Simd256Float32> yzw z lane " + std::to_string(lane));
	}

	// SIMD dot product reduces vec4 components lane-wise.
	mt::Simd256Float32 dot_actual = mt::dot(a, b);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		float expected = ax.element(lane) * bx.element(lane) + ay.element(lane) * by.element(lane) + az.element(lane) * bz.element(lane) + aw.element(lane) * bw.element(lane);
		test.check_equal(dot_actual.element(lane), expected, "vec4<Simd256Float32> dot lane " + std::to_string(lane));
	}

	// SIMD normalization returns a new unit-length vector per lane.
	mt::vec4<mt::Simd256Float32> normalized = mt::normalize(a);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		float length = std::sqrt(ax.element(lane) * ax.element(lane) + ay.element(lane) * ay.element(lane) + az.element(lane) * az.element(lane) + aw.element(lane) * aw.element(lane));
		// Normalize uses sqrt and division, so compare with tolerance.
		test.check_close(normalized.x.element(lane), ax.element(lane) / length, "vec4<Simd256Float32> normalize x lane " + std::to_string(lane));
		test.check_close(normalized.y.element(lane), ay.element(lane) / length, "vec4<Simd256Float32> normalize y lane " + std::to_string(lane));
		test.check_close(normalized.z.element(lane), az.element(lane) / length, "vec4<Simd256Float32> normalize z lane " + std::to_string(lane));
		test.check_close(normalized.w.element(lane), aw.element(lane) / length, "vec4<Simd256Float32> normalize w lane " + std::to_string(lane));
	}

	// SIMD component-wise floor.
	actual = mt::floor(mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(-1.75f), mt::Simd256Float32(2.25f), mt::Simd256Float32(3.5f), mt::Simd256Float32(-4.5f)));
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), -2.0f, "vec4<Simd256Float32> floor x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec4<Simd256Float32> floor y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 3.0f, "vec4<Simd256Float32> floor z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), -5.0f, "vec4<Simd256Float32> floor w lane " + std::to_string(lane));
	}

	// SIMD wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(-1.0f), mt::Simd256Float32(7.0f), mt::Simd256Float32(12.0f), mt::Simd256Float32(5.0f)), 0.0f, 5.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 4.0f, "vec4<Simd256Float32> wrap x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 2.0f, "vec4<Simd256Float32> wrap y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 2.0f, "vec4<Simd256Float32> wrap z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 0.0f, "vec4<Simd256Float32> wrap w lane " + std::to_string(lane));
	}

	// SIMD refract uses per-lane masks when total internal reflection happens.
	actual = mt::refract(mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(-1.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)), mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)), 1.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec4<Simd256Float32> refract x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), -1.0f, "vec4<Simd256Float32> refract y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0f, "vec4<Simd256Float32> refract z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 0.0f, "vec4<Simd256Float32> refract w lane " + std::to_string(lane));
	}

	actual = mt::refract(mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)), mt::vec4<mt::Simd256Float32>(mt::Simd256Float32(0.0f), mt::Simd256Float32(1.0f), mt::Simd256Float32(0.0f), mt::Simd256Float32(0.0f)), 2.0f);
	for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
		test.check_equal(actual.x.element(lane), 0.0f, "vec4<Simd256Float32> refract total internal x lane " + std::to_string(lane));
		test.check_equal(actual.y.element(lane), 0.0f, "vec4<Simd256Float32> refract total internal y lane " + std::to_string(lane));
		test.check_equal(actual.z.element(lane), 0.0f, "vec4<Simd256Float32> refract total internal z lane " + std::to_string(lane));
		test.check_equal(actual.w.element(lane), 0.0f, "vec4<Simd256Float32> refract total internal w lane " + std::to_string(lane));
	}

	// Randomized SIMD arithmetic checks each lane through element().
	std::mt19937 rng(0x20260522u);
	for (int i = 0; i < 250; ++i) {
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			ax.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			ay.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			az.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			aw.set_element(lane, random_float(rng, -1000.0f, 1000.0f));
			bx.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
			by.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
			bz.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
			bw.set_element(lane, safe_divisor(random_float(rng, -1000.0f, 1000.0f)));
		}

		a = mt::vec4<mt::Simd256Float32>(ax, ay, az, aw);
		b = mt::vec4<mt::Simd256Float32>(bx, by, bz, bw);

		actual = a + b;
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) + bx.element(lane), "vec4<Simd256Float32> random add x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) + by.element(lane), "vec4<Simd256Float32> random add y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) + bz.element(lane), "vec4<Simd256Float32> random add z lane " + std::to_string(lane));
			test.check_equal(actual.w.element(lane), aw.element(lane) + bw.element(lane), "vec4<Simd256Float32> random add w lane " + std::to_string(lane));
		}

		actual = a / b;
		for (int lane = 0; lane < mt::Simd256Float32::number_of_elements(); ++lane) {
			test.check_equal(actual.x.element(lane), ax.element(lane) / bx.element(lane), "vec4<Simd256Float32> random divide x lane " + std::to_string(lane));
			test.check_equal(actual.y.element(lane), ay.element(lane) / by.element(lane), "vec4<Simd256Float32> random divide y lane " + std::to_string(lane));
			test.check_equal(actual.z.element(lane), az.element(lane) / bz.element(lane), "vec4<Simd256Float32> random divide z lane " + std::to_string(lane));
			test.check_equal(actual.w.element(lane), aw.element(lane) / bw.element(lane), "vec4<Simd256Float32> random divide w lane " + std::to_string(lane));
		}
	}
}

int main() {
	TestContext test;

	test_vec4_simd256_float(test);

	return test.result();
}
