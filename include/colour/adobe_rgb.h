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

Type: AdobeRgb

Specification:
- Transfer-encoded Adobe RGB (1998), also represented by CSS Color 4 as a98-rgb.
- Specification references:
  https://www.w3.org/TR/css-color-4/#predefined-a98-rgb
  https://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf

Reference white:
- Adobe RGB (1998) uses D65, with chromaticity x = 0.3127, y = 0.3290.
- This implementation converts through this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_linear() applies the Adobe RGB (1998) pure power transfer function over the nominal 0..1 range.
- to_xyz() converts through AdobeRgbLinear and the Adobe RGB (1998) D65 RGB-to-XYZ matrix.
- to_hsv() and to_hsl() are conventional component-space transforms over transfer-encoded Adobe RGB values.

Components:
- red, green, blue: nominal 0..1 transfer-encoded colour channels; out-of-range values are preserved unless clamped.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, grey, RGB, RGBA.
- Operations: premultiply_alpha(), un_premultiply_alpha(), clamp(), clip(), clip_rgb(), fit_gamut(), no_negatives(), sanitise_floats().
- Conversion: to_linear(), to_hsv(), to_hsl(), to_xyz().

*********************************************************************************************************/
#pragma once

#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the linear Adobe RGB (1998) type.
template <ColourComponent F> struct AdobeRgbLinear;

// Forward declaration of the transfer-encoded Adobe RGB (1998) HSV type.
template <ColourComponent F> struct AdobeRgbHsv;

// Forward declaration of the transfer-encoded Adobe RGB (1998) HSL type.
template <ColourComponent F> struct AdobeRgbHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

/**************************************************************************************************
 * AdobeRgb floating point or Simd floating point colour type.
 * Colourspace : Adobe RGB (1998), transfer-encoded
 * Definition  : https://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct AdobeRgb {
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
	AdobeRgb() = default;

	// Default destructor.
	~AdobeRgb() = default;

	// Construct with a grey value and full alpha.
	AdobeRgb(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	AdobeRgb(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with Adobe RGB (1998) RGBA values.
	AdobeRgb(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	AdobeRgb premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	AdobeRgb un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	AdobeRgb clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	AdobeRgb clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	AdobeRgb clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	AdobeRgb fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	AdobeRgb no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	AdobeRgb sanitise_floats() const noexcept;

	// Convert to linear Adobe RGB (1998) RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	AdobeRgbLinear<F> to_linear() const noexcept;

	// Convert to Adobe RGB (1998) HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	AdobeRgbHsv<F> to_hsv() const noexcept;

	// Convert to Adobe RGB (1998) HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	AdobeRgbHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::premultiply_alpha() const noexcept {
	return AdobeRgb(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return AdobeRgb(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return AdobeRgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return AdobeRgb(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return AdobeRgb(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AdobeRgb<F> AdobeRgb<F>::sanitise_floats() const noexcept {
	return AdobeRgb(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

} // namespace mt
