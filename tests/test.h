/********************************************************************************************************

Authors:		(c) 2026 Maths Town

Licence:		The MIT License

*********************************************************************************************************
Tiny test helpers.
*********************************************************************************************************/

#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

struct TestContext {
	int passed = 0;
	int failed = 0;

	bool check(bool condition, const std::string& message) {
		if (condition) {
			++passed;
			return true;
		}

		++failed;
		std::cout << "[FAIL] " << message << "\n";
		return false;
	}

	bool check_equal(float actual, float expected, const std::string& message) {
		return check(actual == expected, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
	}

	bool check_equal(double actual, double expected, const std::string& message) {
		return check(actual == expected, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
	}

	bool check_close(float actual, float expected, const std::string& message) {
		const float scale = std::max(1.0f, std::max(std::fabs(actual), std::fabs(expected)));
		return check(std::fabs(actual - expected) <= 1.0e-6f * scale, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
	}

	bool check_close(double actual, double expected, const std::string& message) {
		const double scale = std::max(1.0, std::max(std::fabs(actual), std::fabs(expected)));
		return check(std::fabs(actual - expected) <= 1.0e-10 * scale, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
	}

	bool check_close(float actual, double expected, float tolerance, const std::string& message) {
		const float expected_value = static_cast<float>(expected);
		const float scale = std::max(1.0f, std::max(std::fabs(actual), std::fabs(expected_value)));
		return check(std::fabs(actual - expected_value) <= tolerance * scale, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected_value));
	}

	bool check_close(double actual, double expected, double tolerance, const std::string& message) {
		const double scale = std::max(1.0, std::max(std::fabs(actual), std::fabs(expected)));
		return check(std::fabs(actual - expected) <= tolerance * scale, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
	}

	bool check_close_hue(float actual, double expected, float tolerance, const std::string& message) {
		const float wrapped_actual = actual - std::floor(actual);
		const float wrapped_expected = static_cast<float>(expected - std::floor(expected));
		const float direct_distance = std::fabs(wrapped_actual - wrapped_expected);
		const float hue_distance = std::min(direct_distance, 1.0f - direct_distance);
		const float scale = std::max(1.0f, std::max(std::fabs(wrapped_actual), std::fabs(wrapped_expected)));
		return check(hue_distance <= tolerance * scale, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(static_cast<float>(expected)));
	}

	bool check_close_hue(double actual, double expected, double tolerance, const std::string& message) {
		const double wrapped_actual = actual - std::floor(actual);
		const double wrapped_expected = expected - std::floor(expected);
		const double direct_distance = std::fabs(wrapped_actual - wrapped_expected);
		const double hue_distance = std::min(direct_distance, 1.0 - direct_distance);
		const double scale = std::max(1.0, std::max(std::fabs(wrapped_actual), std::fabs(wrapped_expected)));
		return check(hue_distance <= tolerance * scale, message + " actual=" + std::to_string(actual) + " expected=" + std::to_string(expected));
	}

	int result() const {
		if (failed != 0) {
			std::cout << "\nTests failed: " << failed << "\n";
			std::cout << "Tests passed: " << passed << "\n";
		}
		return failed == 0 ? 0 : 1;
	}
};
