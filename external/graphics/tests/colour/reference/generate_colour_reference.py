"""
********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Generate colour conversion reference data using colour-science.
*********************************************************************************************************
"""

from __future__ import annotations

import math
import warnings
from pathlib import Path

try:
	import colour
	import numpy as np
except ImportError as error:
	raise SystemExit(
		"Missing reference generation dependency. Install with:\n"
		"    python -m pip install colour-science==0.4.6 numpy==1.25.2\n"
		f"\nImport error: {error}"
	)


REFERENCE_DIR = Path(__file__).parent
OBSERVER = "CIE 1931 2 Degree Standard Observer"
D65 = colour.CCS_ILLUMINANTS[OBSERVER]["D65"]
D50 = colour.CCS_ILLUMINANTS[OBSERVER]["D50"]
D65_XYZ = colour.xy_to_XYZ(D65)
D50_XYZ = colour.xy_to_XYZ(D50)
ACES_WHITE = colour.RGB_COLOURSPACES["ACES2065-1"].whitepoint
ACES_WHITE_XYZ = colour.xy_to_XYZ(ACES_WHITE)
DEFAULT_RGB_CHROMATIC_ADAPTATION_TRANSFORM = "Bradford"
CSS_RGB_CHROMATIC_ADAPTATION_TRANSFORM = "Bradford"
CAT02_CHROMATIC_ADAPTATION_TRANSFORM = "CAT02"
LAB_CHROMATIC_ADAPTATION_TRANSFORM = "Bradford"
OBSOLETE_REFERENCE_HEADERS = [
	"perceptual_reference.h",
	"xyz_reference.h",
]
HSV_HSL_MIN_CHROMA = 1.0e-4
HSL_MIN_DENOMINATOR = 1.0e-4

CSS_NAMED_COLOURS = {
	"aliceblue": "#f0f8ff",
	"antiquewhite": "#faebd7",
	"aqua": "#00ffff",
	"aquamarine": "#7fffd4",
	"azure": "#f0ffff",
	"beige": "#f5f5dc",
	"bisque": "#ffe4c4",
	"black": "#000000",
	"blanchedalmond": "#ffebcd",
	"blue": "#0000ff",
	"blueviolet": "#8a2be2",
	"brown": "#a52a2a",
	"burlywood": "#deb887",
	"cadetblue": "#5f9ea0",
	"chartreuse": "#7fff00",
	"chocolate": "#d2691e",
	"coral": "#ff7f50",
	"cornflowerblue": "#6495ed",
	"cornsilk": "#fff8dc",
	"crimson": "#dc143c",
	"cyan": "#00ffff",
	"darkblue": "#00008b",
	"darkcyan": "#008b8b",
	"darkgoldenrod": "#b8860b",
	"darkgray": "#a9a9a9",
	"darkgreen": "#006400",
	"darkgrey": "#a9a9a9",
	"darkkhaki": "#bdb76b",
	"darkmagenta": "#8b008b",
	"darkolivegreen": "#556b2f",
	"darkorange": "#ff8c00",
	"darkorchid": "#9932cc",
	"darkred": "#8b0000",
	"darksalmon": "#e9967a",
	"darkseagreen": "#8fbc8f",
	"darkslateblue": "#483d8b",
	"darkslategray": "#2f4f4f",
	"darkslategrey": "#2f4f4f",
	"darkturquoise": "#00ced1",
	"darkviolet": "#9400d3",
	"deeppink": "#ff1493",
	"deepskyblue": "#00bfff",
	"dimgray": "#696969",
	"dimgrey": "#696969",
	"dodgerblue": "#1e90ff",
	"firebrick": "#b22222",
	"floralwhite": "#fffaf0",
	"forestgreen": "#228b22",
	"fuchsia": "#ff00ff",
	"gainsboro": "#dcdcdc",
	"ghostwhite": "#f8f8ff",
	"gold": "#ffd700",
	"goldenrod": "#daa520",
	"gray": "#808080",
	"green": "#008000",
	"greenyellow": "#adff2f",
	"grey": "#808080",
	"honeydew": "#f0fff0",
	"hotpink": "#ff69b4",
	"indianred": "#cd5c5c",
	"indigo": "#4b0082",
	"ivory": "#fffff0",
	"khaki": "#f0e68c",
	"lavender": "#e6e6fa",
	"lavenderblush": "#fff0f5",
	"lawngreen": "#7cfc00",
	"lemonchiffon": "#fffacd",
	"lightblue": "#add8e6",
	"lightcoral": "#f08080",
	"lightcyan": "#e0ffff",
	"lightgoldenrodyellow": "#fafad2",
	"lightgray": "#d3d3d3",
	"lightgreen": "#90ee90",
	"lightgrey": "#d3d3d3",
	"lightpink": "#ffb6c1",
	"lightsalmon": "#ffa07a",
	"lightseagreen": "#20b2aa",
	"lightskyblue": "#87cefa",
	"lightslategray": "#778899",
	"lightslategrey": "#778899",
	"lightsteelblue": "#b0c4de",
	"lightyellow": "#ffffe0",
	"lime": "#00ff00",
	"limegreen": "#32cd32",
	"linen": "#faf0e6",
	"magenta": "#ff00ff",
	"maroon": "#800000",
	"mediumaquamarine": "#66cdaa",
	"mediumblue": "#0000cd",
	"mediumorchid": "#ba55d3",
	"mediumpurple": "#9370db",
	"mediumseagreen": "#3cb371",
	"mediumslateblue": "#7b68ee",
	"mediumspringgreen": "#00fa9a",
	"mediumturquoise": "#48d1cc",
	"mediumvioletred": "#c71585",
	"midnightblue": "#191970",
	"mintcream": "#f5fffa",
	"mistyrose": "#ffe4e1",
	"moccasin": "#ffe4b5",
	"navajowhite": "#ffdead",
	"navy": "#000080",
	"oldlace": "#fdf5e6",
	"olive": "#808000",
	"olivedrab": "#6b8e23",
	"orange": "#ffa500",
	"orangered": "#ff4500",
	"orchid": "#da70d6",
	"palegoldenrod": "#eee8aa",
	"palegreen": "#98fb98",
	"paleturquoise": "#afeeee",
	"palevioletred": "#db7093",
	"papayawhip": "#ffefd5",
	"peachpuff": "#ffdab9",
	"peru": "#cd853f",
	"pink": "#ffc0cb",
	"plum": "#dda0dd",
	"powderblue": "#b0e0e6",
	"purple": "#800080",
	"rebeccapurple": "#663399",
	"red": "#ff0000",
	"rosybrown": "#bc8f8f",
	"royalblue": "#4169e1",
	"saddlebrown": "#8b4513",
	"salmon": "#fa8072",
	"sandybrown": "#f4a460",
	"seagreen": "#2e8b57",
	"seashell": "#fff5ee",
	"sienna": "#a0522d",
	"silver": "#c0c0c0",
	"skyblue": "#87ceeb",
	"slateblue": "#6a5acd",
	"slategray": "#708090",
	"slategrey": "#708090",
	"snow": "#fffafa",
	"springgreen": "#00ff7f",
	"steelblue": "#4682b4",
	"tan": "#d2b48c",
	"teal": "#008080",
	"thistle": "#d8bfd8",
	"tomato": "#ff6347",
	"turquoise": "#40e0d0",
	"violet": "#ee82ee",
	"wheat": "#f5deb3",
	"white": "#ffffff",
	"whitesmoke": "#f5f5f5",
	"yellow": "#ffff00",
	"yellowgreen": "#9acd32",
}

