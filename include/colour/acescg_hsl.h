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

Type: AcescgHsl

Specification:
- Conventional HSL coordinates over scene-linear ACEScg AP1 RGB components.
- HSL is not part of the ACEScg specification.
- Underlying RGB specification:
  https://docs.acescentral.com/encodings/acescg/

Note: This library stores hue angles in the range 0.0 to 1.0; you must manually multiply by 360 to get degrees, and divide degree values by 360 to store them.

Reference white:
- Inherits the ACES white point from Acescg, x = 0.32168, y = 0.33767.

Conversions:
- Acescg::to_hsl() applies the common RGB to HSL transform to scene-linear AP1 values.
- to_acescg() applies the inverse HSL to RGB transform.
- No transfer conversion, RGB-to-XYZ matrix conversion, or chromatic adaptation is performed by this type.
- wrap_hue() wraps hue to a 0..1 turn.

Components:
- hue: 0..1 turn.
- saturation: normally 0..1 for non-negative RGB input.
- lightness: scene-linear lightness component; not limited to 0..1 by the type.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, HSL, HSLA.
- Operations: clamp(), no_negatives(), sanitise_floats(), wrap_hue().
- Conversion: to_acescg().

*********************************************************************************************************/
#pragma once

#include <cmath>
#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the linear ACEScg type.
template <ColourComponent F> struct Acescg;

/**************************************************************************************************
 * HSL coordinates for linear ACEScg.
 * Colourspace : Linear ACEScg HSL
 * RGB Source  : https://docs.acescentral.com/encodings/acescg/
 * HSL Model   : HSL transform applied to linear ACEScg components.
 * Hue         : 0..1 turn
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct AcescgHsl {
	// Component type stored in each channel.
	using ComponentType = F;

	// Hue component, measured as a 0..1 turn.
	F hue{};

	// Saturation component.
	F saturation{};

	// Lightness component.
	F lightness{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	AcescgHsl() = default;

	// Default destructor.
	~AcescgHsl() = default;

	// Construct with HSL values and full alpha.
	AcescgHsl(F hue, F saturation, F lightness) noexcept : hue(hue), saturation(saturation), lightness(lightness), alpha(F(1)) { }

	// Construct with HSLA values.
	AcescgHsl(F hue, F saturation, F lightness, F alpha) noexcept : hue(hue), saturation(saturation), lightness(lightness), alpha(alpha) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	AcescgHsl clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	AcescgHsl no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	AcescgHsl sanitise_floats() const noexcept;

	// Wrap hue to the 0..1 range.
	[[nodiscard]]
	AcescgHsl wrap_hue() const noexcept;

	// Convert to linear ACEScg RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Acescg<F> to_acescg() const noexcept;
};

/**************************************************************************************************
 * Return this HSL colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AcescgHsl<F> AcescgHsl<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return AcescgHsl(max(zero, min(hue, one)), max(zero, min(saturation, one)), max(zero, min(lightness, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this HSL colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AcescgHsl<F> AcescgHsl<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return AcescgHsl(max(zero, hue), max(zero, saturation), max(zero, lightness), max(zero, alpha));
}

/**************************************************************************************************
 * Return this HSL colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AcescgHsl<F> AcescgHsl<F>::sanitise_floats() const noexcept {
	return AcescgHsl(detail::sanitise_component(hue), detail::sanitise_component(saturation), detail::sanitise_component(lightness), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this HSL colour with hue wrapped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline AcescgHsl<F> AcescgHsl<F>::wrap_hue() const noexcept {
	return AcescgHsl(detail::wrap_hue_turn(hue), saturation, lightness, alpha);
}

} // namespace mt
