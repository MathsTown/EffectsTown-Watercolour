# Maths Town Graphics Support Library

A small header-only library made for use with Maths Town projects.  I've made it public in case it useful to anyone. (Permissive MIT license)

These libraries are designed to work with my simd library. All ops can be be used on simd vectors.  (You need the simd library also)

It contains:
- A glsl style linear algebra (linalg.h)
- A floating point colour management and conversion library (colour.h)


## Adding to your Project
This repo is designed to be added to your project using the git subtree command.  You will also need to subtree the simd my repo.

To add the 1st time (to the external folder):
```
git remote add graphics https://github.com/MathsTown/graphics.git
git fetch graphics
git subtree add --prefix=external/graphics graphics main --squash
```

To update:
```
git fetch graphics
git subtree pull --prefix=external/graphics graphics main --squash
```

Add this path to your list of includes:
```
external/graphics/include
external/simd/include
```

## Linear Algebra

The `linalg` headers provide small, header-only vector and matrix types for scalar floating point code and Maths Town SIMD component types. Include everything with:

```cpp
#include "linalg.h"
```

Available types are `mt::Vec2`, `mt::Vec3`, `mt::Vec4`, `mt::Mat2`, `mt::Mat3`, and `mt::Mat4`, with GLSL-style aliases `vec2`, `vec3`, `vec4`, `mat2`, `mat3`, and `mat4`.

The vector API includes component-wise arithmetic, scalar arithmetic, swizzles, length/magnitude/normalize, dot/cross, distance, reflect/refract, mix/lerp, clamp/min/max, wrap/remap/rescale, smoothstep/step, degrees/radians, trig, exp/log/pow, and fused multiply-add helpers. The matrix API includes column-major `Mat2`, `Mat3`, and `Mat4` storage, row/column access, matrix and scalar arithmetic, matrix multiplication, vector-matrix multiplication in both orders, determinant, transpose, and inverse.

## Colour

The `colour` headers provide small, header-only colour types, colour-space conversions, gamut helpers, blend operations, and matte operations for scalar floating point code and Maths Town SIMD component types. Include everything with:

```cpp
#include "colour.h"
```

Supported colour families include:

- Generic RGB carriers: `mt::GenericRgb`, `mt::Colour8`
- sRGB: `mt::Srgb`, `mt::SrgbLinear`, `mt::SrgbHsv`, `mt::SrgbHsl`, `mt::SrgbLinearHsv`, `mt::SrgbLinearHsl`
- Display P3: `mt::DisplayP3`, `mt::DisplayP3Linear`, `mt::DisplayP3Hsv`, `mt::DisplayP3Hsl`
- Adobe RGB (1998): `mt::AdobeRgb`, `mt::AdobeRgbLinear`, `mt::AdobeRgbHsv`, `mt::AdobeRgbHsl`, `mt::AdobeRgbLinearHsv`, `mt::AdobeRgbLinearHsl`
- ProPhoto RGB: `mt::ProPhotoRgb`, `mt::ProPhotoRgbLinear`, `mt::ProPhotoRgbHsv`, `mt::ProPhotoRgbHsl`, `mt::ProPhotoRgbLinearHsv`, `mt::ProPhotoRgbLinearHsl`
- Rec.709: `mt::Rec709Oetf`, `mt::Rec709Gamma24`, `mt::Rec709Linear`, `mt::Rec709OetfHsv`, `mt::Rec709OetfHsl`, `mt::Rec709Gamma24Hsv`, `mt::Rec709Gamma24Hsl`, `mt::Rec709LinearHsv`, `mt::Rec709LinearHsl`
- Rec.2020: `mt::Rec2020Oetf`, `mt::Rec2020Gamma24`, `mt::Rec2020Linear`, `mt::Rec2020OetfHsv`, `mt::Rec2020OetfHsl`, `mt::Rec2020Gamma24Hsv`, `mt::Rec2020Gamma24Hsl`, `mt::Rec2020LinearHsv`, `mt::Rec2020LinearHsl`
- ACES2065-1 / AP0: `mt::Aces2065_1`, `mt::Aces2065_1Hsv`, `mt::Aces2065_1Hsl`
- ACEScg / AP1: `mt::Acescg`, `mt::AcescgHsv`, `mt::AcescgHsl`
- XYZ: `mt::Xyz`, `mt::XyzD50`, `mt::XyzAces`
- CIE Lab/LCh: `mt::Cielab`, `mt::Cielch`
- Oklab/Oklch: `mt::Oklab`, `mt::Oklch`