RGB_FAMILIES = {
	"srgb": {
		"colourspace": "sRGB",
		"encoded": True,
		"linear_field": "srgb_linear",
		"encoded_field": "srgb",
	},
	"display_p3": {
		"colourspace": "Display P3",
		"encoded": True,
		"linear_field": "display_p3_linear",
		"encoded_field": "display_p3",
	},
	"rec709_oetf": {
		"colourspace": "ITU-R BT.709",
		"encoded": True,
		"linear_field": "rec709_linear",
		"encoded_field": "rec709_oetf",
	},
	"rec2020_oetf": {
		"colourspace": "ITU-R BT.2020",
		"encoded": True,
		"linear_field": "rec2020_linear",
		"encoded_field": "rec2020_oetf",
	},
	"adobe_rgb": {
		"colourspace": "Adobe RGB (1998)",
		"encoded": True,
		"linear_field": "adobe_rgb_linear",
		"encoded_field": "adobe_rgb",
	},
	"prophoto_rgb": {
		"colourspace": "ProPhoto RGB",
		"encoded": True,
		"linear_field": "prophoto_rgb_linear",
		"encoded_field": "prophoto_rgb",
		"chromatic_adaptation_transform": CSS_RGB_CHROMATIC_ADAPTATION_TRANSFORM,
	},
	"aces2065_1": {
		"colourspace": "ACES2065-1",
		"encoded": False,
		"linear_field": "aces2065_1",
	},
	"acescg": {
		"colourspace": "ACEScg",
		"encoded": False,
		"linear_field": "acescg",
	},
}

SRGB_RGB_TO_XYZ = np.array(
	[
		[0.4123907992659593, 0.3575843393838780, 0.1804807884018343],
		[0.2126390058715103, 0.7151686787677559, 0.07219231536073371],
		[0.01933081871559182, 0.1191947797946259, 0.9505321522496606],
	]
)

SRGB_XYZ_TO_RGB = np.array(
	[
		[3.240969941904523, -1.537383177570094, -0.4986107602930034],
		[-0.9692436362808798, 1.875967501507721, 0.04155505740717563],
		[0.05563007969699364, -0.2039769588889765, 1.056971514242879],
	]
)

DISPLAY_P3_RGB_TO_XYZ = np.array(
	[
		[0.4865709486482, 0.2656676931691, 0.1982172852344],
		[0.2289745640697, 0.6917385218365, 0.07928691409374],
		[-0.00000000000000003972075516933, 0.0451133818589, 1.043944368901],
	]
)

DISPLAY_P3_XYZ_TO_RGB = np.array(
	[
		[2.493496911941, -0.931383617919, -0.402710784451],
		[-0.829488969562, 1.762664060318, 0.023624685842],
		[0.035845830244, -0.076172389268, 0.956884524008],
	]
)

PROPHOTO_RGB_TO_XYZ_D50 = np.array(
	[
		[0.79776664490064230, 0.13518129740053308, 0.03134773412839220],
		[0.28807482881940130, 0.71183523424187300, 0.00008993693872564],
		[0.00000000000000000, 0.00000000000000000, 0.82510460251046020],
	]
)

PROPHOTO_XYZ_D50_TO_RGB = np.array(
	[
		[1.3457868816471583, -0.25557208737979464, -0.05110186497554526],
		[-0.5446307051249019, 1.5082477428451468, 0.02052744743642139],
		[0.0, 0.0, 1.2119675456389454],
	]
)

SOURCE_SPACE_TO_RGB_FAMILY = {
	"srgb": ("srgb", True),
	"srgb_linear": ("srgb", False),
	"display_p3": ("display_p3", True),
	"display_p3_linear": ("display_p3", False),
	"rec709_oetf": ("rec709_oetf", True),
	"rec709_linear": ("rec709_oetf", False),
	"rec2020_oetf": ("rec2020_oetf", True),
	"rec2020_linear": ("rec2020_oetf", False),
	"adobe_rgb": ("adobe_rgb", True),
	"adobe_rgb_linear": ("adobe_rgb", False),
	"prophoto_rgb": ("prophoto_rgb", True),
	"prophoto_rgb_linear": ("prophoto_rgb", False),
	"aces2065_1": ("aces2065_1", False),
	"acescg": ("acescg", False),
}


def rgb_chromatic_adaptation_transform(family_name: str) -> str:
	return RGB_FAMILIES[family_name].get("chromatic_adaptation_transform", DEFAULT_RGB_CHROMATIC_ADAPTATION_TRANSFORM)


def rgb_pair_chromatic_adaptation_transform(source_family_name: str, target_family_name: str) -> str:
	source_transform = rgb_chromatic_adaptation_transform(source_family_name)
	target_transform = rgb_chromatic_adaptation_transform(target_family_name)
	if source_transform == CSS_RGB_CHROMATIC_ADAPTATION_TRANSFORM or target_transform == CSS_RGB_CHROMATIC_ADAPTATION_TRANSFORM:
		return CSS_RGB_CHROMATIC_ADAPTATION_TRANSFORM
	return source_transform


def hex_to_rgb(hex_value: str) -> list[float]:
	return [
		int(hex_value[1:3], 16) / 255.0,
		int(hex_value[3:5], 16) / 255.0,
		int(hex_value[5:7], 16) / 255.0,
	]


def finite_number_or_none(value: float) -> float | None:
	value = float(value)
	if not math.isfinite(value):
		return None
	if abs(value) < 5.0e-15:
		value = 0.0
	if value == 0.0:
		return 0.0
	return value


def values_or_none(values: list[float] | np.ndarray, alpha: float | None = None) -> list[float | None]:
	result = [finite_number_or_none(value) for value in values]
	if alpha is not None:
		result.append(finite_number_or_none(alpha))
	return result


def all_finite(values: list[float] | np.ndarray) -> bool:
	return all(math.isfinite(float(value)) for value in values)


def wrap_hue(hue: float) -> float:
	wrapped = hue - math.floor(hue)
	if math.isclose(wrapped, 0.0, abs_tol=1.0e-12) or math.isclose(wrapped, 1.0, abs_tol=1.0e-12):
		return 0.0
	return wrapped


def oklch_to_project_hue_turn(oklch: np.ndarray) -> np.ndarray:
	result = np.array(oklch)
	result[2] = wrap_hue(float(result[2]) / 360.0)
	return result


def cielch_to_project_hue_turn(cielch: np.ndarray) -> np.ndarray:
	result = np.array(cielch)
	result[2] = wrap_hue(float(result[2]) / 360.0)
	return result


def srgb_transfer_to_linear(rgb: list[float] | np.ndarray) -> np.ndarray:
	values = np.array(rgb)
	magnitudes = np.abs(values)
	signs = np.where(values < 0.0, -1.0, 1.0)
	low = values / 12.92
	high = signs * np.power((magnitudes + 0.055) / 1.055, 2.4)
	return np.where(magnitudes <= 0.04045, low, high)


def linear_to_srgb_transfer(rgb: list[float] | np.ndarray) -> np.ndarray:
	values = np.array(rgb)
	magnitudes = np.abs(values)
	signs = np.where(values < 0.0, -1.0, 1.0)
	low = values * 12.92
	high = signs * (1.055 * np.power(magnitudes, 1.0 / 2.4) - 0.055)
	return np.where(magnitudes <= 0.0031308, low, high)


def prophoto_transfer_to_linear(rgb: list[float] | np.ndarray) -> np.ndarray:
	values = np.array(rgb)
	magnitudes = np.abs(values)
	signs = np.where(values < 0.0, -1.0, 1.0)
	low = values / 16.0
	high = signs * np.power(magnitudes, 1.8)
	return np.where(magnitudes <= 16.0 / 512.0, low, high)


def linear_to_prophoto_transfer(rgb: list[float] | np.ndarray) -> np.ndarray:
	values = np.array(rgb)
	magnitudes = np.abs(values)
	signs = np.where(values < 0.0, -1.0, 1.0)
	low = values * 16.0
	high = signs * np.power(magnitudes, 1.0 / 1.8)
	return np.where(magnitudes <= 1.0 / 512.0, low, high)


