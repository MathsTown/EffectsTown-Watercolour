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

Type: Cielab

Specification:
- CIE 1976 L*a*b*.
- Reference: CSS Color 4 Lab definition
  https://www.w3.org/TR/css-color-4/#lab-colors

Reference white:
- This implementation is D50-referenced and converts through XyzD50.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- XyzD50::to_cielab() and to_xyz_d50() use the CIE 1976 Lab equations with the standard epsilon/kappa transition.
- to_cielch() is a polar conversion over the a/b opponent plane. Hue is a 0..1 turn.

Components:
- lightness: L*, normally 0..100.
- a: green-red opponent axis, signed.
- b: blue-yellow opponent axis, signed.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, Lab, LabA.
- Operations: clamp(), no_negatives(), sanitise_floats().
- Conversion: to_cielch(), to_xyz_d50().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the CIE XYZ D50 type.
template <ColourComponent F> struct XyzD50;

// Forward declaration of the CIELCH D50 type.
template <ColourComponent F> struct Cielch;

/**************************************************************************************************
 * CIELAB floating point or Simd floating point colour type.
 * Colourspace : CIELAB D50
 * Definition  : CIE 1976 L*a*b*
 * White point : D50
 * Components  : lightness, a, b, alpha
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct Cielab {
	// Component type stored in each channel.
	using ComponentType = F;

	// L* perceptual lightness component.
	F lightness{};

	// a* green-red opponent component.
	F a{};

	// b* blue-yellow opponent component.
	F b{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	Cielab() = default;

	// Default destructor.
	~Cielab() = default;

	// Construct with CIELAB values and full alpha.
	Cielab(F lightness, F green_red, F blue_yellow) noexcept : lightness(lightness), a(green_red), b(blue_yellow), alpha(F(1)) { }

	// Construct with CIELAB values and alpha.
	Cielab(F lightness, F green_red, F blue_yellow, F alpha_value) noexcept : lightness(lightness), a(green_red), b(blue_yellow), alpha(alpha_value) { }

	// Clamp lightness to 0..100 and alpha to 0..1.
	[[nodiscard]]
	Cielab clamp() const noexcept;

	// Clamp negative lightness and alpha to zero.
	[[nodiscard]]
	Cielab no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Cielab sanitise_floats() const noexcept;

	// Convert to CIELCH D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	Cielch<F> to_cielch() const noexcept;

	// Convert to CIE XYZ D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzD50<F> to_xyz_d50() const noexcept;
};

/**************************************************************************************************
 * Return this CIELAB colour with bounded components clamped to their valid ranges.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielab<F> Cielab<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	const F one_hundred = F(100);
	return Cielab(max(zero, min(lightness, one_hundred)), a, b, max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this CIELAB colour with negative bounded components clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielab<F> Cielab<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Cielab(max(zero, lightness), a, b, max(zero, alpha));
}

/**************************************************************************************************
 * Return this CIELAB colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Cielab<F> Cielab<F>::sanitise_floats() const noexcept {
	return Cielab(detail::sanitise_component(lightness), detail::sanitise_component(a), detail::sanitise_component(b), detail::sanitise_component(alpha));
}

} // namespace mt
