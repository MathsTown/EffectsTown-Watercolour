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

Type: Oklch

Specification:
- Oklch, polar form of Oklab.
- Reference:
  https://bottosson.github.io/posts/oklab/

Note: This library stores hue angles in the range 0.0 to 1.0; you must manually multiply by 360 to get degrees, and divide degree values by 360 to store them.

Reference white:
- The Oklab definition assumes D65-referenced XYZ input.
- This implementation uses this library's CIE XYZ D65 hub via Oklab.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_oklab() is a polar/cartesian conversion over the Oklab a/b opponent plane.
- to_xyz() converts through Oklab using the original Oklab definition.
- wrap_hue() wraps hue to a 0..1 turn.

Components:
- lightness: perceptual lightness, normally 0..1 for in-gamut display-referred colours.
- chroma: normally 0 or greater.
- hue: 0..1 turn.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, Oklch, OklchA.
- Operations: clamp(), no_negatives(), sanitise_floats(), wrap_hue().
- Conversion: to_oklab(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <cmath>
#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the Oklab type.
template <ColourComponent F> struct Oklab;

/**************************************************************************************************
 * Oklch floating point or Simd floating point colour type.
 * Colourspace : Oklch
 * Definition  : https://bottosson.github.io/posts/oklab/
 * Components  : lightness, chroma, hue, alpha
 * Hue         : 0..1 turn
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct Oklch {
	// Component type stored in each channel.
	using ComponentType = F;

	// Perceptual lightness component.
	F lightness{};

	// Chroma component.
	F chroma{};

	// Hue component, measured as a 0..1 turn.
	F hue{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	Oklch() = default;

	// Default destructor.
	~Oklch() = default;

	// Construct with Oklch values and full alpha.
	Oklch(F lightness, F chroma, F hue) noexcept : lightness(lightness), chroma(chroma), hue(hue), alpha(F(1)) { }

	// Construct with Oklch values and alpha.
	Oklch(F lightness, F chroma, F hue, F alpha_value) noexcept : lightness(lightness), chroma(chroma), hue(hue), alpha(alpha_value) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Oklch clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Oklch no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Oklch sanitise_floats() const noexcept;

	// Wrap hue to the 0..1 range.
	[[nodiscard]]
	Oklch wrap_hue() const noexcept;

	// Convert to Oklab (implemented in colour_conversions.h).
	[[nodiscard]]
	Oklab<F> to_oklab() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Return this Oklch colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklch<F> Oklch<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Oklch(max(zero, min(lightness, one)), max(zero, min(chroma, one)), max(zero, min(hue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this Oklch colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklch<F> Oklch<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Oklch(max(zero, lightness), max(zero, chroma), max(zero, hue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this Oklch colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklch<F> Oklch<F>::sanitise_floats() const noexcept {
	return Oklch(detail::sanitise_component(lightness), detail::sanitise_component(chroma), detail::sanitise_component(hue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this Oklch colour with hue wrapped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklch<F> Oklch<F>::wrap_hue() const noexcept {
	return Oklch(lightness, chroma, detail::wrap_hue_turn(hue), alpha);
}

} // namespace mt
