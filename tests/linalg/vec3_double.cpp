/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar vec3<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

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

void test_vec3_double(TestContext& test) {
	// Shared simple inputs for double checks.
	mt::vec3<double> a(6.0, -8.0, 10.0);
	mt::vec3<double> b(2.0, 4.0, 5.0);
	double s = 3.0;

	// Vector plus vector.
	mt::vec3<double> actual = a + b;
	test.check_equal(actual.x, 8.0, "vec3<double> add x");
	test.check_equal(actual.y, -4.0, "vec3<double> add y");
	test.check_equal(actual.z, 15.0, "vec3<double> add z");

	// Vector divided by vector.
	actual = a / b;
	test.check_equal(actual.x, 3.0, "vec3<double> divide x");
	test.check_equal(actual.y, -2.0, "vec3<double> divide y");
	test.check_equal(actual.z, 2.0, "vec3<double> divide z");

	// Scalar minus vector.
	actual = s - b;
	test.check_equal(actual.x, 1.0, "vec3<double> scalar minus vector x");
	test.check_equal(actual.y, -1.0, "vec3<double> scalar minus vector y");
	test.check_equal(actual.z, -2.0, "vec3<double> scalar minus vector z");

	// Equality is available for scalar component types.
	test.check(a == mt::vec3<double>(6.0, -8.0, 10.0), "vec3<double> equality true");
	test.check(!(a == b), "vec3<double> equality false");

	// Swizzles reduce vec3 to vec2 in component order.
	mt::vec2<double> swizzle = a.xy();
	test.check_equal(swizzle.x, 6.0, "vec3<double> xy x");
	test.check_equal(swizzle.y, -8.0, "vec3<double> xy y");
	swizzle = a.yz();
	test.check_equal(swizzle.x, -8.0, "vec3<double> yz x");
	test.check_equal(swizzle.y, 10.0, "vec3<double> yz y");
	swizzle = a.xz();
	test.check_equal(swizzle.x, 6.0, "vec3<double> xz x");
	test.check_equal(swizzle.y, 10.0, "vec3<double> xz y");

	// Dot product and length.
	test.check_equal(mt::dot(a, b), 30.0, "vec3<double> dot");
	test.check_equal(mt::vec3<double>(3.0, 4.0, 12.0).length(), 13.0, "vec3<double> length");

	// Cross product follows the right-hand rule.
	actual = mt::cross(mt::vec3<double>(1.0, 0.0, 0.0), mt::vec3<double>(0.0, 1.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec3<double> cross x");
	test.check_equal(actual.y, 0.0, "vec3<double> cross y");
	test.check_equal(actual.z, 1.0, "vec3<double> cross z");

	// Distance computes the length between two points.
	test.check_equal(mt::distance(mt::vec3<double>(1.0, 2.0, 3.0), mt::vec3<double>(4.0, 6.0, 15.0)), 13.0, "vec3<double> distance");

	// Normalization returns a new unit-length vector.
	actual = mt::vec3<double>(3.0, 4.0, 12.0).normalize();
	// Normalize uses sqrt and division, so compare with tolerance.
	test.check_close(actual.x, 3.0 / 13.0, "vec3<double> normalize x");
	test.check_close(actual.y, 4.0 / 13.0, "vec3<double> normalize y");
	test.check_close(actual.z, 12.0 / 13.0, "vec3<double> normalize z");

	actual = mt::normalize(mt::vec3<double>(3.0, 4.0, 12.0));
	test.check_close(actual.x, 3.0 / 13.0, "vec3<double> free normalize x");
	test.check_close(actual.y, 4.0 / 13.0, "vec3<double> free normalize y");
	test.check_close(actual.z, 12.0 / 13.0, "vec3<double> free normalize z");

	// Component-wise math functions.
	mt::vec3<double> c(-1.75, 2.25, 3.5);
	actual = mt::floor(c);
	test.check_equal(actual.x, -2.0, "vec3<double> floor x");
	test.check_equal(actual.y, 2.0, "vec3<double> floor y");
	test.check_equal(actual.z, 3.0, "vec3<double> floor z");

	actual = mt::ceil(c);
	test.check_equal(actual.x, -1.0, "vec3<double> ceil x");
	test.check_equal(actual.y, 3.0, "vec3<double> ceil y");
	test.check_equal(actual.z, 4.0, "vec3<double> ceil z");

	actual = mt::round(mt::vec3<double>(-1.5, 2.5, 3.5));
	test.check_equal(actual.x, -2.0, "vec3<double> round x");
	test.check_equal(actual.y, 3.0, "vec3<double> round y");
	test.check_equal(actual.z, 4.0, "vec3<double> round z");

	actual = mt::fract(c);
	test.check_equal(actual.x, 0.25, "vec3<double> fract x");
	test.check_equal(actual.y, 0.25, "vec3<double> fract y");
	test.check_equal(actual.z, 0.5, "vec3<double> fract z");

	actual = mt::trunc(c);
	test.check_equal(actual.x, -1.0, "vec3<double> trunc x");
	test.check_equal(actual.y, 2.0, "vec3<double> trunc y");
	test.check_equal(actual.z, 3.0, "vec3<double> trunc z");

	actual = mt::abs(c);
	test.check_equal(actual.x, 1.75, "vec3<double> abs x");
	test.check_equal(actual.y, 2.25, "vec3<double> abs y");
	test.check_equal(actual.z, 3.5, "vec3<double> abs z");

	actual = mt::sqrt(mt::vec3<double>(9.0, 16.0, 25.0));
	test.check_equal(actual.x, 3.0, "vec3<double> sqrt x");
	test.check_equal(actual.y, 4.0, "vec3<double> sqrt y");
	test.check_equal(actual.z, 5.0, "vec3<double> sqrt z");

	actual = mt::exp(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec3<double> exp x");
	test.check_equal(actual.y, 1.0, "vec3<double> exp y");
	test.check_equal(actual.z, 1.0, "vec3<double> exp z");

	actual = mt::exp2(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec3<double> exp2 x");
	test.check_equal(actual.y, 1.0, "vec3<double> exp2 y");
	test.check_equal(actual.z, 1.0, "vec3<double> exp2 z");

	actual = mt::log(mt::vec3<double>(1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec3<double> log x");
	test.check_equal(actual.y, 0.0, "vec3<double> log y");
	test.check_equal(actual.z, 0.0, "vec3<double> log z");

	actual = mt::log2(mt::vec3<double>(1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec3<double> log2 x");
	test.check_equal(actual.y, 0.0, "vec3<double> log2 y");
	test.check_equal(actual.z, 0.0, "vec3<double> log2 z");

	actual = mt::sin(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec3<double> sin x");
	test.check_equal(actual.y, 0.0, "vec3<double> sin y");
	test.check_equal(actual.z, 0.0, "vec3<double> sin z");

	actual = mt::cos(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec3<double> cos x");
	test.check_equal(actual.y, 1.0, "vec3<double> cos y");
	test.check_equal(actual.z, 1.0, "vec3<double> cos z");

	actual = mt::tan(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec3<double> tan x");
	test.check_equal(actual.y, 0.0, "vec3<double> tan y");
	test.check_equal(actual.z, 0.0, "vec3<double> tan z");

	actual = mt::asin(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec3<double> asin x");
	test.check_equal(actual.y, 0.0, "vec3<double> asin y");
	test.check_equal(actual.z, 0.0, "vec3<double> asin z");

	actual = mt::acos(mt::vec3<double>(1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec3<double> acos x");
	test.check_equal(actual.y, 0.0, "vec3<double> acos y");
	test.check_equal(actual.z, 0.0, "vec3<double> acos z");

	actual = mt::atan(mt::vec3<double>(0.0, 0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec3<double> atan x");
	test.check_equal(actual.y, 0.0, "vec3<double> atan y");
	test.check_equal(actual.z, 0.0, "vec3<double> atan z");

	actual = mt::atan2(mt::vec3<double>(0.0, 0.0, 0.0), mt::vec3<double>(1.0, 1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec3<double> atan2 x");
	test.check_equal(actual.y, 0.0, "vec3<double> atan2 y");
	test.check_equal(actual.z, 0.0, "vec3<double> atan2 z");

	// Component-wise min, max, clamp, power, and fused operations.
	actual = mt::min(mt::vec3<double>(1.0, 5.0, 9.0), mt::vec3<double>(2.0, 4.0, 10.0));
	test.check_equal(actual.x, 1.0, "vec3<double> min x");
	test.check_equal(actual.y, 4.0, "vec3<double> min y");
	test.check_equal(actual.z, 9.0, "vec3<double> min z");

	actual = mt::min(3.0, mt::vec3<double>(1.0, 5.0, 9.0));
	test.check_equal(actual.x, 1.0, "vec3<double> min scalar vector x");
	test.check_equal(actual.y, 3.0, "vec3<double> min scalar vector y");
	test.check_equal(actual.z, 3.0, "vec3<double> min scalar vector z");

	actual = mt::max(mt::vec3<double>(1.0, 5.0, 9.0), mt::vec3<double>(2.0, 4.0, 10.0));
	test.check_equal(actual.x, 2.0, "vec3<double> max x");
	test.check_equal(actual.y, 5.0, "vec3<double> max y");
	test.check_equal(actual.z, 10.0, "vec3<double> max z");

	actual = mt::max(mt::vec3<double>(1.0, 5.0, 9.0), 3.0);
	test.check_equal(actual.x, 3.0, "vec3<double> max vector scalar x");
	test.check_equal(actual.y, 5.0, "vec3<double> max vector scalar y");
	test.check_equal(actual.z, 9.0, "vec3<double> max vector scalar z");

	actual = mt::clamp(mt::vec3<double>(-0.5, 0.5, 1.5));
	test.check_equal(actual.x, 0.0, "vec3<double> clamp 0..1 x");
	test.check_equal(actual.y, 0.5, "vec3<double> clamp 0..1 y");
	test.check_equal(actual.z, 1.0, "vec3<double> clamp 0..1 z");

	actual = mt::clamp(mt::vec3<double>(-2.0, 2.0, 6.0), mt::vec3<double>(-1.0, 1.0, 3.0), mt::vec3<double>(1.0, 4.0, 5.0));
	test.check_equal(actual.x, -1.0, "vec3<double> clamp vector bounds x");
	test.check_equal(actual.y, 2.0, "vec3<double> clamp vector bounds y");
	test.check_equal(actual.z, 5.0, "vec3<double> clamp vector bounds z");

	actual = mt::pow(mt::vec3<double>(2.0, 4.0, 9.0), mt::vec3<double>(3.0, 0.5, 0.5));
	test.check_close(actual.x, 8.0, "vec3<double> pow vector x");
	test.check_close(actual.y, 2.0, "vec3<double> pow vector y");
	test.check_close(actual.z, 3.0, "vec3<double> pow vector z");

	actual = mt::pow(mt::vec3<double>(2.0, 4.0, 8.0), 2.0);
	test.check_close(actual.x, 4.0, "vec3<double> pow scalar x");
	test.check_close(actual.y, 16.0, "vec3<double> pow scalar y");
	test.check_close(actual.z, 64.0, "vec3<double> pow scalar z");

	actual = mt::fma(mt::vec3<double>(2.0, -3.0, 4.0), mt::vec3<double>(4.0, 5.0, 6.0), mt::vec3<double>(1.0, 2.0, 3.0));
	test.check_equal(actual.x, 9.0, "vec3<double> fma x");
	test.check_equal(actual.y, -13.0, "vec3<double> fma y");
	test.check_equal(actual.z, 27.0, "vec3<double> fma z");

	// Wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec3<double>(-1.0, 7.0, 12.0), 0.0, 5.0);
	test.check_equal(actual.x, 4.0, "vec3<double> wrap x");
	test.check_equal(actual.y, 2.0, "vec3<double> wrap y");
	test.check_equal(actual.z, 2.0, "vec3<double> wrap z");

	actual = mt::smoothstep(mt::vec3<double>(0.0, 0.0, 0.0), mt::vec3<double>(10.0, 10.0, 10.0), mt::vec3<double>(0.0, 5.0, 10.0));
	test.check_equal(actual.x, 0.0, "vec3<double> smoothstep x");
	test.check_equal(actual.y, 0.5, "vec3<double> smoothstep y");
	test.check_equal(actual.z, 1.0, "vec3<double> smoothstep z");

	actual = mt::mix(mt::vec3<double>(0.0, 10.0, 20.0), mt::vec3<double>(10.0, 20.0, 30.0), 0.25);
	test.check_equal(actual.x, 2.5, "vec3<double> mix x");
	test.check_equal(actual.y, 12.5, "vec3<double> mix y");
	test.check_equal(actual.z, 22.5, "vec3<double> mix z");

	actual = mt::lerp(mt::vec3<double>(0.0, 10.0, 20.0), mt::vec3<double>(10.0, 20.0, 30.0), 0.5);
	test.check_equal(actual.x, 5.0, "vec3<double> lerp x");
	test.check_equal(actual.y, 15.0, "vec3<double> lerp y");
	test.check_equal(actual.z, 25.0, "vec3<double> lerp z");

	actual = mt::reflect(mt::vec3<double>(1.0, -1.0, 2.0), mt::vec3<double>(0.0, 1.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec3<double> reflect x");
	test.check_equal(actual.y, 1.0, "vec3<double> reflect y");
	test.check_equal(actual.z, 2.0, "vec3<double> reflect z");

	actual = mt::refract(mt::vec3<double>(0.0, -1.0, 0.0), mt::vec3<double>(0.0, 1.0, 0.0), 1.0);
	test.check_equal(actual.x, 0.0, "vec3<double> refract x");
	test.check_equal(actual.y, -1.0, "vec3<double> refract y");
	test.check_equal(actual.z, 0.0, "vec3<double> refract z");

	actual = mt::refract(mt::vec3<double>(1.0, 0.0, 0.0), mt::vec3<double>(0.0, 1.0, 0.0), 2.0);
	test.check_equal(actual.x, 0.0, "vec3<double> refract total internal x");
	test.check_equal(actual.y, 0.0, "vec3<double> refract total internal y");
	test.check_equal(actual.z, 0.0, "vec3<double> refract total internal z");

	// Randomized arithmetic checks compare to direct scalar expressions.
	std::mt19937 rng(0x20260517u);
	for (int i = 0; i < 1000; ++i) {
		double ax = random_double(rng, -1000.0, 1000.0);
		double ay = random_double(rng, -1000.0, 1000.0);
		double az = random_double(rng, -1000.0, 1000.0);
		double bx = safe_divisor(random_double(rng, -1000.0, 1000.0));
		double by = safe_divisor(random_double(rng, -1000.0, 1000.0));
		double bz = safe_divisor(random_double(rng, -1000.0, 1000.0));

		mt::vec3<double> left(ax, ay, az);
		mt::vec3<double> right(bx, by, bz);

		actual = left + right;
		test.check_equal(actual.x, ax + bx, "vec3<double> random add x");
		test.check_equal(actual.y, ay + by, "vec3<double> random add y");
		test.check_equal(actual.z, az + bz, "vec3<double> random add z");

		actual = left / right;
		test.check_equal(actual.x, ax / bx, "vec3<double> random divide x");
		test.check_equal(actual.y, ay / by, "vec3<double> random divide y");
		test.check_equal(actual.z, az / bz, "vec3<double> random divide z");
	}
}

int main() {
	TestContext test;

	test_vec3_double(test);

	return test.result();
}
