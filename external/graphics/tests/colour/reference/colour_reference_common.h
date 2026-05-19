/********************************************************************************************************

Generated colour reference data.
Common colour reference structs.

Source:      colour-science 0.4.6
Generator:   tests/colour/reference/generate_colour_reference.py
Canonical:   CIE XYZ D65, Y=1
Observer:    CIE 1931 2 Degree Standard Observer
RGB CAT:     Bradford by default; CAT02 arrays are generated separately where the C++ API exposes explicit CAT02 methods
Lab CAT:     Bradford

 * Common structs and metadata shared by the generated colour reference headers.
 * Each value array contains four doubles.
 * For RGB-like data the order is r, g, b, a.
 * For CIE XYZ data the order is X, Y, Z, alpha.
 * For HSV/HSL data the order is h, s, v/l, a.
 * For CIELAB data the order is L*, a*, b*, alpha.
 * For CIELCH data the order is L*, C*, h, alpha; h is a 0..1 turn.
 * For Oklab data the order is L, a, b, alpha.
 * For Oklch data the order is L, C, h, alpha; h is a 0..1 turn.
 * Case groups include CSS named sRGB colours, alpha cases, wide-gamut RGB samples, and numeric stress samples.
 *
 * Alpha is copied unchanged into every representation.
 * Values are not clipped. Arrays contain finite reference rows only.
 * When present, HSV/HSL values are RGB-coordinate transforms with hue as a 0..1 turn.
 * HSV/HSL arrays omit numeric stress rows and near-achromatic rows where hue is numerically unstable.
 * HSV/HSL saturation, value, and lightness are not clamped for over-range RGB source data.
 * Hue is normalised with h - floor(h), matching the C++ colour conversion helpers.
 *
 * Do not edit by hand. Regenerate with generate_colour_reference.py.
 *********************************************************************************************************/
#pragma once

namespace colour_reference {

inline constexpr const char* reference_library = "colour-science 0.4.6";
inline constexpr const char* canonical_xyz = "CIE XYZ D65, Y=1";
inline constexpr const char* rgb_chromatic_adaptation_transform = "Bradford by default; CAT02 arrays are generated separately where the C++ API exposes explicit CAT02 methods";
inline constexpr const char* lab_chromatic_adaptation_transform = "Bradford";
inline constexpr int source_case_count = 241;

struct ColourReferenceCase {
	const char* name;
	const char* group;
	double source[4];
	double expected[4];
};

} // namespace colour_reference