Shared colour concepts describe the generic API surface:

- `mt::ColourComponent`: scalar floating point and Maths Town SIMD floating point component types
- `mt::RgbaColour`: RGB/RGBA colour types with gamut and clipping operations
- `mt::XyzaColour`: XYZ-style colour types
- `mt::HsvaColour`: HSV-style colour types
- `mt::HslaColour`: HSL-style colour types
- `mt::LabaColour`: Lab-style colour types
- `mt::LchaColour`: LCh-style colour types

RGB types support operations such as:

- construction from grey, RGB, and RGBA component values
- explicit construction from `mt::GenericRgb` channel values
- conversion to colour-space-neutral render/API values with `to_generic_rgb()`
- alpha handling with `premultiply_alpha()` and `un_premultiply_alpha()`
- range handling with `clamp()`, `clip()`, `clip_rgb()`, `no_negatives()`, and `sanitise_floats()`
- gamut mapping with `fit_gamut()`
- transfer conversion between encoded and linear forms, for example `to_linear()`, `to_srgb()`, and `to_rec709_oetf()`
- conversion to XYZ with `to_xyz()`
- conversion to HSV and HSL companion types with `to_hsv()` and `to_hsl()`
- arithmetic operators for linear-light RGB types and ACES RGB types
- SIMD component support through the same API shape where the component type is a Maths Town SIMD float

`mt::GenericRgb` is a colour-space-neutral RGB/RGBA carrier for renderer and host boundaries. Colour-space RGB types convert to it implicitly by copying channel values, while conversion from `GenericRgb` back to a colour-space RGB type is explicit because it interprets generic channel values in that target space. `mt::Colour8` is a scalar 8-bit RGBA carrier with conversions to and from `GenericRgb<float>`.

Shared RGB blend and matte operations include:

- normal/source-over compositing with `blend_normal()`
- separable blend modes: `blend_multiply()`, `blend_screen()`, `blend_overlay()`, `blend_darken()`, `blend_lighten()`, `blend_color_dodge()`, `blend_color_burn()`, `blend_hard_light()`, `blend_soft_light()`, `blend_difference()`, `blend_exclusion()`
- arithmetic blend modes: `blend_linear_burn()`, `blend_linear_dodge()`, `blend_add()`, `blend_subtract()`, `blend_divide()`, `blend_linear_light()`, `blend_vivid_light()`, `blend_pin_light()`, `blend_hard_mix()`
- non-separable blend modes: `blend_hue()`, `blend_saturation()`, `blend_color()`, `blend_luminosity()`
- matte helpers: `matte_stencil_alpha()`, `matte_stencil_luma()`, `matte_silhouette_alpha()`, `matte_silhouette_luma()`, `matte_alpha_add()`

XYZ, Lab, and LCh-style types support operations such as:

- range handling with `clamp()`, `no_negatives()`, and `sanitise_floats()`
- conversion between native white points, including D65, D50, and ACES white where appropriate
- conversion between Cartesian and polar perceptual forms, such as `to_cielch()`, `to_cielab()`, `to_oklch()`, and `to_oklab()`
- hue wrapping for LCh-style types with `wrap_hue()`

## Compiler Specific Notes

### GCC
On Windows, with GCC use options: "-Wa,-muse-unaligned-vector-move" when using simd types as GCC can emit aligned loads on unaligned data. See [GCC bug 54412](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412).

## Running Tests

The tests in this repository are mainly AI generated, so they may not be perfect, but they seem reasonable comprehensive.  Colour conversion data to test against is generated in Python.

Note: The test script pulls the main branch of the simd project automatically.

From the repository root, run the full local compiler test script:

```powershell
.\test.ps1
```

This configures, builds, and runs the tests with AVX2 enabled for:

- MSVC
- ClangCL
- MinGW GCC

The script creates separate build folders under `build/` for each compiler and runs each executable through CTest.

To run a single default test build manually:

```powershell
cmake -S tests -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
