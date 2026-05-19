/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Simple test harness entry point.

Current scope:
1) Runtime CPU feature detection and reporting.
2) Basic harness result summary with process exit code.
3) Calling per-domain/type test groups.
*********************************************************************************************************/

#include <iostream>

#include "../include/environment.h"
#include "../include/simd-cpuid.h"

#include "test_f32.h"
#include "test_f64.h"
#include "test_harness.h"
#include "test_i32.h"
#include "test_i64.h"
#include "test_u32.h"
#include "test_u64.h"

using namespace mt;

void TestHarness::add_result(const std::string& name, bool passed, const std::string& detail) {
    results.push_back(TestResult{name, passed ? TestStatus::pass : TestStatus::fail, detail});
    if (!passed) {
        halted = true;
        if (!halt_warning_added) {
            warnings.push_back("Testing was halted due to an error.");
            halt_warning_added = true;
        }
    }
}

void TestHarness::add_skipped(const std::string& name, const std::string& detail) {
    results.push_back(TestResult{name, TestStatus::skip, detail});
}

void TestHarness::add_warning(const std::string& warning) {
    warnings.push_back(warning);
}

bool TestHarness::should_halt() const {
    return halted;
}

int TestHarness::summarize_and_exit_code() const {
    int passed = 0;
    int failed = 0;
    int skipped = 0;

    std::cout << "\n===== Test Results =====\n";
    for (const auto& result : results) {
        if (result.status == TestStatus::pass) {
            std::cout << "[PASS] ";
            ++passed;
        } else if (result.status == TestStatus::fail) {
            std::cout << "[FAIL] ";
            ++failed;
        } else {
            std::cout << "[SKIP] ";
            ++skipped;
        }

        std::cout << result.name;
        if (!result.detail.empty()) {
            std::cout << " - " << result.detail;
        }
        std::cout << "\n";
    }

    std::cout << "------------------------\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Skipped: " << skipped << "\n";
    std::cout << "Total : " << (passed + failed + skipped) << "\n";
    if (!warnings.empty()) {
        std::cout << "Warnings:\n";
        for (const auto& warning : warnings) {
            std::cout << "- " << warning << "\n";
        }
    }
    std::cout << "========================\n";

    if (failed != 0) {
        std::cout << "\n***=== TESTS FAILED ===***\n";
    }

    return failed == 0 ? 0 : 1;
}

static void print_environment_summary() {
    const char* math_backend = mt::environment::use_svml ? "SVML" : "NATIVE";
    const char* x86_dispatch_mode = mt::environment::uses_runtime_x86_dispatch ? "Runtime" : "Compile-time";

    std::cout << "===== Build Environment =====\n";
    std::cout << "Visual Studio : " << (mt::environment::is_visual_studio ? "Yes" : "No") << "\n";
    std::cout << "Clang         : " << (mt::environment::is_clang ? "Yes" : "No") << "\n";
    std::cout << "GCC           : " << (mt::environment::is_gcc ? "Yes" : "No") << "\n";
    std::cout << "x86_64 target : " << (mt::environment::is_x86_64 ? "Yes" : "No") << "\n";
    std::cout << "x86 dispatch  : " << x86_dispatch_mode << "\n";
    std::cout << "x86 L1 types  : " << (mt::environment::compiler_can_use_x86_64_level_1_types ? "Yes" : "No") << "\n";
    std::cout << "x86 L3 types  : " << (mt::environment::compiler_can_use_x86_64_level_3_types ? "Yes" : "No") << "\n";
    std::cout << "x86 L4 types  : " << (mt::environment::compiler_can_use_x86_64_level_4_types ? "Yes" : "No") << "\n";
    std::cout << "Wasm target   : " << (mt::environment::is_wasm ? "Yes" : "No") << "\n";
    std::cout << "Wasm SIMD L1  : " << (mt::environment::is_wasm_simd_level_1 ? "Yes" : "No") << "\n";
    std::cout << "Math backend  : " << math_backend << "\n";
    std::cout << "=============================\n";
}

