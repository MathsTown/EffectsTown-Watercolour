/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Float32 SIMD tests.
Uses independent scalar/std:: math as the reference implementation.
*********************************************************************************************************/

#include "test_f32.h"

#include <bit>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "../include/simd-cpuid.h"
#include "../include/simd-f32.h"

using namespace mt;

namespace {

enum class ArithmeticOp {
    add,
    sub,
    mul,
    div
};

enum class UnaryMathOp {
    floor_op,
    ceil_op,
    trunc_op,
    round_op,
    fract_op,
    sqrt_op,
    abs_op,
    exp_op,
    exp2_op,
    exp10_op,
    expm1_op,
    log_op,
    log1p_op,
    log2_op,
    log10_op,
    cbrt_op,
    sin_op,
    cos_op,
    tan_op,
    asin_op,
    acos_op,
    atan_op,
    tanh_op,
    sinh_op,
    cosh_op,
    asinh_op,
    acosh_op,
    atanh_op
};

enum class BinaryMathOp {
    pow_op,
    hypot_op,
    atan2_op
};

enum class CompareMathOp {
    min_op,
    max_op
};

enum class FmaOp {
    fma_op,
    fms_op,
    fnma_op,
    fnms_op
};

enum class ArithmeticPath {
    vector_vector,
    vector_scalar_right,
    scalar_left_vector,
    compound_vector,
    compound_scalar
};

constexpr ArithmeticPath kArithmeticPaths[] = {
    ArithmeticPath::vector_vector,
    ArithmeticPath::vector_scalar_right,
    ArithmeticPath::scalar_left_vector,
    ArithmeticPath::compound_vector,
    ArithmeticPath::compound_scalar
};

std::string_view path_name(ArithmeticPath path) {
    if (path == ArithmeticPath::vector_vector) {
        return "vector op vector";
    }
    if (path == ArithmeticPath::vector_scalar_right) {
        return "vector op scalar";
    }
    if (path == ArithmeticPath::scalar_left_vector) {
        return "scalar op vector";
    }
    if (path == ArithmeticPath::compound_vector) {
        return "compound op vector";
    }
    return "compound op scalar";
}

bool float_matches_fallback(float actual, float expected) {
    if (std::isnan(actual) && std::isnan(expected)) {
        return true;
    }
    if (std::isinf(actual) || std::isinf(expected)) {
        return actual == expected;
    }

    const float abs_diff = std::fabs(actual - expected);
    const float scale = std::max(1.0f, std::max(std::fabs(actual), std::fabs(expected)));
    return abs_diff <= (2.0e-6f * scale);
}

bool float_matches_exact(float actual, float expected) {
    if (std::isnan(actual) && std::isnan(expected)) {
        return true;
    }
    return std::bit_cast<uint32_t>(actual) == std::bit_cast<uint32_t>(expected);
}

float apply_scalar_binary(float lhs, float rhs, ArithmeticOp op) {
    if (op == ArithmeticOp::add) {
        return lhs + rhs;
    }
    if (op == ArithmeticOp::sub) {
        return lhs - rhs;
    }
    if (op == ArithmeticOp::mul) {
        return lhs * rhs;
    }
    return lhs / rhs;
}

template <typename SimdType>
void apply_simd_op_with_path(
    const SimdType& a,
    const SimdType& b,
    float scalar,
    ArithmeticOp op,
    ArithmeticPath path,
    SimdType& out) {
    if (path == ArithmeticPath::vector_vector) {
        if (op == ArithmeticOp::add) { out = a + b; return; }
        if (op == ArithmeticOp::sub) { out = a - b; return; }
        if (op == ArithmeticOp::mul) { out = a * b; return; }
        out = a / b;
        return;
    }
    if (path == ArithmeticPath::vector_scalar_right) {
        if (op == ArithmeticOp::add) { out = a + scalar; return; }
        if (op == ArithmeticOp::sub) { out = a - scalar; return; }
        if (op == ArithmeticOp::mul) { out = a * scalar; return; }
        out = a / scalar;
        return;
    }
    if (path == ArithmeticPath::scalar_left_vector) {
        if (op == ArithmeticOp::add) { out = scalar + b; return; }
        if (op == ArithmeticOp::sub) { out = scalar - b; return; }
        if (op == ArithmeticOp::mul) { out = scalar * b; return; }
        out = scalar / b;
        return;
    }

    out = a;
    if (path == ArithmeticPath::compound_vector) {
        if (op == ArithmeticOp::add) { out += b; }
        else if (op == ArithmeticOp::sub) { out -= b; }
        else if (op == ArithmeticOp::mul) { out *= b; }
        else { out /= b; }
        return;
    }

    if (op == ArithmeticOp::add) { out += scalar; }
    else if (op == ArithmeticOp::sub) { out -= scalar; }
    else if (op == ArithmeticOp::mul) { out *= scalar; }
    else { out /= scalar; }
}

float apply_scalar_op_with_path(
    float lhs,
    float rhs,
    float scalar,
    ArithmeticOp op,
    ArithmeticPath path) {
    if (path == ArithmeticPath::vector_vector) {
        return apply_scalar_binary(lhs, rhs, op);
    }
    if (path == ArithmeticPath::vector_scalar_right) {
        return apply_scalar_binary(lhs, scalar, op);
    }
    if (path == ArithmeticPath::scalar_left_vector) {
        return apply_scalar_binary(scalar, rhs, op);
    }
    if (path == ArithmeticPath::compound_vector) {
        return apply_scalar_binary(lhs, rhs, op);
    }

    return apply_scalar_binary(lhs, scalar, op);
}

float finite_scalar_for_op(ArithmeticOp op) {
    if (op == ArithmeticOp::add) { return 3.75f; }
    if (op == ArithmeticOp::sub) { return -2.5f; }
    if (op == ArithmeticOp::mul) { return -1.25f; }
    return 2.0f;
}

std::string_view unary_math_name(UnaryMathOp op) {
    switch (op) {
    case UnaryMathOp::floor_op: return "floor";
    case UnaryMathOp::ceil_op: return "ceil";
    case UnaryMathOp::trunc_op: return "trunc";
    case UnaryMathOp::round_op: return "round";
    case UnaryMathOp::fract_op: return "fract";
    case UnaryMathOp::sqrt_op: return "sqrt";
    case UnaryMathOp::abs_op: return "abs";
    case UnaryMathOp::exp_op: return "exp";
    case UnaryMathOp::exp2_op: return "exp2";
    case UnaryMathOp::exp10_op: return "exp10";
    case UnaryMathOp::expm1_op: return "expm1";
    case UnaryMathOp::log_op: return "log";
    case UnaryMathOp::log1p_op: return "log1p";
    case UnaryMathOp::log2_op: return "log2";
    case UnaryMathOp::log10_op: return "log10";
    case UnaryMathOp::cbrt_op: return "cbrt";
    case UnaryMathOp::sin_op: return "sin";
    case UnaryMathOp::cos_op: return "cos";
    case UnaryMathOp::tan_op: return "tan";
    case UnaryMathOp::asin_op: return "asin";
    case UnaryMathOp::acos_op: return "acos";
    case UnaryMathOp::atan_op: return "atan";
    case UnaryMathOp::tanh_op: return "tanh";
    case UnaryMathOp::sinh_op: return "sinh";
    case UnaryMathOp::cosh_op: return "cosh";
    case UnaryMathOp::asinh_op: return "asinh";
    case UnaryMathOp::acosh_op: return "acosh";
    case UnaryMathOp::atanh_op: return "atanh";
    }
    return "unknown";
}

std::string_view binary_math_name(BinaryMathOp op) {
    switch (op) {
    case BinaryMathOp::pow_op: return "pow";
    case BinaryMathOp::hypot_op: return "hypot";
    case BinaryMathOp::atan2_op: return "atan2";
    }
    return "unknown";
}

std::string_view compare_math_name(CompareMathOp op) {
    switch (op) {
    case CompareMathOp::min_op: return "min";
    case CompareMathOp::max_op: return "max";
    }
    return "unknown";
}

std::string_view fma_name(FmaOp op) {
    switch (op) {
    case FmaOp::fma_op: return "fma";
    case FmaOp::fms_op: return "fms";
    case FmaOp::fnma_op: return "fnma";
    case FmaOp::fnms_op: return "fnms";
    }
    return "unknown";
}

float round_nearest_even_default(float value) {
    // Project default: round() uses nearest-even (banker's rounding).
    return std::nearbyint(value);
}

float apply_scalar_unary(float value, UnaryMathOp op) {
    switch (op) {
    case UnaryMathOp::floor_op: return std::floor(value);
    case UnaryMathOp::ceil_op: return std::ceil(value);
    case UnaryMathOp::trunc_op: return std::trunc(value);
    case UnaryMathOp::round_op: return round_nearest_even_default(value);
    case UnaryMathOp::fract_op: return value - std::floor(value);
    case UnaryMathOp::sqrt_op: return std::sqrt(value);
    case UnaryMathOp::abs_op: return std::abs(value);
    case UnaryMathOp::exp_op: return std::exp(value);
    case UnaryMathOp::exp2_op: return std::exp2(value);
    case UnaryMathOp::exp10_op: return std::pow(10.0f, value);
    case UnaryMathOp::expm1_op: return std::expm1(value);
    case UnaryMathOp::log_op: return std::log(value);
    case UnaryMathOp::log1p_op: return std::log1p(value);
    case UnaryMathOp::log2_op: return std::log2(value);
    case UnaryMathOp::log10_op: return std::log10(value);
    case UnaryMathOp::cbrt_op: return std::cbrt(value);
    case UnaryMathOp::sin_op: return std::sin(value);
    case UnaryMathOp::cos_op: return std::cos(value);
    case UnaryMathOp::tan_op: return std::tan(value);
    case UnaryMathOp::asin_op: return std::asin(value);
    case UnaryMathOp::acos_op: return std::acos(value);
    case UnaryMathOp::atan_op: return std::atan(value);
    case UnaryMathOp::tanh_op: return std::tanh(value);
    case UnaryMathOp::sinh_op: return std::sinh(value);
    case UnaryMathOp::cosh_op: return std::cosh(value);
    case UnaryMathOp::asinh_op: return std::asinh(value);
    case UnaryMathOp::acosh_op: return std::acosh(value);
    case UnaryMathOp::atanh_op: return std::atanh(value);
    }
    return value;
}

float apply_scalar_binary_math(float lhs, float rhs, BinaryMathOp op) {
    switch (op) {
    case BinaryMathOp::pow_op: return std::pow(lhs, rhs);
    case BinaryMathOp::hypot_op: return std::hypot(lhs, rhs);
    case BinaryMathOp::atan2_op: return std::atan2(lhs, rhs);
    }
    return lhs;
}

float apply_scalar_compare_math(float lhs, float rhs, CompareMathOp op) {
    switch (op) {
    case CompareMathOp::min_op: return std::min(lhs, rhs);
    case CompareMathOp::max_op: return std::max(lhs, rhs);
    }
    return lhs;
}

float apply_scalar_clamp_unit(float value) {
    return std::min(std::max(value, 0.0f), 1.0f);
}

float apply_scalar_clamp_range(float value, float lo, float hi) {
    return std::min(std::max(value, lo), hi);
}

float apply_scalar_fma(float a, float b, float c, FmaOp op) {
    switch (op) {
    case FmaOp::fma_op: return std::fma(a, b, c);
    case FmaOp::fms_op: return std::fma(a, b, -c);
    case FmaOp::fnma_op: return std::fma(-a, b, c);
    case FmaOp::fnms_op: return std::fma(-a, b, -c);
    }
    return a;
}

template <typename SimdType>
SimdType apply_simd_unary_math(const SimdType& value, UnaryMathOp op) {
    switch (op) {
    case UnaryMathOp::floor_op: return floor(value);
    case UnaryMathOp::ceil_op: return ceil(value);
    case UnaryMathOp::trunc_op: return trunc(value);
    case UnaryMathOp::round_op: return round(value);
    case UnaryMathOp::fract_op: return fract(value);
    case UnaryMathOp::sqrt_op: return sqrt(value);
    case UnaryMathOp::abs_op: return abs(value);
    case UnaryMathOp::exp_op: return exp(value);
    case UnaryMathOp::exp2_op: return exp2(value);
    case UnaryMathOp::exp10_op: return exp10(value);
    case UnaryMathOp::expm1_op: return expm1(value);
    case UnaryMathOp::log_op: return log(value);
    case UnaryMathOp::log1p_op: return log1p(value);
    case UnaryMathOp::log2_op: return log2(value);
    case UnaryMathOp::log10_op: return log10(value);
    case UnaryMathOp::cbrt_op: return cbrt(value);
    case UnaryMathOp::sin_op: return sin(value);
    case UnaryMathOp::cos_op: return cos(value);
    case UnaryMathOp::tan_op: return tan(value);
    case UnaryMathOp::asin_op: return asin(value);
    case UnaryMathOp::acos_op: return acos(value);
    case UnaryMathOp::atan_op: return atan(value);
    case UnaryMathOp::tanh_op: return tanh(value);
    case UnaryMathOp::sinh_op: return sinh(value);
    case UnaryMathOp::cosh_op: return cosh(value);
    case UnaryMathOp::asinh_op: return asinh(value);
    case UnaryMathOp::acosh_op: return acosh(value);
    case UnaryMathOp::atanh_op: return atanh(value);
    }
    return value;
}

template <typename SimdType>
SimdType apply_simd_binary_math(const SimdType& lhs, const SimdType& rhs, BinaryMathOp op) {
    switch (op) {
    case BinaryMathOp::pow_op: return pow(lhs, rhs);
    case BinaryMathOp::hypot_op: return hypot(lhs, rhs);
    case BinaryMathOp::atan2_op: return atan2(lhs, rhs);
    }
    return lhs;
}

template <typename SimdType>
SimdType apply_simd_compare_math(const SimdType& lhs, const SimdType& rhs, CompareMathOp op) {
    switch (op) {
    case CompareMathOp::min_op: return min(lhs, rhs);
    case CompareMathOp::max_op: return max(lhs, rhs);
    }
    return lhs;
}

template <typename SimdType>
SimdType apply_simd_fma(const SimdType& a, const SimdType& b, const SimdType& c, FmaOp op) {
    switch (op) {
    case FmaOp::fma_op: return fma(a, b, c);
    case FmaOp::fms_op: return fms(a, b, c);
    case FmaOp::fnma_op: return fnma(a, b, c);
    case FmaOp::fnms_op: return fnms(a, b, c);
    }
    return a;
}

float sample_unary_input(std::mt19937& rng, UnaryMathOp op) {
    switch (op) {
    case UnaryMathOp::floor_op:
    case UnaryMathOp::ceil_op:
    case UnaryMathOp::trunc_op:
    case UnaryMathOp::round_op:
    case UnaryMathOp::fract_op: {
        std::uniform_real_distribution<float> d(-1000.0f, 1000.0f);
        return d(rng);
    }
    case UnaryMathOp::sqrt_op: {
        std::uniform_real_distribution<float> d(0.0f, 1.0e6f);
        return d(rng);
    }
    case UnaryMathOp::log_op:
    case UnaryMathOp::log2_op: {
        std::uniform_real_distribution<float> d(1.0e-6f, 1.0e6f);
        return d(rng);
    }
    case UnaryMathOp::log10_op: {
        std::uniform_real_distribution<float> d(1.0e-6f, 1.0e6f);
        return d(rng);
    }
    case UnaryMathOp::log1p_op: {
        std::uniform_real_distribution<float> d(-0.9999f, 1.0e6f);
        return d(rng);
    }
    case UnaryMathOp::asin_op:
    case UnaryMathOp::acos_op: {
        std::uniform_real_distribution<float> d(-1.0f, 1.0f);
        return d(rng);
    }
    case UnaryMathOp::atanh_op: {
        std::uniform_real_distribution<float> d(-0.99f, 0.99f);
        return d(rng);
    }
    case UnaryMathOp::acosh_op: {
        std::uniform_real_distribution<float> d(1.0f, 1.0e6f);
        return d(rng);
    }
    case UnaryMathOp::exp_op:
    case UnaryMathOp::expm1_op: {
        std::uniform_real_distribution<float> d(-20.0f, 20.0f);
        return d(rng);
    }
    case UnaryMathOp::exp2_op: {
        std::uniform_real_distribution<float> d(-30.0f, 30.0f);
        return d(rng);
    }
    case UnaryMathOp::exp10_op: {
        std::uniform_real_distribution<float> d(-10.0f, 10.0f);
        return d(rng);
    }
    case UnaryMathOp::sinh_op:
    case UnaryMathOp::cosh_op: {
        std::uniform_real_distribution<float> d(-10.0f, 10.0f);
        return d(rng);
    }
    case UnaryMathOp::tan_op: {
        std::uniform_real_distribution<float> d(-1.0f, 1.0f);
        return d(rng);
    }
    default: {
        std::uniform_real_distribution<float> d(-100.0f, 100.0f);
        return d(rng);
    }
    }
}

std::pair<float, float> sample_binary_input(std::mt19937& rng, BinaryMathOp op) {
    switch (op) {
    case BinaryMathOp::pow_op: {
        std::uniform_real_distribution<float> base_dist(1.0e-4f, 100.0f);
        std::uniform_real_distribution<float> exp_dist(-5.0f, 5.0f);
        return {base_dist(rng), exp_dist(rng)};
    }
    case BinaryMathOp::hypot_op: {
        std::uniform_real_distribution<float> d(-1.0e4f, 1.0e4f);
        return {d(rng), d(rng)};
    }
    case BinaryMathOp::atan2_op: {
        std::uniform_real_distribution<float> d(-1.0e4f, 1.0e4f);
        return {d(rng), d(rng)};
    }
    }
    return {0.0f, 0.0f};
}

std::pair<float, float> sample_compare_input(std::mt19937& rng) {
    std::uniform_real_distribution<float> d(-1.0e4f, 1.0e4f);
    return {d(rng), d(rng)};
}

std::tuple<float, float, float> sample_clamp_input(std::mt19937& rng) {
    std::uniform_real_distribution<float> value_dist(-1.0e4f, 1.0e4f);
    std::uniform_real_distribution<float> bound_dist(-2.0e3f, 2.0e3f);
    float lo = bound_dist(rng);
    float hi = bound_dist(rng);
    if (lo > hi) {
        std::swap(lo, hi);
    }
    return { value_dist(rng), lo, hi };
}

std::tuple<float, float, float> sample_fma_input(std::mt19937& rng) {
    std::uniform_real_distribution<float> a_dist(-1.0e4f, 1.0e4f);
    std::uniform_real_distribution<float> b_dist(-1.0e4f, 1.0e4f);
    std::uniform_real_distribution<float> c_dist(-1.0e4f, 1.0e4f);
    return { a_dist(rng), b_dist(rng), c_dist(rng) };
}

template <typename SimdType>
bool run_float32_binary_test_for_type(
    const std::string& type_name,
    const std::string& op_name,
    ArithmeticOp op,
    CpuInformation cpu,
    TestHarness& harness) {
    const std::string test_name = type_name + " Float32 " + op_name;

    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937 rng(20260224u);
    std::uniform_real_distribution<float> small_dist(-1000.0f, 1000.0f);
    std::uniform_real_distribution<float> tiny_dist(-1.0e-30f, 1.0e-30f);
    std::uniform_real_distribution<float> large_dist(-1.0e30f, 1.0e30f);

    for (int iteration = 0; iteration < 1200; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            float lhs = 0.0f;
            float rhs = 1.0f;
            const int selector = (iteration + lane) % 3;
            if (selector == 0) {
                lhs = small_dist(rng);
                rhs = small_dist(rng);
            } else if (selector == 1) {
                lhs = tiny_dist(rng);
                rhs = large_dist(rng);
            } else {
                lhs = large_dist(rng);
                rhs = tiny_dist(rng);
            }
            if (op == ArithmeticOp::div && rhs == 0.0f) {
                rhs = 1.0f;
            }
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const float scalar = finite_scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const float lhs = a.element(lane);
                const float rhs = b.element(lane);
                const float expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const float actual = result.element(lane);
                if (!float_matches_exact(actual, expected)) {
                    harness.add_result(
                        test_name,
                        false,
                        "Random mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    const float finite_edges[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        std::numeric_limits<float>::denorm_min(),
        -std::numeric_limits<float>::denorm_min(),
        1.0e-30f,
        -1.0e-30f,
        1.0e30f,
        -1.0e30f,
        std::numeric_limits<float>::min(),
        -std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max() * 0.5f,
        -std::numeric_limits<float>::max() * 0.5f
    };
    constexpr int finite_edge_count = static_cast<int>(sizeof(finite_edges) / sizeof(finite_edges[0]));

    for (int base = 0; base < finite_edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const float lhs = finite_edges[(base + lane) % finite_edge_count];
            float rhs = finite_edges[(base * 3 + lane + 1) % finite_edge_count];
            if (op == ArithmeticOp::div && rhs == 0.0f) { rhs = 1.0f; }
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const float scalar = finite_scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const float lhs = a.element(lane);
                const float rhs = b.element(lane);
                const float expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const float actual = result.element(lane);
                if (!float_matches_exact(actual, expected)) {
                    harness.add_result(
                        test_name,
                        false,
                        "Finite edge mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    const float special_edges[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN(),
        -std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max()
    };
    constexpr int special_edge_count = static_cast<int>(sizeof(special_edges) / sizeof(special_edges[0]));

    for (int base = 0; base < special_edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const float lhs = special_edges[(base + lane) % special_edge_count];
            const float rhs = special_edges[(base * 5 + lane + 1) % special_edge_count];
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const float scalar = special_edges[(base * 7 + 3) % special_edge_count];
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const float lhs = a.element(lane);
                const float rhs = b.element(lane);
                const float expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const float actual = result.element(lane);
                if (!float_matches_exact(actual, expected)) {
                    harness.add_result(
                        test_name,
                        false,
                        "Special edge mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    harness.add_result(test_name, true, "Random + finite/special edges matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_float32_math_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937 rng(20260304u);

    const UnaryMathOp unary_ops[] = {
        UnaryMathOp::floor_op, UnaryMathOp::ceil_op, UnaryMathOp::trunc_op, UnaryMathOp::round_op, UnaryMathOp::fract_op,
        UnaryMathOp::sqrt_op, UnaryMathOp::abs_op, UnaryMathOp::exp_op, UnaryMathOp::exp2_op,
        UnaryMathOp::exp10_op, UnaryMathOp::expm1_op, UnaryMathOp::log_op, UnaryMathOp::log1p_op,
        UnaryMathOp::log2_op, UnaryMathOp::log10_op, UnaryMathOp::cbrt_op, UnaryMathOp::sin_op, UnaryMathOp::cos_op,
        UnaryMathOp::tan_op, UnaryMathOp::asin_op, UnaryMathOp::acos_op, UnaryMathOp::atan_op,
        UnaryMathOp::tanh_op, UnaryMathOp::sinh_op, UnaryMathOp::cosh_op, UnaryMathOp::asinh_op,
        UnaryMathOp::acosh_op, UnaryMathOp::atanh_op
    };

    for (const UnaryMathOp op : unary_ops) {
        const std::string test_name = type_name + " Float32 " + std::string(unary_math_name(op));
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType value{};
            for (int lane = 0; lane < lanes; ++lane) {
                value.set_element(lane, sample_unary_input(rng, op));
            }

            const SimdType result = apply_simd_unary_math(value, op);
            for (int lane = 0; lane < lanes; ++lane) {
                const float expected = apply_scalar_unary(value.element(lane), op);
                const float actual = result.element(lane);
                if (!float_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        if (op == UnaryMathOp::round_op) {
            constexpr float tie_inputs[] = {
                -4.5f, -3.5f, -2.5f, -1.5f, -0.5f,
                 0.5f,  1.5f,  2.5f,  3.5f,  4.5f
            };
            constexpr int tie_input_count = static_cast<int>(sizeof(tie_inputs) / sizeof(tie_inputs[0]));
            alignas(SimdType) SimdType tie_values{};
            for (int lane = 0; lane < lanes; ++lane) {
                tie_values.set_element(lane, tie_inputs[lane % tie_input_count]);
            }
            const SimdType tie_result = apply_simd_unary_math(tie_values, op);
            for (int lane = 0; lane < lanes; ++lane) {
                const float expected = round_nearest_even_default(tie_values.element(lane));
                if (!float_matches_fallback(tie_result.element(lane), expected)) {
                    harness.add_result(test_name, false, "nearest-even tie mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    const BinaryMathOp binary_ops[] = { BinaryMathOp::pow_op, BinaryMathOp::hypot_op, BinaryMathOp::atan2_op };
    for (const BinaryMathOp op : binary_ops) {
        const std::string test_name = type_name + " Float32 " + std::string(binary_math_name(op));
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType lhs{};
            alignas(SimdType) SimdType rhs{};
            for (int lane = 0; lane < lanes; ++lane) {
                const auto [a, b] = sample_binary_input(rng, op);
                lhs.set_element(lane, a);
                rhs.set_element(lane, b);
            }

            const SimdType result = apply_simd_binary_math(lhs, rhs, op);
            for (int lane = 0; lane < lanes; ++lane) {
                const float expected = apply_scalar_binary_math(lhs.element(lane), rhs.element(lane), op);
                const float actual = result.element(lane);
                if (!float_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    const CompareMathOp compare_ops[] = { CompareMathOp::min_op, CompareMathOp::max_op };
    for (const CompareMathOp op : compare_ops) {
        const std::string test_name = type_name + " Float32 " + std::string(compare_math_name(op));
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType lhs{};
            alignas(SimdType) SimdType rhs{};
            for (int lane = 0; lane < lanes; ++lane) {
                const auto [a, b] = sample_compare_input(rng);
                lhs.set_element(lane, a);
                rhs.set_element(lane, b);
            }

            const SimdType result = apply_simd_compare_math(lhs, rhs, op);
            for (int lane = 0; lane < lanes; ++lane) {
                const float expected = apply_scalar_compare_math(lhs.element(lane), rhs.element(lane), op);
                const float actual = result.element(lane);
                if (!float_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    {
        const std::string test_name = type_name + " Float32 clamp";
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType value{};
            alignas(SimdType) SimdType lo{};
            alignas(SimdType) SimdType hi{};
            for (int lane = 0; lane < lanes; ++lane) {
                const auto [v, min_v, max_v] = sample_clamp_input(rng);
                value.set_element(lane, v);
                lo.set_element(lane, min_v);
                hi.set_element(lane, max_v);
            }

            const SimdType unit_result = clamp(value);
            const SimdType range_result = clamp(value, lo, hi);
            for (int lane = 0; lane < lanes; ++lane) {
                const float unit_expected = apply_scalar_clamp_unit(value.element(lane));
                const float range_expected = apply_scalar_clamp_range(value.element(lane), lo.element(lane), hi.element(lane));
                if (!float_matches_fallback(unit_result.element(lane), unit_expected)) {
                    harness.add_result(test_name, false, "unit clamp mismatch, lane " + std::to_string(lane));
                    return false;
                }
                if (!float_matches_fallback(range_result.element(lane), range_expected)) {
                    harness.add_result(test_name, false, "range clamp mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    const FmaOp fma_ops[] = { FmaOp::fma_op, FmaOp::fms_op, FmaOp::fnma_op, FmaOp::fnms_op };
    for (const FmaOp op : fma_ops) {
        const std::string test_name = type_name + " Float32 " + std::string(fma_name(op));
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType a{};
            alignas(SimdType) SimdType b{};
            alignas(SimdType) SimdType c{};
            for (int lane = 0; lane < lanes; ++lane) {
                const auto [av, bv, cv] = sample_fma_input(rng);
                a.set_element(lane, av);
                b.set_element(lane, bv);
                c.set_element(lane, cv);
            }

            const SimdType result = apply_simd_fma(a, b, c, op);
            for (int lane = 0; lane < lanes; ++lane) {
                const float expected = apply_scalar_fma(a.element(lane), b.element(lane), c.element(lane), op);
                const float actual = result.element(lane);
                if (!float_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    return true;
}

[[maybe_unused]] bool reciprocal_matches_true(float actual, float expected) {
    if (std::isnan(actual) && std::isnan(expected)) {
        return true;
    }
    if (std::isinf(actual) || std::isinf(expected)) {
        return actual == expected;
    }

    const float abs_diff = std::fabs(actual - expected);
    const float scale = std::max(1.0f, std::fabs(expected));
    return abs_diff <= (7.5e-4f * scale);
}

template <typename SimdType, typename UIntType>
bool run_float32_helper_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Float32 helpers";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    if (SimdType::size_of_element() != static_cast<int>(sizeof(float))) {
        harness.add_result(test_name, false, "size_of_element() mismatch");
        return false;
    }

    {
        const SimdType sequential = SimdType::make_sequential(-3.5f);
        for (int lane = 0; lane < lanes; ++lane) {
            const float expected = -3.5f + static_cast<float>(lane);
            if (!float_matches_fallback(sequential.element(lane), expected)) {
                harness.add_result(test_name, false, "make_sequential mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        const SimdType set_value = SimdType::make_set1(4.25f);
        for (int lane = 0; lane < lanes; ++lane) {
            if (!float_matches_fallback(set_value.element(lane), 4.25f)) {
                harness.add_result(test_name, false, "make_set1 mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        alignas(UIntType) UIntType ints{};
        for (int lane = 0; lane < lanes; ++lane) {
            ints.set_element(lane, static_cast<uint32_t>(lane * 17 + 3));
        }
        const SimdType converted = SimdType::make_from_int32(ints);
        for (int lane = 0; lane < lanes; ++lane) {
            const float expected = static_cast<float>(static_cast<int32_t>(ints.element(lane)));
            if (!float_matches_fallback(converted.element(lane), expected)) {
                harness.add_result(test_name, false, "make_from_int32 mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        constexpr uint32_t signed_edge_values[] = {
            0x00000000u,
            0x00000001u,
            0x7fffffffu,
            0x80000000u,
            0x80000001u,
            0xffffffffu
        };
        alignas(UIntType) UIntType ints{};
        for (int lane = 0; lane < lanes; ++lane) {
            ints.set_element(lane, signed_edge_values[lane % static_cast<int>(sizeof(signed_edge_values) / sizeof(signed_edge_values[0]))]);
        }
        const SimdType converted = SimdType::make_from_int32(ints);
        for (int lane = 0; lane < lanes; ++lane) {
            const uint32_t raw = ints.element(lane);
            const float expected = static_cast<float>(static_cast<int32_t>(raw));
            if (!float_matches_fallback(converted.element(lane), expected)) {
                harness.add_result(test_name, false, "make_from_int32 signed-edge mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        constexpr uint32_t bit_patterns[] = {
            0x00000000u,
            0x80000000u,
            0x3f800000u,
            0xbf800000u,
            0x40490fdbu,
            0x7f800000u,
            0xff800000u,
            0x7fc00000u
        };
        alignas(SimdType) SimdType values{};
        for (int lane = 0; lane < lanes; ++lane) {
            values.set_element(lane, std::bit_cast<float>(bit_patterns[lane % static_cast<int>(sizeof(bit_patterns) / sizeof(bit_patterns[0]))]));
        }
        const UIntType bits = values.bitcast_to_uint();
        for (int lane = 0; lane < lanes; ++lane) {
            const uint32_t expected = bit_patterns[lane % static_cast<int>(sizeof(bit_patterns) / sizeof(bit_patterns[0]))];
            if (bits.element(lane) != expected) {
                harness.add_result(test_name, false, "bitcast_to_uint mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        std::mt19937 rng(20260304u);
        std::uniform_real_distribution<float> value_dist(-50.0f, 50.0f);
        for (int iteration = 0; iteration < 300; ++iteration) {
            alignas(SimdType) SimdType value{};
            for (int lane = 0; lane < lanes; ++lane) {
                float lane_value = value_dist(rng);
                if (std::fabs(lane_value) < 0.25f) {
                    lane_value = (lane % 2 == 0) ? 0.25f : -0.25f;
                }
                value.set_element(lane, lane_value);
            }

            const SimdType reciprocal = reciprocal_approx(value);
            const SimdType scalar_clamped = clamp(value, -2.25f, 7.5f);
            for (int lane = 0; lane < lanes; ++lane) {
                const float lane_value = value.element(lane);
                const float reciprocal_expected = 1.0f / lane_value;
                const float clamp_expected = apply_scalar_clamp_range(lane_value, -2.25f, 7.5f);
                const bool reciprocal_ok = []<typename T>(float actual, float expected) {
                    if constexpr (std::is_same_v<T, FallbackFloat32>) {
                        return actual == expected;
                    }
                    else {
                        return reciprocal_matches_true(actual, expected);
                    }
                }.template operator()<SimdType>(reciprocal.element(lane), reciprocal_expected);
                if (!reciprocal_ok) {
                    harness.add_result(test_name, false, "reciprocal_approx mismatch, lane " + std::to_string(lane));
                    return false;
                }
                if (!float_matches_fallback(scalar_clamped.element(lane), clamp_expected)) {
                    harness.add_result(test_name, false, "scalar clamp mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    harness.add_result(test_name, true, "helpers matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_float32_compare_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Float32 compare/blend";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> value_dist(-1000.0f, 1000.0f);
    std::uniform_real_distribution<float> choice_dist(-100.0f, 100.0f);

    for (int iteration = 0; iteration < 500; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType if_true{};
        alignas(SimdType) SimdType if_false{};
        alignas(SimdType) SimdType zero(0.0f);
        alignas(SimdType) SimdType ones(1.0f);
        alignas(SimdType) SimdType nan_input{};

        for (int lane = 0; lane < lanes; ++lane) {
            const float av = value_dist(rng);
            float bv = value_dist(rng);
            if (bv == av) {
                bv = std::nextafter(av, std::numeric_limits<float>::infinity());
            }
            a.set_element(lane, av);
            b.set_element(lane, bv);
            if_true.set_element(lane, choice_dist(rng));
            if_false.set_element(lane, choice_dist(rng));
            nan_input.set_element(lane, (lane % 3 == 0) ? std::numeric_limits<float>::quiet_NaN() : av);
        }

        const auto mask_eq = compare_equal(a, b);
        const auto mask_lt = compare_less(a, b);
        const auto mask_le = compare_less_equal(a, b);
        const auto mask_gt = compare_greater(a, b);
        const auto mask_ge = compare_greater_equal(a, b);
        const auto mask_nan = isnan(nan_input);
        const auto mask_eq_nan_self = compare_equal(nan_input, nan_input);

        const SimdType blend_eq = blend(zero, ones, mask_eq);
        const SimdType blend_lt = blend(zero, ones, mask_lt);
        const SimdType blend_le = blend(zero, ones, mask_le);
        const SimdType blend_gt = blend(zero, ones, mask_gt);
        const SimdType blend_ge = blend(zero, ones, mask_ge);
        const SimdType blend_nan = blend(zero, ones, mask_nan);
        const SimdType blend_eq_nan_self = blend(zero, ones, mask_eq_nan_self);

        const SimdType if_eq = if_equal(a, b, if_true, if_false);
        const SimdType if_lt = if_less(a, b, if_true, if_false);
        const SimdType if_le = if_less_equal(a, b, if_true, if_false);
        const SimdType if_gt = if_greater(a, b, if_true, if_false);
        const SimdType if_ge = if_greater_equal(a, b, if_true, if_false);
        const SimdType if_is_nan = if_nan(nan_input, if_true, if_false);

        for (int lane = 0; lane < lanes; ++lane) {
            const bool eq = a.element(lane) == b.element(lane);
            const bool lt = a.element(lane) < b.element(lane);
            const bool le = a.element(lane) <= b.element(lane);
            const bool gt = a.element(lane) > b.element(lane);
            const bool ge = a.element(lane) >= b.element(lane);
            const bool is_nan = std::isnan(nan_input.element(lane));

            if (!float_matches_exact(blend_eq.element(lane), eq ? 1.0f : 0.0f) ||
                !float_matches_exact(blend_lt.element(lane), lt ? 1.0f : 0.0f) ||
                !float_matches_exact(blend_le.element(lane), le ? 1.0f : 0.0f) ||
                !float_matches_exact(blend_gt.element(lane), gt ? 1.0f : 0.0f) ||
                !float_matches_exact(blend_ge.element(lane), ge ? 1.0f : 0.0f) ||
                !float_matches_exact(blend_nan.element(lane), is_nan ? 1.0f : 0.0f) ||
                !float_matches_exact(blend_eq_nan_self.element(lane), is_nan ? 0.0f : 1.0f)) {
                harness.add_result(test_name, false, "blend(compare/isnan) mismatch, lane " + std::to_string(lane));
                return false;
            }

            const float t = if_true.element(lane);
            const float f = if_false.element(lane);
            if (!float_matches_fallback(if_eq.element(lane), eq ? t : f) ||
                !float_matches_fallback(if_lt.element(lane), lt ? t : f) ||
                !float_matches_fallback(if_le.element(lane), le ? t : f) ||
                !float_matches_fallback(if_gt.element(lane), gt ? t : f) ||
                !float_matches_fallback(if_ge.element(lane), ge ? t : f) ||
                !float_matches_fallback(if_is_nan.element(lane), is_nan ? t : f)) {
                harness.add_result(test_name, false, "if_* mismatch, lane " + std::to_string(lane));
                return false;
            }
        }

        const auto all_true_mask = compare_equal(a, a);
        const auto all_false_mask = compare_equal(a, b);
        if (!test_all_true(all_true_mask) || test_all_false(all_true_mask)) {
            harness.add_result(test_name, false, "test_all_true/false failed for all-true mask");
            return false;
        }
        if (!test_all_false(all_false_mask) || test_all_true(all_false_mask)) {
            harness.add_result(test_name, false, "test_all_true/false failed for all-false mask");
            return false;
        }

        if constexpr (SimdType::number_of_elements() > 1) {
            alignas(SimdType) SimdType partial_a = a;
            alignas(SimdType) SimdType partial_b = a;
            partial_b.set_element(0, std::nextafter(partial_a.element(0), std::numeric_limits<float>::infinity()));
            const auto partial_mask = compare_equal(partial_a, partial_b);
            if (test_all_true(partial_mask) || test_all_false(partial_mask)) {
                harness.add_result(test_name, false, "test_all_true/false failed for partial mask");
                return false;
            }
        }
    }

    harness.add_result(test_name, true, "compare/blend/if_* matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_float32_suite_for_type(const char* type_name, CpuInformation cpu, TestHarness& harness) {
    return run_float32_binary_test_for_type<SimdType>(type_name, "addition", ArithmeticOp::add, cpu, harness) &&
           run_float32_binary_test_for_type<SimdType>(type_name, "subtraction", ArithmeticOp::sub, cpu, harness) &&
           run_float32_binary_test_for_type<SimdType>(type_name, "multiplication", ArithmeticOp::mul, cpu, harness) &&
           run_float32_binary_test_for_type<SimdType>(type_name, "division", ArithmeticOp::div, cpu, harness) &&
           run_float32_compare_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_float32_helper_test_for_type<SimdType, typename SimdType::U>(type_name, cpu, harness) &&
           run_float32_math_test_for_type<SimdType>(type_name, cpu, harness);
}

bool run_float32_scalar_common_ops_test(TestHarness& harness) {
    constexpr const char* test_name = "float scalar common compare/blend";

    const float samples[] = {
        -4.0f,
        -0.0f,
        0.0f,
        1.5f,
        42.0f,
        std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN()
    };

    for (float a : samples) {
        for (float b : samples) {
            const float if_true = a + 7.0f;
            const float if_false = b - 3.0f;

            const bool eq = (a == b);
            const bool lt = (a < b);
            const bool le = (a <= b);
            const bool gt = (a > b);
            const bool ge = (a >= b);
            const bool a_is_nan = std::isnan(a);

            if (compare_equal(a, b) != eq ||
                compare_less(a, b) != lt ||
                compare_less_equal(a, b) != le ||
                compare_greater(a, b) != gt ||
                compare_greater_equal(a, b) != ge ||
                isnan(a) != a_is_nan) {
                harness.add_result(test_name, false, "compare/isnan mismatch");
                return false;
            }

            if (!float_matches_exact(blend(if_false, if_true, eq), eq ? if_true : if_false) ||
                !float_matches_exact(if_equal(a, b, if_true, if_false), eq ? if_true : if_false) ||
                !float_matches_exact(if_less(a, b, if_true, if_false), lt ? if_true : if_false) ||
                !float_matches_exact(if_less_equal(a, b, if_true, if_false), le ? if_true : if_false) ||
                !float_matches_exact(if_greater(a, b, if_true, if_false), gt ? if_true : if_false) ||
                !float_matches_exact(if_greater_equal(a, b, if_true, if_false), ge ? if_true : if_false) ||
                !float_matches_exact(if_nan(a, if_true, if_false), a_is_nan ? if_true : if_false)) {
                harness.add_result(test_name, false, "blend/if_* mismatch");
                return false;
            }
        }
    }

    harness.add_result(test_name, true, "raw float uses shared compare/blend/if_* helpers");
    return true;
}

} // namespace

void run_float32_arithmetic_tests(TestHarness& harness) {
    std::cout << "\n===== Float32 Arithmetic Tests =====\n";
    if (harness.should_halt()) {
        return;
    }
    CpuInformation cpu{};
#define MT_RUN_OR_HALT(expr) do { if (!(expr)) { return; } } while (false)
    MT_RUN_OR_HALT(run_float32_scalar_common_ops_test(harness));
    MT_RUN_OR_HALT(run_float32_suite_for_type<FallbackFloat32>("Fallback", cpu, harness));
#if MT_SIMD_ARCH_X64 || (MT_SIMD_ARCH_WASM && defined(__wasm_simd128__))
    MT_RUN_OR_HALT(run_float32_suite_for_type<Simd128Float32>("Simd128", cpu, harness));
#if MT_SIMD_ALLOW_LEVEL3_TYPES
    MT_RUN_OR_HALT(run_float32_suite_for_type<Simd256Float32>("Simd256", cpu, harness));
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
    MT_RUN_OR_HALT(run_float32_suite_for_type<Simd512Float32>("Simd512", cpu, harness));
#endif
#endif
#undef MT_RUN_OR_HALT
    std::cout << "==================================\n";
}





