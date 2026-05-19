/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Colour8 tests.
*********************************************************************************************************/

#include "../test.h"

#include <bit>
#include <cstdint>
#include <string>
#include <type_traits>

#include "colour/colour8.h"

void test_colour8(TestContext& test) {
	static_assert(sizeof(mt::Colour8) == 4);
	static_assert(std::is_trivially_copyable_v<mt::Colour8>);
	static_assert(std::is_standard_layout_v<mt::Colour8>);
	static_assert(!mt::RgbaColour<mt::Colour8>);

	mt::Colour8 default_value;
	test.check(default_value.red == 0, "Colour8 default red");
	test.check(default_value.green == 0, "Colour8 default green");
	test.check(default_value.blue == 0, "Colour8 default blue");
	test.check(default_value.alpha == 0xff, "Colour8 default alpha");

	mt::Colour8 rgba(0x12, 0x34, 0x56, 0x78);
	test.check(rgba.red == 0x12, "Colour8 rgba red");
	test.check(rgba.green == 0x34, "Colour8 rgba green");
	test.check(rgba.blue == 0x56, "Colour8 rgba blue");
	test.check(rgba.alpha == 0x78, "Colour8 rgba alpha");
	test.check(rgba.to_uint32() == 0x12345678u, "Colour8 to_uint32 RGBA order");
	test.check(rgba.to_uint32_keep_memory_layout() == std::bit_cast<std::uint32_t>(rgba), "Colour8 to_uint32_keep_memory_layout");
	test.check(rgba.to_string() == "Colour8{r=18, g=52, b=86, a=120}", "Colour8 to_string");

	mt::GenericRgb<float> generic = rgba.to_generic_rgb();
	test.check_close(generic.red, 18.0f / 255.0f, "Colour8 to_generic_rgb red");
	test.check_close(generic.green, 52.0f / 255.0f, "Colour8 to_generic_rgb green");
	test.check_close(generic.blue, 86.0f / 255.0f, "Colour8 to_generic_rgb blue");
	test.check_close(generic.alpha, 120.0f / 255.0f, "Colour8 to_generic_rgb alpha");

	mt::Colour8 from_generic(mt::GenericRgb<float>(0.5f, -0.25f, 1.25f, 0.25f));
	test.check(from_generic.red == 127, "Colour8 from GenericRgb red truncates");
	test.check(from_generic.green == 0, "Colour8 from GenericRgb green clips low");
	test.check(from_generic.blue == 0xff, "Colour8 from GenericRgb blue clips high");
	test.check(from_generic.alpha == 63, "Colour8 from GenericRgb alpha truncates");
}

int main() {
	TestContext test;

	test_colour8(test);

	return test.result();
}
