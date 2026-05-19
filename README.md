# Maths Town Graphics Support Library

A small header-only library made for use with Maths Town projects.  I've made it public in case it useful to anyone. (Permissive MIT license)

## Adding to your Project
This repo is designed to be added to your project using the git subtree command.

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
external/graphics/external/simd/include (or the simd code directly if you have subtreed it in.)
```

## Linear Algebra

The `linalg` headers provide small, header-only vector and matrix types for scalar floating point code and Maths Town SIMD component types. Include everything with:

```cpp
#include "linalg.h"
```

Available types are `mt::Vec2`, `mt::Vec3`, `mt::Vec4`, `mt::Mat2`, `mt::Mat3`, and `mt::Mat4`, with GLSL-style aliases `vec2`, `vec3`, `vec4`, `mat2`, `mat3`, and `mat4`.

The vector API includes component-wise arithmetic, scalar arithmetic, swizzles, length/magnitude/normalize, dot/cross, distance, reflect/refract, mix/lerp, clamp/min/max, wrap/remap/rescale, smoothstep/step, degrees/radians, trig, exp/log/pow, and fused multiply-add helpers. The matrix API includes column-major `Mat2`, `Mat3`, and `Mat4` storage, row/column access, matrix and scalar arithmetic, matrix multiplication, vector-matrix multiplication in both orders, determinant, transpose, and inverse.

## Compiler Specific Notes

### GCC
On Windows, with GCC use options: "-Wa,-muse-unaligned-vector-move" when using simd types as GCC can emit aligned loads on unaligned data. See [GCC bug 54412](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412).

## Running Tests

The tests in this repository are mainly AI generated. 

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
