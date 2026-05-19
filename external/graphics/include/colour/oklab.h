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

Type: Oklab

Specification:
- Oklab as defined by Bjorn Ottosson.
- Reference:
  https://bottosson.github.io/posts/oklab/

Reference white:
- The Oklab definition assumes D65-referenced XYZ input.
- This implementation uses this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- Xyz::to_oklab() and to_xyz() use the matrices and nonlinear cube-root model from the original Oklab definition.
- to_oklch() is a polar conversion over the a/b opponent plane. Hue is a 0..1 turn.

Components:
- lightness: perceptual lightness, normally 0..1 for in-gamut display-referred colours.
- a: green-red opponent axis, signed.
- b: blue-yellow opponent axis, signed.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, Oklab, OklabA.
- Operations: clamp(), no_negatives(), sanitise_floats().
- Conversion: to_oklch(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the Oklch type.
template <ColourComponent F> struct Oklch;

/**************************************************************************************************
 * Oklab floating point or Simd floating point colour type.
 * Colourspace : Oklab
 * Definition  : https://bottosson.github.io/posts/oklab/
 * Components  : lightness, a, b, alpha
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct Oklab {
	// Component type stored in each channel.
	using ComponentType = F;

	// Perceptual lightness component.
	F lightness{};

	// Green-red opponent component.
	F a{};

	// Blue-yellow opponent component.
	F b{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	Oklab() = default;

	// Default destructor.
	~Oklab() = default;

	// Construct with Oklab values and full alpha.
	Oklab(F lightness, F green_red, F blue_yellow) noexcept : lightness(lightness), a(green_red), b(blue_yellow), alpha(F(1)) { }

	// Construct with Oklab values and alpha.
	Oklab(F lightness, F green_red, F blue_yellow, F alpha_value) noexcept : lightness(lightness), a(green_red), b(blue_yellow), alpha(alpha_value) { }

	// Clamp lightness and alpha to the 0..1 range.
	[[nodiscard]]
	Oklab clamp() const noexcept;

	// Clamp negative lightness and alpha to zero.
	[[nodiscard]]
	Oklab no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Oklab sanitise_floats() const noexcept;

	// Convert to Oklch (implemented in colour_conversions.h).
	[[nodiscard]]
	Oklch<F> to_oklch() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Return this Oklab colour with bounded components clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklab<F> Oklab<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Oklab(max(zero, min(lightness, one)), a, b, max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this Oklab colour with negative bounded components clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklab<F> Oklab<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Oklab(max(zero, lightness), a, b, max(zero, alpha));
}

/**************************************************************************************************
 * Return this Oklab colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Oklab<F> Oklab<F>::sanitise_floats() const noexcept {
	return Oklab(detail::sanitise_component(lightness), detail::sanitise_component(a), detail::sanitise_component(b), detail::sanitise_component(alpha));
}

} // namespace mt
