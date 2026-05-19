/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar vec2<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

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

void test_vec2_float(TestContext& test) {
	// Default constructor zero-initializes both components.
	mt::vec2<float> default_value;
	test.check_equal(default_value.x, 0.0f, "vec2<float> default x");
	test.check_equal(default_value.y, 0.0f, "vec2<float> default y");

	// Splat constructor copies one value to both components.
	mt::vec2<float> splat(3.0f);
	test.check_equal(splat.x, 3.0f, "vec2<float> splat x");
	test.check_equal(splat.y, 3.0f, "vec2<float> splat y");

	// Shared simple inputs for exact arithmetic checks.
	mt::vec2<float> a(6.0f, -8.0f);
	mt::vec2<float> b(2.0f, 4.0f);
	float s = 3.0f;

	// Vector plus vector.
	mt::vec2<float> actual = a + b;
	test.check_equal(actual.x, 8.0f, "vec2<float> add x");
	test.check_equal(actual.y, -4.0f, "vec2<float> add y");

	// Vector minus vector.
	actual = a - b;
	test.check_equal(actual.x, 4.0f, "vec2<float> subtract x");
	test.check_equal(actual.y, -12.0f, "vec2<float> subtract y");

	// Vector times vector.
	actual = a * b;
	test.check_equal(actual.x, 12.0f, "vec2<float> multiply x");
	test.check_equal(actual.y, -32.0f, "vec2<float> multiply y");

	// Vector divided by vector.
	actual = a / b;
	test.check_equal(actual.x, 3.0f, "vec2<float> divide x");
	test.check_equal(actual.y, -2.0f, "vec2<float> divide y");

	// Vector plus scalar.
	actual = a + s;
	test.check_equal(actual.x, 9.0f, "vec2<float> vector plus scalar x");
	test.check_equal(actual.y, -5.0f, "vec2<float> vector plus scalar y");

	// Scalar plus vector.
	actual = s + b;
	test.check_equal(actual.x, 5.0f, "vec2<float> scalar plus vector x");
	test.check_equal(actual.y, 7.0f, "vec2<float> scalar plus vector y");

	// Vector minus scalar.
	actual = a - s;
	test.check_equal(actual.x, 3.0f, "vec2<float> vector minus scalar x");
	test.check_equal(actual.y, -11.0f, "vec2<float> vector minus scalar y");

	// Scalar minus vector.
	actual = s - b;
	test.check_equal(actual.x, 1.0f, "vec2<float> scalar minus vector x");
	test.check_equal(actual.y, -1.0f, "vec2<float> scalar minus vector y");

	// Vector times scalar.
	actual = b * s;
	test.check_equal(actual.x, 6.0f, "vec2<float> vector multiply scalar x");
	test.check_equal(actual.y, 12.0f, "vec2<float> vector multiply scalar y");

	// Scalar times vector.
	actual = s * b;
	test.check_equal(actual.x, 6.0f, "vec2<float> scalar multiply vector x");
	test.check_equal(actual.y, 12.0f, "vec2<float> scalar multiply vector y");

	// Vector divided by scalar.
	actual = a / 2.0f;
	test.check_equal(actual.x, 3.0f, "vec2<float> vector divide scalar x");
	test.check_equal(actual.y, -4.0f, "vec2<float> vector divide scalar y");

	// Scalar divided by vector.
	actual = s / b;
	test.check_equal(actual.x, 1.5f, "vec2<float> scalar divide vector x");
	test.check_equal(actual.y, 0.75f, "vec2<float> scalar divide vector y");

	// Compound vector add mutates in place.
	actual = a;
	actual += b;
	test.check_equal(actual.x, 8.0f, "vec2<float> compound add x");
	test.check_equal(actual.y, -4.0f, "vec2<float> compound add y");

	// Compound scalar subtract mutates in place.
	actual = a;
	actual -= s;
	test.check_equal(actual.x, 3.0f, "vec2<float> compound scalar subtract x");
	test.check_equal(actual.y, -11.0f, "vec2<float> compound scalar subtract y");

	// Remaining compound assignments mutate in place.
	actual = a;
	actual -= b;
	test.check_equal(actual.x, 4.0f, "vec2<float> compound subtract x");
	test.check_equal(actual.y, -12.0f, "vec2<float> compound subtract y");

	actual = b;
	actual *= a;
	test.check_equal(actual.x, 12.0f, "vec2<float> compound multiply x");
	test.check_equal(actual.y, -32.0f, "vec2<float> compound multiply y");

	actual = a;
	actual /= b;
	test.check_equal(actual.x, 3.0f, "vec2<float> compound divide x");
	test.check_equal(actual.y, -2.0f, "vec2<float> compound divide y");

	actual = b;
	actual += s;
	test.check_equal(actual.x, 5.0f, "vec2<float> compound scalar add x");
	test.check_equal(actual.y, 7.0f, "vec2<float> compound scalar add y");

	actual = b;
	actual *= s;
	test.check_equal(actual.x, 6.0f, "vec2<float> compound scalar multiply x");
	test.check_equal(actual.y, 12.0f, "vec2<float> compound scalar multiply y");

	actual = a;
	actual /= 2.0f;
	test.check_equal(actual.x, 3.0f, "vec2<float> compound scalar divide x");
	test.check_equal(actual.y, -4.0f, "vec2<float> compound scalar divide y");

	// Unary minus flips every component.
	actual = -a;
	test.check_equal(actual.x, -6.0f, "vec2<float> unary minus x");
	test.check_equal(actual.y, 8.0f, "vec2<float> unary minus y");

	// Equality is available for scalar component types.
	test.check(a == mt::vec2<float>(6.0f, -8.0f), "vec2<float> equality true");
	test.check(!(a == b), "vec2<float> equality false");

	// Dot product reduces two components to one scalar.
	float dot_expected = 6.0f * 2.0f + -8.0f * 4.0f;
	test.check_equal(mt::dot(a, b), dot_expected, "vec2<float> dot");

	// Length and magnitude aliases.
	test.check_equal(a.length(), 10.0f, "vec2<float> member length");
	test.check_equal(mt::length(a), 10.0f, "vec2<float> free length");
	test.check_equal(mt::magnitude(a), 10.0f, "vec2<float> free magnitude");

	// Distance computes the length between two points.
	test.check_equal(mt::distance(mt::vec2<float>(1.0f, 2.0f), mt::vec2<float>(4.0f, 6.0f)), 5.0f, "vec2<float> distance");

	// Normalization returns a new unit-length vector.
	actual = a.normalize();
	// Normalize uses sqrt and division, so compare with tolerance.
	test.check_close(actual.x, 0.6f, "vec2<float> normalize x");
	test.check_close(actual.y, -0.8f, "vec2<float> normalize y");

	actual = mt::normalize(a);
	test.check_close(actual.x, 0.6f, "vec2<float> free normalize x");
	test.check_close(actual.y, -0.8f, "vec2<float> free normalize y");

	// Shared input for component-wise math functions.
	mt::vec2<float> c(-1.75f, 2.25f);

	// Component-wise floor.
	actual = mt::floor(c);
	test.check_equal(actual.x, -2.0f, "vec2<float> floor x");
	test.check_equal(actual.y, 2.0f, "vec2<float> floor y");

	// Component-wise ceiling.
	actual = mt::ceil(c);
	test.check_equal(actual.x, -1.0f, "vec2<float> ceil x");
	test.check_equal(actual.y, 3.0f, "vec2<float> ceil y");

	// Component-wise round.
	actual = mt::round(mt::vec2<float>(-1.5f, 2.5f));
	test.check_equal(actual.x, -2.0f, "vec2<float> round x");
	test.check_equal(actual.y, 3.0f, "vec2<float> round y");

	// Component-wise fractional part.
	actual = mt::fract(c);
	test.check_equal(actual.x, 0.25f, "vec2<float> fract x");
	test.check_equal(actual.y, 0.25f, "vec2<float> fract y");

	// Component-wise truncation.
	actual = mt::trunc(c);
	test.check_equal(actual.x, -1.0f, "vec2<float> trunc x");
	test.check_equal(actual.y, 2.0f, "vec2<float> trunc y");

	// Component-wise absolute value.
	actual = mt::abs(c);
	test.check_equal(actual.x, 1.75f, "vec2<float> abs x");
	test.check_equal(actual.y, 2.25f, "vec2<float> abs y");

	// Component-wise square root.
	actual = mt::sqrt(mt::vec2<float>(9.0f, 16.0f));
	test.check_equal(actual.x, 3.0f, "vec2<float> sqrt x");
	test.check_equal(actual.y, 4.0f, "vec2<float> sqrt y");

	// Component-wise exponentials and logarithms.
	actual = mt::exp(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec2<float> exp x");
	test.check_equal(actual.y, 1.0f, "vec2<float> exp y");

	actual = mt::exp2(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec2<float> exp2 x");
	test.check_equal(actual.y, 1.0f, "vec2<float> exp2 y");

	actual = mt::log(mt::vec2<float>(1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> log x");
	test.check_equal(actual.y, 0.0f, "vec2<float> log y");

	actual = mt::log2(mt::vec2<float>(1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> log2 x");
	test.check_equal(actual.y, 0.0f, "vec2<float> log2 y");

	// Angle conversion functions convert between degrees and radians.
	actual = mt::radians(mt::vec2<float>(180.0f, 90.0f));
	test.check_close(actual.x, 3.14159265358979323846f, "vec2<float> radians x");
	test.check_close(actual.y, 1.57079632679489661923f, "vec2<float> radians y");

	actual = mt::degrees(mt::vec2<float>(3.14159265358979323846f, 1.57079632679489661923f));
	test.check_close(actual.x, 180.0f, "vec2<float> degrees x");
	test.check_close(actual.y, 90.0f, "vec2<float> degrees y");

	const mt::vec2<float> member_degrees(180.0f, 90.0f);
	actual = member_degrees.radians();
	test.check_close(actual.x, 3.14159265358979323846f, "vec2<float> member radians x");
	test.check_close(actual.y, 1.57079632679489661923f, "vec2<float> member radians y");

	const mt::vec2<float> member_radians(3.14159265358979323846f, 1.57079632679489661923f);
	actual = member_radians.degrees();
	test.check_close(actual.x, 180.0f, "vec2<float> member degrees x");
	test.check_close(actual.y, 90.0f, "vec2<float> member degrees y");

	// Component-wise trigonometry.
	actual = mt::sin(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> sin x");
	test.check_equal(actual.y, 0.0f, "vec2<float> sin y");

	actual = mt::cos(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec2<float> cos x");
	test.check_equal(actual.y, 1.0f, "vec2<float> cos y");

	actual = mt::tan(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> tan x");
	test.check_equal(actual.y, 0.0f, "vec2<float> tan y");

	actual = mt::asin(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> asin x");
	test.check_equal(actual.y, 0.0f, "vec2<float> asin y");

	actual = mt::acos(mt::vec2<float>(1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> acos x");
	test.check_equal(actual.y, 0.0f, "vec2<float> acos y");

	actual = mt::atan(mt::vec2<float>(0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> atan x");
	test.check_equal(actual.y, 0.0f, "vec2<float> atan y");

	actual = mt::atan2(mt::vec2<float>(0.0f, 0.0f), mt::vec2<float>(1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> atan2 x");
	test.check_equal(actual.y, 0.0f, "vec2<float> atan2 y");

	// Component-wise minimum.
	actual = mt::min(mt::vec2<float>(1.0f, 5.0f), mt::vec2<float>(2.0f, 4.0f));
	test.check_equal(actual.x, 1.0f, "vec2<float> min x");
	test.check_equal(actual.y, 4.0f, "vec2<float> min y");

	actual = mt::min(mt::vec2<float>(1.0f, 5.0f), 3.0f);
	test.check_equal(actual.x, 1.0f, "vec2<float> min vector scalar x");
	test.check_equal(actual.y, 3.0f, "vec2<float> min vector scalar y");

	actual = mt::min(3.0f, mt::vec2<float>(1.0f, 5.0f));
	test.check_equal(actual.x, 1.0f, "vec2<float> min scalar vector x");
	test.check_equal(actual.y, 3.0f, "vec2<float> min scalar vector y");

	// Component-wise maximum.
	actual = mt::max(mt::vec2<float>(1.0f, 5.0f), mt::vec2<float>(2.0f, 4.0f));
	test.check_equal(actual.x, 2.0f, "vec2<float> max x");
	test.check_equal(actual.y, 5.0f, "vec2<float> max y");

	actual = mt::max(mt::vec2<float>(1.0f, 5.0f), 3.0f);
	test.check_equal(actual.x, 3.0f, "vec2<float> max vector scalar x");
	test.check_equal(actual.y, 5.0f, "vec2<float> max vector scalar y");

	actual = mt::max(3.0f, mt::vec2<float>(1.0f, 5.0f));
	test.check_equal(actual.x, 3.0f, "vec2<float> max scalar vector x");
	test.check_equal(actual.y, 5.0f, "vec2<float> max scalar vector y");

	// Component-wise power.
	actual = mt::pow(mt::vec2<float>(2.0f, 4.0f), mt::vec2<float>(3.0f, 0.5f));
	test.check_close(actual.x, 8.0f, "vec2<float> pow vector x");
	test.check_close(actual.y, 2.0f, "vec2<float> pow vector y");

	actual = mt::pow(mt::vec2<float>(2.0f, 4.0f), 2.0f);
	test.check_close(actual.x, 4.0f, "vec2<float> pow scalar x");
	test.check_close(actual.y, 16.0f, "vec2<float> pow scalar y");

	// Component-wise fused operations.
	actual = mt::fma(mt::vec2<float>(2.0f, -3.0f), mt::vec2<float>(4.0f, 5.0f), mt::vec2<float>(1.0f, 2.0f));
	test.check_equal(actual.x, 9.0f, "vec2<float> fma x");
	test.check_equal(actual.y, -13.0f, "vec2<float> fma y");

	actual = mt::fms(mt::vec2<float>(2.0f, -3.0f), mt::vec2<float>(4.0f, 5.0f), mt::vec2<float>(1.0f, 2.0f));
	test.check_equal(actual.x, 7.0f, "vec2<float> fms x");
	test.check_equal(actual.y, -17.0f, "vec2<float> fms y");

	actual = mt::fnma(mt::vec2<float>(2.0f, -3.0f), mt::vec2<float>(4.0f, 5.0f), mt::vec2<float>(1.0f, 2.0f));
	test.check_equal(actual.x, -7.0f, "vec2<float> fnma x");
	test.check_equal(actual.y, 17.0f, "vec2<float> fnma y");

	actual = mt::fnms(mt::vec2<float>(2.0f, -3.0f), mt::vec2<float>(4.0f, 5.0f), mt::vec2<float>(1.0f, 2.0f));
	test.check_equal(actual.x, -9.0f, "vec2<float> fnms x");
	test.check_equal(actual.y, 13.0f, "vec2<float> fnms y");

	// Clamp to the default 0..1 range.
	actual = mt::clamp(mt::vec2<float>(-0.5f, 1.5f));
	test.check_equal(actual.x, 0.0f, "vec2<float> clamp 0..1 x");
	test.check_equal(actual.y, 1.0f, "vec2<float> clamp 0..1 y");

	// Clamp using scalar bounds.
	actual = mt::clamp(mt::vec2<float>(-2.0f, 6.0f), -1.0f, 4.0f);
	test.check_equal(actual.x, -1.0f, "vec2<float> clamp scalar bounds x");
	test.check_equal(actual.y, 4.0f, "vec2<float> clamp scalar bounds y");

	// Clamp using vector bounds.
	actual = mt::clamp(mt::vec2<float>(-2.0f, 6.0f), mt::vec2<float>(-1.0f, 2.0f), mt::vec2<float>(1.0f, 4.0f));
	test.check_equal(actual.x, -1.0f, "vec2<float> clamp vector bounds x");
	test.check_equal(actual.y, 4.0f, "vec2<float> clamp vector bounds y");

	// Wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec2<float>(-1.0f, 7.0f), 0.0f, 5.0f);
	test.check_equal(actual.x, 4.0f, "vec2<float> wrap x");
	test.check_equal(actual.y, 2.0f, "vec2<float> wrap y");

	// Range helpers work with vector bounds.
	actual = mt::rescale_from_01(mt::vec2<float>(0.25f, 0.75f), mt::vec2<float>(10.0f, 20.0f), mt::vec2<float>(20.0f, 40.0f));
	test.check_equal(actual.x, 12.5f, "vec2<float> rescale_from_01 x");
	test.check_equal(actual.y, 35.0f, "vec2<float> rescale_from_01 y");

	actual = mt::rescale_to_01(mt::vec2<float>(12.5f, 35.0f), mt::vec2<float>(10.0f, 20.0f), mt::vec2<float>(20.0f, 40.0f));
	test.check_equal(actual.x, 0.25f, "vec2<float> rescale_to_01 x");
	test.check_equal(actual.y, 0.75f, "vec2<float> rescale_to_01 y");

	actual = mt::rescale(mt::vec2<float>(5.0f, 6.0f), mt::vec2<float>(0.0f, 0.0f), mt::vec2<float>(10.0f, 12.0f), mt::vec2<float>(20.0f, 30.0f), mt::vec2<float>(40.0f, 50.0f));
	test.check_equal(actual.x, 30.0f, "vec2<float> rescale x");
	test.check_equal(actual.y, 40.0f, "vec2<float> rescale y");

	actual = mt::remap(mt::vec2<float>(5.0f, 6.0f), mt::vec2<float>(0.0f, 0.0f), mt::vec2<float>(10.0f, 12.0f), mt::vec2<float>(20.0f, 30.0f), mt::vec2<float>(40.0f, 50.0f));
	test.check_equal(actual.x, 30.0f, "vec2<float> remap x");
	test.check_equal(actual.y, 40.0f, "vec2<float> remap y");

	// Range helpers also accept scalar bounds.
	actual = mt::rescale_from_01(mt::vec2<float>(0.25f, 0.75f), 10.0f, 20.0f);
	test.check_equal(actual.x, 12.5f, "vec2<float> rescale_from_01 scalar x");
	test.check_equal(actual.y, 17.5f, "vec2<float> rescale_from_01 scalar y");

	actual = mt::rescale_to_01(mt::vec2<float>(12.5f, 17.5f), 10.0f, 20.0f);
	test.check_equal(actual.x, 0.25f, "vec2<float> rescale_to_01 scalar x");
	test.check_equal(actual.y, 0.75f, "vec2<float> rescale_to_01 scalar y");

	actual = mt::rescale(mt::vec2<float>(5.0f, 7.5f), 0.0f, 10.0f, 20.0f, 40.0f);
	test.check_equal(actual.x, 30.0f, "vec2<float> rescale scalar x");
	test.check_equal(actual.y, 35.0f, "vec2<float> rescale scalar y");

	actual = mt::remap(mt::vec2<float>(5.0f, 7.5f), 0.0f, 10.0f, 20.0f, 40.0f);
	test.check_equal(actual.x, 30.0f, "vec2<float> remap scalar x");
	test.check_equal(actual.y, 35.0f, "vec2<float> remap scalar y");

	actual = mt::smoothstep(mt::vec2<float>(0.0f, 0.0f), mt::vec2<float>(10.0f, 10.0f), mt::vec2<float>(0.0f, 5.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> smoothstep x");
	test.check_equal(actual.y, 0.5f, "vec2<float> smoothstep y");

	actual = mt::step(mt::vec2<float>(0.5f, 1.5f), mt::vec2<float>(0.25f, 2.0f));
	test.check_equal(actual.x, 0.0f, "vec2<float> step vector x");
	test.check_equal(actual.y, 1.0f, "vec2<float> step vector y");

	actual = mt::step(0.5f, mt::vec2<float>(0.25f, 0.75f));
	test.check_equal(actual.x, 0.0f, "vec2<float> step scalar edge x");
	test.check_equal(actual.y, 1.0f, "vec2<float> step scalar edge y");

	actual = mt::step(mt::vec2<float>(0.5f, 1.5f), 1.0f);
	test.check_equal(actual.x, 1.0f, "vec2<float> step scalar value x");
	test.check_equal(actual.y, 0.0f, "vec2<float> step scalar value y");

	actual = mt::mix(mt::vec2<float>(0.0f, 10.0f), mt::vec2<float>(10.0f, 20.0f), 0.25f);
	test.check_equal(actual.x, 2.5f, "vec2<float> mix x");
	test.check_equal(actual.y, 12.5f, "vec2<float> mix y");

	actual = mt::lerp(mt::vec2<float>(0.0f, 10.0f), mt::vec2<float>(10.0f, 20.0f), 0.5f);
	test.check_equal(actual.x, 5.0f, "vec2<float> lerp x");
	test.check_equal(actual.y, 15.0f, "vec2<float> lerp y");

	// Reflect incident vector around a normalized normal.
	actual = mt::reflect(mt::vec2<float>(1.0f, -1.0f), mt::vec2<float>(0.0f, 1.0f));
	test.check_equal(actual.x, 1.0f, "vec2<float> reflect x");
	test.check_equal(actual.y, 1.0f, "vec2<float> reflect y");

	// Refract with eta 1.0 leaves this incident vector unchanged.
	actual = mt::refract(mt::vec2<float>(0.0f, -1.0f), mt::vec2<float>(0.0f, 1.0f), 1.0f);
	test.check_equal(actual.x, 0.0f, "vec2<float> refract x");
	test.check_equal(actual.y, -1.0f, "vec2<float> refract y");

	// Refract returns zero when total internal reflection happens.
	actual = mt::refract(mt::vec2<float>(1.0f, 0.0f), mt::vec2<float>(0.0f, 1.0f), 2.0f);
	test.check_equal(actual.x, 0.0f, "vec2<float> refract total internal x");
	test.check_equal(actual.y, 0.0f, "vec2<float> refract total internal y");

	// Randomized arithmetic checks compare to direct scalar expressions.
	std::mt19937 rng(0x20260512u);
	for (int i = 0; i < 1000; ++i) {
		float ax = random_float(rng, -1000.0f, 1000.0f);
		float ay = random_float(rng, -1000.0f, 1000.0f);
		float bx = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float by = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float scalar = safe_divisor(random_float(rng, -1000.0f, 1000.0f));

		mt::vec2<float> left(ax, ay);
		mt::vec2<float> right(bx, by);

		actual = left + right;
		test.check_equal(actual.x, ax + bx, "vec2<float> random add x");
		test.check_equal(actual.y, ay + by, "vec2<float> random add y");

		actual = left - right;
		test.check_equal(actual.x, ax - bx, "vec2<float> random subtract x");
		test.check_equal(actual.y, ay - by, "vec2<float> random subtract y");

		actual = left * right;
		test.check_equal(actual.x, ax * bx, "vec2<float> random multiply x");
		test.check_equal(actual.y, ay * by, "vec2<float> random multiply y");

		actual = left / right;
		test.check_equal(actual.x, ax / bx, "vec2<float> random divide x");
		test.check_equal(actual.y, ay / by, "vec2<float> random divide y");

		actual = left + scalar;
		test.check_equal(actual.x, ax + scalar, "vec2<float> random vector scalar add x");
		test.check_equal(actual.y, ay + scalar, "vec2<float> random vector scalar add y");
	}
}

int main() {
	TestContext test;

	test_vec2_float(test);

	return test.result();
}
