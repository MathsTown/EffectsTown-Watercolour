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

Type: Cielch

Specification:
- CIE 1976 L*C*h, polar form of CIE L*a*b*.
- Reference: CSS Color 4 Lab/LCH definition
  https://www.w3.org/TR/css-color-4/#lab-colors

Note: This library stores hue angles in the range 0.0 to 1.0; you must manually multiply by 360 to get degrees, and divide degree values by 360 to store them.

Reference white:
- This implementation is D50-referenced and converts through XyzD50 via Cielab.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_cielab() is a polar/cartesian conversion over the Lab a/b opponent plane.
- to_xyz_d50() converts through Cielab using the CIE 1976 Lab equations.
- wrap_hue() wraps hue to a 0..1 turn.

Components:
- lightness: L*, normally 0..100.
- chroma: C*, normally 0 or greater.
- hue: 0..1 turn.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, LCh, LChA.
- Operations: clamp(), no_negatives(), sanitise_floats(), wrap_hue().
- Conversion: to_cielab(), to_xyz_d50().

*********************************************************************************************************/
#pragma once

#include <cmath>
#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the CIE XYZ D50 type.
template <ColourComponent F> struct XyzD50;

// Forward declaration of the CIELAB D50 type.
template <ColourComponent F> struct Cielab;

/**************************************************************************************************
 * CIELCH floating point or Simd floating point colour type.
 * Colourspace : CIELCH D50
 * Definition  : Polar form of CIE 1976 L*a*b*
 * White point : D50
 * Components  : lightness, chroma, hue, alpha
 * Hue         : 0..1 turn
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct Cielch {
	// Component type stored in each channel.
	using ComponentType = F;

	// L* perceptual lightness component.
	F lightness{};

	// C* chroma component.
	F chroma{};

	// Hue component, measured as a 0..1 turn.
	F hue{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	Cielch() = default;

	// Default destructor.
	~Cielch() = default;

	// Construct with CIELCH values and full alpha.
	Cielch(F lightness, F chroma, F hue) noexcept : lightness(lightness), chroma(chroma), hue(hue), alpha(F(1)) { }

	// Construct with CIELCH values and alpha.
	Cielch(F lightness, F chroma, F hue, F alpha_value) noexcept : lightness(lightness), chroma(chroma), hue(hue), alpha(alpha_value) { }

	// Clamp lightness to 0..100, chroma to 0 or greater, hue to 0..1, and alpha to 0..1.
	[[nodiscard]]
	Cielch clamp() const noexcept;

	// Clamp negative bounded components to zero.
	[[nodiscard]]
	Cielch no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Cielch sanitise_floats() const noexcept;

	// Wrap hue to the 0..1 range.
	[[nodiscard]]
	Cielch wrap_hue() const noexcept;

	// Convert to CIELAB D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	Cielab<F> to_cielab() const noexcept;

	// Convert to CIE XYZ D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzD50<F> to_xyz_d50() const noexcept;
};

/**************************************************************************************************
 * Return this CIELCH colour with bounded components clamped to their valid ranges.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielch<F> Cielch<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	const F one_hundred = F(100);
	return Cielch(max(zero, min(lightness, one_hundred)), max(zero, chroma), max(zero, min(hue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this CIELCH colour with negative bounded components clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielch<F> Cielch<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Cielch(max(zero, lightness), max(zero, chroma), max(zero, hue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this CIELCH colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielch<F> Cielch<F>::sanitise_floats() const noexcept {
	return Cielch(detail::sanitise_component(lightness), detail::sanitise_component(chroma), detail::sanitise_component(hue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this CIELCH colour with hue wrapped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielch<F> Cielch<F>::wrap_hue() const noexcept {
	return Cielch(lightness, chroma, detail::wrap_hue_turn(hue), alpha);
}

} // namespace mt
