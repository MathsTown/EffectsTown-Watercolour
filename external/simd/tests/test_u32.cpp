/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
UInt32 SIMD tests.
Uses independent scalar/std::bit reference calculations.
*********************************************************************************************************/

#include "test_u32.h"

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
#include "../include/simd-uint32.h"

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

struct U32Pair {
    uint32_t lhs;
    uint32_t rhs;
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

uint32_t sanitize_divisor(uint32_t rhs) {
    return rhs == 0u ? 1u : rhs;
}

uint32_t apply_scalar_binary(uint32_t lhs, uint32_t rhs, ArithmeticOp op) {
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
    uint32_t scalar,
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

uint32_t apply_scalar_op_with_path(
    uint32_t lhs,
    uint32_t rhs,
    uint32_t scalar,
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

uint32_t scalar_for_op(ArithmeticOp op) {
    if (op == ArithmeticOp::add) { return 37u; }
    if (op == ArithmeticOp::sub) { return 19u; }
    if (op == ArithmeticOp::mul) { return 3u; }
    return 5u;
}

const U32Pair* edge_pairs_for_op(ArithmeticOp op) {
    static constexpr U32Pair kAddPairs[] = {
        {0u, 0u},
        {1u, 1u},
        {std::numeric_limits<uint32_t>::max(), 1u},      // overflow wrap
        {std::numeric_limits<uint32_t>::max(), 2u},      // overflow wrap
        {0x80000000u, 0x80000000u},                      // high bit carry
        {123456789u, 4000000000u},
        {std::numeric_limits<uint32_t>::max() - 5u, 10u},
        {17u, 999999937u},
        {4000000000u, 4000000000u}
    };
    static constexpr U32Pair kSubPairs[] = {
        {0u, 0u},
        {1u, 1u},
        {0u, 1u},                                         // underflow wrap
        {0u, std::numeric_limits<uint32_t>::max()},      // underflow wrap
        {17u, 999999937u},
        {123456789u, 4000000000u},
        {std::numeric_limits<uint32_t>::max(), 1u},
        {0x80000000u, 0x80000001u},
        {4000000000u, 4000000000u}
    };
    static constexpr U32Pair kMulPairs[] = {
        {0u, 0u},
        {1u, 1u},
        {std::numeric_limits<uint32_t>::max(), 2u},      // overflow wrap
        {0x80000000u, 2u},                               // wraps to 0
        {0xFFFFFFFFu, 0xFFFFFFFFu},                      // overflow wrap
        {65536u, 65536u},                                // wraps to 0
        {123456789u, 4000000000u},
        {17u, 999999937u},
        {0xF0000000u, 0x10u}
    };
    static constexpr U32Pair kDivPairs[] = {
        {0u, 1u},
        {1u, 1u},
        {std::numeric_limits<uint32_t>::max(), 1u},
        {std::numeric_limits<uint32_t>::max(), 2u},
        {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()},
        {123456789u, 3u},
        {4000000000u, 17u},
        {0x80000000u, 0x10000u},
        {0xF0000000u, 0x10u}
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

uint32_t apply_scalar_bitwise_binary(uint32_t lhs, uint32_t rhs, BitwiseOp op) {
    if (op == BitwiseOp::bit_and) { return lhs & rhs; }
    if (op == BitwiseOp::bit_or) { return lhs | rhs; }
    return lhs ^ rhs;
}

uint32_t apply_scalar_bitwise_compound(uint32_t lhs, uint32_t rhs, BitwiseOp op) {
    if (op == BitwiseOp::bit_and) { return lhs & rhs; }
    if (op == BitwiseOp::bit_or) { return lhs | rhs; }
    return lhs ^ rhs;
}

template <typename SimdType>
bool run_uint32_bitwise_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt32 bitwise";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937 rng(20260224u);
    std::uniform_int_distribution<uint32_t> dist(0u, std::numeric_limits<uint32_t>::max());

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
                const uint32_t lhs = a.element(lane);
                const uint32_t rhs = b.element(lane);
                const uint32_t expected_binary = apply_scalar_bitwise_binary(lhs, rhs, op);
                const uint32_t expected_compound = apply_scalar_bitwise_compound(lhs, rhs, op);
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

    const uint32_t edges[] = {
        0u,
        1u,
        std::numeric_limits<uint32_t>::max(),
        0x55555555u,
        0xAAAAAAAAu,
        0x12345678u,
        0x87654321u,
        0x80000000u,
        0x7FFFFFFFu
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
                const uint32_t lhs = a.element(lane);
                const uint32_t rhs = b.element(lane);
                const uint32_t expected_binary = apply_scalar_bitwise_binary(lhs, rhs, op);
                const uint32_t expected_compound = apply_scalar_bitwise_compound(lhs, rhs, op);
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
            const uint32_t lhs = a.element(lane);
            const uint32_t expected = ~lhs;
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
bool run_uint32_shift_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt32 shifts";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    constexpr int shift_counts[] = {0, 1, 2, 7, 15, 30, 31};
    constexpr int rotate_counts[] = {0, 1, 3, 7, 13, 31, 32, 33, -1, -33};
    std::mt19937 rng(20260224u);
    std::uniform_int_distribution<uint32_t> dist(0u, std::numeric_limits<uint32_t>::max());

    for (const int shift : shift_counts) {
        for (int iteration = 0; iteration < 500; ++iteration) {
            alignas(SimdType) SimdType values{};
            for (int lane = 0; lane < lanes; ++lane) {
                values.set_element(lane, dist(rng));
            }

            alignas(SimdType) SimdType left_result = values << shift;
            alignas(SimdType) SimdType right_result = values >> shift;
            for (int lane = 0; lane < lanes; ++lane) {
                const uint32_t v = values.element(lane);
                const uint32_t expected_left = v << shift;
                const uint32_t expected_right = v >> shift;
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
                values.set_element(lane, dist(rng));
            }

            alignas(SimdType) SimdType left_rot = rotl(values, bits);
            alignas(SimdType) SimdType right_rot = rotr(values, bits);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint32_t v = values.element(lane);
                const uint32_t expected_l = std::rotl(v, bits);
                const uint32_t expected_r = std::rotr(v, bits);
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
bool run_uint32_minmax_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt32 min/max";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937 rng(20260224u);
    std::uniform_int_distribution<uint32_t> dist(0u, std::numeric_limits<uint32_t>::max());

    for (int iteration = 0; iteration < 800; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            a.set_element(lane, dist(rng));
            b.set_element(lane, dist(rng));
        }

        alignas(SimdType) SimdType min_result = min(a, b);
        alignas(SimdType) SimdType max_result = max(a, b);

        for (int lane = 0; lane < lanes; ++lane) {
            const uint32_t lhs = a.element(lane);
            const uint32_t rhs = b.element(lane);
            const uint32_t expected_min = std::min(lhs, rhs);
            const uint32_t expected_max = std::max(lhs, rhs);
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

    const uint32_t edges[] = {
        0u,
        1u,
        std::numeric_limits<uint32_t>::max(),
        0x80000000u,
        0x7FFFFFFFu,
        0x55555555u,
        0xAAAAAAAAu,
        123456789u,
        4000000000u
    };
    constexpr int edge_count = static_cast<int>(sizeof(edges) / sizeof(edges[0]));

    for (int base = 0; base < edge_count; ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            a.set_element(lane, edges[(base + lane) % edge_count]);
            b.set_element(lane, edges[(base * 5 + lane + 1) % edge_count]);
        }

        alignas(SimdType) SimdType min_result = min(a, b);
        alignas(SimdType) SimdType max_result = max(a, b);
        for (int lane = 0; lane < lanes; ++lane) {
            const uint32_t lhs = a.element(lane);
            const uint32_t rhs = b.element(lane);
            const uint32_t expected_min = std::min(lhs, rhs);
            const uint32_t expected_max = std::max(lhs, rhs);
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
bool run_uint32_compare_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt32 compare/blend";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::random_device rd;
    std::seed_seq seed{rd(), rd(), rd(), rd()};
    std::mt19937 rng(seed);
    std::uniform_int_distribution<uint32_t> dist(0u, std::numeric_limits<uint32_t>::max());

    for (int iteration = 0; iteration < 800; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        alignas(SimdType) SimdType if_true{};
        alignas(SimdType) SimdType if_false{};
        alignas(SimdType) SimdType zero(0u);
        alignas(SimdType) SimdType ones(std::numeric_limits<uint32_t>::max());
        alignas(SimdType) SimdType b_diff{};

        for (int lane = 0; lane < lanes; ++lane) {
            const uint32_t av = dist(rng);
            const uint32_t bv = dist(rng);
            a.set_element(lane, av);
            b.set_element(lane, bv);
            if_true.set_element(lane, dist(rng));
            if_false.set_element(lane, dist(rng));
            b_diff.set_element(lane, av ^ 1u);
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

            const uint32_t all_ones = std::numeric_limits<uint32_t>::max();
            if (blend_eq.element(lane) != (eq ? all_ones : 0u) ||
                blend_lt.element(lane) != (lt ? all_ones : 0u) ||
                blend_le.element(lane) != (le ? all_ones : 0u) ||
                blend_gt.element(lane) != (gt ? all_ones : 0u) ||
                blend_ge.element(lane) != (ge ? all_ones : 0u)) {
                harness.add_result(test_name, false, "blend(compare) mismatch, lane " + std::to_string(lane));
                return false;
            }

            const uint32_t t = if_true.element(lane);
            const uint32_t f = if_false.element(lane);
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
            partial_b.set_element(0, partial_a.element(0) ^ 1u);
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
bool run_uint32_metadata_test_for_type(const std::string& type_name, CpuInformation cpu, TestHarness& harness) {
    const std::string test_name = type_name + " UInt32 metadata";
    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    if (SimdType::size_of_element() != static_cast<int>(sizeof(uint32_t))) {
        harness.add_result(test_name, false, "size_of_element() mismatch");
        return false;
    }

    const SimdType sequential = SimdType::make_sequential(17u);
    for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
        const uint32_t expected = 17u + static_cast<uint32_t>(lane);
        if (sequential.element(lane) != expected) {
            harness.add_result(test_name, false, "make_sequential mismatch, lane " + std::to_string(lane));
            return false;
        }
    }

    {
        const uint32_t start = std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(SimdType::number_of_elements() - 1);
        const SimdType wrapped = SimdType::make_sequential(start);
        for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
            const uint32_t expected = start + static_cast<uint32_t>(lane);
            if (wrapped.element(lane) != expected) {
                harness.add_result(test_name, false, "make_sequential wrap mismatch, lane " + std::to_string(lane));
                return false;
            }
        }
    }

    const SimdType set_value = SimdType::make_set1(29u);
    for (int lane = 0; lane < SimdType::number_of_elements(); ++lane) {
        if (set_value.element(lane) != 29u) {
            harness.add_result(test_name, false, "make_set1 mismatch, lane " + std::to_string(lane));
            return false;
        }
    }

    harness.add_result(test_name, true, "make_sequential, make_set1, and size_of_element() matched");
    return true;
}

template <typename SimdType>
bool run_uint32_binary_test_for_type(
    const std::string& type_name,
    const std::string& op_name,
    ArithmeticOp op,
    CpuInformation cpu,
    TestHarness& harness) {
    const std::string test_name = type_name + " UInt32 " + op_name;

    if (!SimdType::cpu_supported(cpu) || !SimdType::compiler_supported()) {
        return true;
    }

    constexpr int lanes = SimdType::number_of_elements();
    std::mt19937 rng(20260224u);
    std::uniform_int_distribution<uint32_t> dist(0u, std::numeric_limits<uint32_t>::max());

    for (int iteration = 0; iteration < 1200; ++iteration) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};

        for (int lane = 0; lane < lanes; ++lane) {
            const uint32_t lhs = dist(rng);
            uint32_t rhs = dist(rng);
            if (op == ArithmeticOp::div) {
                rhs = sanitize_divisor(rhs);
            }
            a.set_element(lane, lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const uint32_t scalar = scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint32_t lhs = a.element(lane);
                const uint32_t rhs = b.element(lane);
                const uint32_t expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const uint32_t actual = result.element(lane);
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

    const U32Pair* pairs = edge_pairs_for_op(op);
    for (int base = 0; base < edge_pair_count(); ++base) {
        alignas(SimdType) SimdType a{};
        alignas(SimdType) SimdType b{};
        for (int lane = 0; lane < lanes; ++lane) {
            const U32Pair pair = pairs[(base + lane) % edge_pair_count()];
            uint32_t rhs = pair.rhs;
            if (op == ArithmeticOp::div) {
                rhs = sanitize_divisor(rhs);
            }
            a.set_element(lane, pair.lhs);
            b.set_element(lane, rhs);
        }

        for (const ArithmeticPath path : kArithmeticPaths) {
            const uint32_t scalar = scalar_for_op(op);
            alignas(SimdType) SimdType result{};
            apply_simd_op_with_path(a, b, scalar, op, path, result);
            for (int lane = 0; lane < lanes; ++lane) {
                const uint32_t lhs = a.element(lane);
                const uint32_t rhs = b.element(lane);
                const uint32_t expected = apply_scalar_op_with_path(lhs, rhs, scalar, op, path);
                const uint32_t actual = result.element(lane);
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

    harness.add_result(test_name, true, "Random + overflow/underflow edge cases matched scalar reference");
    return true;
}

template <typename SimdType>
bool run_uint32_suite_for_type(const char* type_name, CpuInformation cpu, TestHarness& harness) {
    return run_uint32_metadata_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint32_binary_test_for_type<SimdType>(type_name, "addition", ArithmeticOp::add, cpu, harness) &&
           run_uint32_binary_test_for_type<SimdType>(type_name, "subtraction", ArithmeticOp::sub, cpu, harness) &&
           run_uint32_binary_test_for_type<SimdType>(type_name, "multiplication", ArithmeticOp::mul, cpu, harness) &&
           run_uint32_binary_test_for_type<SimdType>(type_name, "division", ArithmeticOp::div, cpu, harness) &&
           run_uint32_minmax_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint32_compare_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint32_bitwise_test_for_type<SimdType>(type_name, cpu, harness) &&
           run_uint32_shift_test_for_type<SimdType>(type_name, cpu, harness);
}

} // namespace

void run_uint32_arithmetic_tests(TestHarness& harness) {
    std::cout << "\n===== UInt32 Arithmetic Tests =====\n";
    if (harness.should_halt()) {
        return;
    }
    CpuInformation cpu{};
#define MT_RUN_OR_HALT(expr) do { if (!(expr)) { return; } } while (false)
    MT_RUN_OR_HALT(run_uint32_suite_for_type<FallbackUInt32>("Fallback", cpu, harness));
#if MT_SIMD_ARCH_X64 || (MT_SIMD_ARCH_WASM && defined(__wasm_simd128__))
    MT_RUN_OR_HALT(run_uint32_suite_for_type<Simd128UInt32>("Simd128", cpu, harness));
#if MT_SIMD_ALLOW_LEVEL3_TYPES
    MT_RUN_OR_HALT(run_uint32_suite_for_type<Simd256UInt32>("Simd256", cpu, harness));
#endif
#if MT_SIMD_ALLOW_LEVEL4_TYPES
    MT_RUN_OR_HALT(run_uint32_suite_for_type<Simd512UInt32>("Simd512", cpu, harness));
#endif
#endif
#undef MT_RUN_OR_HALT
    std::cout << "===================================\n";
}
