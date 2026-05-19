/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar vec4<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

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

void test_vec4_double(TestContext& test) {
	// Shared simple inputs for double checks.
	mt::vec4<double> a(6.0, -8.0, 10.0, -12.0);
	mt::vec4<double> b(2.0, 4.0, 5.0, 6.0);
	double s = 3.0;

	// Constructor can combine two vec2 values.
	mt::vec4<double> from_xy_zw(mt::vec2<double>(1.0, 2.0), mt::vec2<double>(3.0, 4.0));
	test.check_equal(from_xy_zw.x, 1.0, "vec4<double> vec2,vec2 constructor x");
	test.check_equal(from_xy_zw.y, 2.0, "vec4<double> vec2,vec2 constructor y");
	test.check_equal(from_xy_zw.z, 3.0, "vec4<double> vec2,vec2 constructor z");
	test.check_equal(from_xy_zw.w, 4.0, "vec4<double> vec2,vec2 constructor w");

	// Vector plus vector.
	mt::vec4<double> actual = a + b;
	test.check_equal(actual.x, 8.0, "vec4<double> add x");
	test.check_equal(actual.y, -4.0, "vec4<double> add y");
	test.check_equal(actual.z, 15.0, "vec4<double> add z");
	test.check_equal(actual.w, -6.0, "vec4<double> add w");

	// Vector divided by vector.
	actual = a / b;
	test.check_equal(actual.x, 3.0, "vec4<double> divide x");
	test.check_equal(actual.y, -2.0, "vec4<double> divide y");
	test.check_equal(actual.z, 2.0, "vec4<double> divide z");
	test.check_equal(actual.w, -2.0, "vec4<double> divide w");

	// Scalar minus vector.
	actual = s - b;
	test.check_equal(actual.x, 1.0, "vec4<double> scalar minus vector x");
	test.check_equal(actual.y, -1.0, "vec4<double> scalar minus vector y");
	test.check_equal(actual.z, -2.0, "vec4<double> scalar minus vector z");
	test.check_equal(actual.w, -3.0, "vec4<double> scalar minus vector w");

	// Equality is available for scalar component types.
	test.check(a == mt::vec4<double>(6.0, -8.0, 10.0, -12.0), "vec4<double> equality true");
	test.check(!(a == b), "vec4<double> equality false");

	// Vec2 swizzles reduce vec4 to vec2 in component order.
	mt::vec2<double> swizzle2 = a.xy();
	test.check_equal(swizzle2.x, 6.0, "vec4<double> xy x");
	test.check_equal(swizzle2.y, -8.0, "vec4<double> xy y");
	swizzle2 = a.xz();
	test.check_equal(swizzle2.x, 6.0, "vec4<double> xz x");
	test.check_equal(swizzle2.y, 10.0, "vec4<double> xz y");
	swizzle2 = a.xw();
	test.check_equal(swizzle2.x, 6.0, "vec4<double> xw x");
	test.check_equal(swizzle2.y, -12.0, "vec4<double> xw y");
	swizzle2 = a.yz();
	test.check_equal(swizzle2.x, -8.0, "vec4<double> yz x");
	test.check_equal(swizzle2.y, 10.0, "vec4<double> yz y");
	swizzle2 = a.yw();
	test.check_equal(swizzle2.x, -8.0, "vec4<double> yw x");
	test.check_equal(swizzle2.y, -12.0, "vec4<double> yw y");
	swizzle2 = a.zw();
	test.check_equal(swizzle2.x, 10.0, "vec4<double> zw x");
	test.check_equal(swizzle2.y, -12.0, "vec4<double> zw y");

	// Vec3 swizzles reduce vec4 to vec3 in component order.
	mt::vec3<double> swizzle3 = a.xyz();
	test.check_equal(swizzle3.x, 6.0, "vec4<double> xyz x");
	test.check_equal(swizzle3.y, -8.0, "vec4<double> xyz y");
	test.check_equal(swizzle3.z, 10.0, "vec4<double> xyz z");
	swizzle3 = a.xyw();
	test.check_equal(swizzle3.x, 6.0, "vec4<double> xyw x");
	test.check_equal(swizzle3.y, -8.0, "vec4<double> xyw y");
	test.check_equal(swizzle3.z, -12.0, "vec4<double> xyw z");
	swizzle3 = a.xzw();
	test.check_equal(swizzle3.x, 6.0, "vec4<double> xzw x");
	test.check_equal(swizzle3.y, 10.0, "vec4<double> xzw y");
	test.check_equal(swizzle3.z, -12.0, "vec4<double> xzw z");
	swizzle3 = a.yzw();
	test.check_equal(swizzle3.x, -8.0, "vec4<double> yzw x");
	test.check_equal(swizzle3.y, 10.0, "vec4<double> yzw y");
	test.check_equal(swizzle3.z, -12.0, "vec4<double> yzw z");

	// Dot product and length.
	test.check_equal(mt::dot(a, b), -42.0, "vec4<double> dot");
	test.check_equal(mt::vec4<double>(2.0, 4.0, 4.0, 8.0).length(), 10.0, "vec4<double> length");

	// Distance computes the length between two points.
	test.check_equal(mt::distance(mt::vec4<double>(1.0, 2.0, 3.0, 4.0), mt::vec4<double>(3.0, 6.0, 7.0, 12.0)), 10.0, "vec4<double> distance");

	// Normalization returns a new unit-length vector.
	actual = mt::vec4<double>(2.0, 4.0, 4.0, 8.0).normalize();
	// Normalize uses sqrt and division, so compare with tolerance.
	test.check_close(actual.x, 2.0 / 10.0, "vec4<double> normalize x");
	test.check_close(actual.y, 4.0 / 10.0, "vec4<double> normalize y");
	test.check_close(actual.z, 4.0 / 10.0, "vec4<double> normalize z");
	test.check_close(actual.w, 8.0 / 10.0, "vec4<double> normalize w");

	actual = mt::normalize(mt::vec4<double>(2.0, 4.0, 4.0, 8.0));
	test.check_close(actual.x, 2.0 / 10.0, "vec4<double> free normalize x");
	test.check_close(actual.y, 4.0 / 10.0, "vec4<double> free normalize y");
	test.check_close(actual.z, 4.0 / 10.0, "vec4<double> free normalize z");
	test.check_close(actual.w, 8.0 / 10.0, "vec4<double> free normalize w");

	// Component-wise math functions.
	mt::vec4<double> c(-1.75, 2.25, 3.5, -4.5);
	actual = mt::floor(c);
	test.check_equal(actual.x, -2.0, "vec4<double> floor x");
	test.check_equal(actual.y, 2.0, "vec4<double> floor y");
	test.check_equal(actual.z, 3.0, "vec4<double> floor z");
	test.check_equal(actual.w, -5.0, "vec4<double> floor w");

	actual = mt::ceil(c);
	test.check_equal(actual.x, -1.0, "vec4<double> ceil x");
	test.check_equal(actual.y, 3.0, "vec4<double> ceil y");
	test.check_equal(actual.z, 4.0, "vec4<double> ceil z");
	test.check_equal(actual.w, -4.0, "vec4<double> ceil w");

	actual = mt::round(mt::vec4<double>(-1.5, 2.5, 3.5, -4.5));
	test.check_equal(actual.x, -2.0, "vec4<double> round x");
	test.check_equal(actual.y, 3.0, "vec4<double> round y");
	test.check_equal(actual.z, 4.0, "vec4<double> round z");
	test.check_equal(actual.w, -5.0, "vec4<double> round w");

	actual = mt::fract(c);
	test.check_equal(actual.x, 0.25, "vec4<double> fract x");
	test.check_equal(actual.y, 0.25, "vec4<double> fract y");
	test.check_equal(actual.z, 0.5, "vec4<double> fract z");
	test.check_equal(actual.w, 0.5, "vec4<double> fract w");

	actual = mt::trunc(c);
	test.check_equal(actual.x, -1.0, "vec4<double> trunc x");
	test.check_equal(actual.y, 2.0, "vec4<double> trunc y");
	test.check_equal(actual.z, 3.0, "vec4<double> trunc z");
	test.check_equal(actual.w, -4.0, "vec4<double> trunc w");

	actual = mt::abs(c);
	test.check_equal(actual.x, 1.75, "vec4<double> abs x");
	test.check_equal(actual.y, 2.25, "vec4<double> abs y");
	test.check_equal(actual.z, 3.5, "vec4<double> abs z");
	test.check_equal(actual.w, 4.5, "vec4<double> abs w");

	actual = mt::sqrt(mt::vec4<double>(9.0, 16.0, 25.0, 36.0));
	test.check_equal(actual.x, 3.0, "vec4<double> sqrt x");
	test.check_equal(actual.y, 4.0, "vec4<double> sqrt y");
	test.check_equal(actual.z, 5.0, "vec4<double> sqrt z");
	test.check_equal(actual.w, 6.0, "vec4<double> sqrt w");

	actual = mt::exp(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec4<double> exp x");
	test.check_equal(actual.y, 1.0, "vec4<double> exp y");
	test.check_equal(actual.z, 1.0, "vec4<double> exp z");
	test.check_equal(actual.w, 1.0, "vec4<double> exp w");

	actual = mt::exp2(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec4<double> exp2 x");
	test.check_equal(actual.y, 1.0, "vec4<double> exp2 y");
	test.check_equal(actual.z, 1.0, "vec4<double> exp2 z");
	test.check_equal(actual.w, 1.0, "vec4<double> exp2 w");

	actual = mt::log(mt::vec4<double>(1.0, 1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec4<double> log x");
	test.check_equal(actual.y, 0.0, "vec4<double> log y");
	test.check_equal(actual.z, 0.0, "vec4<double> log z");
	test.check_equal(actual.w, 0.0, "vec4<double> log w");

	actual = mt::log2(mt::vec4<double>(1.0, 1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec4<double> log2 x");
	test.check_equal(actual.y, 0.0, "vec4<double> log2 y");
	test.check_equal(actual.z, 0.0, "vec4<double> log2 z");
	test.check_equal(actual.w, 0.0, "vec4<double> log2 w");

	actual = mt::sin(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec4<double> sin x");
	test.check_equal(actual.y, 0.0, "vec4<double> sin y");
	test.check_equal(actual.z, 0.0, "vec4<double> sin z");
	test.check_equal(actual.w, 0.0, "vec4<double> sin w");

	actual = mt::cos(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec4<double> cos x");
	test.check_equal(actual.y, 1.0, "vec4<double> cos y");
	test.check_equal(actual.z, 1.0, "vec4<double> cos z");
	test.check_equal(actual.w, 1.0, "vec4<double> cos w");

	actual = mt::tan(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec4<double> tan x");
	test.check_equal(actual.y, 0.0, "vec4<double> tan y");
	test.check_equal(actual.z, 0.0, "vec4<double> tan z");
	test.check_equal(actual.w, 0.0, "vec4<double> tan w");

	actual = mt::asin(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec4<double> asin x");
	test.check_equal(actual.y, 0.0, "vec4<double> asin y");
	test.check_equal(actual.z, 0.0, "vec4<double> asin z");
	test.check_equal(actual.w, 0.0, "vec4<double> asin w");

	actual = mt::acos(mt::vec4<double>(1.0, 1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec4<double> acos x");
	test.check_equal(actual.y, 0.0, "vec4<double> acos y");
	test.check_equal(actual.z, 0.0, "vec4<double> acos z");
	test.check_equal(actual.w, 0.0, "vec4<double> acos w");

	actual = mt::atan(mt::vec4<double>(0.0, 0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec4<double> atan x");
	test.check_equal(actual.y, 0.0, "vec4<double> atan y");
	test.check_equal(actual.z, 0.0, "vec4<double> atan z");
	test.check_equal(actual.w, 0.0, "vec4<double> atan w");

	actual = mt::atan2(mt::vec4<double>(0.0, 0.0, 0.0, 0.0), mt::vec4<double>(1.0, 1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec4<double> atan2 x");
	test.check_equal(actual.y, 0.0, "vec4<double> atan2 y");
	test.check_equal(actual.z, 0.0, "vec4<double> atan2 z");
	test.check_equal(actual.w, 0.0, "vec4<double> atan2 w");

	// Component-wise min, max, clamp, power, and fused operations.
	actual = mt::min(mt::vec4<double>(1.0, 5.0, 9.0, 12.0), mt::vec4<double>(2.0, 4.0, 10.0, 11.0));
	test.check_equal(actual.x, 1.0, "vec4<double> min x");
	test.check_equal(actual.y, 4.0, "vec4<double> min y");
	test.check_equal(actual.z, 9.0, "vec4<double> min z");
	test.check_equal(actual.w, 11.0, "vec4<double> min w");

	actual = mt::min(mt::vec4<double>(1.0, 5.0, 9.0, 12.0), 3.0);
	test.check_equal(actual.x, 1.0, "vec4<double> min vector scalar x");
	test.check_equal(actual.y, 3.0, "vec4<double> min vector scalar y");
	test.check_equal(actual.z, 3.0, "vec4<double> min vector scalar z");
	test.check_equal(actual.w, 3.0, "vec4<double> min vector scalar w");

	actual = mt::max(mt::vec4<double>(1.0, 5.0, 9.0, 12.0), mt::vec4<double>(2.0, 4.0, 10.0, 11.0));
	test.check_equal(actual.x, 2.0, "vec4<double> max x");
	test.check_equal(actual.y, 5.0, "vec4<double> max y");
	test.check_equal(actual.z, 10.0, "vec4<double> max z");
	test.check_equal(actual.w, 12.0, "vec4<double> max w");

	actual = mt::max(3.0, mt::vec4<double>(1.0, 5.0, 9.0, 12.0));
	test.check_equal(actual.x, 3.0, "vec4<double> max scalar vector x");
	test.check_equal(actual.y, 5.0, "vec4<double> max scalar vector y");
	test.check_equal(actual.z, 9.0, "vec4<double> max scalar vector z");
	test.check_equal(actual.w, 12.0, "vec4<double> max scalar vector w");

	actual = mt::clamp(mt::vec4<double>(-0.5, 0.5, 1.5, 0.75));
	test.check_equal(actual.x, 0.0, "vec4<double> clamp 0..1 x");
	test.check_equal(actual.y, 0.5, "vec4<double> clamp 0..1 y");
	test.check_equal(actual.z, 1.0, "vec4<double> clamp 0..1 z");
	test.check_equal(actual.w, 0.75, "vec4<double> clamp 0..1 w");

	actual = mt::clamp(mt::vec4<double>(-2.0, 2.0, 6.0, 3.0), mt::vec4<double>(-1.0, 1.0, 3.0, 4.0), mt::vec4<double>(1.0, 4.0, 5.0, 8.0));
	test.check_equal(actual.x, -1.0, "vec4<double> clamp vector bounds x");
	test.check_equal(actual.y, 2.0, "vec4<double> clamp vector bounds y");
	test.check_equal(actual.z, 5.0, "vec4<double> clamp vector bounds z");
	test.check_equal(actual.w, 4.0, "vec4<double> clamp vector bounds w");

	actual = mt::pow(mt::vec4<double>(2.0, 4.0, 9.0, 16.0), mt::vec4<double>(3.0, 0.5, 0.5, 0.5));
	test.check_close(actual.x, 8.0, "vec4<double> pow vector x");
	test.check_close(actual.y, 2.0, "vec4<double> pow vector y");
	test.check_close(actual.z, 3.0, "vec4<double> pow vector z");
	test.check_close(actual.w, 4.0, "vec4<double> pow vector w");

	actual = mt::pow(mt::vec4<double>(2.0, 4.0, 8.0, 16.0), 2.0);
	test.check_close(actual.x, 4.0, "vec4<double> pow scalar x");
	test.check_close(actual.y, 16.0, "vec4<double> pow scalar y");
	test.check_close(actual.z, 64.0, "vec4<double> pow scalar z");
	test.check_close(actual.w, 256.0, "vec4<double> pow scalar w");

	actual = mt::fma(mt::vec4<double>(2.0, -3.0, 4.0, -5.0), mt::vec4<double>(4.0, 5.0, 6.0, 7.0), mt::vec4<double>(1.0, 2.0, 3.0, 4.0));
	test.check_equal(actual.x, 9.0, "vec4<double> fma x");
	test.check_equal(actual.y, -13.0, "vec4<double> fma y");
	test.check_equal(actual.z, 27.0, "vec4<double> fma z");
	test.check_equal(actual.w, -31.0, "vec4<double> fma w");

	// Wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec4<double>(-1.0, 7.0, 12.0, 5.0), 0.0, 5.0);
	test.check_equal(actual.x, 4.0, "vec4<double> wrap x");
	test.check_equal(actual.y, 2.0, "vec4<double> wrap y");
	test.check_equal(actual.z, 2.0, "vec4<double> wrap z");
	test.check_equal(actual.w, 0.0, "vec4<double> wrap w");

	actual = mt::smoothstep(mt::vec4<double>(0.0, 0.0, 0.0, 0.0), mt::vec4<double>(10.0, 10.0, 10.0, 10.0), mt::vec4<double>(0.0, 5.0, 10.0, 15.0));
	test.check_equal(actual.x, 0.0, "vec4<double> smoothstep x");
	test.check_equal(actual.y, 0.5, "vec4<double> smoothstep y");
	test.check_equal(actual.z, 1.0, "vec4<double> smoothstep z");
	test.check_equal(actual.w, 1.0, "vec4<double> smoothstep w");

	actual = mt::mix(mt::vec4<double>(0.0, 10.0, 20.0, 30.0), mt::vec4<double>(10.0, 20.0, 30.0, 40.0), 0.25);
	test.check_equal(actual.x, 2.5, "vec4<double> mix x");
	test.check_equal(actual.y, 12.5, "vec4<double> mix y");
	test.check_equal(actual.z, 22.5, "vec4<double> mix z");
	test.check_equal(actual.w, 32.5, "vec4<double> mix w");

	actual = mt::lerp(mt::vec4<double>(0.0, 10.0, 20.0, 30.0), mt::vec4<double>(10.0, 20.0, 30.0, 40.0), 0.5);
	test.check_equal(actual.x, 5.0, "vec4<double> lerp x");
	test.check_equal(actual.y, 15.0, "vec4<double> lerp y");
	test.check_equal(actual.z, 25.0, "vec4<double> lerp z");
	test.check_equal(actual.w, 35.0, "vec4<double> lerp w");

	actual = mt::reflect(mt::vec4<double>(1.0, -1.0, 2.0, 3.0), mt::vec4<double>(0.0, 1.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec4<double> reflect x");
	test.check_equal(actual.y, 1.0, "vec4<double> reflect y");
	test.check_equal(actual.z, 2.0, "vec4<double> reflect z");
	test.check_equal(actual.w, 3.0, "vec4<double> reflect w");

	actual = mt::refract(mt::vec4<double>(0.0, -1.0, 0.0, 0.0), mt::vec4<double>(0.0, 1.0, 0.0, 0.0), 1.0);
	test.check_equal(actual.x, 0.0, "vec4<double> refract x");
	test.check_equal(actual.y, -1.0, "vec4<double> refract y");
	test.check_equal(actual.z, 0.0, "vec4<double> refract z");
	test.check_equal(actual.w, 0.0, "vec4<double> refract w");

	actual = mt::refract(mt::vec4<double>(1.0, 0.0, 0.0, 0.0), mt::vec4<double>(0.0, 1.0, 0.0, 0.0), 2.0);
	test.check_equal(actual.x, 0.0, "vec4<double> refract total internal x");
	test.check_equal(actual.y, 0.0, "vec4<double> refract total internal y");
	test.check_equal(actual.z, 0.0, "vec4<double> refract total internal z");
	test.check_equal(actual.w, 0.0, "vec4<double> refract total internal w");

	// Randomized arithmetic checks compare to direct scalar expressions.
	std::mt19937 rng(0x20260521u);
	for (int i = 0; i < 1000; ++i) {
		double ax = random_double(rng, -1000.0, 1000.0);
		double ay = random_double(rng, -1000.0, 1000.0);
		double az = random_double(rng, -1000.0, 1000.0);
		double aw = random_double(rng, -1000.0, 1000.0);
		double bx = safe_divisor(random_double(rng, -1000.0, 1000.0));
		double by = safe_divisor(random_double(rng, -1000.0, 1000.0));
		double bz = safe_divisor(random_double(rng, -1000.0, 1000.0));
		double bw = safe_divisor(random_double(rng, -1000.0, 1000.0));

		mt::vec4<double> left(ax, ay, az, aw);
		mt::vec4<double> right(bx, by, bz, bw);

		actual = left + right;
		test.check_equal(actual.x, ax + bx, "vec4<double> random add x");
		test.check_equal(actual.y, ay + by, "vec4<double> random add y");
		test.check_equal(actual.z, az + bz, "vec4<double> random add z");
		test.check_equal(actual.w, aw + bw, "vec4<double> random add w");

		actual = left / right;
		test.check_equal(actual.x, ax / bx, "vec4<double> random divide x");
		test.check_equal(actual.y, ay / by, "vec4<double> random divide y");
		test.check_equal(actual.z, az / bz, "vec4<double> random divide z");
		test.check_equal(actual.w, aw / bw, "vec4<double> random divide w");
	}
}

int main() {
	TestContext test;

	test_vec4_double(test);

	return test.result();
}
