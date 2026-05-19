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

Type: XyzAces

Specification:
- CIE XYZ tristimulus values referenced to the ACES white point used by ACES2065-1 and ACEScg.
- ACES white point reference:
  https://docs.acescentral.com/white-point/

Reference white:
- ACES white point, x = 0.32168, y = 0.33767.
- This is not this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- Aces2065_1 and Acescg use XyzAces as their native XYZ connection space.
- to_xyz() adapts ACES-white XYZ to this library's CIE XYZ D65 hub using Bradford chromatic adaptation.
- to_xyz_with_cat02() provides the explicit CAT02 adaptation path.
- to_aces2065_1() and to_acescg() apply the AP0 and AP1 inverse RGB matrices.

Components:
- x, y, z: relative tristimulus values; not RGB-clamped and commonly outside 0..1 for saturated colours.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, XYZ, XYZA.
- Operations: clamp(), no_negatives(), sanitise_floats().
- Conversion: to_xyz(), to_xyz_with_cat02(), to_aces2065_1(), to_acescg().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the ACES2065-1 type.
template <ColourComponent F> struct Aces2065_1;

// Forward declaration of the ACEScg type.
template <ColourComponent F> struct Acescg;

/**************************************************************************************************
 * XyzAces floating point or Simd floating point colour type.
 * Colourspace : CIE XYZ with the ACES white point
 * Definition  : SMPTE ST 2065-1 / ACES2065-1
 * Whitepoint  : ACES white, x = 0.32168, y = 0.33767
 * Scale       : Y = 1
 * PixelOrder  : XYZA
 * Alpha       : Assumed not-premultiplied.
 * ************************************************************************************************/
template <ColourComponent F> struct XyzAces {
	// Component type stored in each channel.
	using ComponentType = F;

	// X tristimulus component.
	F x{};

	// Y tristimulus component.
	F y{};

	// Z tristimulus component.
	F z{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	XyzAces() = default;

	// Default destructor.
	~XyzAces() = default;

	// Construct with XYZ values and full alpha.
	XyzAces(F x_value, F y_value, F z_value) noexcept : x(x_value), y(y_value), z(z_value), alpha(F(1)) { }

	// Construct with XYZA values.
	XyzAces(F x_value, F y_value, F z_value, F alpha_value) noexcept : x(x_value), y(y_value), z(z_value), alpha(alpha_value) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	XyzAces clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	XyzAces no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	XyzAces sanitise_floats() const noexcept;

	// Convert to CIE XYZ D65 with Bradford chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;

	// Convert to CIE XYZ D65 with CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz_with_cat02() const noexcept;

	// Convert to ACES2065-1 RGBA using the exact AP0 matrix inverse (implemented in colour_conversions.h).
	[[nodiscard]]
	Aces2065_1<F> to_aces2065_1() const noexcept;

	// Convert to ACEScg RGBA using the exact AP1 matrix inverse (implemented in colour_conversions.h).
	[[nodiscard]]
	Acescg<F> to_acescg() const noexcept;
};

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> XyzAces<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return XyzAces(max(zero, min(x, one)), max(zero, min(y, one)), max(zero, min(z, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> XyzAces<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return XyzAces(max(zero, x), max(zero, y), max(zero, z), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzAces<F> XyzAces<F>::sanitise_floats() const noexcept {
	return XyzAces(detail::sanitise_component(x), detail::sanitise_component(y), detail::sanitise_component(z), detail::sanitise_component(alpha));
}

} // namespace mt
