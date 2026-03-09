# Maths Town SIMD Library

A small header-only SIMD library made for use with Maths Town projects.  I've made it public in case it useful to anyone. (Permissive MIT license)

## Adding to your Project
This repo is designed to be added to your project using the git subtree command.

To add the 1st time (to the 3rd-party folder):
```
git remote add simd https://github.com/MathsTown/simd.git
git fetch simd
git subtree add --prefix=3rd-party/simd simd main --squash
```

To update:
```
git fetch simd
git subtree pull --prefix=3rd-party/simd simd main --squash
```

Add this path to your list of includes:
```
3rd-party/simd/include
```


## Overview.
This library abstracts SIMD types into more useful types.  It has two goals.
- Allow generic code to be written that is optimised for different SIMD CPUs. 
- Allow generic code to run at different precision levels (same code for f32 or f64).  This is common need for my fractal code.

c++20 is required.

This repository is being prepared as a standalone open source library that can be used via git subtree.

### CPU Levels.
The code targets 4 different CPU levels:
 - Fallback - A fallback for non x86_64, such as wasm.
 - x86_64 level 1 (SSE2 +) - Covers practically all x86_64 CPUs.  128 bit insructions.
 - x86_64 level 3 (AVX2 +) - CPUs that fully implement 256bit instructions.
 - x86_64 level 4 (AVX-512 +) - CPUs that fully implement 512bit instructions.

 - Extra instructions (such as SSE4.1, AVX) are used if detected at compile time.

Generally, CPUs are restricted to the lower support level if they only have partial implmentation of an instruction size.  For example, there are some older CPUs that support AVX but not AVX2, these will need to use 128bit instrucitons.

For Windows, the best option is one build per x86-64 level.  Dispatch at download or install time. It is also possible to dispatch to different dynamic library files (.dll) at runtime.  Functions for runtime detection are included.

For Linux, compile from source.

Runtime dispatch is supported on MSVC only, but is much slower than compile time dispatch (using 512bit instructions when the compiler is not optimising 512 bit registers is sub-optimal).  


### Types
- Fallback:
  - FallbackUInt32
  - FallbackInt32
  - FallbackUInt64
  - FallbackInt64
  - FallbackFloat32
  - FallbackFloat64

- 128 bit (level 1):
  - Simd128Uint32
  - Simd128Int32
  - Simd128Uint64
  - Simd128Int64
  - Simd128Float32
  - Simd128Float64

- 256 bit (level 3):
  - Simd256Uint32
  - Simd256Int32
  - Simd256Uint64
  - Simd256Int64
  - Simd256Float32
  - Simd256Float64

- 512 bit (level 4):
  - Simd512Uint32
  - Simd512Int32
  - Simd512Uint64
  - Simd512Int64
  - Simd512Float32
  - Simd512Float64

### Generic Functions
You can write generic code for any floating point type (f32 or f64) like so:
```
template <SimdFloat S>
void my_function(S x, Sy); 
```
SimdFloat is a concept that will accept any of the floating point types above, or any user type that implements the concept.

### Concepts
Concepts allow you to specify the minimum level of functionality required by your generic function.

Simd - Base SIMD-like value type
 - storage/layout checks (`v`, `sizeof(v)`, `number_of_elements()`)
 - copy/move/default construction
 - support checks (`cpu_supported()`, `compiler_supported()`)
 - lane metadata (`size_of_element()`, `number_of_elements()`)
 - lane access (`element(i)`, `set_element(i, value)`)
 - arithmetic with same type (`+`, `-`, `*`, `/` and compound assignments)
 - construction helper (`make_sequential(...)`)

SimdSigned - Signed numeric SIMD type
 - everything in `Simd`
 - unary negation (`-x`)
 - absolute value (`abs(x)`)

SimdReal - Real-valued SIMD type (floating point or fixed-point style)
 - everything in `SimdSigned`
 - rounding family (`floor`, `ceil`, `trunc`, `round`, `fract`)
 - bounds helpers (`min`, `max`, `clamp`)

SimdFloat - Floating-point SIMD type
 - everything in `SimdReal`
 - reciprocal approximation (`reciprocal_approx`)
 - fused operations (`fma`, `fms`, `fnma`, `fnms`)

SimdFloat32 / SimdFloat64 - Precision constrained floating SIMD types
 - everything in `SimdFloat`
 - lane element type is exactly `float` / `double`

SimdMath - Real SIMD type with extended math support
 - everything in `SimdReal`
 - roots and powers (`sqrt`, `cbrt`, `pow`, `hypot`)
 - trig/inverse trig (`sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`)
 - hyperbolic/inverse hyperbolic (`sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`)
 - exponential/log family (`exp`, `exp2`, `exp10`, `expm1`, `log`, `log1p`, `log2`, `log10`)

SimdCompareOps - SIMD type with compare/blend operations
 - everything in `Simd`
 - compare functions (`compare_equal`, `compare_less`, `compare_less_equal`, `compare_greater`, `compare_greater_equal`)
 - branchless select helpers (`if_equal`, `if_less`, `if_less_equal`, `if_greater`, `if_greater_equal`)
 - masked blend (`blend(a, b, mask)`)

SimdInteger - Integer SIMD type (signed or unsigned)
 - everything in `Simd`
 - scalar arithmetic overloads (`simd op scalar`, `scalar op simd`, compound scalar assignments)
 - bitwise operations (`&`, `|`, `^`, `~` and compound assignments)
 - shifts (`<<`, `>>`)
 - integer bounds helpers (`min`, `max`)

