/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Readable scalar common helper tests.
*********************************************************************************************************/

#include "../test.h"

#include "linalg.h"

void test_common(TestContext& test) {
	test.check_equal(mt::fract(-1.75f), 0.25f, "common fract negative");
	test.check_equal(mt::fms(2.0f, 4.0f, 1.0f), 7.0f, "common fms");
	test.check_equal(mt::fnma(2.0f, 4.0f, 1.0f), -7.0f, "common fnma");
	test.check_equal(mt::fnms(2.0f, 4.0f, 1.0f), -9.0f, "common fnms");
	test.check_close(mt::radians(180.0f), 3.14159265358979323846f, "common radians float");
	test.check_close(mt::degrees(3.14159265358979323846), 180.0, "common degrees double");

	test.check_equal(mt::rescale_from_01(0.25f, 10.0f, 30.0f), 15.0f, "common rescale_from_01");
	test.check_equal(mt::rescale_to_01(15.0f, 10.0f, 30.0f), 0.25f, "common rescale_to_01");
	test.check_equal(mt::rescale(5.0f, 0.0f, 10.0f, 20.0f, 40.0f), 30.0f, "common rescale");
	test.check_equal(mt::remap(5.0f, 0.0f, 10.0f, 20.0f, 40.0f), 30.0f, "common remap");

	test.check_equal(mt::wrap(-1.0f, 0.0f, 5.0f), 4.0f, "common wrap below range");
	test.check_equal(mt::wrap(7.5f, 5.0f, 7.0f), 5.5f, "common wrap non-zero range");

	test.check_equal(mt::clamp(-0.5f), 0.0f, "common clamp default low");
	test.check_equal(mt::clamp(1.5f), 1.0f, "common clamp default high");
	test.check_equal(mt::clamp(7.0f, -2.0f, 4.0f), 4.0f, "common clamp custom high");
	test.check(mt::clamp_to_int(7.6f, 0, 5) == 5, "common clamp_to_int high");
	test.check(mt::clamp_to_int(-1.2f, 0, 5) == 0, "common clamp_to_int low");
	test.check(mt::clamp_to_int(2.5f, 0, 5) == 3, "common clamp_to_int round");

	test.check_equal(mt::smoothstep(0.0f, 10.0f, 5.0f), 0.5f, "common smoothstep midpoint");
	test.check_equal(mt::smoothstep(0.0f, 10.0f, -1.0f), 0.0f, "common smoothstep below");
	test.check_equal(mt::smoothstep(0.0f, 10.0f, 11.0f), 1.0f, "common smoothstep above");
	test.check_equal(mt::smoothstep(0.0f, 10.0f, 5.0), 0.5, "common smoothstep float edges");

	test.check_equal(mt::step(2.0f, 1.0f), 0.0f, "common step below");
	test.check_equal(mt::step(2.0f, 3.0f), 1.0f, "common step above");
	test.check_equal(mt::mix(10.0f, 20.0f, 0.25f), 12.5f, "common mix scalar");
	test.check_equal(mt::lerp(10.0f, 20.0f, 0.5f), 15.0f, "common lerp scalar");

	mt::vec2<float> reflected = mt::reflect(mt::vec2<float>(1.0f, -1.0f), mt::vec2<float>(0.0f, 1.0f));
	test.check_equal(reflected.x, 1.0f, "common reflect x");
	test.check_equal(reflected.y, 1.0f, "common reflect y");

	mt::mat4<float> identity = mt::mat4<float>::identity();
	test.check_equal(identity(0, 0), 1.0f, "common umbrella mat4 identity 00");
	test.check_equal(identity(3, 3), 1.0f, "common umbrella mat4 identity 33");

	mt::vec2<float> refracted = mt::refract(mt::vec2<float>(0.0f, -1.0f), mt::vec2<float>(0.0f, 1.0f), 0.5f);
	test.check_equal(refracted.x, 0.0f, "common refract x");
	test.check_equal(refracted.y, -1.0f, "common refract y");

	refracted = mt::refract(mt::vec2<float>(1.0f, 0.0f), mt::vec2<float>(0.0f, 1.0f), 2.0f);
	test.check_equal(refracted.x, 0.0f, "common refract total internal x");
	test.check_equal(refracted.y, 0.0f, "common refract total internal y");
}

int main() {
	TestContext test;

	test_common(test);

	return test.result();
}
