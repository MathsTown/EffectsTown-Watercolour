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

Type: GenericRgb

Description:
- Generic floating point or SIMD floating point RGBA carrier.
- This type deliberately has no attached RGB colour space.
- Converting a colour-space RGB type to GenericRgb copies channel values.
- Constructing a colour-space RGB type from GenericRgb explicitly interprets those channel values in
  that target colour space.

*********************************************************************************************************/
#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>
#include "common_colour.h"

namespace mt {

/**************************************************************************************************
 * Generic RGB carrier type.
 * Colourspace : None
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F = float> struct GenericRgb {
	// Component type stored in each channel.
	using ComponentType = F;

	// Red component.
	F red{};

	// Green component.
	F green{};

	// Blue component.
	F blue{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	GenericRgb() = default;

	// Default destructor.
	~GenericRgb() = default;

	// Construct with a grey value and full alpha.
	explicit GenericRgb(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with red, green, and blue values and full alpha.
	GenericRgb(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with red, green, blue, and alpha values.
	GenericRgb(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Implicitly construct from any same-component RGB colour-space type.
	template <RgbaColour C>
	    requires std::same_as<typename C::ComponentType, F>
	GenericRgb(const C& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Return this colour as a generic RGB carrier.
	[[nodiscard]]
	GenericRgb to_generic_rgb() const noexcept {
		return *this;
	}

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	GenericRgb premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	GenericRgb un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	GenericRgb clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	GenericRgb clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	GenericRgb clip_rgb() const noexcept;

	// Fit this generic colour into a unit RGB range.
	[[nodiscard]]
	GenericRgb fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	GenericRgb no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	GenericRgb sanitise_floats() const noexcept;
};

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::premultiply_alpha() const noexcept {
	return GenericRgb(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return GenericRgb(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return GenericRgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return GenericRgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Fit this generic colour into a unit RGB range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::fit_gamut() const noexcept {
	return clip_rgb();
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return GenericRgb(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> GenericRgb<F>::sanitise_floats() const noexcept {
	return GenericRgb(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

static_assert(RgbaColour<GenericRgb<float>>);
static_assert(std::is_trivially_copyable_v<GenericRgb<float>>);
static_assert(std::is_standard_layout_v<GenericRgb<float>>);
static_assert(sizeof(GenericRgb<float>) == sizeof(float) * 4);

} // namespace mt
