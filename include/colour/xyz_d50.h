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

Type: XyzD50

Specification:
- CIE 1931 XYZ with D50 reference white.
- Specification reference: CSS Color 4 predefined xyz-d50
  https://www.w3.org/TR/css-color-4/#predefined-xyz

Reference white:
- D50, with chromaticity x = 0.3457, y = 0.3585.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_xyz() adapts D50 XYZ to this library's CIE XYZ D65 hub using Bradford chromatic adaptation.
- to_xyz_with_cat02() provides the explicit CAT02 adaptation path.
- Cielab, Cielch, and ProPhoto RGB use XyzD50 as their native XYZ connection space.

Components:
- x, y, z: relative tristimulus values; not RGB-clamped and commonly outside 0..1 for saturated colours.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, XYZ, XYZA.
- Operations: clamp(), no_negatives(), sanitise_floats().
- Conversion: to_xyz(), to_xyz_with_cat02(), to_cielab(), to_cielch(), ProPhoto RGB.

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

// Forward declaration of the CIELAB D50 type.
template <ColourComponent F> struct Cielab;

// Forward declaration of the CIELCH D50 type.
template <ColourComponent F> struct Cielch;

// Forward declaration of the transfer-encoded ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgb;

// Forward declaration of the linear ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgbLinear;

/**************************************************************************************************
 * XyzD50 floating point or Simd floating point colour type.
 * Colourspace : CIE XYZ D50
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-xyz
 * Adaptation  : Bradford chromatic adaptation from CIE XYZ D65.
 * Scale       : Y = 1
 * PixelOrder  : XYZA
 * Alpha       : Assumed not-premultiplied.
 * ************************************************************************************************/
template <ColourComponent F> struct XyzD50 {
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
	XyzD50() = default;

	// Default destructor.
	~XyzD50() = default;

	// Construct with XYZ values and full alpha.
	XyzD50(F x_value, F y_value, F z_value) noexcept : x(x_value), y(y_value), z(z_value), alpha(F(1)) { }

	// Construct with XYZ and alpha values.
	XyzD50(F x_value, F y_value, F z_value, F alpha_value) noexcept : x(x_value), y(y_value), z(z_value), alpha(alpha_value) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	XyzD50 clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	XyzD50 no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	XyzD50 sanitise_floats() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;

	// Convert to CIE XYZ D65 with CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz_with_cat02() const noexcept;

	// Convert to CIELAB D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	Cielab<F> to_cielab() const noexcept;

	// Convert to CIELCH D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	Cielch<F> to_cielch() const noexcept;

	// Convert to linear ProPhoto RGB RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinear<F> to_prophoto_rgb_linear() const noexcept;

	// Convert to transfer-encoded ProPhoto RGB RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgb<F> to_prophoto_rgb() const noexcept;
};

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> XyzD50<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return XyzD50(max(zero, min(x, one)), max(zero, min(y, one)), max(zero, min(z, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> XyzD50<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return XyzD50(max(zero, x), max(zero, y), max(zero, z), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline XyzD50<F> XyzD50<F>::sanitise_floats() const noexcept {
	return XyzD50(detail::sanitise_component(x), detail::sanitise_component(y), detail::sanitise_component(z), detail::sanitise_component(alpha));
}

} // namespace mt
