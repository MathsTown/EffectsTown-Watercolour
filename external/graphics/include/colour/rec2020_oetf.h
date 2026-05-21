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

Type: Rec2020Oetf

Specification:
- OETF-encoded Rec.2020 RGB using the ITU-R BT.2020 primaries and BT.2020 opto-electronic transfer function.
- Specification: ITU-R BT.2020
  https://www.itu.int/rec/R-REC-BT.2020

Reference white:
- Rec.2020 uses D65, with chromaticity x = 0.3127, y = 0.3290.
- This implementation converts through this library's CIE XYZ D65 hub.
- XYZ values are relative tristimulus values with Y = 1.0 for diffuse white.

Conversions:
- to_linear() applies the inverse BT.2020 opto-electronic transfer function using alpha = 1.099 and beta = 0.018.
- to_xyz() converts through Rec2020Linear and the Rec.2020 D65 RGB-to-XYZ matrix.
- to_hsv() and to_hsl() are conventional component-space transforms over OETF-encoded Rec.2020 values.
- This is not the CSS Color 4 rec2020 pure gamma 2.4 transfer model.

Components:
- red, green, blue: nominal 0..1 OETF-encoded colour channels; out-of-range values are preserved unless clamped.
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
#include "generic_rgb.h"

namespace mt {

// Forward declaration of the linear Rec.2020 type.
template <ColourComponent F> struct Rec2020Linear;

// Forward declaration of the OETF-encoded Rec.2020 HSV type.
template <ColourComponent F> struct Rec2020OetfHsv;

// Forward declaration of the OETF-encoded Rec.2020 HSL type.
template <ColourComponent F> struct Rec2020OetfHsl;

// Forward declaration of the CIE XYZ D65 type.
template <ColourComponent F> struct Xyz;

/**************************************************************************************************
 * Rec2020Oetf floating point or Simd floating point colour type.
 * Colourspace : Rec.2020, OETF-encoded
 * Definition  : https://www.itu.int/rec/R-REC-BT.2020
 * PixelOrder  : RGBA
 * Alpha       : Assumed not-premultiplied for all operations except unpremultiply_alpha().
 * ************************************************************************************************/
template <ColourComponent F> struct Rec2020Oetf {
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
	Rec2020Oetf() = default;

	// Default destructor.
	~Rec2020Oetf() = default;

	// Construct with a grey value and full alpha.
	Rec2020Oetf(F grey) noexcept : red(grey), green(grey), blue(grey), alpha(F(1)) { }

	// Construct with RGB values and full alpha.
	Rec2020Oetf(F red, F green, F blue) noexcept : red(red), green(green), blue(blue), alpha(F(1)) { }

	// Construct with Rec.2020 RGBA values.
	Rec2020Oetf(F red, F green, F blue, F alpha) noexcept : red(red), green(green), blue(blue), alpha(alpha) { }

	// Explicitly construct from generic RGB channel values.
	explicit Rec2020Oetf(const GenericRgb<F>& colour) noexcept : red(colour.red), green(colour.green), blue(colour.blue), alpha(colour.alpha) { }

	// Premultiply alpha into the red, green, and blue components.
	[[nodiscard]]
	Rec2020Oetf premultiply_alpha() const noexcept;

	// Un-premultiply alpha from the red, green, and blue components.
	[[nodiscard]]
	Rec2020Oetf un_premultiply_alpha() const noexcept;

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Rec2020Oetf clamp() const noexcept;

	// Clip all components to the 0..1 range.
	[[nodiscard]]
	Rec2020Oetf clip() const noexcept;

	// Clip the red, green, and blue components to the 0..1 range, preserving alpha.
	[[nodiscard]]
	Rec2020Oetf clip_rgb() const noexcept;

	// Fit this colour into its RGB gamut (implemented in gamut.h).
	[[nodiscard]]
	Rec2020Oetf fit_gamut() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Rec2020Oetf no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Rec2020Oetf sanitise_floats() const noexcept;

	// Convert to generic RGB channel values.
	[[nodiscard]]
	GenericRgb<F> to_generic_rgb() const noexcept;

	// Convert to linear Rec.2020 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec2020Linear<F> to_linear() const noexcept;

	// Convert to Rec.2020 HSV (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec2020OetfHsv<F> to_hsv() const noexcept;

	// Convert to Rec.2020 HSL (implemented in colour_conversions.h).
	[[nodiscard]]
	Rec2020OetfHsl<F> to_hsl() const noexcept;

	// Convert to CIE XYZ D65 (implemented in colour_conversions.h).
	[[nodiscard]]
	Xyz<F> to_xyz() const noexcept;
};

/**************************************************************************************************
 * Return this colour with alpha multiplied into the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::premultiply_alpha() const noexcept {
	return Rec2020Oetf(red * alpha, green * alpha, blue * alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with alpha divided out of the colour channels.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::un_premultiply_alpha() const noexcept {
	const F safe_alpha = if_equal(alpha, F(0), F(1), alpha);
	return Rec2020Oetf(red / safe_alpha, green / safe_alpha, blue / safe_alpha, alpha);
}

/**************************************************************************************************
 * Return this colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Rec2020Oetf(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this colour with every component clipped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::clip() const noexcept {
	return clamp();
}

/**************************************************************************************************
 * Return this colour with red, green, and blue clipped to the 0..1 range, preserving alpha.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::clip_rgb() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Rec2020Oetf(max(zero, min(red, one)), max(zero, min(green, one)), max(zero, min(blue, one)), alpha);
}

/**************************************************************************************************
 * Return this colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Rec2020Oetf(max(zero, red), max(zero, green), max(zero, blue), max(zero, alpha));
}

/**************************************************************************************************
 * Return this colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Rec2020Oetf<F> Rec2020Oetf<F>::sanitise_floats() const noexcept {
	return Rec2020Oetf(detail::sanitise_component(red), detail::sanitise_component(green), detail::sanitise_component(blue), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this colour as generic RGB channel values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline GenericRgb<F> Rec2020Oetf<F>::to_generic_rgb() const noexcept {
	return GenericRgb<F>(red, green, blue, alpha);
}

} // namespace mt
