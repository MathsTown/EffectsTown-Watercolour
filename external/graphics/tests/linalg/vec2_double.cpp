/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar vec2<double> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

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

void test_vec2_double(TestContext& test) {
	// Constructors initialize double components directly.
	mt::vec2<double> default_value;
	test.check_equal(default_value.x, 0.0, "vec2<double> default x");
	test.check_equal(default_value.y, 0.0, "vec2<double> default y");

	mt::vec2<double> splat(3.0);
	test.check_equal(splat.x, 3.0, "vec2<double> splat x");
	test.check_equal(splat.y, 3.0, "vec2<double> splat y");

	// Shared simple inputs for double checks.
	mt::vec2<double> a(6.0, -8.0);
	mt::vec2<double> b(2.0, 4.0);
	double s = 3.0;

	// Vector plus vector.
	mt::vec2<double> actual = a + b;
	test.check_equal(actual.x, 8.0, "vec2<double> add x");
	test.check_equal(actual.y, -4.0, "vec2<double> add y");

	// Vector minus vector.
	actual = a - b;
	test.check_equal(actual.x, 4.0, "vec2<double> subtract x");
	test.check_equal(actual.y, -12.0, "vec2<double> subtract y");

	// Vector times vector.
	actual = a * b;
	test.check_equal(actual.x, 12.0, "vec2<double> multiply x");
	test.check_equal(actual.y, -32.0, "vec2<double> multiply y");

	// Vector divided by vector.
	actual = a / b;
	test.check_equal(actual.x, 3.0, "vec2<double> divide x");
	test.check_equal(actual.y, -2.0, "vec2<double> divide y");

	// Scalar and vector overloads work on either side.
	actual = a + s;
	test.check_equal(actual.x, 9.0, "vec2<double> vector plus scalar x");
	test.check_equal(actual.y, -5.0, "vec2<double> vector plus scalar y");

	actual = s + b;
	test.check_equal(actual.x, 5.0, "vec2<double> scalar plus vector x");
	test.check_equal(actual.y, 7.0, "vec2<double> scalar plus vector y");

	actual = a - s;
	test.check_equal(actual.x, 3.0, "vec2<double> vector minus scalar x");
	test.check_equal(actual.y, -11.0, "vec2<double> vector minus scalar y");

	// Scalar minus vector.
	actual = s - b;
	test.check_equal(actual.x, 1.0, "vec2<double> scalar minus vector x");
	test.check_equal(actual.y, -1.0, "vec2<double> scalar minus vector y");

	actual = b * s;
	test.check_equal(actual.x, 6.0, "vec2<double> vector multiply scalar x");
	test.check_equal(actual.y, 12.0, "vec2<double> vector multiply scalar y");

	actual = s * b;
	test.check_equal(actual.x, 6.0, "vec2<double> scalar multiply vector x");
	test.check_equal(actual.y, 12.0, "vec2<double> scalar multiply vector y");

	actual = a / 2.0;
	test.check_equal(actual.x, 3.0, "vec2<double> vector divide scalar x");
	test.check_equal(actual.y, -4.0, "vec2<double> vector divide scalar y");

	actual = s / b;
	test.check_equal(actual.x, 1.5, "vec2<double> scalar divide vector x");
	test.check_equal(actual.y, 0.75, "vec2<double> scalar divide vector y");

	// Compound assignments mutate in place.
	actual = a;
	actual += b;
	test.check_equal(actual.x, 8.0, "vec2<double> compound add x");
	test.check_equal(actual.y, -4.0, "vec2<double> compound add y");

	actual = a;
	actual -= b;
	test.check_equal(actual.x, 4.0, "vec2<double> compound subtract x");
	test.check_equal(actual.y, -12.0, "vec2<double> compound subtract y");

	actual = b;
	actual *= a;
	test.check_equal(actual.x, 12.0, "vec2<double> compound multiply x");
	test.check_equal(actual.y, -32.0, "vec2<double> compound multiply y");

	actual = a;
	actual /= b;
	test.check_equal(actual.x, 3.0, "vec2<double> compound divide x");
	test.check_equal(actual.y, -2.0, "vec2<double> compound divide y");

	actual = b;
	actual += s;
	test.check_equal(actual.x, 5.0, "vec2<double> compound scalar add x");
	test.check_equal(actual.y, 7.0, "vec2<double> compound scalar add y");

	actual = a;
	actual -= s;
	test.check_equal(actual.x, 3.0, "vec2<double> compound scalar subtract x");
	test.check_equal(actual.y, -11.0, "vec2<double> compound scalar subtract y");

	actual = b;
	actual *= s;
	test.check_equal(actual.x, 6.0, "vec2<double> compound scalar multiply x");
	test.check_equal(actual.y, 12.0, "vec2<double> compound scalar multiply y");

	actual = a;
	actual /= 2.0;
	test.check_equal(actual.x, 3.0, "vec2<double> compound scalar divide x");
	test.check_equal(actual.y, -4.0, "vec2<double> compound scalar divide y");

	actual = -a;
	test.check_equal(actual.x, -6.0, "vec2<double> unary minus x");
	test.check_equal(actual.y, 8.0, "vec2<double> unary minus y");

	// Equality is available for scalar component types.
	test.check(a == mt::vec2<double>(6.0, -8.0), "vec2<double> equality true");
	test.check(!(a == b), "vec2<double> equality false");

	// Dot product and length.
	test.check_equal(mt::dot(a, b), -20.0, "vec2<double> dot");
	test.check_equal(a.length(), 10.0, "vec2<double> length");
	test.check_equal(mt::length(a), 10.0, "vec2<double> free length");
	test.check_equal(mt::magnitude(a), 10.0, "vec2<double> free magnitude");
	test.check_equal(mt::distance(mt::vec2<double>(1.0, 2.0), mt::vec2<double>(4.0, 6.0)), 5.0, "vec2<double> distance");

	// Normalization returns a new unit-length vector.
	actual = a.normalize();
	// Normalize uses sqrt and division, so compare with tolerance.
	test.check_close(actual.x, 0.6, "vec2<double> normalize x");
	test.check_close(actual.y, -0.8, "vec2<double> normalize y");

	actual = mt::normalize(a);
	test.check_close(actual.x, 0.6, "vec2<double> free normalize x");
	test.check_close(actual.y, -0.8, "vec2<double> free normalize y");

	// Component-wise math functions.
	mt::vec2<double> c(-1.75, 2.25);
	actual = mt::floor(c);
	test.check_equal(actual.x, -2.0, "vec2<double> floor x");
	test.check_equal(actual.y, 2.0, "vec2<double> floor y");

	actual = mt::ceil(c);
	test.check_equal(actual.x, -1.0, "vec2<double> ceil x");
	test.check_equal(actual.y, 3.0, "vec2<double> ceil y");

	actual = mt::round(mt::vec2<double>(-1.5, 2.5));
	test.check_equal(actual.x, -2.0, "vec2<double> round x");
	test.check_equal(actual.y, 3.0, "vec2<double> round y");

	actual = mt::fract(c);
	test.check_equal(actual.x, 0.25, "vec2<double> fract x");
	test.check_equal(actual.y, 0.25, "vec2<double> fract y");

	actual = mt::trunc(c);
	test.check_equal(actual.x, -1.0, "vec2<double> trunc x");
	test.check_equal(actual.y, 2.0, "vec2<double> trunc y");

	actual = mt::abs(c);
	test.check_equal(actual.x, 1.75, "vec2<double> abs x");
	test.check_equal(actual.y, 2.25, "vec2<double> abs y");

	actual = mt::sqrt(mt::vec2<double>(9.0, 16.0));
	test.check_equal(actual.x, 3.0, "vec2<double> sqrt x");
	test.check_equal(actual.y, 4.0, "vec2<double> sqrt y");

	actual = mt::exp(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec2<double> exp x");
	test.check_equal(actual.y, 1.0, "vec2<double> exp y");

	actual = mt::exp2(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec2<double> exp2 x");
	test.check_equal(actual.y, 1.0, "vec2<double> exp2 y");

	actual = mt::log(mt::vec2<double>(1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec2<double> log x");
	test.check_equal(actual.y, 0.0, "vec2<double> log y");

	actual = mt::log2(mt::vec2<double>(1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec2<double> log2 x");
	test.check_equal(actual.y, 0.0, "vec2<double> log2 y");

	actual = mt::sin(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec2<double> sin x");
	test.check_equal(actual.y, 0.0, "vec2<double> sin y");

	actual = mt::cos(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 1.0, "vec2<double> cos x");
	test.check_equal(actual.y, 1.0, "vec2<double> cos y");

	actual = mt::tan(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec2<double> tan x");
	test.check_equal(actual.y, 0.0, "vec2<double> tan y");

	actual = mt::asin(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec2<double> asin x");
	test.check_equal(actual.y, 0.0, "vec2<double> asin y");

	actual = mt::acos(mt::vec2<double>(1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec2<double> acos x");
	test.check_equal(actual.y, 0.0, "vec2<double> acos y");

	actual = mt::atan(mt::vec2<double>(0.0, 0.0));
	test.check_equal(actual.x, 0.0, "vec2<double> atan x");
	test.check_equal(actual.y, 0.0, "vec2<double> atan y");

	actual = mt::atan2(mt::vec2<double>(0.0, 0.0), mt::vec2<double>(1.0, 1.0));
	test.check_equal(actual.x, 0.0, "vec2<double> atan2 x");
	test.check_equal(actual.y, 0.0, "vec2<double> atan2 y");

	// Component-wise min, max, clamp, power, and fused operations.
	actual = mt::min(mt::vec2<double>(1.0, 5.0), mt::vec2<double>(2.0, 4.0));
	test.check_equal(actual.x, 1.0, "vec2<double> min x");
	test.check_equal(actual.y, 4.0, "vec2<double> min y");

	actual = mt::min(mt::vec2<double>(1.0, 5.0), 3.0);
	test.check_equal(actual.x, 1.0, "vec2<double> min vector scalar x");
	test.check_equal(actual.y, 3.0, "vec2<double> min vector scalar y");

	actual = mt::max(mt::vec2<double>(1.0, 5.0), mt::vec2<double>(2.0, 4.0));
	test.check_equal(actual.x, 2.0, "vec2<double> max x");
	test.check_equal(actual.y, 5.0, "vec2<double> max y");

	actual = mt::max(3.0, mt::vec2<double>(1.0, 5.0));
	test.check_equal(actual.x, 3.0, "vec2<double> max scalar vector x");
	test.check_equal(actual.y, 5.0, "vec2<double> max scalar vector y");

	actual = mt::clamp(mt::vec2<double>(-0.5, 1.5));
	test.check_equal(actual.x, 0.0, "vec2<double> clamp 0..1 x");
	test.check_equal(actual.y, 1.0, "vec2<double> clamp 0..1 y");

	actual = mt::clamp(mt::vec2<double>(-2.0, 6.0), mt::vec2<double>(-1.0, 2.0), mt::vec2<double>(1.0, 4.0));
	test.check_equal(actual.x, -1.0, "vec2<double> clamp vector bounds x");
	test.check_equal(actual.y, 4.0, "vec2<double> clamp vector bounds y");

	actual = mt::pow(mt::vec2<double>(2.0, 4.0), mt::vec2<double>(3.0, 0.5));
	test.check_close(actual.x, 8.0, "vec2<double> pow vector x");
	test.check_close(actual.y, 2.0, "vec2<double> pow vector y");

	actual = mt::pow(mt::vec2<double>(2.0, 4.0), 2.0);
	test.check_close(actual.x, 4.0, "vec2<double> pow scalar x");
	test.check_close(actual.y, 16.0, "vec2<double> pow scalar y");

	actual = mt::fma(mt::vec2<double>(2.0, -3.0), mt::vec2<double>(4.0, 5.0), mt::vec2<double>(1.0, 2.0));
	test.check_equal(actual.x, 9.0, "vec2<double> fma x");
	test.check_equal(actual.y, -13.0, "vec2<double> fma y");

	actual = mt::fms(mt::vec2<double>(2.0, -3.0), mt::vec2<double>(4.0, 5.0), mt::vec2<double>(1.0, 2.0));
	test.check_equal(actual.x, 7.0, "vec2<double> fms x");
	test.check_equal(actual.y, -17.0, "vec2<double> fms y");

	actual = mt::fnma(mt::vec2<double>(2.0, -3.0), mt::vec2<double>(4.0, 5.0), mt::vec2<double>(1.0, 2.0));
	test.check_equal(actual.x, -7.0, "vec2<double> fnma x");
	test.check_equal(actual.y, 17.0, "vec2<double> fnma y");

	actual = mt::fnms(mt::vec2<double>(2.0, -3.0), mt::vec2<double>(4.0, 5.0), mt::vec2<double>(1.0, 2.0));
	test.check_equal(actual.x, -9.0, "vec2<double> fnms x");
	test.check_equal(actual.y, 13.0, "vec2<double> fnms y");

	// Wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec2<double>(-1.0, 7.0), 0.0, 5.0);
	test.check_equal(actual.x, 4.0, "vec2<double> wrap x");
	test.check_equal(actual.y, 2.0, "vec2<double> wrap y");

	actual = mt::rescale_from_01(mt::vec2<double>(0.25, 0.75), 10.0, 20.0);
	test.check_equal(actual.x, 12.5, "vec2<double> rescale_from_01 scalar x");
	test.check_equal(actual.y, 17.5, "vec2<double> rescale_from_01 scalar y");

	actual = mt::rescale_to_01(mt::vec2<double>(12.5, 17.5), 10.0, 20.0);
	test.check_equal(actual.x, 0.25, "vec2<double> rescale_to_01 scalar x");
	test.check_equal(actual.y, 0.75, "vec2<double> rescale_to_01 scalar y");

	actual = mt::rescale(mt::vec2<double>(5.0, 7.5), 0.0, 10.0, 20.0, 40.0);
	test.check_equal(actual.x, 30.0, "vec2<double> rescale scalar x");
	test.check_equal(actual.y, 35.0, "vec2<double> rescale scalar y");

	actual = mt::remap(mt::vec2<double>(5.0, 7.5), 0.0, 10.0, 20.0, 40.0);
	test.check_equal(actual.x, 30.0, "vec2<double> remap scalar x");
	test.check_equal(actual.y, 35.0, "vec2<double> remap scalar y");

	actual = mt::smoothstep(mt::vec2<double>(0.0, 0.0), mt::vec2<double>(10.0, 10.0), mt::vec2<double>(0.0, 5.0));
	test.check_equal(actual.x, 0.0, "vec2<double> smoothstep x");
	test.check_equal(actual.y, 0.5, "vec2<double> smoothstep y");

	actual = mt::step(mt::vec2<double>(0.5, 1.5), mt::vec2<double>(0.25, 2.0));
	test.check_equal(actual.x, 0.0, "vec2<double> step vector x");
	test.check_equal(actual.y, 1.0, "vec2<double> step vector y");

	actual = mt::step(0.5, mt::vec2<double>(0.25, 0.75));
	test.check_equal(actual.x, 0.0, "vec2<double> step scalar edge x");
	test.check_equal(actual.y, 1.0, "vec2<double> step scalar edge y");

	actual = mt::mix(mt::vec2<double>(0.0, 10.0), mt::vec2<double>(10.0, 20.0), 0.25);
	test.check_equal(actual.x, 2.5, "vec2<double> mix x");
	test.check_equal(actual.y, 12.5, "vec2<double> mix y");

	actual = mt::lerp(mt::vec2<double>(0.0, 10.0), mt::vec2<double>(10.0, 20.0), 0.5);
	test.check_equal(actual.x, 5.0, "vec2<double> lerp x");
	test.check_equal(actual.y, 15.0, "vec2<double> lerp y");

	actual = mt::reflect(mt::vec2<double>(1.0, -1.0), mt::vec2<double>(0.0, 1.0));
	test.check_equal(actual.x, 1.0, "vec2<double> reflect x");
	test.check_equal(actual.y, 1.0, "vec2<double> reflect y");

	actual = mt::refract(mt::vec2<double>(0.0, -1.0), mt::vec2<double>(0.0, 1.0), 1.0);
	test.check_equal(actual.x, 0.0, "vec2<double> refract x");
	test.check_equal(actual.y, -1.0, "vec2<double> refract y");

	actual = mt::refract(mt::vec2<double>(1.0, 0.0), mt::vec2<double>(0.0, 1.0), 2.0);
	test.check_equal(actual.x, 0.0, "vec2<double> refract total internal x");
	test.check_equal(actual.y, 0.0, "vec2<double> refract total internal y");

	// Randomized arithmetic checks compare to direct scalar expressions.
	std::mt19937 rng(0x20260513u);
	for (int i = 0; i < 1000; ++i) {
		double ax = random_double(rng, -1000.0, 1000.0);
		double ay = random_double(rng, -1000.0, 1000.0);
		double bx = safe_divisor(random_double(rng, -1000.0, 1000.0));
		double by = safe_divisor(random_double(rng, -1000.0, 1000.0));

		mt::vec2<double> left(ax, ay);
		mt::vec2<double> right(bx, by);

		actual = left + right;
		test.check_equal(actual.x, ax + bx, "vec2<double> random add x");
		test.check_equal(actual.y, ay + by, "vec2<double> random add y");

		actual = left / right;
		test.check_equal(actual.x, ax / bx, "vec2<double> random divide x");
		test.check_equal(actual.y, ay / by, "vec2<double> random divide y");
	}
}

int main() {
	TestContext test;

	test_vec2_double(test);

	return test.result();
}
