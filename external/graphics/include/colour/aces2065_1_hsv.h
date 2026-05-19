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

Type: Aces2065_1Hsv

Specification:
- Conventional HSV coordinates over scene-linear ACES2065-1 AP0 RGB components.
- HSV is not part of the ACES2065-1 specification.
- Underlying RGB specification:
  https://docs.acescentral.com/encodings/aces2065-1/

Note: This library stores hue angles in the range 0.0 to 1.0; you must manually multiply by 360 to get degrees, and divide degree values by 360 to store them.

Reference white:
- Inherits the ACES white point from Aces2065_1, x = 0.32168, y = 0.33767.

Conversions:
- Aces2065_1::to_hsv() applies the common RGB to HSV transform to scene-linear AP0 values.
- to_aces2065_1() applies the inverse HSV to RGB transform.
- No transfer conversion, RGB-to-XYZ matrix conversion, or chromatic adaptation is performed by this type.
- wrap_hue() wraps hue to a 0..1 turn.

Components:
- hue: 0..1 turn.
- saturation: normally 0..1 for non-negative RGB input.
- value: scene-linear value component; not limited to 0..1 by the type.
- alpha: non-colour opacity channel, normally 0..1 and passed through unchanged.

API:
- Construction: default, HSV, HSVA.
- Operations: clamp(), no_negatives(), sanitise_floats(), wrap_hue().
- Conversion: to_aces2065_1().

*********************************************************************************************************/
#pragma once

#include <cmath>
#include <concepts>
#include "simd-concepts.h"
#include "common_colour.h"

namespace mt {

// Forward declaration of the linear ACES2065-1 type.
template <ColourComponent F> struct Aces2065_1;

/**************************************************************************************************
 * HSV coordinates for linear ACES2065-1.
 * Colourspace : Linear ACES2065-1 HSV
 * RGB Source  : https://docs.acescentral.com/encodings/aces2065-1/
 * HSV Model   : Conventional HSV transform over linear ACES2065-1 components.
 * Hue         : 0..1 turn
 * Alpha       : Passed through unchanged by conversions.
 * ************************************************************************************************/
template <ColourComponent F> struct Aces2065_1Hsv {
	// Component type stored in each channel.
	using ComponentType = F;

	// Hue component, measured as a 0..1 turn.
	F hue{};

	// Saturation component.
	F saturation{};

	// Value component.
	F value{};

	// Alpha component.
	F alpha{ F(1) };

	// Construct black with full alpha.
	Aces2065_1Hsv() = default;

	// Default destructor.
	~Aces2065_1Hsv() = default;

	// Construct with HSV values and full alpha.
	Aces2065_1Hsv(F hue, F saturation, F value) noexcept : hue(hue), saturation(saturation), value(value), alpha(F(1)) { }

	// Construct with HSVA values.
	Aces2065_1Hsv(F hue, F saturation, F value, F alpha) noexcept : hue(hue), saturation(saturation), value(value), alpha(alpha) { }

	// Clamp all components to the 0..1 range.
	[[nodiscard]]
	Aces2065_1Hsv clamp() const noexcept;

	// Clamp all components to 0 or greater.
	[[nodiscard]]
	Aces2065_1Hsv no_negatives() const noexcept;

	// Replace NaN, infinities, and negative zero with finite canonical values.
	[[nodiscard]]
	Aces2065_1Hsv sanitise_floats() const noexcept;

	// Wrap hue to the 0..1 range.
	[[nodiscard]]
	Aces2065_1Hsv wrap_hue() const noexcept;

	// Convert to linear ACES2065-1 RGBA (implemented in colour_conversions.h).
	[[nodiscard]]
	Aces2065_1<F> to_aces2065_1() const noexcept;
};

/**************************************************************************************************
 * Return this HSV colour with every component clamped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1Hsv<F> Aces2065_1Hsv<F>::clamp() const noexcept {
	using std::max; // For correct overload resolution
	using std::min; // For correct overload resolution
	const F zero = F(0);
	const F one = F(1);
	return Aces2065_1Hsv(max(zero, min(hue, one)), max(zero, min(saturation, one)), max(zero, min(value, one)), max(zero, min(alpha, one)));
}

/**************************************************************************************************
 * Return this HSV colour with every negative component clamped to zero.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1Hsv<F> Aces2065_1Hsv<F>::no_negatives() const noexcept {
	using std::max; // For correct overload resolution
	const F zero = F(0);
	return Aces2065_1Hsv(max(zero, hue), max(zero, saturation), max(zero, value), max(zero, alpha));
}

/**************************************************************************************************
 * Return this HSV colour with invalid floating point values replaced by finite canonical values.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1Hsv<F> Aces2065_1Hsv<F>::sanitise_floats() const noexcept {
	return Aces2065_1Hsv(detail::sanitise_component(hue), detail::sanitise_component(saturation), detail::sanitise_component(value), detail::sanitise_component(alpha));
}

/**************************************************************************************************
 * Return this HSV colour with hue wrapped to the 0..1 range.
 * ************************************************************************************************/
template <ColourComponent F>
[[nodiscard]]
inline Aces2065_1Hsv<F> Aces2065_1Hsv<F>::wrap_hue() const noexcept {
	return Aces2065_1Hsv(detail::wrap_hue_turn(hue), saturation, value, alpha);
}

} // namespace mt
