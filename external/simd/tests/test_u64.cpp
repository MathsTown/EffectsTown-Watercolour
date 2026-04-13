/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
UInt64 SIMD tests.
Uses independent scalar/std::bit reference calculations.
*********************************************************************************************************/

#include "test_u64.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string_view>
#include <type_traits>

#include "../include/simd-cpuid.h"
#include "../include/simd-concepts.h"
#include "../include/simd-uint64.h"

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

struct U64Pair {
    uint64_t lhs;
    uint64_t rhs;
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

uint64_t sanitize_divisor(uint64_t rhs) {
    return rhs == 0ull ? 1ull : rhs;
}

uint64_t apply_scalar_binary(uint64_t lhs, uint64_t rhs, ArithmeticOp op) {
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
void set_lane(SimdType& value, int lane, uint64_t lane_value) {
    value.set_element(lane, lane_value);
}

template <typename SimdType>
void apply_simd_op_with_path(
    const SimdType& a,
    const SimdType& b,
    uint64_t scalar,
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

uint64_t apply_scalar_op_with_path(
    uint64_t lhs,
    uint64_t rhs,
    uint64_t scalar,
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

uint64_t scalar_for_op(ArithmeticOp op) {
    if (op == ArithmeticOp::add) { return 37ull; }
    if (op == ArithmeticOp::sub) { return 19ull; }
    if (op == ArithmeticOp::mul) { return 3ull; }
    return 5ull;
}

const U64Pair* edge_pairs_for_op(ArithmeticOp op) {
    static constexpr U64Pair kAddPairs[] = {
        {0ull, 0ull},
        {1ull, 1ull},
        {std::numeric_limits<uint64_t>::max(), 1ull},             // overflow wrap
        {std::numeric_limits<uint64_t>::max(), 2ull},             // overflow wrap
        {0xFFFFFFFF00000000ull, 0x0000000100000000ull},           // high word carry
        {0xF000000000000001ull, 0x1000000000000000ull},
        {0x8000000000000000ull, 0x8000000000000000ull},
        {1234567890123456789ull, 10000000000000000000ull},
        {0x7fff000000000001ull, 0x0100000000000000ull}
    };
    static constexpr U64Pair kSubPairs[] = {
        {0ull, 0ull},
        {1ull, 1ull},
        {0ull, 1ull},                                              // underflow wrap
        {0ull, std::numeric_limits<uint64_t>::max()},             // underflow wrap
        {0x0000000100000000ull, 0x0000000200000000ull},           // high word borrow
        {0x1000000000000000ull, 0xF000000000000001ull},
        {0x8000000000000000ull, 0x8000000000000001ull},
        {1234567890123456789ull, 10000000000000000000ull},
        {0x7fff000000000001ull, 0x0100000000000000ull}
    };
    static constexpr U64Pair kMulPairs[] = {
        {0ull, 0ull},
        {1ull, 1ull},
        {std::numeric_limits<uint64_t>::max(), 2ull},             // overflow wrap
        {0x8000000000000000ull, 2ull},                            // wraps to 0
        {0xFFFFFFFF00000000ull, 0x0000000100000000ull},
        {0x100000000ull, 0x100000000ull},                         // wraps to 0
        {0xF000000000000001ull, 0x10ull},
        {123456789ull, 987654321ull},
        {0x7fff000000000001ull, 0x0000000000000011ull}
    };
    static constexpr U64Pair kDivPairs[] = {
        {0ull, 1ull},
        {1ull, 1ull},
        {std::numeric_limits<uint64_t>::max(), 1ull},
        {std::numeric_limits<uint64_t>::max(), 2ull},
        {0xFFFFFFFF00000000ull, 0x0000000100000000ull},
        {0xF000000000000001ull, 0x1000000000000000ull},
        {0x8000000000000000ull, 0x100000000ull},
        {1234567890123456789ull, 3ull},
        {10000000000000000000ull, 17ull}
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

uint64_t apply_scalar_bitwise_binary(uint64_t lhs, uint64_t rhs, BitwiseOp op) {
    if (op == BitwiseOp::bit_and) { return lhs & rhs; }
    if (op == BitwiseOp::bit_or) { return lhs | rhs; }
    return lhs ^ rhs;
}

uint64_t apply_scalar_bitwise_compound(uint64_t lhs, uint64_t rhs, BitwiseOp op) {
    if (op == BitwiseOp::bit_and) { return lhs & rhs; }
    if (op == BitwiseOp::bit_or) { return lhs | rhs; }
    return lhs ^ rhs;
}

template <typename SimdType>
bool run_uint64_bitwise_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt64 bitwise";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<uint64_t> dist(0ull, std::numeric_limits<uint64_t>::max());

    for (const BitwiseOp op : kBitwiseOps) {
        for (int iteration = 0; iteration < 800; ++iteration) {
            alignas(SimdType) SimdType a{};
            alignas(SimdType) SimdType b{};
            for (int lane = 0; lane < lanes; ++lane) {
                set_lane(a, lane, dist(rng));
                set_lane(b, lane, dist(rng));
            }

            alignas(SimdType) SimdType binary_result{};
            alignas(SimdType) SimdType compound_result{};
            apply_bitwise_binary(a, b, op, binary_result);
            apply_bitwise_compound(a, b, op, compound_result);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint64_t lhs = a.element(lane);
                const uint64_t rhs = b.element(lane);
                const uint64_t expected_binary = apply_scalar_bitwise_binary(lhs, rhs, op);
                const uint64_t expected_compound = apply_scalar_bitwise_compound(lhs, rhs, op);
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

    const uint64_t edges[] = {
        0ull,
        1ull,
        std::numeric_limits<uint64_t>::max(),
        0x5555555555555555ull,
        0xAAAAAAAAAAAAAAAAull,
        0x123456789abcdef0ull,
        0x876543210fedcba9ull,
        0x8000000000000000ull,
        0x7FFFFFFFFFFFFFFFull
    };
    constexpr int edge_count = static_cast<int>(sizeof(edges) / sizeof(edges[0]));

    for (const BitwiseOp op : kBitwiseOps) {
        for (int base = 0; base < edge_count; ++base) {
            alignas(SimdType) SimdType a{};
            alignas(SimdType) SimdType b{};
            for (int lane = 0; lane < lanes; ++lane) {
                set_lane(a, lane, edges[(base + lane) % edge_count]);
                set_lane(b, lane, edges[(base * 5 + lane + 1) % edge_count]);
            }

            alignas(SimdType) SimdType binary_result{};
            alignas(SimdType) SimdType compound_result{};
            apply_bitwise_binary(a, b, op, binary_result);
            apply_bitwise_compound(a, b, op, compound_result);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint64_t lhs = a.element(lane);
                const uint64_t rhs = b.element(lane);
                const uint64_t expected_binary = apply_scalar_bitwise_binary(lhs, rhs, op);
                const uint64_t expected_compound = apply_scalar_bitwise_compound(lhs, rhs, op);
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
            set_lane(a, lane, edges[(base + lane) % edge_count]);
        }
        alignas(SimdType) SimdType result = ~a;
        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t lhs = a.element(lane);
            const uint64_t expected = ~lhs;
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
bool run_uint64_shift_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt64 shifts";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    constexpr int shift_counts[] = {0, 1, 7, 15, 31, 47, 63};
    constexpr int rotate_counts[] = {0, 1, 5, 13, 31, 63, 64, 65, -1, -65};
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<uint64_t> dist(0ull, std::numeric_limits<uint64_t>::max());

    for (const int shift : shift_counts) {
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType values{};
            for (int lane = 0; lane < lanes; ++lane) {
                set_lane(values, lane, dist(rng));
            }

            alignas(SimdType) SimdType left_result = values << shift;
            alignas(SimdType) SimdType right_result = values >> shift;
            for (int lane = 0; lane < lanes; ++lane) {
                const uint64_t v = values.element(lane);
                const uint64_t expected_left = v << shift;
                const uint64_t expected_right = v >> shift;
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

    for (const int bits : rotate_counts) {
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType values{};
            for (int lane = 0; lane < lanes; ++lane) {
                set_lane(values, lane, dist(rng));
            }

            alignas(SimdType) SimdType left_rot = rotl(values, bits);
            alignas(SimdType) SimdType right_rot = rotr(values, bits);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint64_t v = values.element(lane);
                const uint64_t expected_l = std::rotl(v, bits);
                const uint64_t expected_r = std::rotr(v, bits);
                if (left_rot.element(lane) != expected_l) {
                    harness.add_result(test_name, false, "rotl mismatch, bits=" + std::to_string(bits) + ", lane " + std::to_string(lane));
                    return false;
                }
                if (right_rot.element(lane) != expected_r) {
                    harness.add_result(test_name, false, "rotr mismatch, bits=" + std::to_string(bits) + ", lane " + std::to_string(lane));
                    return false;
                }
            }
        }
    }

    harness.add_result(test_name, true, "<< >> rotl rotr matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_uint64_minmax_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt64 min/max";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<uint64_t> dist(0ull, std::numeric_limits<uint64_t>::max());

    for (int iteration = 0; iteration < 800; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            set_lane(a, lane, dist(rng));
            set_lane(b, lane, dist(rng));
        }

        alignas(SimdType) SimdType min_result = min(a, b);
        alignas(SimdType) SimdType max_result = max(a, b);

        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t lhs = a.element(lane);
            const uint64_t rhs = b.element(lane);
            const uint64_t expected_min = std::min(lhs, rhs);
            const uint64_t expected_max = std::max(lhs, rhs);
            if (min_result.element(lane) != expected_min) {
                harness.add_result(test_name, false, "min mismatch, lane " + std::to_string(lane));
                return false;
            }
            if (max_result.element(lane) != expected_max) {
                harness.add_result(test_name, false, "max mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    const uint64_t edges[] = {
        0ull,
        1ull,
        std::numeric_limits<uint64_t>::max(),
        0x8000000000000000ull,
        0x7FFFFFFFFFFFFFFFull,
        0x5555555555555555ull,
        0xAAAAAAAAAAAAAAAAull,
        1234567890123456789ull,
        10000000000000000000ull
    };
    constexpr int edge_count = static_cast<int>(sizeof(edges) / sizeof(edges[0]));

    for (int base = 0; base < edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            set_lane(a, lane, edges[(base + lane) % edge_count]);
            set_lane(b, lane, edges[(base * 5 + lane + 1) % edge_count]);
        }

        alignas(SimdType) SimdType min_result = min(a, b);
        alignas(SimdType) SimdType max_result = max(a, b);
        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t lhs = a.element(lane);
            const uint64_t rhs = b.element(lane);
            const uint64_t expected_min = std::min(lhs, rhs);
            const uint64_t expected_max = std::max(lhs, rhs);
            if (min_result.element(lane) != expected_min || max_result.element(lane) != expected_max) {
                harness.add_result(test_name, false, "edge mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    harness.add_result(test_name, true, "min/max matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_uint64_compare_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt64 compare/blend";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<uint64_t> dist(0ull, std::numeric_limits<uint64_t>::max());

    for (int iteration = 0; iteration < 700; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType if_true{};
        alignas(SimdType) SimdType if_false{};
        alignas(SimdType) SimdType zero(0ull);
        alignas(SimdType) SimdType ones(std::numeric_limits<uint64_t>::max());
        alignas(SimdType) SimdType b_diff{};

        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t av = dist(rng);
            const uint64_t bv = dist(rng);
            set_lane(a, lane, av);
            set_lane(b, lane, bv);
            set_lane(if_true, lane, dist(rng));
            set_lane(if_false, lane, dist(rng));
            set_lane(b_diff, lane, av ^ 1ull);
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

            const uint64_t all_ones = std::numeric_limits<uint64_t>::max();
            if (blend_eq.element(lane) != (eq ? all_ones : 0ull) ||
                blend_lt.element(lane) != (lt ? all_ones : 0ull) ||
                blend_le.element(lane) != (le ? all_ones : 0ull) ||
                blend_gt.element(lane) != (gt ? all_ones : 0ull) ||
                blend_ge.element(lane) != (ge ? all_ones : 0ull)) {
                harness.add_result(test_name, false, "blend(compare) mismatch, lane " + std::to_string(lane));
                return false;
            }

            const uint64_t t = if_true.element(lane);
            const uint64_t f = if_false.element(lane);
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
    }

    {
        constexpr uint64_t edge_values[] = {
            0ull,
            1ull,
            2ull,
            0x7fffffffffffffffull,
            0x8000000000000000ull,
            0x8000000000000001ull,
            0xfffffffffffffffeull,
            0xffffffffffffffffull
        };
        constexpr int edge_count = static_cast<int>(sizeof(edge_values) / sizeof(edge_values[0]));

        for (int base = 0; base < edge_count; ++base) {
            alignas(SimdType) SimdType a{};
            alignas(SimdType) SimdType b{};
            alignas(SimdType) SimdType if_true{};
            alignas(SimdType) SimdType if_false{};
            alignas(SimdType) SimdType zero(0ull);
            alignas(SimdType) SimdType ones(std::numeric_limits<uint64_t>::max());

            for (int lane = 0; lane < lanes; ++lane) {
                set_lane(a, lane, edge_values[(base + lane) % edge_count]);
                set_lane(b, lane, edge_values[(base + lane + edge_count / 2) % edge_count]);
                set_lane(if_true, lane, edge_values[(base * 3 + lane + 1) % edge_count]);
                set_lane(if_false, lane, edge_values[(base * 5 + lane + 2) % edge_count]);
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
                const uint64_t av = a.element(lane);
                const uint64_t bv = b.element(lane);
                const bool eq = av == bv;
                const bool lt = av < bv;
                const bool le = av <= bv;
                const bool gt = av > bv;
                const bool ge = av >= bv;
                const uint64_t all_ones = std::numeric_limits<uint64_t>::max();
                const uint64_t t = if_true.element(lane);
                const uint64_t f = if_false.element(lane);

                if (blend_eq.element(lane) != (eq ? all_ones : 0ull) ||
                    blend_lt.element(lane) != (lt ? all_ones : 0ull) ||
                    blend_le.element(lane) != (le ? all_ones : 0ull) ||
                    blend_gt.element(lane) != (gt ? all_ones : 0ull) ||
                    blend_ge.element(lane) != (ge ? all_ones : 0ull)) {
                    harness.add_result(test_name, false, "edge blend(compare) mismatch, lane " + std::to_string(lane));
                    return false;
                }

                if (if_eq.element(lane) != (eq ? t : f) ||
                    if_lt.element(lane) != (lt ? t : f) ||
                    if_le.element(lane) != (le ? t : f) ||
                    if_gt.element(lane) != (gt ? t : f) ||
                    if_ge.element(lane) != (ge ? t : f)) {
                    harness.add_result(test_name, false, "edge if_* mismatch, lane " + std::to_string(lane));
                    return false;
                }
            }
        }

        if constexpr (!std::is_same_v<typename SimdType::MaskType, bool>) {
            alignas(SimdType) SimdType partial_a = SimdType::make_set1(0ull);
            alignas(SimdType) SimdType partial_b = SimdType::make_set1(0ull);
            set_lane(partial_b, 0, 1ull);
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
bool run_uint64_metadata_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt64 metadata";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    if (SimdType::size_of_element() != static_cast<int>(sizeof(uint64_t))) {
        harness.add_result(test_name, false, "size_of_element() mismatch");
        return false;
    }

    const SimdType sequential = SimdType::make_sequential(33ull);
    for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
        const uint64_t expected = 33ull + static_cast<uint64_t>(lane);
        if (sequential.element(lane) != expected) {
            harness.add_result(test_name, false, "make_sequential mismatch, lane " + std::to_string(lane));
            return false;
        }
    }

    {
        const uint64_t start = std::numeric_limits<uint64_t>::max() - static_cast<uint64_t>(SimdType::number_of_elements() - 1);
        const SimdType wrapped = SimdType::make_sequential(start);
        for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
            const uint64_t expected = start + static_cast<uint64_t>(lane);
            if (wrapped.element(lane) != expected) {
                harness.add_result(test_name, false, "make_sequential wrap mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    const SimdType set_value = SimdType::make_set1(45ull);
    for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
        if (set_value.element(lane) != 45ull) {
            harness.add_result(test_name, false, "make_set1 mismatch, lane " + std::to_string(lane));
            return false;
        }
    }

    harness.add_result(test_name, true, "make_sequential, make_set1, and size_of_element() matched");
    return true;
}

template <typename SimdType>
bool run_uint64_binary_test_for_type(
    const std::string& type_name,
    const std::string& op_name,
    ArithmeticOp op,
    CpuInformation cpu,
    TestHarness& harness) {
    const std::string test_name = type_name + " UInt64 " + op_name;

    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937_64 rng(20260224ull);
    std::uniform_int_distribution<uint64_t> dist(0ull, std::numeric_limits<uint64_t>::max());

    for (int iteration = 0; iteration < 1200; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};

        for (int lane = 0; lane < lanes; ++lane) {
            const uint64_t lhs = dist(rng);
            uint64_t rhs = dist(rng);
            if (op == ArithmeticOp::div) {
                rhs = sanitize_divisor(rhs);
            }
            set_lane(a, lane, lhs);
            set_lane(b, lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const uint64_t scalar = scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint64_t lhs = a.element(lane);
                const uint64_t rhs = b.element(lane);
                const uint64_t expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const uint64_t actual = result.element(lane);
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

    const U64Pair* pairs = edge_pairs_for_op(op);
    for (int base = 0; base < edge_pair_count(); ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const U64Pair pair = pairs[(base + lane) % edge_pair_count()];
            uint64_t rhs = pair.rhs;
            if (op == ArithmeticOp::div) {
                rhs = sanitize_divisor(rhs);
            }
            set_lane(a, lane, pair.lhs);
            set_lane(b, lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const uint64_t scalar = scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint64_t lhs = a.element(lane);
                const uint64_t rhs = b.element(lane);
                const uint64_t expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const uint64_t actual = result.element(lane);
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

    harness.add_result(test_name, true, "Random + overflow/underflow/high-word edge cases matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_uint64_suite_for_type(const char* type_name, CpuInformation cpu, TestHarness& harness) {
    return run_uint64_metadata_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint64_binary_test_for_type<SimdType>(type_name, "addition", ArithmeticOp::add, cpu, harness) &&
           run_uint64_binary_test_for_type<SimdType>(type_name, "subtraction", ArithmeticOp::sub, cpu, harness) &&
           run_uint64_binary_test_for_type<SimdType>(type_name, "multiplication", ArithmeticOp::mul, cpu, harness) &&
           run_uint64_binary_test_for_type<SimdType>(type_name, "division", ArithmeticOp::div, cpu, harness) &&
           run_uint64_minmax_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint64_compare_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint64_bitwise_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint64_shift_test_for_type<SimdType>(type_name, cpu, harness);
}

} // namespace

void run_uint64_arithmetic_tests(TestHarness& harness) {
    std::cout << "\n===== UInt64 Arithmetic Tests =====\n";
    if (harness.should_halt()) {
        return;
    }
    CpuInformation cpu{};
#define MT_RUN_OR_HALT(expr) do { if (!(expr)) { return; } } while (false)
    MT_RUN_OR_HALT(run_uint64_suite_for_type<FallbackUInt64>("Fallback", cpu, harness));
#if MT_SIMD_ARCH_X64 || (MT_SIMD_ARCH_WASM && defined(__wasm_simd128__))
    MT_RUN_OR_HALT(run_uint64_suite_for_type<Simd128UInt64>("Simd128", cpu, harness));
#if MT_SIMD_ALLOW_LEVEL3_TYPES
    MT_RUN_OR_HALT(run_uint64_suite_for_type<Simd256UInt64>("Simd256", cpu, harness));
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
    MT_RUN_OR_HALT(run_uint64_suite_for_type<Simd512UInt64>("Simd512", cpu, harness));
#endif
#endif
#undef MT_RUN_OR_HALT
    std::cout << "===================================\n";
}
