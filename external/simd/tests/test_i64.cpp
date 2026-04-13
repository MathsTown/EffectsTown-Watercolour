/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Int64 SIMD tests.
Uses independent scalar/two's-complement reference calculations.
*********************************************************************************************************/

#include "test_i64.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string_view>
#include <type_traits>

#include "../include/simd-cpuid.h"
#include "../include/simd-int64.h"

using namespace mt;

namespace {

enum class ArithmeticOp {
    add,
    sub,
    mul,
    div
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

struct I64Pair {
    int64_t lhs;
    int64_t rhs;
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

int64_t sanitize_divisor(int64_t lhs, int64_t rhs) {
    if (rhs == 0) {
        return 1;
    }
    if (lhs == std::numeric_limits<int64_t>::min() && rhs == -1) {
        return 1;
    }
    return rhs;
}

int64_t wrap_add(int64_t lhs, int64_t rhs) {
    const uint64_t lhs_bits = std::bit_cast<uint64_t>(lhs);
    const uint64_t rhs_bits = std::bit_cast<uint64_t>(rhs);
    return std::bit_cast<int64_t>(lhs_bits + rhs_bits);
}

int64_t wrap_sub(int64_t lhs, int64_t rhs) {
    const uint64_t lhs_bits = std::bit_cast<uint64_t>(lhs);
    const uint64_t rhs_bits = std::bit_cast<uint64_t>(rhs);
    return std::bit_cast<int64_t>(lhs_bits - rhs_bits);
}

int64_t wrap_mul(int64_t lhs, int64_t rhs) {
    const uint64_t lhs_bits = std::bit_cast<uint64_t>(lhs);
    const uint64_t rhs_bits = std::bit_cast<uint64_t>(rhs);
    return std::bit_cast<int64_t>(lhs_bits * rhs_bits);
}

int64_t scalar_abs(int64_t value) {
    const uint64_t ux = std::bit_cast<uint64_t>(value);
    const uint64_t sign = ux >> 63;
    const uint64_t mask = 0ull - sign;
    const uint64_t mag = (ux ^ mask) + sign;
    return std::bit_cast<int64_t>(mag);
}

int64_t apply_scalar_binary(int64_t lhs, int64_t rhs, ArithmeticOp op) {
    if (op == ArithmeticOp::add) {
        return wrap_add(lhs, rhs);
    }
    if (op == ArithmeticOp::sub) {
        return wrap_sub(lhs, rhs);
    }
    if (op == ArithmeticOp::mul) {
        return wrap_mul(lhs, rhs);
    }
    return lhs / rhs;
}

template <typename SimdType>
void apply_simd_op_with_path(
    const SimdType& a,
    const SimdType& b,
    int64_t scalar,
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

int64_t apply_scalar_op_with_path(
    int64_t lhs,
    int64_t rhs,
    int64_t scalar,
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

int64_t scalar_for_op(ArithmeticOp op) {
    if (op == ArithmeticOp::add) { return 73; }
    if (op == ArithmeticOp::sub) { return -29; }
    if (op == ArithmeticOp::mul) { return -5; }
    return 7;
}

const I64Pair* edge_pairs_for_op(ArithmeticOp op) {
    static constexpr I64Pair kAddPairs[] = {
        {0ll, 0ll},
        {1ll, -1ll},
        {std::numeric_limits<int64_t>::max(), 0ll},
        {std::numeric_limits<int64_t>::min(), 0ll},
        {std::numeric_limits<int64_t>::max(), -1ll},
        {std::numeric_limits<int64_t>::min(), 1ll},
        {0x7fff000000000001ll, -0x0000000010000000ll},
        {-0x7000000000000000ll, 0x1000000000000000ll},
        {0x1234567800000000ll, -0x0234567800000000ll}
    };
    static constexpr I64Pair kSubPairs[] = {
        {0ll, 0ll},
        {1ll, 1ll},
        {-1ll, -1ll},
        {std::numeric_limits<int64_t>::max(), 0ll},
        {std::numeric_limits<int64_t>::min(), 0ll},
        {std::numeric_limits<int64_t>::max(), 1ll},
        {std::numeric_limits<int64_t>::min(), -1ll},
        {0x7fff000000000001ll, 0x0000000010000000ll},
        {-0x7000000000000000ll, -0x1000000000000000ll}
    };
    static constexpr I64Pair kMulPairs[] = {
        {0ll, 0ll},
        {1ll, -1ll},
        {-1ll, -1ll},
        {1000000ll, 1000000ll},
        {-1000000ll, 1000000ll},
        {0x0000000100000000ll, 2ll},
        {-0x0000000100000000ll, 2ll},
        {std::numeric_limits<int64_t>::max(), 0ll},
        {std::numeric_limits<int64_t>::min(), 0ll}
    };
    static constexpr I64Pair kDivPairs[] = {
        {0ll, 1ll},
        {1ll, 1ll},
        {-1ll, 1ll},
        {std::numeric_limits<int64_t>::max(), 1ll},
        {std::numeric_limits<int64_t>::min(), 1ll},
        {std::numeric_limits<int64_t>::max(), -1ll},
        {std::numeric_limits<int64_t>::min(), 2ll},
        {0x7fff000000000001ll, 3ll},
        {-0x7000000000000000ll, -7ll}
    };

    if (op == ArithmeticOp::add) {
        return kAddPairs;
    }
    if (op == ArithmeticOp::sub) {
        return kSubPairs;
    }
    if (op == ArithmeticOp::mul) {
        return kMulPairs;
    }
    return kDivPairs;
}

constexpr int edge_pair_count() {
    return 9;
}

enum class BitwiseOp {
    bit_and,
    bit_or,
    bit_xor
};

constexpr BitwiseOp kBitwiseOps[] = {
    BitwiseOp::bit_and,
    BitwiseOp::bit_or,
    BitwiseOp::bit_xor
};

std::string_view bitwise_op_name(BitwiseOp op) {
    if (op == BitwiseOp::bit_and) {
        return "and";
    }
    if (op == BitwiseOp::bit_or) {
        return "or";
    }
    return "xor";
}

template <typename SimdType>
void apply_bitwise_binary(const SimdType& a, const SimdType& b, BitwiseOp op, SimdType& out) {
    out = a;
    if (op == BitwiseOp::bit_and) { out &= b; return; }
    if (op == BitwiseOp::bit_or) { out |= b; return; }
    out ^= b;
}

template <typename SimdType>
void apply_bitwise_compound(const SimdType& a, const SimdType& b, BitwiseOp op, SimdType& out) {
    out = a;
    if (op == BitwiseOp::bit_and) { out &= b; }
    else if (op == BitwiseOp::bit_or) { out |= b; }
    else { out ^= b; }
}

int64_t apply_scalar_bitwise_binary(int64_t lhs, int64_t rhs, BitwiseOp op) {
    if (op == BitwiseOp::bit_and) { return lhs & rhs; }
    if (op == BitwiseOp::bit_or) { return lhs | rhs; }
    return lhs ^ rhs;
}

int64_t apply_scalar_bitwise_compound(int64_t lhs, int64_t rhs, BitwiseOp op) {
    if (op == BitwiseOp::bit_and) { return lhs & rhs; }
    if (op == BitwiseOp::bit_or) { return lhs | rhs; }
    return lhs ^ rhs;
}

template <typename SimdType>
bool run_int64_bitwise_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Int64 bitwise";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());

    for (const BitwiseOp op : kBitwiseOps) {
        for (int iteration = 0; iteration < 800; ++iteration) {
            alignas(SimdType) SimdType a{};
            alignas(SimdType) SimdType b{};
            for (int lane = 0; lane < lanes; ++lane) {
                a.set_element(lane, dist(rng));
                b.set_element(lane, dist(rng));
            }

            alignas(SimdType) SimdType binary_result{};
            alignas(SimdType) SimdType compound_result{};
            apply_bitwise_binary(a, b, op, binary_result);
            apply_bitwise_compound(a, b, op, compound_result);
            for (int lane = 0; lane < lanes; ++lane) {
                const int64_t lhs = a.element(lane);
                const int64_t rhs = b.element(lane);
                const int64_t expected_binary = apply_scalar_bitwise_binary(lhs, rhs, op);
                const int64_t expected_compound = apply_scalar_bitwise_compound(lhs, rhs, op);
                if (binary_result.element(lane) != expected_binary) {
                    harness.add_result(
                        test_name,
                        false,
                        std::string(bitwise_op_name(op)) + " mismatch (binary), lane " + std::to_string(lane));
                    return false;
                }
                if (compound_result.element(lane) != expected_compound) {
                    harness.add_result(
                        test_name,
                        false,
                        std::string(bitwise_op_name(op)) + " mismatch (compound), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    const int64_t edges[] = {
        0ll,
        1ll,
        -1ll,
        std::numeric_limits<int64_t>::max(),
        std::numeric_limits<int64_t>::min(),
        static_cast<int64_t>(0x5555555555555555ull),
        static_cast<int64_t>(0xAAAAAAAAAAAAAAAAull),
        static_cast<int64_t>(0x123456789abcdef0ull),
        static_cast<int64_t>(0x876543210fedcba9ull)
    };
    constexpr int edge_count = static_cast<int>(sizeof(edges) / sizeof(edges[0]));

    for (const BitwiseOp op : kBitwiseOps) {
        for (int base = 0; base < edge_count; ++base) {
            alignas(SimdType) SimdType a{};
            alignas(SimdType) SimdType b{};
            for (int lane = 0; lane < lanes; ++lane) {
                a.set_element(lane, edges[(base + lane) % edge_count]);
                b.set_element(lane, edges[(base * 5 + lane + 1) % edge_count]);
            }

            alignas(SimdType) SimdType binary_result{};
            alignas(SimdType) SimdType compound_result{};
            apply_bitwise_binary(a, b, op, binary_result);
            apply_bitwise_compound(a, b, op, compound_result);
            for (int lane = 0; lane < lanes; ++lane) {
                const int64_t lhs = a.element(lane);
                const int64_t rhs = b.element(lane);
                const int64_t expected_binary = apply_scalar_bitwise_binary(lhs, rhs, op);
                const int64_t expected_compound = apply_scalar_bitwise_compound(lhs, rhs, op);
                if (binary_result.element(lane) != expected_binary || compound_result.element(lane) != expected_compound) {
                    harness.add_result(
                        test_name,
                        false,
                        std::string(bitwise_op_name(op)) + " edge mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    for (int base = 0; base < edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        for (int lane = 0; lane < lanes; ++lane) {
            a.set_element(lane, edges[(base + lane) % edge_count]);
        }
        alignas(SimdType) SimdType result = ~a;
        for (int lane = 0; lane < lanes; ++lane) {
            const int64_t lhs = a.element(lane);
            const int64_t expected = ~lhs;
            if (result.element(lane) != expected) {
                harness.add_result(test_name, false, "not mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    harness.add_result(test_name, true, "& | ^ ~ with binary and compound paths matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_int64_shift_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Int64 shifts";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    constexpr int shift_counts[] = {0, 1, 7, 15, 31, 47, 63};
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<int64_t> right_dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());

    for (const int shift : shift_counts) {
        const int64_t max_left = (shift == 0) ? std::numeric_limits<int64_t>::max() : (std::numeric_limits<int64_t>::max() >> shift);
        std::uniform_int_distribution<int64_t> left_dist(0ll, max_left);

        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType left_values{};
            alignas(SimdType) SimdType right_values{};
            for (int lane = 0; lane < lanes; ++lane) {
                left_values.set_element(lane, left_dist(rng));
                right_values.set_element(lane, right_dist(rng));
            }

            alignas(SimdType) SimdType left_result = left_values << shift;
            alignas(SimdType) SimdType right_result = right_values >> shift;
            for (int lane = 0; lane < lanes; ++lane) {
                const int64_t lhs = left_values.element(lane);
                const int64_t rhs = right_values.element(lane);
                const int64_t expected_left = lhs << shift;
                const int64_t expected_right = rhs >> shift;
                if (left_result.element(lane) != expected_left) {
                    harness.add_result(test_name, false, "left shift mismatch, shift=" + std::to_string(shift) + ", lane " + std::to_string(lane));
                    return false;
                }
                if (right_result.element(lane) != expected_right) {
                    harness.add_result(test_name, false, "right shift mismatch, shift=" + std::to_string(shift) + ", lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    harness.add_result(test_name, true, "<< and >> matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_int64_minmax_abs_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Int64 min/max/abs";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
    std::uniform_int_distribution<int64_t> abs_dist(std::numeric_limits<int64_t>::min() + 1, std::numeric_limits<int64_t>::max());

    for (int iteration = 0; iteration < 800; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType abs_input{};
        for (int lane = 0; lane < lanes; ++lane) {
            a.set_element(lane, dist(rng));
            b.set_element(lane, dist(rng));
            abs_input.set_element(lane, abs_dist(rng));
        }

        alignas(SimdType) SimdType min_result = min(a, b);
        alignas(SimdType) SimdType max_result = max(a, b);
        alignas(SimdType) SimdType abs_result = abs(abs_input);

        for (int lane = 0; lane < lanes; ++lane) {
            const int64_t lhs = a.element(lane);
            const int64_t rhs = b.element(lane);
            const int64_t abs_val = abs_input.element(lane);
            const int64_t expected_min = std::min(lhs, rhs);
            const int64_t expected_max = std::max(lhs, rhs);
            const int64_t expected_abs = scalar_abs(abs_val);
            if (min_result.element(lane) != expected_min) {
                harness.add_result(test_name, false, "min mismatch, lane " + std::to_string(lane));
                return false;
            }
            if (max_result.element(lane) != expected_max) {
                harness.add_result(test_name, false, "max mismatch, lane " + std::to_string(lane));
                return false;
            }
            if (abs_result.element(lane) != expected_abs) {
                harness.add_result(test_name, false, "abs mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    const int64_t edges[] = {
        0ll,
        1ll,
        -1ll,
        std::numeric_limits<int64_t>::max(),
        std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max() - 1,
        std::numeric_limits<int64_t>::min() + 1,
        0x7fff000000000001ll,
        -0x7000000000000000ll
    };
    constexpr int edge_count = static_cast<int>(sizeof(edges) / sizeof(edges[0]));

    for (int base = 0; base < edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType abs_input{};
        for (int lane = 0; lane < lanes; ++lane) {
            a.set_element(lane, edges[(base + lane) % edge_count]);
            b.set_element(lane, edges[(base * 5 + lane + 1) % edge_count]);
            int64_t abs_lane = edges[(base * 3 + lane + 2) % edge_count];
            if (abs_lane == std::numeric_limits<int64_t>::min()) {
                abs_lane = std::numeric_limits<int64_t>::min() + 1;
            }
            abs_input.set_element(lane, abs_lane);
        }

        alignas(SimdType) SimdType min_result = min(a, b);
        alignas(SimdType) SimdType max_result = max(a, b);
        alignas(SimdType) SimdType abs_result = abs(abs_input);

        for (int lane = 0; lane < lanes; ++lane) {
            const int64_t lhs = a.element(lane);
            const int64_t rhs = b.element(lane);
            const int64_t abs_val = abs_input.element(lane);
            const int64_t expected_min = std::min(lhs, rhs);
            const int64_t expected_max = std::max(lhs, rhs);
            const int64_t expected_abs = scalar_abs(abs_val);
            if (min_result.element(lane) != expected_min || max_result.element(lane) != expected_max || abs_result.element(lane) != expected_abs) {
                harness.add_result(test_name, false, "edge mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    // Only run abs(INT64_MIN) checks on code paths with defined vector abs support.
    constexpr bool has_defined_abs_min =
        (SimdType::number_of_elements() == 8 && mt::environment::compiler_has_avx512f) ||
        ((SimdType::number_of_elements() == 4 || SimdType::number_of_elements() == 2) &&
            mt::environment::compiler_has_avx512vl && mt::environment::compiler_has_avx512f);
    if constexpr (has_defined_abs_min) {
        alignas(SimdType) SimdType abs_input{};
        for (int lane = 0; lane < lanes; ++lane) {
            abs_input.set_element(lane, std::numeric_limits<int64_t>::min());
        }
        alignas(SimdType) SimdType abs_result = abs(abs_input);
        for (int lane = 0; lane < lanes; ++lane) {
            if (abs_result.element(lane) != std::numeric_limits<int64_t>::min()) {
                harness.add_result(test_name, false, "abs(INT64_MIN) mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    harness.add_result(test_name, true, "min/max/abs matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_int64_compare_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Int64 compare/blend";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());

    for (int iteration = 0; iteration < 700; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType if_true{};
        alignas(SimdType) SimdType if_false{};
        alignas(SimdType) SimdType zero(0);
        alignas(SimdType) SimdType ones(-1);
        alignas(SimdType) SimdType b_diff{};

        for (int lane = 0; lane < lanes; ++lane) {
            const int64_t av = dist(rng);
            const int64_t bv = dist(rng);
            a.set_element(lane, av);
            b.set_element(lane, bv);
            if_true.set_element(lane, dist(rng));
            if_false.set_element(lane, dist(rng));
            b_diff.set_element(lane, av ^ 1ll);
        }

        const auto mask_eq = compare_equal(a, b);
        const auto mask_lt = compare_less(a, b);
        const auto mask_le = compare_less_equal(a, b);
        const auto mask_gt = compare_greater(a, b);
        const auto mask_ge = compare_greater_equal(a, b);

        const SimdType blend_eq = blend(zero, ones, mask_eq);
        const SimdType blend_lt = blend(zero, ones, mask_lt);
        const SimdType blend_le = blend(zero, ones, mask_le);
        const SimdType blend_gt = blend(zero, ones, mask_gt);
        const SimdType blend_ge = blend(zero, ones, mask_ge);

        const SimdType if_eq = if_equal(a, b, if_true, if_false);
        const SimdType if_lt = if_less(a, b, if_true, if_false);
        const SimdType if_le = if_less_equal(a, b, if_true, if_false);
        const SimdType if_gt = if_greater(a, b, if_true, if_false);
        const SimdType if_ge = if_greater_equal(a, b, if_true, if_false);

        for (int lane = 0; lane < lanes; ++lane) {
            const bool eq = a.element(lane) == b.element(lane);
            const bool lt = a.element(lane) < b.element(lane);
            const bool le = a.element(lane) <= b.element(lane);
            const bool gt = a.element(lane) > b.element(lane);
            const bool ge = a.element(lane) >= b.element(lane);

            if (blend_eq.element(lane) != (eq ? -1ll : 0ll) ||
                blend_lt.element(lane) != (lt ? -1ll : 0ll) ||
                blend_le.element(lane) != (le ? -1ll : 0ll) ||
                blend_gt.element(lane) != (gt ? -1ll : 0ll) ||
                blend_ge.element(lane) != (ge ? -1ll : 0ll)) {
                harness.add_result(test_name, false, "blend(compare) mismatch, lane " + std::to_string(lane));
                return false;
            }

            const int64_t t = if_true.element(lane);
            const int64_t f = if_false.element(lane);
            if (if_eq.element(lane) != (eq ? t : f) ||
                if_lt.element(lane) != (lt ? t : f) ||
                if_le.element(lane) != (le ? t : f) ||
                if_gt.element(lane) != (gt ? t : f) ||
                if_ge.element(lane) != (ge ? t : f)) {
                harness.add_result(test_name, false, "if_* mismatch, lane " + std::to_string(lane));
                return false;
            }
        }

        const auto all_true_mask = compare_equal(a, a);
        const auto all_false_mask = compare_equal(a, b_diff);
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
            partial_b.set_element(0, partial_a.element(0) ^ 1ll);
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
bool run_int64_metadata_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Int64 metadata";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    if (SimdType::size_of_element() != static_cast<int>(sizeof(int64_t))) {
        harness.add_result(test_name, false, "size_of_element() mismatch");
        return false;
    }

    const SimdType sequential = SimdType::make_sequential(-41ll);
    for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
        const int64_t expected = -41ll + static_cast<int64_t>(lane);
        if (sequential.element(lane) != expected) {
            harness.add_result(test_name, false, "make_sequential mismatch, lane " + std::to_string(lane));
            return false;
        }
    }

    {
        const int64_t start = std::numeric_limits<int64_t>::max() - (SimdType::number_of_elements() - 1);
        const SimdType wrapped = SimdType::make_sequential(start);
        for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
            const uint64_t expected_u = static_cast<uint64_t>(start) + static_cast<uint64_t>(lane);
            const int64_t expected = static_cast<int64_t>(expected_u);
            if (wrapped.element(lane) != expected) {
                harness.add_result(test_name, false, "make_sequential wrap mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    const SimdType set_value = SimdType::make_set1(57ll);
    for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
        if (set_value.element(lane) != 57ll) {
            harness.add_result(test_name, false, "make_set1 mismatch, lane " + std::to_string(lane));
            return false;
        }
    }

    harness.add_result(test_name, true, "make_sequential, make_set1, and size_of_element() matched");
    return true;
}

template <typename SimdType>
bool run_int64_negation_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " Int64 unary negation";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());

    for (int iteration = 0; iteration < 700; ++iteration) {
        alignas(SimdType) SimdType input{};
        for (int lane = 0; lane < lanes; ++lane) {
            input.set_element(lane, dist(rng));
        }
        const SimdType result = -input;
        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t expected_u = 0ull - static_cast<uint64_t>(input.element(lane));
            const int64_t expected = static_cast<int64_t>(expected_u);
            if (result.element(lane) != expected) {
                harness.add_result(test_name, false, "random mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    constexpr int64_t edges[] = {
        0ll,
        1ll,
        -1ll,
        std::numeric_limits<int64_t>::max(),
        std::numeric_limits<int64_t>::min(),
        0x7000000000000001ll,
        -0x7000000000000000ll
    };
    constexpr int edge_count = static_cast<int>(sizeof(edges) / sizeof(edges[0]));
    for (int base = 0; base < edge_count; ++base) {
        alignas(SimdType) SimdType input{};
        for (int lane = 0; lane < lanes; ++lane) {
            input.set_element(lane, edges[(base + lane) % edge_count]);
        }
        const SimdType result = -input;
        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t expected_u = 0ull - static_cast<uint64_t>(input.element(lane));
            const int64_t expected = static_cast<int64_t>(expected_u);
            if (result.element(lane) != expected) {
                harness.add_result(test_name, false, "edge mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    harness.add_result(test_name, true, "unary negation matched two's-complement reference");
    return true;
}

template <typename SimdType>
bool run_int64_binary_test_for_type(
    const std::string& type_name,
    const std::string& op_name,
    ArithmeticOp op,
    CpuInformation cpu,
    TestHarness& harness) {
    const std::string test_name = type_name + " Int64 " + op_name;

    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<int64_t> add_sub_dist(-1000000000000ll, 1000000000000ll);
    std::uniform_int_distribution<int64_t> mul_dist(-1000000ll, 1000000ll);
    std::uniform_int_distribution<int64_t> div_dist(-1000000000000ll, 1000000000000ll);

    for (int iteration = 0; iteration < 1200; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};

        for (int lane = 0; lane < lanes; ++lane) {
            int64_t lhs = 0;
            int64_t rhs = 1;
            if (op == ArithmeticOp::mul) {
                lhs = mul_dist(rng);
                rhs = mul_dist(rng);
            } else if (op == ArithmeticOp::div) {
                lhs = div_dist(rng);
                rhs = sanitize_divisor(lhs, div_dist(rng));
            } else {
                lhs = add_sub_dist(rng);
                rhs = add_sub_dist(rng);
            }
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const int64_t scalar = scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const int64_t lhs = a.element(lane);
                const int64_t rhs = b.element(lane);
                const int64_t expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const int64_t actual = result.element(lane);
                if (actual != expected) {
                    harness.add_result(
                        test_name,
                        false,
                        "Random mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    const I64Pair* pairs = edge_pairs_for_op(op);
    for (int base = 0; base < edge_pair_count(); ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const I64Pair pair = pairs[(base + lane) % edge_pair_count()];
            int64_t rhs = pair.rhs;
            if (op == ArithmeticOp::div) {
                rhs = sanitize_divisor(pair.lhs, rhs);
            }
            a.set_element(lane, pair.lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const int64_t scalar = scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const int64_t lhs = a.element(lane);
                const int64_t rhs = b.element(lane);
                const int64_t expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const int64_t actual = result.element(lane);
                if (actual != expected) {
                    harness.add_result(
                        test_name,
                        false,
                        "Edge mismatch (" + std::string(path_name(path)) + "), lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    harness.add_result(test_name, true, "Random + directed edge cases matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_int64_suite_for_type(const char* type_name, CpuInformation cpu, TestHarness& harness) {
    return run_int64_metadata_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_int64_binary_test_for_type<SimdType>(type_name, "addition", ArithmeticOp::add, cpu, harness) &&
           run_int64_binary_test_for_type<SimdType>(type_name, "subtraction", ArithmeticOp::sub, cpu, harness) &&
           run_int64_binary_test_for_type<SimdType>(type_name, "multiplication", ArithmeticOp::mul, cpu, harness) &&
           run_int64_binary_test_for_type<SimdType>(type_name, "division", ArithmeticOp::div, cpu, harness) &&
           run_int64_negation_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_int64_minmax_abs_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_int64_compare_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_int64_bitwise_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_int64_shift_test_for_type<SimdType>(type_name, cpu, harness);
}

} // namespace

void run_int64_arithmetic_tests(TestHarness& harness) {
    std::cout << "\n===== Int64 Arithmetic Tests =====\n";
    if (harness.should_halt()) {
        return;
    }
    CpuInformation cpu{};
#define MT_RUN_OR_HALT(expr) do { if (!(expr)) { return; } } while (false)
    MT_RUN_OR_HALT(run_int64_suite_for_type<FallbackInt64>("Fallback", cpu, harness));
#if MT_SIMD_ARCH_X64 || (MT_SIMD_ARCH_WASM && defined(__wasm_simd128__))
    MT_RUN_OR_HALT(run_int64_suite_for_type<Simd128Int64>("Simd128", cpu, harness));
#if MT_SIMD_ALLOW_LEVEL3_TYPES
    MT_RUN_OR_HALT(run_int64_suite_for_type<Simd256Int64>("Simd256", cpu, harness));
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
    MT_RUN_OR_HALT(run_int64_suite_for_type<Simd512Int64>("Simd512", cpu, harness));
#endif
#endif
#undef MT_RUN_OR_HALT
    std::cout << "==================================\n";
}
