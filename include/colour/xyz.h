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

Type: Xyz

Specification:
- CIE 1931 XYZ with D65 reference white.
- Specification reference: CSS Color 4 predefined xyz / xyz-d65
  https://www.w3.org/TR/css-color-4/#predefined-xyz

Reference white:
- D65, with chromaticity x = 0.3127, y = 0.3290.
- This is the library's primary CIE XYZ hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- RGB conversions use each RGB colour space's published primaries, transfer functions, and native white point handling.
- to_xyz_d50() and to_xyz_aces() use Bradford chromatic adaptation by default.
- to_xyz_d50_with_cat02() and to_xyz_aces_with_cat02() provide explicit CAT02 adaptation paths.
- Oklab and Oklch conversions use the D65-referenced Oklab model.

Components:
- x, y, z: relative tristimulus values; not RGB-clamped and commonly outside 0..1 for saturated colours.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, XYZ, XYZA.
- Operations: clamp(), no_negatives(), sanitise_floats().
- Conversion: RGB spaces, XyzD50, XyzAces, Oklab, and Oklch.

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the transfer-encoded sRGBA type.
template <ColourComponent F> struct Srgb;

// Forward declaration of the linear sRGBA type.
template <ColourComponent F> struct SrgbLinear;

// Forward declaration of the transfer-encoded Display P3 type.
template <ColourComponent F> struct DisplayP3;

// Forward declaration of the linear Display P3 type.
template <ColourComponent F> struct DisplayP3Linear;

// Forward declaration of the CIE XYZ D50 type.
template <ColourComponent F> struct XyzD50;

// Forward declaration of the CIE XYZ ACES white type.
template <ColourComponent F> struct XyzAces;

// Forward declaration of the Oklab type.
template <ColourComponent F> struct Oklab;

// Forward declaration of the Oklch type.
template <ColourComponent F> struct Oklch;

// Forward declaration of the transfer-encoded Adobe RGB (1998) type.
template <ColourComponent F> struct AdobeRgb;

// Forward declaration of the linear Adobe RGB (1998) type.
template <ColourComponent F> struct AdobeRgbLinear;

// Forward declaration of the transfer-encoded ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgb;

// Forward declaration of the linear ProPhoto RGB type.
template <ColourComponent F> struct ProPhotoRgbLinear;

// Forward declaration of the transfer-encoded Rec.709 type.
template <ColourComponent F> struct Rec709;

// Forward declaration of the linear Rec.709 type.
template <ColourComponent F> struct Rec709Linear;

// Forward declaration of the transfer-encoded Rec.2020 type.
template <ColourComponent F> struct Rec2020;

// Forward declaration of the linear Rec.2020 type.
template <ColourComponent F> struct Rec2020Linear;

// Forward declaration of the ACES2065-1 type.
template <ColourComponent F> struct Aces2065_1;

// Forward declaration of the ACEScg type.
template <ColourComponent F> struct Acescg;

/**************************************************************************************************
 * Xyz floating point or Simd floating point colour type.
 * Colourspace : CIE XYZ D65
 * Definition  : https://www.w3.org/TR/css-color-4/#predefined-xyz
 * Scale       : Y = 1
 * PixelOrder  : XYZA
 * Alpha       : Assumed not-premultiplied.
 * ************************************************************************************************/
template <ColourComponent F> struct Xyz {
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
	Xyz() = default;

	// Default destructor.
	~Xyz() = default;

	// Construct with XYZ values and full alpha.
	Xyz(F x_value, F y_value, F z_value) noexcept : x(x_value), y(y_value), z(z_value), alpha(F(1)) { }

	// Construct with XYZA values.
	Xyz(F x_value, F y_value, F z_value, F alpha_value) noexcept : x(x_value), y(y_value), z(z_value), alpha(alpha_value) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Xyz clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Xyz no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Xyz sanitise_floats() const noexcept;

	// Convert to linear sRGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	SrgbLinear<F> to_srgb_linear() const noexcept;

	// Convert to transfer-encoded sRGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Srgb<F> to_srgb() const noexcept;

	// Convert to linear Display P3 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	DisplayP3Linear<F> to_display_p3_linear() const noexcept;

	// Convert to transfer-encoded Display P3 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	DisplayP3<F> to_display_p3() const noexcept;

	// Convert to CIE XYZ D50 (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzD50<F> to_xyz_d50() const noexcept;

	// Convert to CIE XYZ D50 with CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzD50<F> to_xyz_d50_with_cat02() const noexcept;

	// Convert to CIE XYZ with ACES white using Bradford chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzAces<F> to_xyz_aces() const noexcept;

	// Convert to CIE XYZ with ACES white using CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	XyzAces<F> to_xyz_aces_with_cat02() const noexcept;

	// Convert to Oklab (implemented in colour_conversions.h).
	[[nodiscard]]
	Oklab<F> to_oklab() const noexcept;

	// Convert to Oklch (implemented in colour_conversions.h).
	[[nodiscard]]
	Oklch<F> to_oklch() const noexcept;

	// Convert to linear Adobe RGB (1998) RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	AdobeRgbLinear<F> to_adobe_rgb_linear() const noexcept;

	// Convert to transfer-encoded Adobe RGB (1998) RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	AdobeRgb<F> to_adobe_rgb() const noexcept;

	// Convert to linear ProPhoto RGB RGBA using Bradford chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinear<F> to_prophoto_rgb_linear() const noexcept;

	// Convert to transfer-encoded ProPhoto RGB RGBA using Bradford chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgb<F> to_prophoto_rgb() const noexcept;

	// Convert to linear ProPhoto RGB RGBA using CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgbLinear<F> to_prophoto_rgb_linear_with_cat02() const noexcept;

	// Convert to transfer-encoded ProPhoto RGB RGBA using CAT02 chromatic adaptation (implemented in colour_conversions.h).
	[[nodiscard]]
	ProPhotoRgb<F> to_prophoto_rgb_with_cat02() const noexcept;

	// Convert to linear Rec.709 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec709Linear<F> to_rec709_linear() const noexcept;

	// Convert to transfer-encoded Rec.709 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec709<F> to_rec709() const noexcept;

	// Convert to linear Rec.2020 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec2020Linear<F> to_rec2020_linear() const noexcept;

	// Convert to transfer-encoded Rec.2020 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec2020<F> to_rec2020() const noexcept;

	// Convert to ACES2065-1 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Aces2065_1<F> to_aces2065_1() const noexcept;

	// Convert to ACEScg RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Acescg<F> to_acescg() const noexcept;
};

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Xyz<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Xyz(max(zero, min(x, one)), max(zero, min(y, one)), max(zero, min(z, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Xyz<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Xyz(max(zero, x), max(zero, y), max(zero, z), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Xyz<F> Xyz<F>::sanitise_floats() const noexcept {
	return Xyz(detail::sanitise_component(x), detail::sanitise_component(y), detail::sanitise_component(z), detail::sanitise_component(alpha));
}

} // namespace mt