SimdUInt - Unsigned integer SIMD type
 - everything in `SimdInteger`
 - bit rotates (`rotl`, `rotr`)

SimdInt - Signed integer SIMD type
 - everything in `SimdInteger`
 - signed helpers inherited via `SimdSigned` (`-x`, `abs(x)`)

SimdUInt32 / SimdUInt64 - Width constrained unsigned integer SIMD types
 - everything in `SimdUInt`
 - lane element type is exactly `uint32_t` / `uint64_t`

SimdInt32 / SimdInt64 - Width constrained signed integer SIMD types
 - everything in `SimdInt`
 - lane element type is exactly `int32_t` / `int64_t`

SimdFloatToInt - Float SIMD type that supports bit reinterpret cast to matching unsigned int lanes
 - everything in `SimdFloat`
 - requires `T::U` (matching unsigned SIMD type)
 - requires `bitcast_to_uint()`

### Concepts Implemented by Types
32-bit Floating Point: Simd512Float32, Simd256Float32, Simd128Float32 & FallbackFloat32 implement:
- Simd
- SimdSigned
- SimdReal
- SimdFloat
- SimdFloat32
- SimdMath (Full Simd for MSVC via SVML, but currently only slower fallback options for GCC and Clang)
- SimdCompareOps
- SimdFloatToInt

64-bit Floating Point: Simd512Float64, Simd256Float64, Simd128Float64 & FallbackFloat64 implement:
- Simd
- SimdSigned
- SimdReal
- SimdFloat
- SimdFloat64
- SimdMath (Full Simd for MSVC via SVML, but currently only slower fallback options for GCC and Clang)
- SimdCompareOps
- SimdFloatToInt

32-bit Integer: Simd512Int32, Simd256Int32, Simd128Int32 & FallbackInt32 implement:
- Simd
- SimdSigned
- SimdInteger
- SimdInt
- SimdInt32
- SimdCompareOps

64-bit Integer: Simd512Int64, Simd256Int64, Simd128Int64 & FallbackInt64 implement:
- Simd
- SimdSigned
- SimdInteger
- SimdInt
- SimdInt64
- SimdCompareOps

32-bit Unsigned Integer: Simd512Uint32, Simd256Uint32, Simd128Uint32 & FallbackUInt32 implement:
- Simd
- SimdInteger
- SimdUInt
- SimdUInt32
- SimdCompareOps

64-bit Unsigned Integer: Simd512Uint64, Simd256Uint64, Simd128Uint64 & FallbackUInt64 implement:
- Simd
- SimdInteger
- SimdUInt
- SimdUInt64
- SimdCompareOps


## Compilers Specific Notes
Transcendental math operations fully implemented on MSVC by using the SVML library.  However other compilers use a simple fallback mechanism where lanes are unpacked and the std library is used.  I will try to implement these operations one day.

### Microsoft C++ Compiler
  Transcendental math operations such as `sin()`, `cos()`, `tan()`, `log()`, and `exp()` use the SVML library so are fully simd.

### Clang
  SVML is not available.  Transcendental math operations such as `sin()`, `cos()`, `tan()`, `log()`, and `exp()` use my code, which are currently just slow fallbacks. 

### GCC
  SVML is not available.  Transcendental math operations such as `sin()`, `cos()`, `tan()`, `log()`, and `exp()` use my code, which are currently just slow fallbacks. 

  GCC disables intrinsics if they are not supported, so types that are not supported will not be exposed when compiling with GCC.


## Testing Suite
If you wish to run tests, these are currently setup to run under PowerShell for Windows.  There are tests for 3 windows compilers: MSVC, gcc and clang (as clang-cl).

Windows (PowerShell), from the repository root (default generator/compiler on your machine):

```powershell
cmake -S tests -B build
cmake --build build --config Release
.\build\Release\simd_test.exe
```

### Test Microsoft C++ Compiler

Windows (PowerShell), explicitly with MSVC (`cl`):

```powershell
cmake -S tests -B build\msvc -G "Visual Studio 17 2022" -A x64
cmake --build build\msvc --config Release
.\build\msvc\Release\simd_test.exe
```

### Test Clang (clang-cl) C++ Compiler

Windows (PowerShell), explicitly with Clang (`clang-cl`):

```powershell
cmake -S tests -B build\clang -G "Visual Studio 17 2022" -A x64 -T ClangCL
cmake --build build\clang --config Release
.\build\clang\Release\simd_test.exe
```

### Test GCC C++ Compiler 
GCC really does not like to see intrinsics from higher levels in the code, even if they are unreachable. There is a separate compilation mode to test each level of support, to make sure I don't rely on runtime checks for this in GCC.  (Whereas other compilers we just check at runtime)

Windows (PowerShell), GCC level 1 mode (128 bit types, `SSE2` minimum):

```powershell
cmake -S tests -B build\gcc-sse2 -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ -DSIMD_TEST_MODE=SSE2
cmake --build build\gcc-sse2
.\build\gcc-sse2\simd_test.exe
```

Windows (PowerShell), GCC level 3 mode (256 bit types, `AVX2` minimum):

```powershell
cmake -S tests -B build\gcc-avx2 -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ -DSIMD_TEST_MODE=AVX2
cmake --build build\gcc-avx2
.\build\gcc-avx2\simd_test.exe
```

Windows (PowerShell), GCC level 4 mode (512 bit types, `AVX-512f and AVX-512dq` minimum):

```powershell
cmake -S tests -B build\gcc-avx512 -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ -DSIMD_TEST_MODE=AVX512
cmake --build build\gcc-avx512
.\build\gcc-avx512\simd_test.exe
```
