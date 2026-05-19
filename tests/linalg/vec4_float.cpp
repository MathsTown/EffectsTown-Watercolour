/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar vec4<float> tests.
*********************************************************************************************************/

#include "../test.h"

#include <cmath>
#include <random>

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

void test_vec4_float(TestContext& test) {
	// Default constructor zero-initializes all components.
	mt::vec4<float> default_value;
	test.check_equal(default_value.x, 0.0f, "vec4<float> default x");
	test.check_equal(default_value.y, 0.0f, "vec4<float> default y");
	test.check_equal(default_value.z, 0.0f, "vec4<float> default z");
	test.check_equal(default_value.w, 0.0f, "vec4<float> default w");

	// Splat constructor copies one value to all components.
	mt::vec4<float> splat(3.0f);
	test.check_equal(splat.x, 3.0f, "vec4<float> splat x");
	test.check_equal(splat.y, 3.0f, "vec4<float> splat y");
	test.check_equal(splat.z, 3.0f, "vec4<float> splat z");
	test.check_equal(splat.w, 3.0f, "vec4<float> splat w");

	// Constructor can append one component to a vec3.
	mt::vec4<float> from_xyz(mt::vec3<float>(1.0f, 2.0f, 3.0f), 4.0f);
	test.check_equal(from_xyz.x, 1.0f, "vec4<float> vec3,w constructor x");
	test.check_equal(from_xyz.y, 2.0f, "vec4<float> vec3,w constructor y");
	test.check_equal(from_xyz.z, 3.0f, "vec4<float> vec3,w constructor z");
	test.check_equal(from_xyz.w, 4.0f, "vec4<float> vec3,w constructor w");

	// Constructor can prepend one component before a vec3.
	mt::vec4<float> from_yzw(1.0f, mt::vec3<float>(2.0f, 3.0f, 4.0f));
	test.check_equal(from_yzw.x, 1.0f, "vec4<float> x,vec3 constructor x");
	test.check_equal(from_yzw.y, 2.0f, "vec4<float> x,vec3 constructor y");
	test.check_equal(from_yzw.z, 3.0f, "vec4<float> x,vec3 constructor z");
	test.check_equal(from_yzw.w, 4.0f, "vec4<float> x,vec3 constructor w");

	// Constructor can combine two vec2 values.
	mt::vec4<float> from_xy_zw(mt::vec2<float>(1.0f, 2.0f), mt::vec2<float>(3.0f, 4.0f));
	test.check_equal(from_xy_zw.x, 1.0f, "vec4<float> vec2,vec2 constructor x");
	test.check_equal(from_xy_zw.y, 2.0f, "vec4<float> vec2,vec2 constructor y");
	test.check_equal(from_xy_zw.z, 3.0f, "vec4<float> vec2,vec2 constructor z");
	test.check_equal(from_xy_zw.w, 4.0f, "vec4<float> vec2,vec2 constructor w");

	// Constructor can append two components to a vec2.
	mt::vec4<float> from_xy(mt::vec2<float>(1.0f, 2.0f), 3.0f, 4.0f);
	test.check_equal(from_xy.x, 1.0f, "vec4<float> vec2,z,w constructor x");
	test.check_equal(from_xy.y, 2.0f, "vec4<float> vec2,z,w constructor y");
	test.check_equal(from_xy.z, 3.0f, "vec4<float> vec2,z,w constructor z");
	test.check_equal(from_xy.w, 4.0f, "vec4<float> vec2,z,w constructor w");

	// Constructor can prepend two components before a vec2.
	mt::vec4<float> from_zw(1.0f, 2.0f, mt::vec2<float>(3.0f, 4.0f));
	test.check_equal(from_zw.x, 1.0f, "vec4<float> x,y,vec2 constructor x");
	test.check_equal(from_zw.y, 2.0f, "vec4<float> x,y,vec2 constructor y");
	test.check_equal(from_zw.z, 3.0f, "vec4<float> x,y,vec2 constructor z");
	test.check_equal(from_zw.w, 4.0f, "vec4<float> x,y,vec2 constructor w");

	// Shared simple inputs for exact arithmetic checks.
	mt::vec4<float> a(6.0f, -8.0f, 10.0f, -12.0f);
	mt::vec4<float> b(2.0f, 4.0f, 5.0f, 6.0f);
	float s = 3.0f;

	// Vector plus vector.
	mt::vec4<float> actual = a + b;
	test.check_equal(actual.x, 8.0f, "vec4<float> add x");
	test.check_equal(actual.y, -4.0f, "vec4<float> add y");
	test.check_equal(actual.z, 15.0f, "vec4<float> add z");
	test.check_equal(actual.w, -6.0f, "vec4<float> add w");

	// Vector minus vector.
	actual = a - b;
	test.check_equal(actual.x, 4.0f, "vec4<float> subtract x");
	test.check_equal(actual.y, -12.0f, "vec4<float> subtract y");
	test.check_equal(actual.z, 5.0f, "vec4<float> subtract z");
	test.check_equal(actual.w, -18.0f, "vec4<float> subtract w");

	// Vector times vector.
	actual = a * b;
	test.check_equal(actual.x, 12.0f, "vec4<float> multiply x");
	test.check_equal(actual.y, -32.0f, "vec4<float> multiply y");
	test.check_equal(actual.z, 50.0f, "vec4<float> multiply z");
	test.check_equal(actual.w, -72.0f, "vec4<float> multiply w");

	// Vector divided by vector.
	actual = a / b;
	test.check_equal(actual.x, 3.0f, "vec4<float> divide x");
	test.check_equal(actual.y, -2.0f, "vec4<float> divide y");
	test.check_equal(actual.z, 2.0f, "vec4<float> divide z");
	test.check_equal(actual.w, -2.0f, "vec4<float> divide w");

	// Vector plus scalar.
	actual = a + s;
	test.check_equal(actual.x, 9.0f, "vec4<float> vector plus scalar x");
	test.check_equal(actual.y, -5.0f, "vec4<float> vector plus scalar y");
	test.check_equal(actual.z, 13.0f, "vec4<float> vector plus scalar z");
	test.check_equal(actual.w, -9.0f, "vec4<float> vector plus scalar w");

	// Scalar plus vector.
	actual = s + b;
	test.check_equal(actual.x, 5.0f, "vec4<float> scalar plus vector x");
	test.check_equal(actual.y, 7.0f, "vec4<float> scalar plus vector y");
	test.check_equal(actual.z, 8.0f, "vec4<float> scalar plus vector z");
	test.check_equal(actual.w, 9.0f, "vec4<float> scalar plus vector w");

	// Vector minus scalar.
	actual = a - s;
	test.check_equal(actual.x, 3.0f, "vec4<float> vector minus scalar x");
	test.check_equal(actual.y, -11.0f, "vec4<float> vector minus scalar y");
	test.check_equal(actual.z, 7.0f, "vec4<float> vector minus scalar z");
	test.check_equal(actual.w, -15.0f, "vec4<float> vector minus scalar w");

	// Scalar minus vector.
	actual = s - b;
	test.check_equal(actual.x, 1.0f, "vec4<float> scalar minus vector x");
	test.check_equal(actual.y, -1.0f, "vec4<float> scalar minus vector y");
	test.check_equal(actual.z, -2.0f, "vec4<float> scalar minus vector z");
	test.check_equal(actual.w, -3.0f, "vec4<float> scalar minus vector w");

	// Vector times scalar.
	actual = b * s;
	test.check_equal(actual.x, 6.0f, "vec4<float> vector multiply scalar x");
	test.check_equal(actual.y, 12.0f, "vec4<float> vector multiply scalar y");
	test.check_equal(actual.z, 15.0f, "vec4<float> vector multiply scalar z");
	test.check_equal(actual.w, 18.0f, "vec4<float> vector multiply scalar w");

	// Scalar times vector.
	actual = s * b;
	test.check_equal(actual.x, 6.0f, "vec4<float> scalar multiply vector x");
	test.check_equal(actual.y, 12.0f, "vec4<float> scalar multiply vector y");
	test.check_equal(actual.z, 15.0f, "vec4<float> scalar multiply vector z");
	test.check_equal(actual.w, 18.0f, "vec4<float> scalar multiply vector w");

	// Vector divided by scalar.
	actual = a / 2.0f;
	test.check_equal(actual.x, 3.0f, "vec4<float> vector divide scalar x");
	test.check_equal(actual.y, -4.0f, "vec4<float> vector divide scalar y");
	test.check_equal(actual.z, 5.0f, "vec4<float> vector divide scalar z");
	test.check_equal(actual.w, -6.0f, "vec4<float> vector divide scalar w");

	// Scalar divided by vector.
	actual = s / b;
	test.check_equal(actual.x, 1.5f, "vec4<float> scalar divide vector x");
	test.check_equal(actual.y, 0.75f, "vec4<float> scalar divide vector y");
	test.check_equal(actual.z, 0.6f, "vec4<float> scalar divide vector z");
	test.check_equal(actual.w, 0.5f, "vec4<float> scalar divide vector w");

	// Compound vector add mutates in place.
	actual = a;
	actual += b;
	test.check_equal(actual.x, 8.0f, "vec4<float> compound add x");
	test.check_equal(actual.y, -4.0f, "vec4<float> compound add y");
	test.check_equal(actual.z, 15.0f, "vec4<float> compound add z");
	test.check_equal(actual.w, -6.0f, "vec4<float> compound add w");

	// Compound scalar subtract mutates in place.
	actual = a;
	actual -= s;
	test.check_equal(actual.x, 3.0f, "vec4<float> compound scalar subtract x");
	test.check_equal(actual.y, -11.0f, "vec4<float> compound scalar subtract y");
	test.check_equal(actual.z, 7.0f, "vec4<float> compound scalar subtract z");
	test.check_equal(actual.w, -15.0f, "vec4<float> compound scalar subtract w");

	// Remaining compound assignments mutate in place.
	actual = a;
	actual -= b;
	test.check_equal(actual.x, 4.0f, "vec4<float> compound subtract x");
	test.check_equal(actual.y, -12.0f, "vec4<float> compound subtract y");
	test.check_equal(actual.z, 5.0f, "vec4<float> compound subtract z");
	test.check_equal(actual.w, -18.0f, "vec4<float> compound subtract w");

	actual = b;
	actual *= a;
	test.check_equal(actual.x, 12.0f, "vec4<float> compound multiply x");
	test.check_equal(actual.y, -32.0f, "vec4<float> compound multiply y");
	test.check_equal(actual.z, 50.0f, "vec4<float> compound multiply z");
	test.check_equal(actual.w, -72.0f, "vec4<float> compound multiply w");

	actual = a;
	actual /= b;
	test.check_equal(actual.x, 3.0f, "vec4<float> compound divide x");
	test.check_equal(actual.y, -2.0f, "vec4<float> compound divide y");
	test.check_equal(actual.z, 2.0f, "vec4<float> compound divide z");
	test.check_equal(actual.w, -2.0f, "vec4<float> compound divide w");

	actual = b;
	actual += s;
	test.check_equal(actual.x, 5.0f, "vec4<float> compound scalar add x");
	test.check_equal(actual.y, 7.0f, "vec4<float> compound scalar add y");
	test.check_equal(actual.z, 8.0f, "vec4<float> compound scalar add z");
	test.check_equal(actual.w, 9.0f, "vec4<float> compound scalar add w");

	actual = b;
	actual *= s;
	test.check_equal(actual.x, 6.0f, "vec4<float> compound scalar multiply x");
	test.check_equal(actual.y, 12.0f, "vec4<float> compound scalar multiply y");
	test.check_equal(actual.z, 15.0f, "vec4<float> compound scalar multiply z");
	test.check_equal(actual.w, 18.0f, "vec4<float> compound scalar multiply w");

	actual = a;
	actual /= 2.0f;
	test.check_equal(actual.x, 3.0f, "vec4<float> compound scalar divide x");
	test.check_equal(actual.y, -4.0f, "vec4<float> compound scalar divide y");
	test.check_equal(actual.z, 5.0f, "vec4<float> compound scalar divide z");
	test.check_equal(actual.w, -6.0f, "vec4<float> compound scalar divide w");

	// Unary minus flips every component.
	actual = -a;
	test.check_equal(actual.x, -6.0f, "vec4<float> unary minus x");
	test.check_equal(actual.y, 8.0f, "vec4<float> unary minus y");
	test.check_equal(actual.z, -10.0f, "vec4<float> unary minus z");
	test.check_equal(actual.w, 12.0f, "vec4<float> unary minus w");

	// Equality is available for scalar component types.
	test.check(a == mt::vec4<float>(6.0f, -8.0f, 10.0f, -12.0f), "vec4<float> equality true");
	test.check(!(a == b), "vec4<float> equality false");

	// Vec2 swizzles reduce vec4 to vec2 in component order.
	mt::vec2<float> swizzle2 = a.xy();
	test.check_equal(swizzle2.x, 6.0f, "vec4<float> xy x");
	test.check_equal(swizzle2.y, -8.0f, "vec4<float> xy y");
	swizzle2 = a.xz();
	test.check_equal(swizzle2.x, 6.0f, "vec4<float> xz x");
	test.check_equal(swizzle2.y, 10.0f, "vec4<float> xz y");
	swizzle2 = a.xw();
	test.check_equal(swizzle2.x, 6.0f, "vec4<float> xw x");
	test.check_equal(swizzle2.y, -12.0f, "vec4<float> xw y");
	swizzle2 = a.yz();
	test.check_equal(swizzle2.x, -8.0f, "vec4<float> yz x");
	test.check_equal(swizzle2.y, 10.0f, "vec4<float> yz y");
	swizzle2 = a.yw();
	test.check_equal(swizzle2.x, -8.0f, "vec4<float> yw x");
	test.check_equal(swizzle2.y, -12.0f, "vec4<float> yw y");
	swizzle2 = a.zw();
	test.check_equal(swizzle2.x, 10.0f, "vec4<float> zw x");
	test.check_equal(swizzle2.y, -12.0f, "vec4<float> zw y");

	// Vec3 swizzles reduce vec4 to vec3 in component order.
	mt::vec3<float> swizzle3 = a.xyz();
	test.check_equal(swizzle3.x, 6.0f, "vec4<float> xyz x");
	test.check_equal(swizzle3.y, -8.0f, "vec4<float> xyz y");
	test.check_equal(swizzle3.z, 10.0f, "vec4<float> xyz z");
	swizzle3 = a.xyw();
	test.check_equal(swizzle3.x, 6.0f, "vec4<float> xyw x");
	test.check_equal(swizzle3.y, -8.0f, "vec4<float> xyw y");
	test.check_equal(swizzle3.z, -12.0f, "vec4<float> xyw z");
	swizzle3 = a.xzw();
	test.check_equal(swizzle3.x, 6.0f, "vec4<float> xzw x");
	test.check_equal(swizzle3.y, 10.0f, "vec4<float> xzw y");
	test.check_equal(swizzle3.z, -12.0f, "vec4<float> xzw z");
	swizzle3 = a.yzw();
	test.check_equal(swizzle3.x, -8.0f, "vec4<float> yzw x");
	test.check_equal(swizzle3.y, 10.0f, "vec4<float> yzw y");
	test.check_equal(swizzle3.z, -12.0f, "vec4<float> yzw z");

	// Dot product reduces four components to one scalar.
	float dot_expected = 6.0f * 2.0f + -8.0f * 4.0f + 10.0f * 5.0f + -12.0f * 6.0f;
	test.check_equal(mt::dot(a, b), dot_expected, "vec4<float> dot");

	// Length and magnitude aliases.
	mt::vec4<float> length_input(2.0f, 4.0f, 4.0f, 8.0f);
	test.check_equal(length_input.length(), 10.0f, "vec4<float> member length");
	test.check_equal(mt::length(length_input), 10.0f, "vec4<float> free length");
	test.check_equal(mt::magnitude(length_input), 10.0f, "vec4<float> free magnitude");

	// Distance computes the length between two points.
	test.check_equal(mt::distance(mt::vec4<float>(1.0f, 2.0f, 3.0f, 4.0f), mt::vec4<float>(3.0f, 6.0f, 7.0f, 12.0f)), 10.0f, "vec4<float> distance");

	// Normalization returns a new unit-length vector.
	actual = length_input.normalize();
	// Normalize uses sqrt and division, so compare with tolerance.
	test.check_close(actual.x, 2.0f / 10.0f, "vec4<float> normalize x");
	test.check_close(actual.y, 4.0f / 10.0f, "vec4<float> normalize y");
	test.check_close(actual.z, 4.0f / 10.0f, "vec4<float> normalize z");
	test.check_close(actual.w, 8.0f / 10.0f, "vec4<float> normalize w");

	actual = mt::normalize(length_input);
	test.check_close(actual.x, 2.0f / 10.0f, "vec4<float> free normalize x");
	test.check_close(actual.y, 4.0f / 10.0f, "vec4<float> free normalize y");
	test.check_close(actual.z, 4.0f / 10.0f, "vec4<float> free normalize z");
	test.check_close(actual.w, 8.0f / 10.0f, "vec4<float> free normalize w");

	// Shared input for component-wise math functions.
	mt::vec4<float> c(-1.75f, 2.25f, 3.5f, -4.5f);

	// Component-wise floor.
	actual = mt::floor(c);
	test.check_equal(actual.x, -2.0f, "vec4<float> floor x");
	test.check_equal(actual.y, 2.0f, "vec4<float> floor y");
	test.check_equal(actual.z, 3.0f, "vec4<float> floor z");
	test.check_equal(actual.w, -5.0f, "vec4<float> floor w");

	// Component-wise ceiling.
	actual = mt::ceil(c);
	test.check_equal(actual.x, -1.0f, "vec4<float> ceil x");
	test.check_equal(actual.y, 3.0f, "vec4<float> ceil y");
	test.check_equal(actual.z, 4.0f, "vec4<float> ceil z");
	test.check_equal(actual.w, -4.0f, "vec4<float> ceil w");

	// Component-wise round.
	actual = mt::round(mt::vec4<float>(-1.5f, 2.5f, 3.5f, -4.5f));
	test.check_equal(actual.x, -2.0f, "vec4<float> round x");
	test.check_equal(actual.y, 3.0f, "vec4<float> round y");
	test.check_equal(actual.z, 4.0f, "vec4<float> round z");
	test.check_equal(actual.w, -5.0f, "vec4<float> round w");

	// Component-wise fractional part.
	actual = mt::fract(c);
	test.check_equal(actual.x, 0.25f, "vec4<float> fract x");
	test.check_equal(actual.y, 0.25f, "vec4<float> fract y");
	test.check_equal(actual.z, 0.5f, "vec4<float> fract z");
	test.check_equal(actual.w, 0.5f, "vec4<float> fract w");

	// Component-wise truncation.
	actual = mt::trunc(c);
	test.check_equal(actual.x, -1.0f, "vec4<float> trunc x");
	test.check_equal(actual.y, 2.0f, "vec4<float> trunc y");
	test.check_equal(actual.z, 3.0f, "vec4<float> trunc z");
	test.check_equal(actual.w, -4.0f, "vec4<float> trunc w");

	// Component-wise absolute value.
	actual = mt::abs(c);
	test.check_equal(actual.x, 1.75f, "vec4<float> abs x");
	test.check_equal(actual.y, 2.25f, "vec4<float> abs y");
	test.check_equal(actual.z, 3.5f, "vec4<float> abs z");
	test.check_equal(actual.w, 4.5f, "vec4<float> abs w");

	// Component-wise square root.
	actual = mt::sqrt(mt::vec4<float>(9.0f, 16.0f, 25.0f, 36.0f));
	test.check_equal(actual.x, 3.0f, "vec4<float> sqrt x");
	test.check_equal(actual.y, 4.0f, "vec4<float> sqrt y");
	test.check_equal(actual.z, 5.0f, "vec4<float> sqrt z");
	test.check_equal(actual.w, 6.0f, "vec4<float> sqrt w");

	// Component-wise exponentials and logarithms.
	actual = mt::exp(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec4<float> exp x");
	test.check_equal(actual.y, 1.0f, "vec4<float> exp y");
	test.check_equal(actual.z, 1.0f, "vec4<float> exp z");
	test.check_equal(actual.w, 1.0f, "vec4<float> exp w");

	actual = mt::exp2(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec4<float> exp2 x");
	test.check_equal(actual.y, 1.0f, "vec4<float> exp2 y");
	test.check_equal(actual.z, 1.0f, "vec4<float> exp2 z");
	test.check_equal(actual.w, 1.0f, "vec4<float> exp2 w");

	actual = mt::log(mt::vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> log x");
	test.check_equal(actual.y, 0.0f, "vec4<float> log y");
	test.check_equal(actual.z, 0.0f, "vec4<float> log z");
	test.check_equal(actual.w, 0.0f, "vec4<float> log w");

	actual = mt::log2(mt::vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> log2 x");
	test.check_equal(actual.y, 0.0f, "vec4<float> log2 y");
	test.check_equal(actual.z, 0.0f, "vec4<float> log2 z");
	test.check_equal(actual.w, 0.0f, "vec4<float> log2 w");

	// Angle conversion functions convert between degrees and radians.
	actual = mt::radians(mt::vec4<float>(180.0f, 90.0f, 45.0f, 30.0f));
	test.check_close(actual.x, 3.14159265358979323846f, "vec4<float> radians x");
	test.check_close(actual.y, 1.57079632679489661923f, "vec4<float> radians y");
	test.check_close(actual.z, 0.78539816339744830962f, "vec4<float> radians z");
	test.check_close(actual.w, 0.52359877559829887308f, "vec4<float> radians w");

	actual = mt::degrees(mt::vec4<float>(3.14159265358979323846f, 1.57079632679489661923f, 0.78539816339744830962f, 0.52359877559829887308f));
	test.check_close(actual.x, 180.0f, "vec4<float> degrees x");
	test.check_close(actual.y, 90.0f, "vec4<float> degrees y");
	test.check_close(actual.z, 45.0f, "vec4<float> degrees z");
	test.check_close(actual.w, 30.0f, "vec4<float> degrees w");

	const mt::vec4<float> member_degrees(180.0f, 90.0f, 45.0f, 30.0f);
	actual = member_degrees.radians();
	test.check_close(actual.x, 3.14159265358979323846f, "vec4<float> member radians x");
	test.check_close(actual.y, 1.57079632679489661923f, "vec4<float> member radians y");
	test.check_close(actual.z, 0.78539816339744830962f, "vec4<float> member radians z");
	test.check_close(actual.w, 0.52359877559829887308f, "vec4<float> member radians w");

	const mt::vec4<float> member_radians(3.14159265358979323846f, 1.57079632679489661923f, 0.78539816339744830962f, 0.52359877559829887308f);
	actual = member_radians.degrees();
	test.check_close(actual.x, 180.0f, "vec4<float> member degrees x");
	test.check_close(actual.y, 90.0f, "vec4<float> member degrees y");
	test.check_close(actual.z, 45.0f, "vec4<float> member degrees z");
	test.check_close(actual.w, 30.0f, "vec4<float> member degrees w");

	// Component-wise trigonometry.
	actual = mt::sin(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> sin x");
	test.check_equal(actual.y, 0.0f, "vec4<float> sin y");
	test.check_equal(actual.z, 0.0f, "vec4<float> sin z");
	test.check_equal(actual.w, 0.0f, "vec4<float> sin w");

	actual = mt::cos(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec4<float> cos x");
	test.check_equal(actual.y, 1.0f, "vec4<float> cos y");
	test.check_equal(actual.z, 1.0f, "vec4<float> cos z");
	test.check_equal(actual.w, 1.0f, "vec4<float> cos w");

	actual = mt::tan(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> tan x");
	test.check_equal(actual.y, 0.0f, "vec4<float> tan y");
	test.check_equal(actual.z, 0.0f, "vec4<float> tan z");
	test.check_equal(actual.w, 0.0f, "vec4<float> tan w");

	actual = mt::asin(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> asin x");
	test.check_equal(actual.y, 0.0f, "vec4<float> asin y");
	test.check_equal(actual.z, 0.0f, "vec4<float> asin z");
	test.check_equal(actual.w, 0.0f, "vec4<float> asin w");

	actual = mt::acos(mt::vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> acos x");
	test.check_equal(actual.y, 0.0f, "vec4<float> acos y");
	test.check_equal(actual.z, 0.0f, "vec4<float> acos z");
	test.check_equal(actual.w, 0.0f, "vec4<float> acos w");

	actual = mt::atan(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> atan x");
	test.check_equal(actual.y, 0.0f, "vec4<float> atan y");
	test.check_equal(actual.z, 0.0f, "vec4<float> atan z");
	test.check_equal(actual.w, 0.0f, "vec4<float> atan w");

	actual = mt::atan2(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f), mt::vec4<float>(1.0f, 1.0f, 1.0f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> atan2 x");
	test.check_equal(actual.y, 0.0f, "vec4<float> atan2 y");
	test.check_equal(actual.z, 0.0f, "vec4<float> atan2 z");
	test.check_equal(actual.w, 0.0f, "vec4<float> atan2 w");

	// Component-wise minimum.
	actual = mt::min(mt::vec4<float>(1.0f, 5.0f, 9.0f, 12.0f), mt::vec4<float>(2.0f, 4.0f, 10.0f, 11.0f));
	test.check_equal(actual.x, 1.0f, "vec4<float> min x");
	test.check_equal(actual.y, 4.0f, "vec4<float> min y");
	test.check_equal(actual.z, 9.0f, "vec4<float> min z");
	test.check_equal(actual.w, 11.0f, "vec4<float> min w");

	actual = mt::min(mt::vec4<float>(1.0f, 5.0f, 9.0f, 12.0f), 3.0f);
	test.check_equal(actual.x, 1.0f, "vec4<float> min vector scalar x");
	test.check_equal(actual.y, 3.0f, "vec4<float> min vector scalar y");
	test.check_equal(actual.z, 3.0f, "vec4<float> min vector scalar z");
	test.check_equal(actual.w, 3.0f, "vec4<float> min vector scalar w");

	actual = mt::min(3.0f, mt::vec4<float>(1.0f, 5.0f, 9.0f, 12.0f));
	test.check_equal(actual.x, 1.0f, "vec4<float> min scalar vector x");
	test.check_equal(actual.y, 3.0f, "vec4<float> min scalar vector y");
	test.check_equal(actual.z, 3.0f, "vec4<float> min scalar vector z");
	test.check_equal(actual.w, 3.0f, "vec4<float> min scalar vector w");

	// Component-wise maximum.
	actual = mt::max(mt::vec4<float>(1.0f, 5.0f, 9.0f, 12.0f), mt::vec4<float>(2.0f, 4.0f, 10.0f, 11.0f));
	test.check_equal(actual.x, 2.0f, "vec4<float> max x");
	test.check_equal(actual.y, 5.0f, "vec4<float> max y");
	test.check_equal(actual.z, 10.0f, "vec4<float> max z");
	test.check_equal(actual.w, 12.0f, "vec4<float> max w");

	actual = mt::max(mt::vec4<float>(1.0f, 5.0f, 9.0f, 12.0f), 3.0f);
	test.check_equal(actual.x, 3.0f, "vec4<float> max vector scalar x");
	test.check_equal(actual.y, 5.0f, "vec4<float> max vector scalar y");
	test.check_equal(actual.z, 9.0f, "vec4<float> max vector scalar z");
	test.check_equal(actual.w, 12.0f, "vec4<float> max vector scalar w");

	actual = mt::max(3.0f, mt::vec4<float>(1.0f, 5.0f, 9.0f, 12.0f));
	test.check_equal(actual.x, 3.0f, "vec4<float> max scalar vector x");
	test.check_equal(actual.y, 5.0f, "vec4<float> max scalar vector y");
	test.check_equal(actual.z, 9.0f, "vec4<float> max scalar vector z");
	test.check_equal(actual.w, 12.0f, "vec4<float> max scalar vector w");

	// Component-wise power.
	actual = mt::pow(mt::vec4<float>(2.0f, 4.0f, 9.0f, 16.0f), mt::vec4<float>(3.0f, 0.5f, 0.5f, 0.5f));
	test.check_close(actual.x, 8.0f, "vec4<float> pow vector x");
	test.check_close(actual.y, 2.0f, "vec4<float> pow vector y");
	test.check_close(actual.z, 3.0f, "vec4<float> pow vector z");
	test.check_close(actual.w, 4.0f, "vec4<float> pow vector w");

	actual = mt::pow(mt::vec4<float>(2.0f, 4.0f, 8.0f, 16.0f), 2.0f);
	test.check_close(actual.x, 4.0f, "vec4<float> pow scalar x");
	test.check_close(actual.y, 16.0f, "vec4<float> pow scalar y");
	test.check_close(actual.z, 64.0f, "vec4<float> pow scalar z");
	test.check_close(actual.w, 256.0f, "vec4<float> pow scalar w");

	// Component-wise fused operations.
	actual = mt::fma(mt::vec4<float>(2.0f, -3.0f, 4.0f, -5.0f), mt::vec4<float>(4.0f, 5.0f, 6.0f, 7.0f), mt::vec4<float>(1.0f, 2.0f, 3.0f, 4.0f));
	test.check_equal(actual.x, 9.0f, "vec4<float> fma x");
	test.check_equal(actual.y, -13.0f, "vec4<float> fma y");
	test.check_equal(actual.z, 27.0f, "vec4<float> fma z");
	test.check_equal(actual.w, -31.0f, "vec4<float> fma w");

	actual = mt::fms(mt::vec4<float>(2.0f, -3.0f, 4.0f, -5.0f), mt::vec4<float>(4.0f, 5.0f, 6.0f, 7.0f), mt::vec4<float>(1.0f, 2.0f, 3.0f, 4.0f));
	test.check_equal(actual.x, 7.0f, "vec4<float> fms x");
	test.check_equal(actual.y, -17.0f, "vec4<float> fms y");
	test.check_equal(actual.z, 21.0f, "vec4<float> fms z");
	test.check_equal(actual.w, -39.0f, "vec4<float> fms w");

	actual = mt::fnma(mt::vec4<float>(2.0f, -3.0f, 4.0f, -5.0f), mt::vec4<float>(4.0f, 5.0f, 6.0f, 7.0f), mt::vec4<float>(1.0f, 2.0f, 3.0f, 4.0f));
	test.check_equal(actual.x, -7.0f, "vec4<float> fnma x");
	test.check_equal(actual.y, 17.0f, "vec4<float> fnma y");
	test.check_equal(actual.z, -21.0f, "vec4<float> fnma z");
	test.check_equal(actual.w, 39.0f, "vec4<float> fnma w");

	actual = mt::fnms(mt::vec4<float>(2.0f, -3.0f, 4.0f, -5.0f), mt::vec4<float>(4.0f, 5.0f, 6.0f, 7.0f), mt::vec4<float>(1.0f, 2.0f, 3.0f, 4.0f));
	test.check_equal(actual.x, -9.0f, "vec4<float> fnms x");
	test.check_equal(actual.y, 13.0f, "vec4<float> fnms y");
	test.check_equal(actual.z, -27.0f, "vec4<float> fnms z");
	test.check_equal(actual.w, 31.0f, "vec4<float> fnms w");

	// Clamp to the default 0..1 range.
	actual = mt::clamp(mt::vec4<float>(-0.5f, 0.5f, 1.5f, 0.75f));
	test.check_equal(actual.x, 0.0f, "vec4<float> clamp 0..1 x");
	test.check_equal(actual.y, 0.5f, "vec4<float> clamp 0..1 y");
	test.check_equal(actual.z, 1.0f, "vec4<float> clamp 0..1 z");
	test.check_equal(actual.w, 0.75f, "vec4<float> clamp 0..1 w");

	// Clamp using scalar bounds.
	actual = mt::clamp(mt::vec4<float>(-2.0f, 2.0f, 6.0f, 3.0f), -1.0f, 4.0f);
	test.check_equal(actual.x, -1.0f, "vec4<float> clamp scalar bounds x");
	test.check_equal(actual.y, 2.0f, "vec4<float> clamp scalar bounds y");
	test.check_equal(actual.z, 4.0f, "vec4<float> clamp scalar bounds z");
	test.check_equal(actual.w, 3.0f, "vec4<float> clamp scalar bounds w");

	// Clamp using vector bounds.
	actual = mt::clamp(mt::vec4<float>(-2.0f, 2.0f, 6.0f, 3.0f), mt::vec4<float>(-1.0f, 1.0f, 3.0f, 4.0f), mt::vec4<float>(1.0f, 4.0f, 5.0f, 8.0f));
	test.check_equal(actual.x, -1.0f, "vec4<float> clamp vector bounds x");
	test.check_equal(actual.y, 2.0f, "vec4<float> clamp vector bounds y");
	test.check_equal(actual.z, 5.0f, "vec4<float> clamp vector bounds z");
	test.check_equal(actual.w, 4.0f, "vec4<float> clamp vector bounds w");

	// Wrap into the half-open range 0..5.
	actual = mt::wrap(mt::vec4<float>(-1.0f, 7.0f, 12.0f, 5.0f), 0.0f, 5.0f);
	test.check_equal(actual.x, 4.0f, "vec4<float> wrap x");
	test.check_equal(actual.y, 2.0f, "vec4<float> wrap y");
	test.check_equal(actual.z, 2.0f, "vec4<float> wrap z");
	test.check_equal(actual.w, 0.0f, "vec4<float> wrap w");

	actual = mt::rescale_from_01(mt::vec4<float>(0.25f, 0.5f, 0.75f, 1.0f), 10.0f, 20.0f);
	test.check_equal(actual.x, 12.5f, "vec4<float> rescale_from_01 scalar x");
	test.check_equal(actual.y, 15.0f, "vec4<float> rescale_from_01 scalar y");
	test.check_equal(actual.z, 17.5f, "vec4<float> rescale_from_01 scalar z");
	test.check_equal(actual.w, 20.0f, "vec4<float> rescale_from_01 scalar w");

	actual = mt::rescale_to_01(mt::vec4<float>(12.5f, 15.0f, 17.5f, 20.0f), 10.0f, 20.0f);
	test.check_equal(actual.x, 0.25f, "vec4<float> rescale_to_01 scalar x");
	test.check_equal(actual.y, 0.5f, "vec4<float> rescale_to_01 scalar y");
	test.check_equal(actual.z, 0.75f, "vec4<float> rescale_to_01 scalar z");
	test.check_equal(actual.w, 1.0f, "vec4<float> rescale_to_01 scalar w");

	actual = mt::rescale(mt::vec4<float>(5.0f, 7.5f, 10.0f, 0.0f), 0.0f, 10.0f, 20.0f, 40.0f);
	test.check_equal(actual.x, 30.0f, "vec4<float> rescale scalar x");
	test.check_equal(actual.y, 35.0f, "vec4<float> rescale scalar y");
	test.check_equal(actual.z, 40.0f, "vec4<float> rescale scalar z");
	test.check_equal(actual.w, 20.0f, "vec4<float> rescale scalar w");

	actual = mt::remap(mt::vec4<float>(5.0f, 7.5f, 10.0f, 0.0f), 0.0f, 10.0f, 20.0f, 40.0f);
	test.check_equal(actual.x, 30.0f, "vec4<float> remap scalar x");
	test.check_equal(actual.y, 35.0f, "vec4<float> remap scalar y");
	test.check_equal(actual.z, 40.0f, "vec4<float> remap scalar z");
	test.check_equal(actual.w, 20.0f, "vec4<float> remap scalar w");

	actual = mt::smoothstep(mt::vec4<float>(0.0f, 0.0f, 0.0f, 0.0f), mt::vec4<float>(10.0f, 10.0f, 10.0f, 10.0f), mt::vec4<float>(0.0f, 5.0f, 10.0f, 15.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> smoothstep x");
	test.check_equal(actual.y, 0.5f, "vec4<float> smoothstep y");
	test.check_equal(actual.z, 1.0f, "vec4<float> smoothstep z");
	test.check_equal(actual.w, 1.0f, "vec4<float> smoothstep w");

	actual = mt::step(mt::vec4<float>(0.5f, 1.5f, 2.5f, 3.5f), mt::vec4<float>(0.25f, 2.0f, 2.5f, 4.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> step vector x");
	test.check_equal(actual.y, 1.0f, "vec4<float> step vector y");
	test.check_equal(actual.z, 1.0f, "vec4<float> step vector z");
	test.check_equal(actual.w, 1.0f, "vec4<float> step vector w");

	actual = mt::step(0.5f, mt::vec4<float>(0.25f, 0.5f, 0.75f, 1.0f));
	test.check_equal(actual.x, 0.0f, "vec4<float> step scalar edge x");
	test.check_equal(actual.y, 1.0f, "vec4<float> step scalar edge y");
	test.check_equal(actual.z, 1.0f, "vec4<float> step scalar edge z");
	test.check_equal(actual.w, 1.0f, "vec4<float> step scalar edge w");

	actual = mt::mix(mt::vec4<float>(0.0f, 10.0f, 20.0f, 30.0f), mt::vec4<float>(10.0f, 20.0f, 30.0f, 40.0f), 0.25f);
	test.check_equal(actual.x, 2.5f, "vec4<float> mix x");
	test.check_equal(actual.y, 12.5f, "vec4<float> mix y");
	test.check_equal(actual.z, 22.5f, "vec4<float> mix z");
	test.check_equal(actual.w, 32.5f, "vec4<float> mix w");

	actual = mt::lerp(mt::vec4<float>(0.0f, 10.0f, 20.0f, 30.0f), mt::vec4<float>(10.0f, 20.0f, 30.0f, 40.0f), 0.5f);
	test.check_equal(actual.x, 5.0f, "vec4<float> lerp x");
	test.check_equal(actual.y, 15.0f, "vec4<float> lerp y");
	test.check_equal(actual.z, 25.0f, "vec4<float> lerp z");
	test.check_equal(actual.w, 35.0f, "vec4<float> lerp w");

	// Reflect incident vector around a normalized normal.
	actual = mt::reflect(mt::vec4<float>(1.0f, -1.0f, 2.0f, 3.0f), mt::vec4<float>(0.0f, 1.0f, 0.0f, 0.0f));
	test.check_equal(actual.x, 1.0f, "vec4<float> reflect x");
	test.check_equal(actual.y, 1.0f, "vec4<float> reflect y");
	test.check_equal(actual.z, 2.0f, "vec4<float> reflect z");
	test.check_equal(actual.w, 3.0f, "vec4<float> reflect w");

	// Refract with eta 1.0 leaves this incident vector unchanged.
	actual = mt::refract(mt::vec4<float>(0.0f, -1.0f, 0.0f, 0.0f), mt::vec4<float>(0.0f, 1.0f, 0.0f, 0.0f), 1.0f);
	test.check_equal(actual.x, 0.0f, "vec4<float> refract x");
	test.check_equal(actual.y, -1.0f, "vec4<float> refract y");
	test.check_equal(actual.z, 0.0f, "vec4<float> refract z");
	test.check_equal(actual.w, 0.0f, "vec4<float> refract w");

	// Refract returns zero when total internal reflection happens.
	actual = mt::refract(mt::vec4<float>(1.0f, 0.0f, 0.0f, 0.0f), mt::vec4<float>(0.0f, 1.0f, 0.0f, 0.0f), 2.0f);
	test.check_equal(actual.x, 0.0f, "vec4<float> refract total internal x");
	test.check_equal(actual.y, 0.0f, "vec4<float> refract total internal y");
	test.check_equal(actual.z, 0.0f, "vec4<float> refract total internal z");
	test.check_equal(actual.w, 0.0f, "vec4<float> refract total internal w");

	// Randomized arithmetic checks compare to direct scalar expressions.
	std::mt19937 rng(0x20260520u);
	for (int i = 0; i < 1000; ++i) {
		float ax = random_float(rng, -1000.0f, 1000.0f);
		float ay = random_float(rng, -1000.0f, 1000.0f);
		float az = random_float(rng, -1000.0f, 1000.0f);
		float aw = random_float(rng, -1000.0f, 1000.0f);
		float bx = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float by = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float bz = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float bw = safe_divisor(random_float(rng, -1000.0f, 1000.0f));
		float scalar = safe_divisor(random_float(rng, -1000.0f, 1000.0f));

		mt::vec4<float> left(ax, ay, az, aw);
		mt::vec4<float> right(bx, by, bz, bw);

		actual = left + right;
		test.check_equal(actual.x, ax + bx, "vec4<float> random add x");
		test.check_equal(actual.y, ay + by, "vec4<float> random add y");
		test.check_equal(actual.z, az + bz, "vec4<float> random add z");
		test.check_equal(actual.w, aw + bw, "vec4<float> random add w");

		actual = left - right;
		test.check_equal(actual.x, ax - bx, "vec4<float> random subtract x");
		test.check_equal(actual.y, ay - by, "vec4<float> random subtract y");
		test.check_equal(actual.z, az - bz, "vec4<float> random subtract z");
		test.check_equal(actual.w, aw - bw, "vec4<float> random subtract w");

		actual = left * right;
		test.check_equal(actual.x, ax * bx, "vec4<float> random multiply x");
		test.check_equal(actual.y, ay * by, "vec4<float> random multiply y");
		test.check_equal(actual.z, az * bz, "vec4<float> random multiply z");
		test.check_equal(actual.w, aw * bw, "vec4<float> random multiply w");

		actual = left / right;
		test.check_equal(actual.x, ax / bx, "vec4<float> random divide x");
		test.check_equal(actual.y, ay / by, "vec4<float> random divide y");
		test.check_equal(actual.z, az / bz, "vec4<float> random divide z");
		test.check_equal(actual.w, aw / bw, "vec4<float> random divide w");

		actual = left + scalar;
		test.check_equal(actual.x, ax + scalar, "vec4<float> random vector scalar add x");
		test.check_equal(actual.y, ay + scalar, "vec4<float> random vector scalar add y");
		test.check_equal(actual.z, az + scalar, "vec4<float> random vector scalar add z");
		test.check_equal(actual.w, aw + scalar, "vec4<float> random vector scalar add w");
	}
}

int main() {
	TestContext test;

	test_vec4_float(test);

	return test.result();
}
