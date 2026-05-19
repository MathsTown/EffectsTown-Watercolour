/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar vec3<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

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

void test_vec3_float(TestContext& test) {
	// Default constructor zero-initializes all components.
	mt::vec3<float> default_value;
	test.check_equal(default_value.x, 0.0f, "vec3<float> default x");
	test.check_equal(default_value.y, 0.0f, "vec3<float> default y");
	test.check_equal(default_value.z, 0.0f, "vec3<float> default z");

	// Splat constructor copies one value to all components.
	mt::vec3<float> splat(3.0f);
	test.check_equal(splat.x, 3.0f, "vec3<float> splat x");
	test.check_equal(splat.y, 3.0f, "vec3<float> splat y");
	test.check_equal(splat.z, 3.0f, "vec3<float> splat z");

	// Constructor can append one component to a vec2.
	mt::vec3<float> from_xy(mt::vec2<float>(1.0f, 2.0f), 3.0f);
	test.check_equal(from_xy.x, 1.0f, "vec3<float> vec2,z constructor x");
	test.check_equal(from_xy.y, 2.0f, "vec3<float> vec2,z constructor y");
	test.check_equal(from_xy.z, 3.0f, "vec3<float> vec2,z constructor z");

	// Constructor can prepend one component before a vec2.
	mt::vec3<float> from_yz(1.0f, mt::vec2<float>(2.0f, 3.0f));
	test.check_equal(from_yz.x, 1.0f, "vec3<float> x,vec2 constructor x");
	test.check_equal(from_yz.y, 2.0f, "vec3<float> x,vec2 constructor y");
	test.check_equal(from_yz.z, 3.0f, "vec3<float> x,vec2 constructor z");

	// Shared simple inputs for exact arithmetic checks.
	mt::vec3<float> a(6.0f, -8.0f, 10.0f);
	mt::vec3<float> b(2.0f, 4.0f, 5.0f);
	float s = 3.0f;

	// Vector plus vector.
	mt::vec3<float> actual = a + b;
	test.check_equal(actual.x, 8.0f, "vec3<float> add x");
	test.check_equal(actual.y, -4.0f, "vec3<float> add y");
	test.check_equal(actual.z, 15.0f, "vec3<float> add z");

	// Vector minus vector.
	actual = a - b;
	test.check_equal(actual.x, 4.0f, "vec3<float> subtract x");
	test.check_equal(actual.y, -12.0f, "vec3<float> subtract y");
	test.check_equal(actual.z, 5.0f, "vec3<float> subtract z");

	// Vector times vector.
	actual = a * b;
	test.check_equal(actual.x, 12.0f, "vec3<float> multiply x");
	test.check_equal(actual.y, -32.0f, "vec3<float> multiply y");
	test.check_equal(actual.z, 50.0f, "vec3<float> multiply z");

	// Vector divided by vector.
	actual = a / b;
	test.check_equal(actual.x, 3.0f, "vec3<float> divide x");
	test.check_equal(actual.y, -2.0f, "vec3<float> divide y");
	test.check_equal(actual.z, 2.0f, "vec3<float> divide z");

	// Vector plus scalar.
	actual = a + s;
	test.check_equal(actual.x, 9.0f, "vec3<float> vector plus scalar x");
	test.check_equal(actual.y, -5.0f, "vec3<float> vector plus scalar y");
	test.check_equal(actual.z, 13.0f, "vec3<float> vector plus scalar z");

	// Scalar plus vector.
	actual = s + b;
	test.check_equal(actual.x, 5.0f, "vec3<float> scalar plus vector x");
	test.check_equal(actual.y, 7.0f, "vec3<float> scalar plus vector y");
	test.check_equal(actual.z, 8.0f, "vec3<float> scalar plus vector z");

	// Vector minus scalar.
	actual = a - s;
	test.check_equal(actual.x, 3.0f, "vec3<float> vector minus scalar x");
	test.check_equal(actual.y, -11.0f, "vec3<float> vector minus scalar y");
	test.check_equal(actual.z, 7.0f, "vec3<float> vector minus scalar z");

	// Scalar minus vector.
	actual = s - b;
	test.check_equal(actual.x, 1.0f, "vec3<float> scalar minus vector x");
	test.check_equal(actual.y, -1.0f, "vec3<float> scalar minus vector y");
	test.check_equal(actual.z, -2.0f, "vec3<float> scalar minus vector z");

	// Vector times scalar.
	actual = b * s;
	test.check_equal(actual.x, 6.0f, "vec3<float> vector multiply scalar x");
	test.check_equal(actual.y, 12.0f, "vec3<float> vector multiply scalar y");
	test.check_equal(actual.z, 15.0f, "vec3<float> vector multiply scalar z");

	// Scalar times vector.
	actual = s * b;
	test.check_equal(actual.x, 6.0f, "vec3<float> scalar multiply vector x");
	test.check_equal(actual.y, 12.0f, "vec3<float> scalar multiply vector y");
	test.check_equal(actual.z, 15.0f, "vec3<float> scalar multiply vector z");

	// Vector divided by scalar.
	actual = a / 2.0f;
	test.check_equal(actual.x, 3.0f, "vec3<float> vector divide scalar x");
	test.check_equal(actual.y, -4.0f, "vec3<float> vector divide scalar y");
	test.check_equal(actual.z, 5.0f, "vec3<float> vector divide scalar z");

	// Scalar divided by vector.
	actual = s / b;
	test.check_equal(actual.x, 1.5f, "vec3<float> scalar divide vector x");
	test.check_equal(actual.y, 0.75f, "vec3<float> scalar divide vector y");
	test.check_equal(actual.z, 0.6f, "vec3<float> scalar divide vector z");

	// Compound vector add mutates in place.
	actual = a;
	actual += b;
	test.check_equal(actual.x, 8.0f, "vec3<float> compound add x");
	test.check_equal(actual.y, -4.0f, "vec3<float> compound add y");
	test.check_equal(actual.z, 15.0f, "vec3<float> compound add z");

	// Compound scalar subtract mutates in place.
	actual = a;
	actual -= s;
	test.check_equal(actual.x, 3.0f, "vec3<float> compound scalar subtract x");
	test.check_equal(actual.y, -11.0f, "vec3<float> compound scalar subtract y");
	test.check_equal(actual.z, 7.0f, "vec3<float> compound scalar subtract z");

	// Remaining compound assignments mutate in place.
	actual = a;
	actual -= b;
	test.check_equal(actual.x, 4.0f, "vec3<float> compound subtract x");
	test.check_equal(actual.y, -12.0f, "vec3<float> compound subtract y");
	test.check_equal(actual.z, 5.0f, "vec3<float> compound subtract z");

	actual = b;
	actual *= a;
	test.check_equal(actual.x, 12.0f, "vec3<float> compound multiply x");
	test.check_equal(actual.y, -32.0f, "vec3<float> compound multiply y");
	test.check_equal(actual.z, 50.0f, "vec3<float> compound multiply z");

	actual = a;
	actual /= b;
	test.check_equal(actual.x, 3.0f, "vec3<float> compound divide x");
	test.check_equal(actual.y, -2.0f, "vec3<float> compound divide y");
	test.check_equal(actual.z, 2.0f, "vec3<float> compound divide z");

	actual = b;
	actual += s;
	test.check_equal(actual.x, 5.0f, "vec3<float> compound scalar add x");
	test.check_equal(actual.y, 7.0f, "vec3<float> compound scalar add y");
	test.check_equal(actual.z, 8.0f, "vec3<float> compound scalar add z");

	actual = b;
	actual *= s;
	test.check_equal(actual.x, 6.0f, "vec3<float> compound scalar multiply x");
	test.check_equal(actual.y, 12.0f, "vec3<float> compound scalar multiply y");
	test.check_equal(actual.z, 15.0f, "vec3<float> compound scalar multiply z");

	actual = a;
	actual /= 2.0f;
	test.check_equal(actual.x, 3.0f, "vec3<float> compound scalar divide x");
	test.check_equal(actual.y, -4.0f, "vec3<float> compound scalar divide y");
	test.check_equal(actual.z, 5.0f, "vec3<float> compound scalar divide z");

	// Unary minus flips every component.
	actual = -a;
	test.check_equal(actual.x, -6.0f, "vec3<float> unary minus x");
	test.check_equal(actual.y, 8.0f, "vec3<float> unary minus y");
	test.check_equal(actual.z, -10.0f, "vec3<float> unary minus z");

	// Equality is available for scalar component types.
	test.check(a == mt::vec3<float>(6.0f, -8.0f, 10.0f), "vec3<float> equality true");
	test.check(!(a == b), "vec3<float> equality false");

	// Swizzles reduce vec3 to vec2 in component order.
	mt::vec2<float> swizzle = a.xy();
	test.check_equal(swizzle.x, 6.0f, "vec3<float> xy x");
	test.check_equal(swizzle.y, -8.0f, "vec3<float> xy y");
	swizzle = a.yz();
	test.check_equal(swizzle.x, -8.0f, "vec3<float> yz x");
	test.check_equal(swizzle.y, 10.0f, "vec3<float> yz y");
	swizzle = a.xz();
	test.check_equal(swizzle.x, 6.0f, "vec3<float> xz x");
	test.check_equal(swizzle.y, 10.0f, "vec3<float> xz y");

	// Dot product reduces three components to one scalar.
	float dot_expected = 6.0f * 2.0f + -8.0f * 4.0f + 10.0f * 5.0f;
	test.check_equal(mt::dot(a, b), dot_expected, "vec3<float> dot");

	// Cross product follows the right-hand rule.
	actual = mt::cross(mt::vec3<float>(1.0f, 0.0f, 0.0f), mt::vec3<float>(0.0f, 1.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> cross x");
	test.check_equal(actual.y, 0.0f, "vec3<float> cross y");
	test.check_equal(actual.z, 1.0f, "vec3<float> cross z");

	// Length and magnitude aliases.
	mt::vec3<float> length_input(3.0f, 4.0f, 12.0f);
	test.check_equal(length_input.length(), 13.0f, "vec3<float> member length");
	test.check_equal(mt::length(length_input), 13.0f, "vec3<float> free length");
	test.check_equal(mt::magnitude(length_input), 13.0f, "vec3<float> free magnitude");

	// Distance computes the length between two points.
	test.check_equal(mt::distance(mt::vec3<float>(1.0f, 2.0f, 3.0f), mt::vec3<float>(4.0f, 6.0f, 15.0f)), 13.0f, "vec3<float> distance");

	// Normalization returns a new unit-length vector.
	actual = length_input.normalize();
	// Normalize uses sqrt and division, so compare with tolerance.
	test.check_close(actual.x, 3.0f / 13.0f, "vec3<float> normalize x");
	test.check_close(actual.y, 4.0f / 13.0f, "vec3<float> normalize y");
	test.check_close(actual.z, 12.0f / 13.0f, "vec3<float> normalize z");

	actual = mt::normalize(length_input);
	test.check_close(actual.x, 3.0f / 13.0f, "vec3<float> free normalize x");
	test.check_close(actual.y, 4.0f / 13.0f, "vec3<float> free normalize y");
	test.check_close(actual.z, 12.0f / 13.0f, "vec3<float> free normalize z");

	// Shared input for component-wise math functions.
	mt::vec3<float> c(-1.75f, 2.25f, 3.5f);

	// Component-wise floor.
	actual = mt::floor(c);
	test.check_equal(actual.x, -2.0f, "vec3<float> floor x");
	test.check_equal(actual.y, 2.0f, "vec3<float> floor y");
	test.check_equal(actual.z, 3.0f, "vec3<float> floor z");

	// Component-wise ceiling.
	actual = mt::ceil(c);
	test.check_equal(actual.x, -1.0f, "vec3<float> ceil x");
	test.check_equal(actual.y, 3.0f, "vec3<float> ceil y");
	test.check_equal(actual.z, 4.0f, "vec3<float> ceil z");

	// Component-wise round.
	actual = mt::round(mt::vec3<float>(-1.5f, 2.5f, 3.5f));
	test.check_equal(actual.x, -2.0f, "vec3<float> round x");
	test.check_equal(actual.y, 3.0f, "vec3<float> round y");
	test.check_equal(actual.z, 4.0f, "vec3<float> round z");

	// Component-wise fractional part.
	actual = mt::fract(c);
	test.check_equal(actual.x, 0.25f, "vec3<float> fract x");
	test.check_equal(actual.y, 0.25f, "vec3<float> fract y");
	test.check_equal(actual.z, 0.5f, "vec3<float> fract z");

	// Component-wise truncation.
	actual = mt::trunc(c);
	test.check_equal(actual.x, -1.0f, "vec3<float> trunc x");
	test.check_equal(actual.y, 2.0f, "vec3<float> trunc y");
	test.check_equal(actual.z, 3.0f, "vec3<float> trunc z");

	// Component-wise absolute value.
	actual = mt::abs(c);
	test.check_equal(actual.x, 1.75f, "vec3<float> abs x");
	test.check_equal(actual.y, 2.25f, "vec3<float> abs y");
	test.check_equal(actual.z, 3.5f, "vec3<float> abs z");

	// Component-wise square root.
	actual = mt::sqrt(mt::vec3<float>(9.0f, 16.0f, 25.0f));
	test.check_equal(actual.x, 3.0f, "vec3<float> sqrt x");
	test.check_equal(actual.y, 4.0f, "vec3<float> sqrt y");
	test.check_equal(actual.z, 5.0f, "vec3<float> sqrt z");

	// Component-wise exponentials and logarithms.
	actual = mt::exp(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec3<float> exp x");
	test.check_equal(actual.y, 1.0f, "vec3<float> exp y");
	test.check_equal(actual.z, 1.0f, "vec3<float> exp z");

	actual = mt::exp2(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec3<float> exp2 x");
	test.check_equal(actual.y, 1.0f, "vec3<float> exp2 y");
	test.check_equal(actual.z, 1.0f, "vec3<float> exp2 z");

	actual = mt::log(mt::vec3<float>(1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> log x");
	test.check_equal(actual.y, 0.0f, "vec3<float> log y");
	test.check_equal(actual.z, 0.0f, "vec3<float> log z");

	actual = mt::log2(mt::vec3<float>(1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> log2 x");
	test.check_equal(actual.y, 0.0f, "vec3<float> log2 y");
	test.check_equal(actual.z, 0.0f, "vec3<float> log2 z");

	// Angle conversion functions convert between degrees and radians.
	actual = mt::radians(mt::vec3<float>(180.0f, 90.0f, 45.0f));
	test.check_close(actual.x, 3.14159265358979323846f, "vec3<float> radians x");
	test.check_close(actual.y, 1.57079632679489661923f, "vec3<float> radians y");
	test.check_close(actual.z, 0.78539816339744830962f, "vec3<float> radians z");

	actual = mt::degrees(mt::vec3<float>(3.14159265358979323846f, 1.57079632679489661923f, 0.78539816339744830962f));
	test.check_close(actual.x, 180.0f, "vec3<float> degrees x");
	test.check_close(actual.y, 90.0f, "vec3<float> degrees y");
	test.check_close(actual.z, 45.0f, "vec3<float> degrees z");

	const mt::vec3<float> member_degrees(180.0f, 90.0f, 45.0f);
	actual = member_degrees.radians();
	test.check_close(actual.x, 3.14159265358979323846f, "vec3<float> member radians x");
	test.check_close(actual.y, 1.57079632679489661923f, "vec3<float> member radians y");
	test.check_close(actual.z, 0.78539816339744830962f, "vec3<float> member radians z");

	const mt::vec3<float> member_radians(3.14159265358979323846f, 1.57079632679489661923f, 0.78539816339744830962f);
	actual = member_radians.degrees();
	test.check_close(actual.x, 180.0f, "vec3<float> member degrees x");
	test.check_close(actual.y, 90.0f, "vec3<float> member degrees y");
	test.check_close(actual.z, 45.0f, "vec3<float> member degrees z");

	// Component-wise trigonometry.
	actual = mt::sin(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> sin x");
	test.check_equal(actual.y, 0.0f, "vec3<float> sin y");
	test.check_equal(actual.z, 0.0f, "vec3<float> sin z");

	actual = mt::cos(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec3<float> cos x");
	test.check_equal(actual.y, 1.0f, "vec3<float> cos y");
	test.check_equal(actual.z, 1.0f, "vec3<float> cos z");

	actual = mt::tan(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> tan x");
	test.check_equal(actual.y, 0.0f, "vec3<float> tan y");
	test.check_equal(actual.z, 0.0f, "vec3<float> tan z");

	actual = mt::asin(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> asin x");
	test.check_equal(actual.y, 0.0f, "vec3<float> asin y");
	test.check_equal(actual.z, 0.0f, "vec3<float> asin z");

	actual = mt::acos(mt::vec3<float>(1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> acos x");
	test.check_equal(actual.y, 0.0f, "vec3<float> acos y");
	test.check_equal(actual.z, 0.0f, "vec3<float> acos z");

	actual = mt::atan(mt::vec3<float>(0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> atan x");
	test.check_equal(actual.y, 0.0f, "vec3<float> atan y");
	test.check_equal(actual.z, 0.0f, "vec3<float> atan z");

	actual = mt::atan2(mt::vec3<float>(0.0f, 0.0f, 0.0f), mt::vec3<float>(1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> atan2 x");
	test.check_equal(actual.y, 0.0f, "vec3<float> atan2 y");
	test.check_equal(actual.z, 0.0f, "vec3<float> atan2 z");

	// Component-wise minimum.
	actual = mt::min(mt::vec3<float>(1.0f, 5.0f, 9.0f), mt::vec3<float>(2.0f, 4.0f, 10.0f));
	test.check_equal(actual.x, 1.0f, "vec3<float> min x");
	test.check_equal(actual.y, 4.0f, "vec3<float> min y");
	test.check_equal(actual.z, 9.0f, "vec3<float> min z");

	actual = mt::min(mt::vec3<float>(1.0f, 5.0f, 9.0f), 3.0f);
	test.check_equal(actual.x, 1.0f, "vec3<float> min vector scalar x");
	test.check_equal(actual.y, 3.0f, "vec3<float> min vector scalar y");
	test.check_equal(actual.z, 3.0f, "vec3<float> min vector scalar z");

	actual = mt::min(3.0f, mt::vec3<float>(1.0f, 5.0f, 9.0f));
	test.check_equal(actual.x, 1.0f, "vec3<float> min scalar vector x");
	test.check_equal(actual.y, 3.0f, "vec3<float> min scalar vector y");
	test.check_equal(actual.z, 3.0f, "vec3<float> min scalar vector z");

	// Component-wise maximum.
	actual = mt::max(mt::vec3<float>(1.0f, 5.0f, 9.0f), mt::vec3<float>(2.0f, 4.0f, 10.0f));
	test.check_equal(actual.x, 2.0f, "vec3<float> max x");
	test.check_equal(actual.y, 5.0f, "vec3<float> max y");
	test.check_equal(actual.z, 10.0f, "vec3<float> max z");

	actual = mt::max(mt::vec3<float>(1.0f, 5.0f, 9.0f), 3.0f);
	test.check_equal(actual.x, 3.0f, "vec3<float> max vector scalar x");
	test.check_equal(actual.y, 5.0f, "vec3<float> max vector scalar y");
	test.check_equal(actual.z, 9.0f, "vec3<float> max vector scalar z");

	actual = mt::max(3.0f, mt::vec3<float>(1.0f, 5.0f, 9.0f));
	test.check_equal(actual.x, 3.0f, "vec3<float> max scalar vector x");
	test.check_equal(actual.y, 5.0f, "vec3<float> max scalar vector y");
	test.check_equal(actual.z, 9.0f, "vec3<float> max scalar vector z");

	// Component-wise power.
	actual = mt::pow(mt::vec3<float>(2.0f, 4.0f, 9.0f), mt::vec3<float>(3.0f, 0.5f, 0.5f));
	test.check_close(actual.x, 8.0f, "vec3<float> pow vector x");
	test.check_close(actual.y, 2.0f, "vec3<float> pow vector y");
	test.check_close(actual.z, 3.0f, "vec3<float> pow vector z");

	actual = mt::pow(mt::vec3<float>(2.0f, 4.0f, 8.0f), 2.0f);
	test.check_close(actual.x, 4.0f, "vec3<float> pow scalar x");
	test.check_close(actual.y, 16.0f, "vec3<float> pow scalar y");
	test.check_close(actual.z, 64.0f, "vec3<float> pow scalar z");

	// Component-wise fused operations.
	actual = mt::fma(mt::vec3<float>(2.0f, -3.0f, 4.0f), mt::vec3<float>(4.0f, 5.0f, 6.0f), mt::vec3<float>(1.0f, 2.0f, 3.0f));
	test.check_equal(actual.x, 9.0f, "vec3<float> fma x");
	test.check_equal(actual.y, -13.0f, "vec3<float> fma y");
	test.check_equal(actual.z, 27.0f, "vec3<float> fma z");

	actual = mt::fms(mt::vec3<float>(2.0f, -3.0f, 4.0f), mt::vec3<float>(4.0f, 5.0f, 6.0f), mt::vec3<float>(1.0f, 2.0f, 3.0f));
	test.check_equal(actual.x, 7.0f, "vec3<float> fms x");
	test.check_equal(actual.y, -17.0f, "vec3<float> fms y");
	test.check_equal(actual.z, 21.0f, "vec3<float> fms z");

	actual = mt::fnma(mt::vec3<float>(2.0f, -3.0f, 4.0f), mt::vec3<float>(4.0f, 5.0f, 6.0f), mt::vec3<float>(1.0f, 2.0f, 3.0f));
	test.check_equal(actual.x, -7.0f, "vec3<float> fnma x");
	test.check_equal(actual.y, 17.0f, "vec3<float> fnma y");
	test.check_equal(actual.z, -21.0f, "vec3<float> fnma z");

	actual = mt::fnms(mt::vec3<float>(2.0f, -3.0f, 4.0f), mt::vec3<float>(4.0f, 5.0f, 6.0f), mt::vec3<float>(1.0f, 2.0f, 3.0f));
	test.check_equal(actual.x, -9.0f, "vec3<float> fnms x");
	test.check_equal(actual.y, 13.0f, "vec3<float> fnms y");
	test.check_equal(actual.z, -27.0f, "vec3<float> fnms z");

	// Clamp to the default 0..1 range.
	actual = mt::clamp(mt::vec3<float>(-0.5f, 0.5f, 1.5f));
	test.check_equal(actual.x, 0.0f, "vec3<float> clamp 0..1 x");
	test.check_equal(actual.y, 0.5f, "vec3<float> clamp 0..1 y");
	test.check_equal(actual.z, 1.0f, "vec3<float> clamp 0..1 z");

	// Clamp using scalar bounds.
	actual = mt::clamp(mt::vec3<float>(-2.0f, 2.0f, 6.0f), -1.0f, 4.0f);
	test.check_equal(actual.x, -1.0f, "vec3<float> clamp scalar bounds x");
	test.check_equal(actual.y, 2.0f, "vec3<float> clamp scalar bounds y");
	test.check_equal(actual.z, 4.0f, "vec3<float> clamp scalar bounds z");

	// Clamp using vector bounds.
	actual = mt::clamp(mt::vec3<float>(-2.0f, 2.0f, 6.0f), mt::vec3<float>(-1.0f, 1.0f, 3.0f), mt::vec3<float>(1.0f, 4.0f, 5.0f));
	test.check_equal(actual.x, -1.0f, "vec3<float> clamp vector bounds x");
	test.check_equal(actual.y, 2.0f, "vec3<float> clamp vector bounds y");
	test.check_equal(actual.z, 5.0f, "vec3<float> clamp vector bounds z");

	// Wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec3<float>(-1.0f, 7.0f, 12.0f), 0.0f, 5.0f);
	test.check_equal(actual.x, 4.0f, "vec3<float> wrap x");
	test.check_equal(actual.y, 2.0f, "vec3<float> wrap y");
	test.check_equal(actual.z, 2.0f, "vec3<float> wrap z");

	actual = mt::rescale_from_01(mt::vec3<float>(0.25f, 0.5f, 0.75f), 10.0f, 20.0f);
	test.check_equal(actual.x, 12.5f, "vec3<float> rescale_from_01 scalar x");
	test.check_equal(actual.y, 15.0f, "vec3<float> rescale_from_01 scalar y");
	test.check_equal(actual.z, 17.5f, "vec3<float> rescale_from_01 scalar z");

	actual = mt::rescale_to_01(mt::vec3<float>(12.5f, 15.0f, 17.5f), 10.0f, 20.0f);
	test.check_equal(actual.x, 0.25f, "vec3<float> rescale_to_01 scalar x");
	test.check_equal(actual.y, 0.5f, "vec3<float> rescale_to_01 scalar y");
	test.check_equal(actual.z, 0.75f, "vec3<float> rescale_to_01 scalar z");

	actual = mt::rescale(mt::vec3<float>(5.0f, 7.5f, 10.0f), 0.0f, 10.0f, 20.0f, 40.0f);
	test.check_equal(actual.x, 30.0f, "vec3<float> rescale scalar x");
	test.check_equal(actual.y, 35.0f, "vec3<float> rescale scalar y");
	test.check_equal(actual.z, 40.0f, "vec3<float> rescale scalar z");

	actual = mt::remap(mt::vec3<float>(5.0f, 7.5f, 10.0f), 0.0f, 10.0f, 20.0f, 40.0f);
	test.check_equal(actual.x, 30.0f, "vec3<float> remap scalar x");
	test.check_equal(actual.y, 35.0f, "vec3<float> remap scalar y");
	test.check_equal(actual.z, 40.0f, "vec3<float> remap scalar z");

	actual = mt::smoothstep(mt::vec3<float>(0.0f, 0.0f, 0.0f), mt::vec3<float>(10.0f, 10.0f, 10.0f), mt::vec3<float>(0.0f, 5.0f, 10.0f));
	test.check_equal(actual.x, 0.0f, "vec3<float> smoothstep x");
	test.check_equal(actual.y, 0.5f, "vec3<float> smoothstep y");
	test.check_equal(actual.z, 1.0f, "vec3<float> smoothstep z");

	actual = mt::step(mt::vec3<float>(0.5f, 1.5f, 2.5f), mt::vec3<float>(0.25f, 2.0f, 2.5f));
	test.check_equal(actual.x, 0.0f, "vec3<float> step vector x");
	test.check_equal(actual.y, 1.0f, "vec3<float> step vector y");
	test.check_equal(actual.z, 1.0f, "vec3<float> step vector z");

	actual = mt::step(0.5f, mt::vec3<float>(0.25f, 0.5f, 0.75f));
	test.check_equal(actual.x, 0.0f, "vec3<float> step scalar edge x");
	test.check_equal(actual.y, 1.0f, "vec3<float> step scalar edge y");
	test.check_equal(actual.z, 1.0f, "vec3<float> step scalar edge z");

	actual = mt::mix(mt::vec3<float>(0.0f, 10.0f, 20.0f), mt::vec3<float>(10.0f, 20.0f, 30.0f), 0.25f);
	test.check_equal(actual.x, 2.5f, "vec3<float> mix x");
	test.check_equal(actual.y, 12.5f, "vec3<float> mix y");
	test.check_equal(actual.z, 22.5f, "vec3<float> mix z");

	actual = mt::lerp(mt::vec3<float>(0.0f, 10.0f, 20.0f), mt::vec3<float>(10.0f, 20.0f, 30.0f), 0.5f);
	test.check_equal(actual.x, 5.0f, "vec3<float> lerp x");
	test.check_equal(actual.y, 15.0f, "vec3<float> lerp y");
	test.check_equal(actual.z, 25.0f, "vec3<float> lerp z");

	// Reflect incident vector around a normalized normal.
	actual = mt::reflect(mt::vec3<float>(1.0f, -1.0f, 2.0f), mt::vec3<float>(0.0f, 1.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec3<float> reflect x");
	test.check_equal(actual.y, 1.0f, "vec3<float> reflect y");
	test.check_equal(actual.z, 2.0f, "vec3<float> reflect z");

	// Refract with eta 1.0 leaves this incident vector unchanged.
	actual = mt::refract(mt::vec3<float>(0.0f, -1.0f, 0.0f), mt::vec3<float>(0.0f, 1.0f, 0.0f), 1.0f);
	test.check_equal(actual.x, 0.0f, "vec3<float> refract x");
	test.check_equal(actual.y, -1.0f, "vec3<float> refract y");
	test.check_equal(actual.z, 0.0f, "vec3<float> refract z");

	// Refract returns zero when total internal reflection happens.
	actual = mt::refract(mt::vec3<float>(1.0f, 0.0f, 0.0f), mt::vec3<float>(0.0f, 1.0f, 0.0f), 2.0f);
	test.check_equal(actual.x, 0.0f, "vec3<float> refract total internal x");
	test.check_equal(actual.y, 0.0f, "vec3<float> refract total internal y");
	test.check_equal(actual.z, 0.0f, "vec3<float> refract total internal z");

	// Randomized arithmetic checks compare to direct scalar expressions.
	std::mt19937 rng(0x20260516u);
	for (int i = 0; i < 1000; ++i) {
		float ax = random_float(rng, -1000.0f, 1000.0f);
		float ay = random_float(rng, -1000.0f, 1000.0f);
		float az = random_float(rng, -1000.0f, 1000.0f);
		float bx = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float by = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float bz = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float scalar = safe_divisor(random_float(rng, -1000.0f, 1000.0f));

		mt::vec3<float> left(ax, ay, az);
		mt::vec3<float> right(bx, by, bz);

		actual = left + right;
		test.check_equal(actual.x, ax + bx, "vec3<float> random add x");
		test.check_equal(actual.y, ay + by, "vec3<float> random add y");
		test.check_equal(actual.z, az + bz, "vec3<float> random add z");

		actual = left - right;
		test.check_equal(actual.x, ax - bx, "vec3<float> random subtract x");
		test.check_equal(actual.y, ay - by, "vec3<float> random subtract y");
		test.check_equal(actual.z, az - bz, "vec3<float> random subtract z");

		actual = left * right;
		test.check_equal(actual.x, ax * bx, "vec3<float> random multiply x");
		test.check_equal(actual.y, ay * by, "vec3<float> random multiply y");
		test.check_equal(actual.z, az * bz, "vec3<float> random multiply z");

		actual = left / right;
		test.check_equal(actual.x, ax / bx, "vec3<float> random divide x");
		test.check_equal(actual.y, ay / by, "vec3<float> random divide y");
		test.check_equal(actual.z, az / bz, "vec3<float> random divide z");

		actual = left + scalar;
		test.check_equal(actual.x, ax + scalar, "vec3<float> random vector scalar add x");
		test.check_equal(actual.y, ay + scalar, "vec3<float> random vector scalar add y");
		test.check_equal(actual.z, az + scalar, "vec3<float> random vector scalar add z");
	}
}

int main() {
	TestContext test;

	test_vec3_float(test);

	return test.result();
}
