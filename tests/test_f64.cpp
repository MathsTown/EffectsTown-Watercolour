/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Float64 SIMD tests.
Uses independent scalar/std:: math as the reference implementation.
*********************************************************************************************************/

#include "test_f64.h"

#include <bit>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string_view>
#include <tuple>
#include <utility>
#include <type_traits>

#include "../include/simd-cpuid.h"
#include "../include/simd-concepts.h"
#include "../include/simd-uint32.h"
#include "../include/simd-uint64.h"
#include "../include/simd-f64.h"

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
    sind_op,
    cosd_op,
    tand_op,
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

bool double_matches_fallback(double actual, double expected) {
    if (std::isnan(actual) && std::isnan(expected)) {
        return true;
    }
    if (std::isinf(actual) || std::isinf(expected)) {
        return actual == expected;
    }

    const double abs_diff = std::fabs(actual - expected);
    const double scale = std::max(1.0, std::max(std::fabs(actual), std::fabs(expected)));
    return abs_diff <= (1.0e-12 * scale);
}

bool double_matches_exact(double actual, double expected) {
    if (std::isnan(actual) && std::isnan(expected)) {
        return true;
    }
    return std::bit_cast<uint64_t>(actual) == std::bit_cast<uint64_t>(expected);
}

double apply_scalar_binary(double lhs, double rhs, ArithmeticOp op) {
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
    double scalar,
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

double apply_scalar_op_with_path(
    double lhs,
    double rhs,
    double scalar,
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

double finite_scalar_for_op(ArithmeticOp op) {
    if (op == ArithmeticOp::add) { return 7.5; }
    if (op == ArithmeticOp::sub) { return -2.25; }
    if (op == ArithmeticOp::mul) { return -1.5; }
    return 2.0;
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
    case UnaryMathOp::sind_op: return "sind";
    case UnaryMathOp::cosd_op: return "cosd";
    case UnaryMathOp::tand_op: return "tand";
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

double apply_scalar_unary(double value, UnaryMathOp op) {
    switch (op) {
    case UnaryMathOp::floor_op: return std::floor(value);
    case UnaryMathOp::ceil_op: return std::ceil(value);
    case UnaryMathOp::trunc_op: return std::trunc(value);
    case UnaryMathOp::round_op: return std::round(value);
    case UnaryMathOp::fract_op: return value - std::floor(value);
    case UnaryMathOp::sqrt_op: return std::sqrt(value);
    case UnaryMathOp::abs_op: return std::abs(value);
    case UnaryMathOp::exp_op: return std::exp(value);
    case UnaryMathOp::exp2_op: return std::exp2(value);
    case UnaryMathOp::exp10_op: return std::pow(10.0, value);
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
    case UnaryMathOp::sind_op: return std::sin(value * (3.14159265358979323846 / 180.0));
    case UnaryMathOp::cosd_op: return std::cos(value * (3.14159265358979323846 / 180.0));
    case UnaryMathOp::tand_op: return std::tan(value * (3.14159265358979323846 / 180.0));
    case UnaryMathOp::tanh_op: return std::tanh(value);
    case UnaryMathOp::sinh_op: return std::sinh(value);
    case UnaryMathOp::cosh_op: return std::cosh(value);
    case UnaryMathOp::asinh_op: return std::asinh(value);
    case UnaryMathOp::acosh_op: return std::acosh(value);
    case UnaryMathOp::atanh_op: return std::atanh(value);
    }
    return value;
}

double apply_scalar_binary_math(double lhs, double rhs, BinaryMathOp op) {
    switch (op) {
    case BinaryMathOp::pow_op: return std::pow(lhs, rhs);
    case BinaryMathOp::hypot_op: return std::hypot(lhs, rhs);
    case BinaryMathOp::atan2_op: return std::atan2(lhs, rhs);
    }
    return lhs;
}

double apply_scalar_compare_math(double lhs, double rhs, CompareMathOp op) {
    switch (op) {
    case CompareMathOp::min_op: return std::min(lhs, rhs);
    case CompareMathOp::max_op: return std::max(lhs, rhs);
    }
    return lhs;
}

double apply_scalar_clamp_unit(double value) {
    return std::min(std::max(value, 0.0), 1.0);
}

double apply_scalar_clamp_range(double value, double lo, double hi) {
    return std::min(std::max(value, lo), hi);
}

double apply_scalar_fma(double a, double b, double c, FmaOp op) {
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
    constexpr double kDegToRad = 3.14159265358979323846 / 180.0;
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
    case UnaryMathOp::sind_op:
        if constexpr (requires(const SimdType& v) { sind(v); }) {
            return sind(value);
        } else {
            SimdType out{};
            for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
                out.set_element(lane, std::sin(value.element(lane) * kDegToRad));
            }
            return out;
        }
    case UnaryMathOp::cosd_op:
        if constexpr (requires(const SimdType& v) { cosd(v); }) {
            return cosd(value);
        } else {
            SimdType out{};
            for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
                out.set_element(lane, std::cos(value.element(lane) * kDegToRad));
            }
            return out;
        }
    case UnaryMathOp::tand_op:
        if constexpr (requires(const SimdType& v) { tand(v); }) {
            return tand(value);
        } else {
            SimdType out{};
            for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
                out.set_element(lane, std::tan(value.element(lane) * kDegToRad));
            }
            return out;
        }
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

double sample_unary_input(std::mt19937_64& rng, UnaryMathOp op) {
    switch (op) {
    case UnaryMathOp::floor_op:
    case UnaryMathOp::ceil_op:
    case UnaryMathOp::trunc_op:
    case UnaryMathOp::round_op:
    case UnaryMathOp::fract_op: {
        std::uniform_real_distribution<double> d(-1.0e6, 1.0e6);
        return d(rng);
    }
    case UnaryMathOp::sqrt_op: {
        std::uniform_real_distribution<double> d(0.0, 1.0e12);
        return d(rng);
    }
    case UnaryMathOp::log_op:
    case UnaryMathOp::log2_op: {
        std::uniform_real_distribution<double> d(1.0e-12, 1.0e12);
        return d(rng);
    }
    case UnaryMathOp::log10_op: {
        std::uniform_real_distribution<double> d(1.0e-12, 1.0e12);
        return d(rng);
    }
    case UnaryMathOp::log1p_op: {
        std::uniform_real_distribution<double> d(-0.999999999999, 1.0e12);
        return d(rng);
    }
    case UnaryMathOp::asin_op:
    case UnaryMathOp::acos_op: {
        std::uniform_real_distribution<double> d(-1.0, 1.0);
        return d(rng);
    }
    case UnaryMathOp::atanh_op: {
        std::uniform_real_distribution<double> d(-0.99, 0.99);
        return d(rng);
    }
    case UnaryMathOp::acosh_op: {
        std::uniform_real_distribution<double> d(1.0, 1.0e12);
        return d(rng);
    }
    case UnaryMathOp::exp_op:
    case UnaryMathOp::expm1_op: {
        std::uniform_real_distribution<double> d(-100.0, 100.0);
        return d(rng);
    }
    case UnaryMathOp::exp2_op: {
        std::uniform_real_distribution<double> d(-100.0, 100.0);
        return d(rng);
    }
    case UnaryMathOp::exp10_op: {
        std::uniform_real_distribution<double> d(-20.0, 20.0);
        return d(rng);
    }
    case UnaryMathOp::sinh_op:
    case UnaryMathOp::cosh_op: {
        std::uniform_real_distribution<double> d(-20.0, 20.0);
        return d(rng);
    }
    case UnaryMathOp::tan_op: {
        std::uniform_real_distribution<double> d(-1.0, 1.0);
        return d(rng);
    }
    default: {
        std::uniform_real_distribution<double> d(-1000.0, 1000.0);
        return d(rng);
    }
    }
}

std::pair<double, double> sample_binary_input(std::mt19937_64& rng, BinaryMathOp op) {
    switch (op) {
    case BinaryMathOp::pow_op: {
        std::uniform_real_distribution<double> base_dist(1.0e-8, 1000.0);
        std::uniform_real_distribution<double> exp_dist(-8.0, 8.0);
        return {base_dist(rng), exp_dist(rng)};
    }
    case BinaryMathOp::hypot_op: {
        std::uniform_real_distribution<double> d(-1.0e8, 1.0e8);
        return {d(rng), d(rng)};
    }
    case BinaryMathOp::atan2_op: {
        std::uniform_real_distribution<double> d(-1.0e8, 1.0e8);
        return {d(rng), d(rng)};
    }
    }
    return {0.0, 0.0};
}

std::pair<double, double> sample_compare_input(std::mt19937_64& rng) {
    std::uniform_real_distribution<double> d(-1.0e8, 1.0e8);
    return {d(rng), d(rng)};
}

std::tuple<double, double, double> sample_clamp_input(std::mt19937_64& rng) {
    std::uniform_real_distribution<double> value_dist(-1.0e8, 1.0e8);
    std::uniform_real_distribution<double> bound_dist(-1.0e6, 1.0e6);
    double lo = bound_dist(rng);
    double hi = bound_dist(rng);
    if (lo > hi) {
        std::swap(lo, hi);
    }
    return { value_dist(rng), lo, hi };
}

std::tuple<double, double, double> sample_fma_input(std::mt19937_64& rng) {
    std::uniform_real_distribution<double> a_dist(-1.0e8, 1.0e8);
    std::uniform_real_distribution<double> b_dist(-1.0e8, 1.0e8);
    std::uniform_real_distribution<double> c_dist(-1.0e8, 1.0e8);
    return { a_dist(rng), b_dist(rng), c_dist(rng) };
}

template <typename SimdType>
bool run_float64_binary_test_for_type(
    const std::string& type_name,
    const std::string& op_name,
    ArithmeticOp op,
    CpuInformation cpu,
    TestHarness& harness) {
    const std::string test_name = type_name + " Float64 " + op_name;

    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_real_distribution<double> small_dist(-1.0e6, 1.0e6);
    std::uniform_real_distribution<double> tiny_dist(-1.0e-200, 1.0e-200);
    std::uniform_real_distribution<double> large_dist(-1.0e200, 1.0e200);

    for (int iteration = 0; iteration < 1000; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};

        for (int lane = 0; lane < lanes; ++lane) {
            double lhs = 0.0;
            double rhs = 1.0;
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

            if (op == ArithmeticOp::div && rhs == 0.0) { rhs = 1.0; }

            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const double scalar = finite_scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const double lhs = a.element(lane);
                const double rhs = b.element(lane);
                const double expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const double actual = result.element(lane);
                if (!double_matches_exact(actual, expected)) {
                    harness.add_result(
                        test_name,
                        false,
                        "Random mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    const double finite_edges[] = {
        0.0,
        -0.0,
        1.0,
        -1.0,
        std::numeric_limits<double>::denorm_min(),
        -std::numeric_limits<double>::denorm_min(),
        1.0e-200,
        -1.0e-200,
        1.0e200,
        -1.0e200,
        std::numeric_limits<double>::min(),
        -std::numeric_limits<double>::min(),
        std::numeric_limits<double>::max() * 0.5,
        -std::numeric_limits<double>::max() * 0.5
    };
    constexpr int finite_edge_count = static_cast<int>(sizeof(finite_edges) / sizeof(finite_edges[0]));

    for (int base = 0; base < finite_edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const double lhs = finite_edges[(base + lane) % finite_edge_count];
            double rhs = finite_edges[(base * 3 + lane + 1) % finite_edge_count];
            if (op == ArithmeticOp::div && rhs == 0.0) { rhs = 1.0; }
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const double scalar = finite_scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const double lhs = a.element(lane);
                const double rhs = b.element(lane);
                const double expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const double actual = result.element(lane);
                if (!double_matches_exact(actual, expected)) {
                    harness.add_result(
                        test_name,
                        false,
                        "Finite edge mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    const double special_edges[] = {
        0.0,
        -0.0,
        1.0,
        -1.0,
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN(),
        -std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::max(),
        -std::numeric_limits<double>::max()
    };
    constexpr int special_edge_count = static_cast<int>(sizeof(special_edges) / sizeof(special_edges[0]));

    for (int base = 0; base < special_edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const double lhs = special_edges[(base + lane) % special_edge_count];
            const double rhs = special_edges[(base * 5 + lane + 1) % special_edge_count];
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const double scalar = special_edges[(base * 7 + 3) % special_edge_count];
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const double lhs = a.element(lane);
                const double rhs = b.element(lane);
                const double expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const double actual = result.element(lane);
                if (!double_matches_exact(actual, expected)) {
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
bool run_float64_math_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260304ull);

    const UnaryMathOp unary_ops[] = {
        UnaryMathOp::floor_op, UnaryMathOp::ceil_op, UnaryMathOp::trunc_op, UnaryMathOp::round_op, UnaryMathOp::fract_op,
        UnaryMathOp::sqrt_op, UnaryMathOp::abs_op, UnaryMathOp::exp_op, UnaryMathOp::exp2_op,
        UnaryMathOp::exp10_op, UnaryMathOp::expm1_op, UnaryMathOp::log_op, UnaryMathOp::log1p_op,
        UnaryMathOp::log2_op, UnaryMathOp::log10_op, UnaryMathOp::cbrt_op, UnaryMathOp::sin_op, UnaryMathOp::cos_op,
        UnaryMathOp::tan_op, UnaryMathOp::asin_op, UnaryMathOp::acos_op, UnaryMathOp::atan_op,
        UnaryMathOp::sind_op, UnaryMathOp::cosd_op, UnaryMathOp::tand_op,
        UnaryMathOp::tanh_op, UnaryMathOp::sinh_op, UnaryMathOp::cosh_op, UnaryMathOp::asinh_op,
        UnaryMathOp::acosh_op, UnaryMathOp::atanh_op
    };

    for (const UnaryMathOp op : unary_ops) {
        const std::string test_name = type_name + " Float64 " + std::string(unary_math_name(op));
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType value{};
            for (int lane = 0; lane < lanes; ++lane) {
                value.set_element(lane, sample_unary_input(rng, op));
            }

            const SimdType result = apply_simd_unary_math(value, op);
            for (int lane = 0; lane < lanes; ++lane) {
                const double expected = apply_scalar_unary(value.element(lane), op);
                const double actual = result.element(lane);
                if (!double_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    const BinaryMathOp binary_ops[] = { BinaryMathOp::pow_op, BinaryMathOp::hypot_op, BinaryMathOp::atan2_op };
    for (const BinaryMathOp op : binary_ops) {
        const std::string test_name = type_name + " Float64 " + std::string(binary_math_name(op));
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
                const double expected = apply_scalar_binary_math(lhs.element(lane), rhs.element(lane), op);
                const double actual = result.element(lane);
                if (!double_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    const CompareMathOp compare_ops[] = { CompareMathOp::min_op, CompareMathOp::max_op };
    for (const CompareMathOp op : compare_ops) {
        const std::string test_name = type_name + " Float64 " + std::string(compare_math_name(op));
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
                const double expected = apply_scalar_compare_math(lhs.element(lane), rhs.element(lane), op);
                const double actual = result.element(lane);
                if (!double_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    {
        const std::string test_name = type_name + " Float64 clamp";
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
                const double unit_expected = apply_scalar_clamp_unit(value.element(lane));
                const double range_expected = apply_scalar_clamp_range(value.element(lane), lo.element(lane), hi.element(lane));
                if (!double_matches_fallback(unit_result.element(lane), unit_expected)) {
                    harness.add_result(test_name, false, "unit clamp mismatch, lane " + std::to_string(lane));
                    return false;
                }
                if (!double_matches_fallback(range_result.element(lane), range_expected)) {
                    harness.add_result(test_name, false, "range clamp mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    const FmaOp fma_ops[] = { FmaOp::fma_op, FmaOp::fms_op, FmaOp::fnma_op, FmaOp::fnms_op };
    for (const FmaOp op : fma_ops) {
        const std::string test_name = type_name + " Float64 " + std::string(fma_name(op));
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
                const double expected = apply_scalar_fma(a.element(lane), b.element(lane), c.element(lane), op);
                const double actual = result.element(lane);
                if (!double_matches_fallback(actual, expected)) {
                    harness.add_result(test_name, false, "mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
        harness.add_result(test_name, true, "matched scalar reference");
    }

    return true;
}

[[maybe_unused]] bool reciprocal_matches_true(double actual, double expected) {
    if (std::isnan(actual) && std::isnan(expected)) {
        return true;
    }
    if (std::isinf(actual) || std::isinf(expected)) {
        return actual == expected;
    }

    const double abs_diff = std::fabs(actual - expected);
    const double scale = std::max(1.0, std::fabs(expected));
    return abs_diff <= (1.0e-4 * scale);
}

template <typename SimdType>
bool run_float64_helper_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Float64 helpers";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    using BitUintType = decltype(std::declval<SimdType>().bitcast_to_uint());
    if (SimdType::size_of_element() != static_cast<int>(sizeof(double))) {
        harness.add_result(test_name, false, "size_of_element() mismatch");
        return false;
    }

    {
        const SimdType sequential = SimdType::make_sequential(-6.25);
        for (int lane = 0; lane < lanes; ++lane) {
            const double expected = -6.25 + static_cast<double>(lane);
            if (!double_matches_fallback(sequential.element(lane), expected)) {
                harness.add_result(test_name, false, "make_sequential mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    if constexpr (requires { SimdType::make_set1(3.5); }) {
        const SimdType set_value = SimdType::make_set1(3.5);
        for (int lane = 0; lane < lanes; ++lane) {
            if (!double_matches_fallback(set_value.element(lane), 3.5)) {
                harness.add_result(test_name, false, "make_set1 mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    if constexpr (requires (BitUintType ints) { SimdType::make_from_uints_52bits(ints); }) {
        alignas(BitUintType) BitUintType ints{};
        constexpr uint64_t kMask52 = 0x000fffffffffffffull;
        for (int lane = 0; lane < lanes; ++lane) {
            ints.set_element(lane, (0x000123456789abcdull + static_cast<uint64_t>(lane) * 97ull) & kMask52);
        }
        const SimdType converted = SimdType::make_from_uints_52bits(ints);
        for (int lane = 0; lane < lanes; ++lane) {
            const double expected = static_cast<double>(ints.element(lane) & kMask52);
            if (!double_matches_fallback(converted.element(lane), expected)) {
                harness.add_result(test_name, false, "make_from_uints_52bits mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        constexpr uint64_t bit_patterns[] = {
            0x0000000000000000ull,
            0x8000000000000000ull,
            0x3ff0000000000000ull,
            0xbff0000000000000ull,
            0x400921fb54442d18ull,
            0x7ff0000000000000ull,
            0xfff0000000000000ull,
            0x7ff8000000000000ull
        };
        alignas(SimdType) SimdType values{};
        for (int lane = 0; lane < lanes; ++lane) {
            values.set_element(lane, std::bit_cast<double>(bit_patterns[lane % static_cast<int>(sizeof(bit_patterns) / sizeof(bit_patterns[0]))]));
        }
        const BitUintType bits = values.bitcast_to_uint();
        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t expected = bit_patterns[lane % static_cast<int>(sizeof(bit_patterns) / sizeof(bit_patterns[0]))];
            if (bits.element(lane) != expected) {
                harness.add_result(test_name, false, "bitcast_to_uint mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    {
        std::mt19937_64 rng(20260304ull);
        std::uniform_real_distribution<double> value_dist(-100.0, 100.0);
        for (int iteration = 0; iteration < 300; ++iteration) {
            alignas(SimdType) SimdType value{};
            for (int lane = 0; lane < lanes; ++lane) {
                double lane_value = value_dist(rng);
                if (std::fabs(lane_value) < 0.5) {
                    lane_value = (lane % 2 == 0) ? 0.5 : -0.5;
                }
                value.set_element(lane, lane_value);
            }

            const SimdType reciprocal = reciprocal_approx(value);
            const SimdType scalar_clamped = clamp(value, -4.5f, 9.25f);
            for (int lane = 0; lane < lanes; ++lane) {
                const double lane_value = value.element(lane);
                const double reciprocal_expected = 1.0 / lane_value;
                const double clamp_expected = apply_scalar_clamp_range(lane_value, -4.5, 9.25);
                const bool reciprocal_ok = []<typename T>(double actual, double expected) {
                    if constexpr (std::is_same_v<T, FallbackFloat64>) {
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
                if (!double_matches_fallback(scalar_clamped.element(lane), clamp_expected)) {
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
bool run_float64_compare_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Float64 compare/blend";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> value_dist(-1.0e6, 1.0e6);
    std::uniform_real_distribution<double> choice_dist(-1000.0, 1000.0);

    for (int iteration = 0; iteration < 500; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType if_true{};
        alignas(SimdType) SimdType if_false{};
        alignas(SimdType) SimdType zero(0.0);
        alignas(SimdType) SimdType ones(1.0);
        alignas(SimdType) SimdType nan_input{};

        for (int lane = 0; lane < lanes; ++lane) {
            const double av = value_dist(rng);
            double bv = value_dist(rng);
            if (bv == av) {
                bv = std::nextafter(av, std::numeric_limits<double>::infinity());
            }
            a.set_element(lane, av);
            b.set_element(lane, bv);
            if_true.set_element(lane, choice_dist(rng));
            if_false.set_element(lane, choice_dist(rng));
            nan_input.set_element(lane, (lane % 3 == 0) ? std::numeric_limits<double>::quiet_NaN() : av);
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

            if (!double_matches_exact(blend_eq.element(lane), eq ? 1.0 : 0.0) ||
                !double_matches_exact(blend_lt.element(lane), lt ? 1.0 : 0.0) ||
                !double_matches_exact(blend_le.element(lane), le ? 1.0 : 0.0) ||
                !double_matches_exact(blend_gt.element(lane), gt ? 1.0 : 0.0) ||
                !double_matches_exact(blend_ge.element(lane), ge ? 1.0 : 0.0) ||
                !double_matches_exact(blend_nan.element(lane), is_nan ? 1.0 : 0.0) ||
                !double_matches_exact(blend_eq_nan_self.element(lane), is_nan ? 0.0 : 1.0)) {
                harness.add_result(test_name, false, "blend(compare/isnan) mismatch, lane " + std::to_string(lane));
                return false;
            }

            const double t = if_true.element(lane);
            const double f = if_false.element(lane);
            if (!double_matches_fallback(if_eq.element(lane), eq ? t : f) ||
                !double_matches_fallback(if_lt.element(lane), lt ? t : f) ||
                !double_matches_fallback(if_le.element(lane), le ? t : f) ||
                !double_matches_fallback(if_gt.element(lane), gt ? t : f) ||
                !double_matches_fallback(if_ge.element(lane), ge ? t : f) ||
                !double_matches_fallback(if_is_nan.element(lane), is_nan ? t : f)) {
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
            partial_b.set_element(0, std::nextafter(partial_a.element(0), std::numeric_limits<double>::infinity()));
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
bool run_float64_suite_for_type(const char* type_name, CpuInformation cpu, TestHarness& harness) {
    return run_float64_binary_test_for_type<SimdType>(type_name, "addition", ArithmeticOp::add, cpu, harness) &&
           run_float64_binary_test_for_type<SimdType>(type_name, "subtraction", ArithmeticOp::sub, cpu, harness) &&
           run_float64_binary_test_for_type<SimdType>(type_name, "multiplication", ArithmeticOp::mul, cpu, harness) &&
           run_float64_binary_test_for_type<SimdType>(type_name, "division", ArithmeticOp::div, cpu, harness) &&
           run_float64_compare_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_float64_helper_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_float64_math_test_for_type<SimdType>(type_name, cpu, harness);
}

} // namespace

void run_float64_arithmetic_tests(TestHarness& harness) {
    std::cout << "\n===== Float64 Arithmetic Tests =====\n";
    if (harness.should_halt()) {
        return;
    }
    CpuInformation cpu{};
#define MT_RUN_OR_HALT(expr) do { if (!(expr)) { return; } } while (false)
    MT_RUN_OR_HALT(run_float64_suite_for_type<FallbackFloat64>("Fallback", cpu, harness));
#if MT_SIMD_ARCH_X64 || (MT_SIMD_ARCH_WASM && defined(__wasm_simd128__))
    MT_RUN_OR_HALT(run_float64_suite_for_type<Simd128Float64>("Simd128", cpu, harness));
#if MT_SIMD_ALLOW_LEVEL3_TYPES
    MT_RUN_OR_HALT(run_float64_suite_for_type<Simd256Float64>("Simd256", cpu, harness));
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
    MT_RUN_OR_HALT(run_float64_suite_for_type<Simd512Float64>("Simd512", cpu, harness));
#endif
#endif
#undef MT_RUN_OR_HALT
    std::cout << "==================================\n";
}