def rgb_to_hsv(rgb: list[float] | np.ndarray, alpha: float) -> list[float | None]:
	if not all_finite(rgb):
		return [None, None, None, finite_number_or_none(alpha)]

	r, g, b = [float(value) for value in rgb]
	max_rgb = max(r, g, b)
	min_rgb = min(r, g, b)
	delta = max_rgb - min_rgb
	safe_delta = 1.0 if delta == 0.0 else delta

	red_hue = ((g - b) / safe_delta) / 6.0
	green_hue = (((b - r) / safe_delta) + 2.0) / 6.0
	blue_hue = (((r - g) / safe_delta) + 4.0) / 6.0

	if max_rgb == r:
		hue = red_hue
	elif max_rgb == g:
		hue = green_hue
	else:
		hue = blue_hue

	hue = wrap_hue(hue)
	if delta == 0.0:
		hue = 0.0

	saturation = 0.0 if max_rgb == 0.0 else delta / max_rgb
	return values_or_none([hue, saturation, max_rgb], alpha)


def rgb_to_hsl(rgb: list[float] | np.ndarray, alpha: float) -> list[float | None]:
	if not all_finite(rgb):
		return [None, None, None, finite_number_or_none(alpha)]

	r, g, b = [float(value) for value in rgb]
	max_rgb = max(r, g, b)
	min_rgb = min(r, g, b)
	delta = max_rgb - min_rgb
	safe_delta = 1.0 if delta == 0.0 else delta
	lightness = (max_rgb + min_rgb) / 2.0

	red_hue = ((g - b) / safe_delta) / 6.0
	green_hue = (((b - r) / safe_delta) + 2.0) / 6.0
	blue_hue = (((r - g) / safe_delta) + 4.0) / 6.0

	if max_rgb == r:
		hue = red_hue
	elif max_rgb == g:
		hue = green_hue
	else:
		hue = blue_hue

	hue = wrap_hue(hue)
	if delta == 0.0:
		hue = 0.0

	denominator = 1.0 - abs(2.0 * lightness - 1.0)
	safe_denominator = 1.0 if denominator == 0.0 else denominator
	saturation = 0.0 if delta == 0.0 else delta / safe_denominator
	return values_or_none([hue, saturation, lightness], alpha)


def rgb_to_xyz_d50(rgb: list[float], source_space: str) -> np.ndarray:
	family_name, source_is_encoded = SOURCE_SPACE_TO_RGB_FAMILY[source_space]
	if family_name != "prophoto_rgb":
		raise ValueError(f"{source_space} does not have a native D50 RGB matrix")

	linear_rgb = prophoto_transfer_to_linear(rgb) if source_is_encoded else np.array(rgb)
	return PROPHOTO_RGB_TO_XYZ_D50 @ linear_rgb


def rgb_to_xyz_d65(rgb: list[float], source_space: str, chromatic_adaptation_transform: str | None = None) -> np.ndarray:
	family_name, source_is_encoded = SOURCE_SPACE_TO_RGB_FAMILY[source_space]
	family = RGB_FAMILIES[family_name]
	if family_name == "srgb":
		linear_rgb = srgb_transfer_to_linear(rgb) if source_is_encoded else np.array(rgb)
		return SRGB_RGB_TO_XYZ @ linear_rgb
	if family_name == "display_p3":
		linear_rgb = srgb_transfer_to_linear(rgb) if source_is_encoded else np.array(rgb)
		return DISPLAY_P3_RGB_TO_XYZ @ linear_rgb
	if family_name == "prophoto_rgb":
		xyz_d50 = rgb_to_xyz_d50(rgb, source_space)
		return adapt_xyz(
			xyz_d50,
			D50_XYZ,
			D65_XYZ,
			chromatic_adaptation_transform or rgb_chromatic_adaptation_transform(family_name),
		)

	with warnings.catch_warnings():
		warnings.simplefilter("ignore", RuntimeWarning)
		return colour.RGB_to_XYZ(
			np.array(rgb),
			family["colourspace"],
			illuminant=D65,
			chromatic_adaptation_transform=chromatic_adaptation_transform or rgb_chromatic_adaptation_transform(family_name),
			apply_cctf_decoding=source_is_encoded,
		)


def rgb_from_xyz_d65(xyz: np.ndarray, family_name: str, encoded: bool, chromatic_adaptation_transform: str | None = None) -> np.ndarray:
	family = RGB_FAMILIES[family_name]
	if family_name == "srgb":
		linear_rgb = SRGB_XYZ_TO_RGB @ np.array(xyz)
		return linear_to_srgb_transfer(linear_rgb) if encoded else linear_rgb
	if family_name == "display_p3":
		linear_rgb = DISPLAY_P3_XYZ_TO_RGB @ np.array(xyz)
		return linear_to_srgb_transfer(linear_rgb) if encoded else linear_rgb
	if family_name == "prophoto_rgb":
		xyz_d50 = adapt_xyz(
			np.array(xyz),
			D65_XYZ,
			D50_XYZ,
			chromatic_adaptation_transform or rgb_chromatic_adaptation_transform(family_name),
		)
		linear_rgb = PROPHOTO_XYZ_D50_TO_RGB @ xyz_d50
		return linear_to_prophoto_transfer(linear_rgb) if encoded else linear_rgb

	with warnings.catch_warnings():
		warnings.simplefilter("ignore", RuntimeWarning)
		return colour.XYZ_to_RGB(
			xyz,
			family["colourspace"],
			illuminant=D65,
			chromatic_adaptation_transform=chromatic_adaptation_transform or rgb_chromatic_adaptation_transform(family_name),
			apply_cctf_encoding=encoded,
		)


def rgb_to_xyz_aces(rgb: list[float] | np.ndarray, family_name: str) -> np.ndarray:
	colourspace = colour.RGB_COLOURSPACES[RGB_FAMILIES[family_name]["colourspace"]]
	return colourspace.matrix_RGB_to_XYZ @ np.array(rgb)


def direct_rgb_to_rgb(rgb: list[float] | np.ndarray, source_family_name: str, target_family_name: str) -> np.ndarray:
	with warnings.catch_warnings():
		warnings.simplefilter("ignore", RuntimeWarning)
		return colour.RGB_to_RGB(
			np.array(rgb),
			RGB_FAMILIES[source_family_name]["colourspace"],
			RGB_FAMILIES[target_family_name]["colourspace"],
			chromatic_adaptation_transform=rgb_pair_chromatic_adaptation_transform(source_family_name, target_family_name),
			apply_cctf_decoding=False,
			apply_cctf_encoding=False,
		)


def transfer_decode_rgb(rgb: list[float] | np.ndarray, family_name: str) -> np.ndarray:
	if family_name in ("srgb", "display_p3"):
		return srgb_transfer_to_linear(rgb)
	if family_name == "prophoto_rgb":
		return prophoto_transfer_to_linear(rgb)

	colourspace = colour.RGB_COLOURSPACES[RGB_FAMILIES[family_name]["colourspace"]]
	if colourspace.cctf_decoding is None:
		return np.array(rgb)
	with warnings.catch_warnings():
		warnings.simplefilter("ignore", RuntimeWarning)
		return colourspace.cctf_decoding(np.array(rgb))


def transfer_encode_rgb(rgb: list[float] | np.ndarray, family_name: str) -> np.ndarray:
	if family_name in ("srgb", "display_p3"):
		return linear_to_srgb_transfer(rgb)
	if family_name == "prophoto_rgb":
		return linear_to_prophoto_transfer(rgb)

	colourspace = colour.RGB_COLOURSPACES[RGB_FAMILIES[family_name]["colourspace"]]
	if colourspace.cctf_encoding is None:
		return np.array(rgb)
	with warnings.catch_warnings():
		warnings.simplefilter("ignore", RuntimeWarning)
		return colourspace.cctf_encoding(np.array(rgb))


def adapt_xyz(xyz: np.ndarray, source_white: np.ndarray, target_white: np.ndarray, chromatic_adaptation_transform: str) -> np.ndarray:
	return colour.chromatic_adaptation(
		xyz,
		source_white,
		target_white,
		method="Von Kries",
		transform=chromatic_adaptation_transform,
	)