static void runtime_cpu_feature_check(TestHarness& harness) {
#if !defined(SIMD_TEST_MIN_LEVEL)
#define SIMD_TEST_MIN_LEVEL 3
#endif
#if defined(_M_X64) || defined(__x86_64)
    CpuInformation cpu{};

    std::cout << "\n===== Runtime CPU SIMD Features =====\n";
    std::cout << cpu.to_string();
    std::cout << "x86_64 CPU Level: " << cpu.get_level() << "\n";
    std::cout << "=====================================\n";

    harness.add_result("Runtime CPUID executed", true, "Detected x86_64 CPU features");
    harness.add_result("SSE present", cpu.has_sse(), "Minimum expected SIMD baseline");
    harness.add_result("SSE2 present", cpu.has_sse2(), "Required for x86_64 SIMD level 1");
    harness.add_result("SSE3 present", cpu.has_sse3(), "Required for x86_64 SIMD level 2");
    harness.add_result("SSSE3 present", cpu.has_ssse3(), "Required for x86_64 SIMD level 2");
    harness.add_result("SSE4.1 present", cpu.has_sse41(), "Required for x86_64 SIMD level 2");
    harness.add_result("SSE4.2 present", cpu.has_sse42(), "Required for x86_64 SIMD level 2");
#if SIMD_TEST_MIN_LEVEL >= 3
    harness.add_result("AVX present", cpu.has_avx(), "Required for x86_64 SIMD level 3");
    harness.add_result("AVX2 present", cpu.has_avx2(), "Required for x86_64 SIMD level 3");
    harness.add_result("FMA present", cpu.has_fma(), "Required for x86_64 SIMD level 3");
    harness.add_result("CPU level >= 3", cpu.get_level() >= 3, "Minimum harness target is AVX2 level");
#else
    harness.add_result("CPU level >= 1", cpu.get_level() >= 1, "Minimum harness target is SSE2 level");
#endif

#if MT_SIMD_ALLOW_LEVEL3_TYPES
    if (!(cpu.has_avx() && cpu.has_avx2() && cpu.has_fma())) {
        harness.add_warning("Level 3 tests were skipped because AVX/AVX2/FMA are unsupported by CPU.");
    }
#else
    harness.add_warning("Level 3 tests were skipped because this build does not include level 3 types.");
#endif

#if MT_SIMD_ALLOW_LEVEL4_TYPES
    if (!(cpu.has_avx512_f() && cpu.has_avx512_dq())) {
        harness.add_warning("Level 4 tests were skipped because AVX-512F/AVX-512DQ are unsupported by CPU.");
    }
#else
    harness.add_warning("Level 4 tests were skipped because this build does not include level 4 types.");
#endif
#else
    CpuInformation cpu{};
    std::cout << "\n===== Runtime CPU SIMD Features =====\n";
    std::cout << cpu.to_string();
    if (mt::environment::is_wasm) {
        std::cout << "Wasm SIMD Level: " << cpu.get_level() << "\n";
    } else {
        std::cout << "Non-x86_64 target. CPUID SIMD probing is skipped.\n";
    }
    std::cout << "=====================================\n";

    harness.add_result("Runtime CPUID executed", true, "Skipped on non-x86_64 target");
    if (mt::environment::is_wasm) {
        harness.add_result("Wasm SIMD level 1 present", cpu.has_wasm_simd(), "Compile-time Wasm SIMD128 support");
    }
    harness.add_warning("Level 3 tests were skipped on non-x86_64 target.");
    harness.add_warning("Level 4 tests were skipped on non-x86_64 target.");
#endif
}

int main() {
    TestHarness harness{};

    print_environment_summary();
    runtime_cpu_feature_check(harness);
    if (!harness.should_halt()) { run_float32_arithmetic_tests(harness); }
    if (!harness.should_halt()) { run_float64_arithmetic_tests(harness); }
    if (!harness.should_halt()) { run_int32_arithmetic_tests(harness); }
    if (!harness.should_halt()) { run_int64_arithmetic_tests(harness); }
    if (!harness.should_halt()) { run_uint32_arithmetic_tests(harness); }
    if (!harness.should_halt()) { run_uint64_arithmetic_tests(harness); }

    return harness.summarize_and_exit_code();
}
