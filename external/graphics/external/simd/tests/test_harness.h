/********************************************************************************************************

Authors:        (c) 2026 Maths Town

Licence:        The MIT License

*********************************************************************************************************
Shared test harness declarations.
Implementation remains in main.cpp.
*********************************************************************************************************/

#pragma once

#include <string>
#include <vector>

enum class TestStatus {
    pass,
    fail,
    skip
};

struct TestResult {
    std::string name;
    TestStatus status;
    std::string detail;
};

class TestHarness {
public:
    void add_result(const std::string& name, bool passed, const std::string& detail = "");
    void add_skipped(const std::string& name, const std::string& detail = "");
    void add_warning(const std::string& warning);
    bool should_halt() const;
    int summarize_and_exit_code() const;

private:
    std::vector<TestResult> results;
    std::vector<std::string> warnings;
    bool halted = false;
    bool halt_warning_added = false;
};