def adapt_xyz_d65_to_d50(xyz: np.ndarray, chromatic_adaptation_transform: str = LAB_CHROMATIC_ADAPTATION_TRANSFORM) -> np.ndarray:
	return adapt_xyz(xyz, D65_XYZ, D50_XYZ, chromatic_adaptation_transform)


def adapt_xyz_d65_to_aces(xyz: np.ndarray, chromatic_adaptation_transform: str = DEFAULT_RGB_CHROMATIC_ADAPTATION_TRANSFORM) -> np.ndarray:
	return adapt_xyz(xyz, D65_XYZ, ACES_WHITE_XYZ, chromatic_adaptation_transform)


def add_rgb_family(record: dict, family_name: str, xyz: np.ndarray, alpha: float) -> None:
	family = RGB_FAMILIES[family_name]
	linear_rgb = rgb_from_xyz_d65(xyz, family_name, encoded=False)
	linear_field = family["linear_field"]
	record[linear_field] = values_or_none(linear_rgb, alpha)
	record[f"{linear_field}_hsv"] = rgb_to_hsv(linear_rgb, alpha)
	record[f"{linear_field}_hsl"] = rgb_to_hsl(linear_rgb, alpha)
	record[f"{linear_field}_to_xyz_d65"] = values_or_none(rgb_to_xyz_d65(linear_rgb, linear_field), alpha)
	if family_name == "prophoto_rgb":
		record[f"{linear_field}_to_xyz_d50"] = values_or_none(rgb_to_xyz_d50(linear_rgb, linear_field), alpha)
		record[f"{linear_field}_to_xyz_d65_cat02"] = values_or_none(rgb_to_xyz_d65(linear_rgb, linear_field, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)
	if family_name in ("aces2065_1", "acescg"):
		record[f"{linear_field}_to_xyz_aces"] = values_or_none(rgb_to_xyz_aces(linear_rgb, family_name), alpha)
		record[f"{linear_field}_to_xyz_d65_cat02"] = values_or_none(rgb_to_xyz_d65(linear_rgb, linear_field, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)

	if family["encoded"]:
		encoded_rgb = rgb_from_xyz_d65(xyz, family_name, encoded=True)
		encoded_field = family["encoded_field"]
		record[encoded_field] = values_or_none(encoded_rgb, alpha)
		record[f"{encoded_field}_hsv"] = rgb_to_hsv(encoded_rgb, alpha)
		record[f"{encoded_field}_hsl"] = rgb_to_hsl(encoded_rgb, alpha)
		record[f"{encoded_field}_to_xyz_d65"] = values_or_none(rgb_to_xyz_d65(encoded_rgb, encoded_field), alpha)
		if family_name == "prophoto_rgb":
			record[f"{encoded_field}_to_xyz_d50"] = values_or_none(rgb_to_xyz_d50(encoded_rgb, encoded_field), alpha)
			record[f"{encoded_field}_to_xyz_d65_cat02"] = values_or_none(rgb_to_xyz_d65(encoded_rgb, encoded_field, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)


def update_rgb_to_xyz_fields(record: dict, family_name: str, alpha: float) -> None:
	family = RGB_FAMILIES[family_name]
	linear_field = family["linear_field"]
	linear_rgb = record.get(linear_field)
	if is_finite_values(linear_rgb):
		record[f"{linear_field}_to_xyz_d65"] = values_or_none(rgb_to_xyz_d65(linear_rgb[:3], linear_field), alpha)
		if family_name == "prophoto_rgb":
			record[f"{linear_field}_to_xyz_d50"] = values_or_none(rgb_to_xyz_d50(linear_rgb[:3], linear_field), alpha)
			record[f"{linear_field}_to_xyz_d65_cat02"] = values_or_none(rgb_to_xyz_d65(linear_rgb[:3], linear_field, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)
		if family_name in ("aces2065_1", "acescg"):
			record[f"{linear_field}_to_xyz_aces"] = values_or_none(rgb_to_xyz_aces(linear_rgb[:3], family_name), alpha)
			record[f"{linear_field}_to_xyz_d65_cat02"] = values_or_none(rgb_to_xyz_d65(linear_rgb[:3], linear_field, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)

	if family["encoded"]:
		encoded_field = family["encoded_field"]
		encoded_rgb = record.get(encoded_field)
		if is_finite_values(encoded_rgb):
			record[f"{encoded_field}_to_xyz_d65"] = values_or_none(rgb_to_xyz_d65(encoded_rgb[:3], encoded_field), alpha)
			if family_name == "prophoto_rgb":
				record[f"{encoded_field}_to_xyz_d50"] = values_or_none(rgb_to_xyz_d50(encoded_rgb[:3], encoded_field), alpha)
				record[f"{encoded_field}_to_xyz_d65_cat02"] = values_or_none(rgb_to_xyz_d65(encoded_rgb[:3], encoded_field, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)


def update_aces_bridge_fields(record: dict, alpha: float) -> None:
	aces2065_1 = record.get("aces2065_1")
	if is_finite_values(aces2065_1):
		record["aces2065_1_to_acescg"] = values_or_none(direct_rgb_to_rgb(aces2065_1[:3], "aces2065_1", "acescg"), alpha)

	acescg = record.get("acescg")
	if is_finite_values(acescg):
		record["acescg_to_aces2065_1"] = values_or_none(direct_rgb_to_rgb(acescg[:3], "acescg", "aces2065_1"), alpha)


def add_xyz_derived_spaces(record: dict, xyz: np.ndarray, alpha: float) -> None:
	record["xyy"] = values_or_none(colour.XYZ_to_xyY(xyz), alpha)

	xyz_d50 = adapt_xyz_d65_to_d50(xyz)
	record["xyz_d50"] = values_or_none(xyz_d50, alpha)
	record["xyz_d50_cat02"] = values_or_none(adapt_xyz_d65_to_d50(xyz, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)
	record["xyz_aces"] = values_or_none(adapt_xyz_d65_to_aces(xyz), alpha)
	record["xyz_aces_cat02"] = values_or_none(adapt_xyz_d65_to_aces(xyz, CAT02_CHROMATIC_ADAPTATION_TRANSFORM), alpha)
	lab_d50 = colour.XYZ_to_Lab(xyz_d50, illuminant=D50)
	record["cielab_d50"] = values_or_none(lab_d50, alpha)
	record["cielch_d50"] = values_or_none(cielch_to_project_hue_turn(colour.Lab_to_LCHab(lab_d50)), alpha)

	oklab = colour.XYZ_to_Oklab(xyz)
	record["oklab"] = values_or_none(oklab, alpha)
	record["oklch"] = values_or_none(oklch_to_project_hue_turn(colour.Oklab_to_Oklch(oklab)), alpha)


def make_record_from_xyz(name: str, group: str, source_space: str, xyz: list[float], alpha: float) -> dict:
	xyz_array = np.array(xyz)
	record = {
		"name": name,
		"group": group,
		"source_space": source_space,
		"source_value": values_or_none(xyz, alpha),
		"xyz_d65": values_or_none(xyz_array, alpha),
	}

	add_xyz_derived_spaces(record, xyz_array, alpha)
	for family_name in RGB_FAMILIES:
		add_rgb_family(record, family_name, xyz_array, alpha)
	update_aces_bridge_fields(record, alpha)
	return record


def make_record_from_rgb(name: str, group: str, source_space: str, rgb: list[float], alpha: float) -> dict:
	xyz = rgb_to_xyz_d65(rgb, source_space)
	record = make_record_from_xyz(name, group, source_space, list(xyz), alpha)
	family_name, source_is_encoded = SOURCE_SPACE_TO_RGB_FAMILY[source_space]
	family = RGB_FAMILIES[family_name]

	record["source_value"] = values_or_none(rgb, alpha)
	if family["encoded"]:
		encoded_field = family["encoded_field"]
		linear_field = family["linear_field"]
		if source_is_encoded:
			encoded_rgb = np.array(rgb)
			linear_rgb = transfer_decode_rgb(rgb, family_name)
		else:
			linear_rgb = np.array(rgb)
			encoded_rgb = transfer_encode_rgb(rgb, family_name)

		record[encoded_field] = values_or_none(encoded_rgb, alpha)
		record[f"{encoded_field}_hsv"] = rgb_to_hsv(encoded_rgb, alpha)
		record[f"{encoded_field}_hsl"] = rgb_to_hsl(encoded_rgb, alpha)
		record[linear_field] = values_or_none(linear_rgb, alpha)
		record[f"{linear_field}_hsv"] = rgb_to_hsv(linear_rgb, alpha)
		record[f"{linear_field}_hsl"] = rgb_to_hsl(linear_rgb, alpha)
	else:
		linear_field = family["linear_field"]
		record[linear_field] = values_or_none(rgb, alpha)
		record[f"{linear_field}_hsv"] = rgb_to_hsv(rgb, alpha)
		record[f"{linear_field}_hsl"] = rgb_to_hsl(rgb, alpha)
	update_rgb_to_xyz_fields(record, family_name, alpha)
	update_aces_bridge_fields(record, alpha)
	return record


def make_css_cases() -> list[dict]:
	return [
		make_record_from_rgb(name, "css_named", "srgb", hex_to_rgb(hex_value), 1.0)
		for name, hex_value in CSS_NAMED_COLOURS.items()
	]


def make_alpha_cases() -> list[dict]:
	return [
		make_record_from_rgb("transparent_black", "alpha", "srgb", [0.0, 0.0, 0.0], 0.0),
		make_record_from_rgb("transparent_white", "alpha", "srgb", [1.0, 1.0, 1.0], 0.0),
		make_record_from_rgb("half_alpha_red", "alpha", "srgb", [1.0, 0.0, 0.0], 0.5),
		make_record_from_rgb("quarter_alpha_rebeccapurple", "alpha", "srgb", hex_to_rgb("#663399"), 0.25),
		make_record_from_rgb("three_quarter_alpha_sky", "alpha", "srgb", [0.2, 0.55, 1.0], 0.75),
	]


def make_wide_gamut_cases() -> list[dict]:
	cases = []
	source_spaces = [
		"display_p3",
		"display_p3_linear",
		"rec2020_oetf",
		"rec2020_linear",
		"adobe_rgb",
		"adobe_rgb_linear",
		"prophoto_rgb",
		"prophoto_rgb_linear",
		"aces2065_1",
		"acescg",
	]
	rgb_values = {
		"red": [1.0, 0.0, 0.0],
		"green": [0.0, 1.0, 0.0],
		"blue": [0.0, 0.0, 1.0],
		"cyan": [0.0, 1.0, 1.0],
		"magenta": [1.0, 0.0, 1.0],
		"yellow": [1.0, 1.0, 0.0],
		"warm_over_white": [1.4, 1.15, 0.8],
		"cool_over_white": [0.75, 1.1, 1.45],
	}
	for source_space in source_spaces:
		for colour_name, rgb in rgb_values.items():
			cases.append(make_record_from_rgb(f"{source_space}_{colour_name}", "wide_gamut", source_space, rgb, 1.0))
	return cases


def make_numeric_stress_cases() -> list[dict]:
	return [
		make_record_from_xyz("xyz_d65_black", "numeric_stress", "xyz_d65", [0.0, 0.0, 0.0], 1.0),
		make_record_from_xyz("xyz_d65_white", "numeric_stress", "xyz_d65", [0.95047, 1.0, 1.08883], 1.0),
		make_record_from_xyz("xyz_d65_over_white", "numeric_stress", "xyz_d65", [1.2, 1.5, 1.0], 1.0),
		make_record_from_xyz("xyz_d65_wide_cyan", "numeric_stress", "xyz_d65", [0.1, 0.2, 0.3], 1.0),
		make_record_from_xyz("xyz_d65_wide_blue", "numeric_stress", "xyz_d65", [0.2, 0.1, 1.2], 1.0),
		make_record_from_rgb("srgb_linear_over_white", "numeric_stress", "srgb_linear", [1.5, 1.2, 0.7], 1.0),
		make_record_from_rgb("srgb_linear_small_negative", "numeric_stress", "srgb_linear", [-0.02, 0.2, 0.4], 1.0),
		make_record_from_rgb("acescg_negative_red", "numeric_stress", "acescg", [-0.05, 0.35, 0.6], 1.0),
	]


def make_metadata(case_count: int) -> dict:
	return {
		"schema_version": 1,
		"generator": "tests/colour/reference/generate_colour_reference.py",
		"reference_library": f"colour-science {colour.__version__}",
		"canonical_xyz": "CIE XYZ D65, Y=1",
		"observer": OBSERVER,
		"rgb_chromatic_adaptation_transform": "Bradford by default; CAT02 arrays are generated separately where the C++ API exposes explicit CAT02 methods",
		"lab_chromatic_adaptation_transform": LAB_CHROMATIC_ADAPTATION_TRANSFORM,
		"alpha_policy": "Alpha is copied unchanged into every representation.",
		"out_of_gamut_policy": "Values are not clipped. Generated arrays contain finite rows only.",
		"hue_policy": "Hue is normalised with h - floor(h), matching the C++ colour conversion helpers.",
		"case_count": case_count,
		"rgb_families": {
			field_name: {
				"colourspace": family["colourspace"],
				"linear_field": family["linear_field"],
				"encoded_field": family.get("encoded_field"),
				"chromatic_adaptation_transform": rgb_chromatic_adaptation_transform(field_name),
			}
			for field_name, family in RGB_FAMILIES.items()
		},
	}


def cpp_number(value: float) -> str:
	text = f"{float(value):.17g}"
	if "e" not in text and "." not in text:
		text += ".0"
	return text


def cpp_values(values: list[float]) -> str:
	return "{ " + ", ".join(cpp_number(value) for value in values) + " }"


def is_finite_values(values: list[float | None] | None) -> bool:
	return values is not None and all(value is not None for value in values)


def is_hsv_or_hsl_field(field: str) -> bool:
	return field.endswith("_hsv") or field.endswith("_hsl")


def is_stable_hsv_hsl_case(case: dict, source_field: str, expected_field: str) -> bool:
	if is_hsv_or_hsl_field(expected_field):
		rgb_field = source_field
		hsv_hsl_field = expected_field
	elif is_hsv_or_hsl_field(source_field):
		rgb_field = expected_field
		hsv_hsl_field = source_field
	else:
		return True

	rgb = case.get(rgb_field)
	if not is_finite_values(rgb):
		return False

	r, g, b = [float(value) for value in rgb[:3]]
	max_rgb = max(r, g, b)
	min_rgb = min(r, g, b)
	delta = max_rgb - min_rgb
	scale = max(1.0, abs(r), abs(g), abs(b))
	if delta <= HSV_HSL_MIN_CHROMA * scale:
		return False

	if hsv_hsl_field.endswith("_hsl"):
		denominator = 1.0 - abs(max_rgb + min_rgb - 1.0)
		if abs(denominator) <= HSL_MIN_DENOMINATOR:
			return False

	return True


def cases_for_fields(cases: list[dict], source_field: str, expected_field: str) -> list[dict]:
	skip_numeric_stress = is_hsv_or_hsl_field(source_field) or is_hsv_or_hsl_field(expected_field)
	return [
		case
		for case in cases
		if is_finite_values(case.get(source_field)) and is_finite_values(case.get(expected_field))
		if not (skip_numeric_stress and case["group"] == "numeric_stress")
		if not skip_numeric_stress or is_stable_hsv_hsl_case(case, source_field, expected_field)
	]


def header_banner(title: str, details: list[str], metadata: dict) -> str:
	detail_text = "\n".join(f" * {line}" for line in details)
	return f"""/********************************************************************************************************

Generated colour reference data.
{title}

Source:      {metadata["reference_library"]}
Generator:   {metadata["generator"]}
Canonical:   {metadata["canonical_xyz"]}
Observer:    {metadata["observer"]}
RGB CAT:     {metadata["rgb_chromatic_adaptation_transform"]}
Lab CAT:     {metadata["lab_chromatic_adaptation_transform"]}

{detail_text}
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
"""


def write_common_header(metadata: dict) -> None:
	path = REFERENCE_DIR / "colour_reference_common.h"
	text = header_banner(
		"Common colour reference structs.",
		[
			"Common structs and metadata shared by the generated colour reference headers.",
			"Each value array contains four doubles.",
			"For RGB-like data the order is r, g, b, a.",
			"For CIE XYZ data the order is X, Y, Z, alpha.",
			"For HSV/HSL data the order is h, s, v/l, a.",
			"For CIELAB data the order is L*, a*, b*, alpha.",
			"For CIELCH data the order is L*, C*, h, alpha; h is a 0..1 turn.",
			"For Oklab data the order is L, a, b, alpha.",
			"For Oklch data the order is L, C, h, alpha; h is a 0..1 turn.",
			"Case groups include CSS named sRGB colours, alpha cases, wide-gamut RGB samples, and numeric stress samples.",
		],
		metadata,
	)
	text += """#pragma once

namespace colour_reference {

inline constexpr const char* reference_library = \"""" + metadata["reference_library"] + """\";
inline constexpr const char* canonical_xyz = \"CIE XYZ D65, Y=1\";
inline constexpr const char* rgb_chromatic_adaptation_transform = \"""" + metadata["rgb_chromatic_adaptation_transform"] + """\";
inline constexpr const char* lab_chromatic_adaptation_transform = \"""" + metadata["lab_chromatic_adaptation_transform"] + """\";
inline constexpr int source_case_count = """ + str(metadata["case_count"]) + """;

struct ColourReferenceCase {
	const char* name;
	const char* group;
	double source[4];
	double expected[4];
};

} // namespace colour_reference
"""
	path.write_text(text, encoding="utf-8")


def write_reference_header(filename: str, title: str, details: list[str], arrays: list[dict], cases: list[dict], metadata: dict) -> int:
	path = REFERENCE_DIR / filename
	text = header_banner(title, details, metadata)
	text += '#pragma once\n\n#include "colour_reference_common.h"\n\nnamespace colour_reference {\n\n'
	total_rows = 0

	for array in arrays:
		rows = cases_for_fields(cases, array["source"], array["expected"])
		total_rows += len(rows)
		text += "/**************************************************************************************************\n"
		text += f" * {array['description']}\n"
		text += f" * Source field   : {array['source']}\n"
		text += f" * Expected field : {array['expected']}\n"
		text += f" * Direction      : {array.get('direction', 'Tests may use source -> expected or expected -> source from the same row.')}\n"
		text += f" * Cases          : {len(rows)}\n"
		text += " * ************************************************************************************************/\n"
		text += f"inline constexpr ColourReferenceCase {array['name']}[] = {{\n"
		for case in rows:
			source = cpp_values(case[array["source"]])
			expected = cpp_values(case[array["expected"]])
			text += f'\t{{ "{case["name"]}", "{case["group"]}", {source}, {expected} }},\n'
		text += "};\n\n"

	text += "} // namespace colour_reference\n"
	path.write_text(text, encoding="utf-8")
	return total_rows


def rgb_reference_arrays(prefix: str, encoded_field: str | None, linear_field: str, include_linear_hsv_hsl: bool) -> list[dict]:
	arrays = []
	if encoded_field is not None:
		arrays.extend(
			[
				{
					"name": f"{encoded_field}_{linear_field}_cases",
					"source": encoded_field,
					"expected": linear_field,
					"description": f"Relationship between transfer-encoded {prefix} RGBA and linear-light {prefix} RGBA.",
				},
				{
					"name": f"{encoded_field}_xyz_d65_cases",
					"source": encoded_field,
					"expected": f"{encoded_field}_to_xyz_d65",
					"description": f"Relationship between transfer-encoded {prefix} RGBA and CIE XYZ D65.",
				},
				{
					"name": f"{encoded_field}_hsv_cases",
					"source": encoded_field,
					"expected": f"{encoded_field}_hsv",
					"description": f"Relationship between transfer-encoded {prefix} RGBA and HSV coordinates.",
				},
				{
					"name": f"{encoded_field}_hsl_cases",
					"source": encoded_field,
					"expected": f"{encoded_field}_hsl",
					"description": f"Relationship between transfer-encoded {prefix} RGBA and HSL coordinates.",
				},
			]
		)

	arrays.extend(
		[
			{
				"name": f"{linear_field}_xyz_d65_cases",
				"source": linear_field,
				"expected": f"{linear_field}_to_xyz_d65",
				"description": f"Relationship between linear-light {prefix} RGBA and CIE XYZ D65.",
			},
		]
	)

	if include_linear_hsv_hsl:
		arrays.extend(
			[
				{
					"name": f"{linear_field}_hsv_cases",
					"source": linear_field,
					"expected": f"{linear_field}_hsv",
					"description": f"Relationship between linear-light {prefix} RGBA and HSV coordinates.",
				},
				{
					"name": f"{linear_field}_hsl_cases",
					"source": linear_field,
					"expected": f"{linear_field}_hsl",
					"description": f"Relationship between linear-light {prefix} RGBA and HSL coordinates.",
				},
			]
		)

	return arrays


def encoded_rgb_header_details(
	header_name: str,
	colourspace_key: str,
	encoded_field: str,
	linear_field: str,
	whitepoint: str,
	transfer_function: str,
	include_linear_hsv_hsl: bool,
	type_status: str,
) -> list[str]:
	details = [
		f"Header scope: {header_name}, plus CIE XYZ D65 conversion endpoints.",
		f"colour-science RGB colourspace key: {colourspace_key}.",
		f"Whitepoint: {whitepoint}.",
		f"Coordinate field `{encoded_field}`: transfer-encoded {header_name} RGBA, ordered r, g, b, a.",
		f"Coordinate field `{linear_field}`: linear-light {header_name} RGBA, ordered r, g, b, a.",
		f"Transfer function for `{encoded_field}` <-> `{linear_field}`: {transfer_function}.",
		f"Coordinate field `xyz_d65`: CIE XYZ with D65 reference white and Y = 1, ordered X, Y, Z, alpha.",
		f"Coordinate field `{encoded_field}_hsv`: HSV derived from `{encoded_field}`, ordered h, s, v, a; h is a 0..1 turn.",
		f"Coordinate field `{encoded_field}_hsl`: HSL derived from `{encoded_field}`, ordered h, s, l, a; h is a 0..1 turn.",
	]
	if include_linear_hsv_hsl:
		details.extend(
			[
				f"Coordinate field `{linear_field}_hsv`: HSV derived from `{linear_field}`, ordered h, s, v, a; h is a 0..1 turn.",
				f"Coordinate field `{linear_field}_hsl`: HSL derived from `{linear_field}`, ordered h, s, l, a; h is a 0..1 turn.",
			]
		)
	else:
		details.append(f"No `{linear_field}_hsv` or `{linear_field}_hsl` arrays are exported for this header.")
	details.append(type_status)
	return details


def linear_rgb_header_details(header_name: str, colourspace_key: str, linear_fields: list[str], whitepoint: str, type_status: str) -> list[str]:
	details = [
		f"Header scope: {header_name}, plus CIE XYZ D65 and ACES-white XYZ conversion endpoints.",
		f"colour-science RGB colourspace key(s): {colourspace_key}.",
		f"Native whitepoint: {whitepoint}.",
		"All RGB fields in this header are linear-light RGBA, ordered r, g, b, a.",
		"Coordinate field `xyz_d65`: CIE XYZ with D65 reference white and Y = 1, ordered X, Y, Z, alpha.",
		"Coordinate field `xyz_aces`: CIE XYZ with the ACES reference white and Y = 1, ordered X, Y, Z, alpha.",
		"ACES RGB values are converted exactly to `xyz_aces`; default `xyz_d65` rows use Bradford adaptation and `_cat02` rows preserve the explicit CAT02 path.",
	]
	for field in linear_fields:
		field_label = {
			"aces2065_1": "ACES2065-1",
			"acescg": "ACEScg",
		}.get(field, field)
		details.extend(
			[
				f"Coordinate field `{field}`: linear-light {field_label} RGBA, ordered r, g, b, a.",
				f"Coordinate field `{field}_hsv`: HSV derived from `{field}`, ordered h, s, v, a; h is a 0..1 turn.",
				f"Coordinate field `{field}_hsl`: HSL derived from `{field}`, ordered h, s, l, a; h is a 0..1 turn.",
			]
		)
	details.append(type_status)
	return details


def export_headers(cases: list[dict], metadata: dict) -> None:
	for obsolete_header in OBSOLETE_REFERENCE_HEADERS:
		(REFERENCE_DIR / obsolete_header).unlink(missing_ok=True)

	write_common_header(metadata)

	header_specs = [
		{
			"filename": "srgb_reference.h",
			"title": "sRGB reference data.",
			"details": encoded_rgb_header_details(
				"sRGB",
				"sRGB",
				"srgb",
				"srgb_linear",
				"D65",
				"standard sRGB transfer function",
				include_linear_hsv_hsl=True,
				type_status="The C++ library currently implements all coordinate fields named in this header for sRGB. sRGB XYZ rows use the CSS Color 4 matrix derived from the official sRGB/BT.709 primaries and D65 white point.",
			),
			"arrays": rgb_reference_arrays("sRGB", "srgb", "srgb_linear", include_linear_hsv_hsl=True),
		},
		{
			"filename": "display_p3_reference.h",
			"title": "Display P3 reference data.",
			"details": encoded_rgb_header_details(
				"Display P3",
				"Display P3",
				"display_p3",
				"display_p3_linear",
				"D65",
				"sRGB transfer function",
				include_linear_hsv_hsl=False,
				type_status="The C++ library currently implements `display_p3`, `display_p3_linear`, `display_p3_hsv`, and `display_p3_hsl`.",
			),
			"arrays": rgb_reference_arrays("Display P3", "display_p3", "display_p3_linear", include_linear_hsv_hsl=False),
		},
		{
			"filename": "rec709_reference.h",
			"title": "Rec.709 reference data.",
			"details": encoded_rgb_header_details(
				"Rec.709",
				"ITU-R BT.709",
				"rec709_oetf",
				"rec709_linear",
				"D65",
				"Rec.709 transfer function from colour-science",
				include_linear_hsv_hsl=True,
				type_status="The C++ library currently implements all coordinate fields named in this header for Rec.709.",
			),
			"arrays": rgb_reference_arrays("Rec.709", "rec709_oetf", "rec709_linear", include_linear_hsv_hsl=True),
		},
		{
			"filename": "rec2020_reference.h",
			"title": "Rec.2020 reference data.",
			"details": encoded_rgb_header_details(
				"Rec.2020",
				"ITU-R BT.2020",
				"rec2020_oetf",
				"rec2020_linear",
				"D65",
				"Rec.2020 transfer function from colour-science",
				include_linear_hsv_hsl=True,
				type_status="The C++ library currently implements all coordinate fields named in this header for Rec.2020.",
			),
			"arrays": rgb_reference_arrays("Rec.2020", "rec2020_oetf", "rec2020_linear", include_linear_hsv_hsl=True),
		},
		{
			"filename": "adobe_rgb_reference.h",
			"title": "Adobe RGB reference data.",
			"details": encoded_rgb_header_details(
				"Adobe RGB",
				"Adobe RGB (1998)",
				"adobe_rgb",
				"adobe_rgb_linear",
				"D65",
				"Adobe RGB transfer function from colour-science",
				include_linear_hsv_hsl=True,
				type_status="The C++ library currently implements all coordinate fields named in this header for Adobe RGB. Some encoded rows are omitted when the transfer function produces non-finite values for out-of-gamut inputs.",
			),
			"arrays": rgb_reference_arrays("Adobe RGB", "adobe_rgb", "adobe_rgb_linear", include_linear_hsv_hsl=True),
		},
		{
			"filename": "prophoto_rgb_reference.h",
			"title": "ProPhoto RGB reference data.",
			"details": encoded_rgb_header_details(
				"ProPhoto RGB",
				"ProPhoto RGB",
				"prophoto_rgb",
				"prophoto_rgb_linear",
				"D50",
				"ROMM RGB transfer function from CSS Color 4",
				include_linear_hsv_hsl=True,
				type_status="The C++ library implements ProPhoto RGB with native D50 conversion, Bradford-adapted D65 conversion by default, and explicit CAT02 D65 conversion arrays.",
			),
			"arrays": [
				*rgb_reference_arrays("ProPhoto RGB", "prophoto_rgb", "prophoto_rgb_linear", include_linear_hsv_hsl=True),
				{
					"name": "prophoto_rgb_xyz_d50_cases",
					"source": "prophoto_rgb",
					"expected": "prophoto_rgb_to_xyz_d50",
					"description": "Relationship between transfer-encoded ProPhoto RGB RGBA and CIE XYZ D50.",
				},
				{
					"name": "prophoto_rgb_xyz_d65_cat02_cases",
					"source": "prophoto_rgb",
					"expected": "prophoto_rgb_to_xyz_d65_cat02",
					"description": "Relationship between transfer-encoded ProPhoto RGB RGBA and CIE XYZ D65 using CAT02 chromatic adaptation.",
				},
				{
					"name": "prophoto_rgb_linear_xyz_d50_cases",
					"source": "prophoto_rgb_linear",
					"expected": "prophoto_rgb_linear_to_xyz_d50",
					"description": "Relationship between linear-light ProPhoto RGB RGBA and CIE XYZ D50.",
				},
				{
					"name": "prophoto_rgb_linear_xyz_d65_cat02_cases",
					"source": "prophoto_rgb_linear",
					"expected": "prophoto_rgb_linear_to_xyz_d65_cat02",
					"description": "Relationship between linear-light ProPhoto RGB RGBA and CIE XYZ D65 using CAT02 chromatic adaptation.",
				},
			],
		},
		{
			"filename": "aces_reference.h",
			"title": "ACES reference data.",
			"details": linear_rgb_header_details(
				"ACES2065-1 and ACEScg",
				"ACES2065-1, ACEScg",
				["aces2065_1", "acescg"],
				"ACES whitepoint, approximately D60",
				"The C++ library implements exact ACES-white XYZ conversions, Bradford-adapted D65 conversions, and explicit CAT02 D65 conversions. The ACES2065-1 <-> ACEScg bridge arrays are direct colour.RGB_to_RGB reference rows, not XYZ-hub rows.",
			),
			"arrays": [
				*rgb_reference_arrays("ACES2065-1", None, "aces2065_1", include_linear_hsv_hsl=True),
				{
					"name": "aces2065_1_xyz_aces_cases",
					"source": "aces2065_1",
					"expected": "aces2065_1_to_xyz_aces",
					"description": "Exact relationship between ACES2065-1 RGBA and CIE XYZ with ACES white.",
				},
				{
					"name": "aces2065_1_xyz_d65_cat02_cases",
					"source": "aces2065_1",
					"expected": "aces2065_1_to_xyz_d65_cat02",
					"description": "Relationship between ACES2065-1 RGBA and CIE XYZ D65 using CAT02 chromatic adaptation.",
				},
				*rgb_reference_arrays("ACEScg", None, "acescg", include_linear_hsv_hsl=True),
				{
					"name": "acescg_xyz_aces_cases",
					"source": "acescg",
					"expected": "acescg_to_xyz_aces",
					"description": "Exact relationship between ACEScg RGBA and CIE XYZ with ACES white.",
				},
				{
					"name": "acescg_xyz_d65_cat02_cases",
					"source": "acescg",
					"expected": "acescg_to_xyz_d65_cat02",
					"description": "Relationship between ACEScg RGBA and CIE XYZ D65 using CAT02 chromatic adaptation.",
				},
				{
					"name": "aces2065_1_acescg_cases",
					"source": "aces2065_1",
					"expected": "aces2065_1_to_acescg",
					"description": "Direct relationship between ACES2065-1 RGBA and ACEScg RGBA.",
					"direction": "Tests use source -> expected. The reverse direct bridge has its own array.",
				},
				{
					"name": "acescg_aces2065_1_cases",
					"source": "acescg",
					"expected": "acescg_to_aces2065_1",
					"description": "Direct relationship between ACEScg RGBA and ACES2065-1 RGBA.",
					"direction": "Tests use source -> expected. The reverse direct bridge has its own array.",
				},
			],
		},
		{
			"filename": "xyy_reference.h",
			"title": "xyY reference data.",
			"details": [
				"Header scope: CIE xyY coordinates generated from the canonical CIE XYZ D65 hub.",
				"Coordinate field `xyz_d65`: CIE XYZ with D65 reference white and Y = 1, ordered X, Y, Z, alpha.",
				"Coordinate field `xyy`: CIE xyY coordinates, ordered x chromaticity, y chromaticity, luminance Y, alpha.",
			],
			"arrays": [
				{
					"name": "xyz_d65_xyy_cases",
					"source": "xyz_d65",
					"expected": "xyy",
					"description": "Relationship between CIE XYZ D65 and CIE xyY coordinates.",
				},
			],
		},
		{
			"filename": "xyz_d50_reference.h",
			"title": "CIE XYZ D50 reference data.",
			"details": [
				"Header scope: CIE XYZ D65 and CIE XYZ D50 chromatic adaptation reference data.",
				"Coordinate field `xyz_d65`: canonical CIE XYZ with D65 reference white and Y = 1, ordered X, Y, Z, alpha.",
				"Coordinate field `xyz_d50`: CIE XYZ adapted from `xyz_d65` to D50 with Bradford chromatic adaptation, ordered X, Y, Z, alpha.",
				"Coordinate field `xyz_d50_cat02`: CIE XYZ adapted from `xyz_d65` to D50 with CAT02 chromatic adaptation, ordered X, Y, Z, alpha.",
				"White points are the CIE 1931 2 Degree Standard Observer D65 and D50 illuminants from colour-science.",
				"These arrays are reference data for the C++ Xyz <-> XyzD50 conversion members, including explicit CAT02 variants.",
			],
			"arrays": [
				{
					"name": "xyz_d65_xyz_d50_cases",
					"source": "xyz_d65",
					"expected": "xyz_d50",
					"description": "Relationship between CIE XYZ D65 and Bradford-adapted CIE XYZ D50.",
				},
				{
					"name": "xyz_d65_xyz_d50_cat02_cases",
					"source": "xyz_d65",
					"expected": "xyz_d50_cat02",
					"description": "Relationship between CIE XYZ D65 and CAT02-adapted CIE XYZ D50.",
				},
			],
		},
		{
			"filename": "xyz_aces_reference.h",
			"title": "CIE XYZ ACES-white reference data.",
			"details": [
				"Header scope: CIE XYZ D65 and CIE XYZ with the ACES reference white.",
				"Coordinate field `xyz_d65`: canonical CIE XYZ with D65 reference white and Y = 1, ordered X, Y, Z, alpha.",
				"Coordinate field `xyz_aces`: CIE XYZ adapted from `xyz_d65` to ACES white with Bradford chromatic adaptation, ordered X, Y, Z, alpha.",
				"Coordinate field `xyz_aces_cat02`: CIE XYZ adapted from `xyz_d65` to ACES white with CAT02 chromatic adaptation, ordered X, Y, Z, alpha.",
				"The ACES white point is x = 0.32168, y = 0.33767.",
				"These arrays are reference data for the C++ Xyz <-> XyzAces conversion members.",
			],
			"arrays": [
				{
					"name": "xyz_d65_xyz_aces_cases",
					"source": "xyz_d65",
					"expected": "xyz_aces",
					"description": "Relationship between CIE XYZ D65 and Bradford-adapted CIE XYZ with ACES white.",
				},
				{
					"name": "xyz_d65_xyz_aces_cat02_cases",
					"source": "xyz_d65",
					"expected": "xyz_aces_cat02",
					"description": "Relationship between CIE XYZ D65 and CAT02-adapted CIE XYZ with ACES white.",
				},
			],
		},
		{
			"filename": "cielab_reference.h",
			"title": "CIELAB reference data.",
			"details": [
				"Header scope: CIE XYZ D50, CIELAB D50, and CIELCH D50 reference data.",
				"CIELAB's common interchange/reference white is D50, so this header uses D50 Lab values.",
				"Coordinate field `xyz_d50`: CIE XYZ adapted from `xyz_d65` to D50 with Bradford chromatic adaptation, ordered X, Y, Z, alpha.",
				"Coordinate field `cielab_d50`: CIELAB using D50 reference white, ordered L*, a*, b*, alpha.",
				"Coordinate field `cielch_d50`: CIELCH using D50 reference white, ordered L*, C*, h, alpha; h is a 0..1 turn.",
				"Use xyz_d50_reference.h for the CIE XYZ D65 <-> CIE XYZ D50 adaptation rows.",
			],
			"arrays": [
				{
					"name": "xyz_d50_cielab_d50_cases",
					"source": "xyz_d50",
					"expected": "cielab_d50",
					"description": "Relationship between CIE XYZ D50 and CIELAB D50.",
				},
				{
					"name": "xyz_d50_cielch_d50_cases",
					"source": "xyz_d50",
					"expected": "cielch_d50",
					"description": "Relationship between CIE XYZ D50 and CIELCH D50.",
				},
			],
		},
		{
			"filename": "oklab_reference.h",
			"title": "Oklab reference data.",
			"details": [
				"Header scope: Oklab and Oklch generated from the canonical CIE XYZ D65 hub.",
				"Coordinate field `xyz_d65`: CIE XYZ with D65 reference white and Y = 1, ordered X, Y, Z, alpha.",
				"Coordinate field `oklab`: Oklab coordinates, ordered L, a, b, alpha.",
				"Coordinate field `oklch`: Oklch coordinates, ordered L, C, h, alpha; h is a 0..1 turn.",
				"Oklab uses colour-science's D65-based Oklab implementation.",
			],
			"arrays": [
				{
					"name": "xyz_d65_oklab_cases",
					"source": "xyz_d65",
					"expected": "oklab",
					"description": "Relationship between CIE XYZ D65 and Oklab.",
				},
				{
					"name": "xyz_d65_oklch_cases",
					"source": "xyz_d65",
					"expected": "oklch",
					"description": "Relationship between CIE XYZ D65 and Oklch.",
				},
			],
		},
	]

	for spec in header_specs:
		row_count = write_reference_header(spec["filename"], spec["title"], spec["details"], spec["arrays"], cases, metadata)
		print(f"Wrote {REFERENCE_DIR / spec['filename']} with {row_count} rows.")


def main() -> None:
	for family in RGB_FAMILIES.values():
		if family["colourspace"] not in colour.RGB_COLOURSPACES:
			raise SystemExit(f"colour-science does not have RGB colourspace: {family['colourspace']}")

	cases = []
	cases.extend(make_css_cases())
	cases.extend(make_alpha_cases())
	cases.extend(make_wide_gamut_cases())
	cases.extend(make_numeric_stress_cases())

	metadata = make_metadata(len(cases))
	export_headers(cases, metadata)


if __name__ == "__main__":
	main()
