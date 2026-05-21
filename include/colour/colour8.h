/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************

Type: Colour8

Description:
- Scalar 8-bit RGBA colour carrier.
- Colour8 has no conversion knowledge except to and from GenericRgb<float>.

*********************************************************************************************************/
#pragma once

#include <bit>
#include <cstdint>
#include <string>
#include <type_traits>
#include "generic_rgb.h"

namespace mt {

namespace detail {

	/**************************************************************************************************
	 * Convert a float component in nominal 0..1 range to an 8-bit component.
	 * ************************************************************************************************/
	[[nodiscard]]
	inline std::uint8_t float_to_colour8_component(float value) noexcept {
		const float scaled = value * 255.0f;
		if (scaled <= 0.0f) {
			return 0;
		}
		if (scaled >= 255.0f) {
			return 0xff;
		}
		return static_cast<std::uint8_t>(scaled);
	}

} // namespace detail

/**************************************************************************************************
 * Scalar 8-bit RGBA colour carrier.
 * ************************************************************************************************/
struct Colour8 {
	// Red component.
	std::uint8_t red{};

	// Green component.
	std::uint8_t green{};

	// Blue component.
	std::uint8_t blue{};

	// Alpha component.
	std::uint8_t alpha{ 0xff };

	// Construct black with full alpha.
	Colour8() = default;

	// Construct with red, green, blue, and alpha values.
	constexpr Colour8(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 0xff) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from a generic scalar RGB colour.
	explicit Colour8(const GenericRgb<float>& colour) noexcept : red(detail::float_to_colour8_component(colour.red)), green(detail::float_to_colour8_component(colour.green)), blue(detail::float_to_colour8_component(colour.blue)), alpha(detail::float_to_colour8_component(colour.alpha)) { }

	// Convert to generic scalar RGB.
	[[nodiscard]]
	GenericRgb<float> to_generic_rgb() const noexcept {
		constexpr float scale = 1.0f / 255.0f;
		return GenericRgb<float>(float(red) * scale, float(green) * scale, float(blue) * scale, float(alpha) * scale);
	}

	// Return this colour as an unsigned 32-bit value in RGBA order.
	[[nodiscard]]
	std::uint32_t to_uint32() const noexcept {
		return (std::uint32_t(red) << 24) | (std::uint32_t(green) << 16) | (std::uint32_t(blue) << 8) | std::uint32_t(alpha);
	}

	// Return an unsigned 32-bit value with the same memory layout as this struct.
	[[nodiscard]]
	std::uint32_t to_uint32_keep_memory_layout() const noexcept {
		return std::bit_cast<std::uint32_t>(*this);
	}

	// Convert to string.
	[[nodiscard]]
	std::string to_string() const {
		return "Colour8{r=" + std::to_string(static_cast<unsigned>(red)) + ", g=" + std::to_string(static_cast<unsigned>(green)) + ", b=" + std::to_string(static_cast<unsigned>(blue)) + ", a=" + std::to_string(static_cast<unsigned>(alpha)) + "}";
	}
};

static_assert(sizeof(Colour8) == sizeof(std::uint8_t) * 4);
static_assert(std::is_trivially_copyable_v<Colour8>);
static_assert(std::is_standard_layout_v<Colour8>);

} // namespace mt
