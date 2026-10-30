#pragma once

#include <chrono>
#include <format>
#include <string>

namespace seek::cli {

// Measures the time since it was created, for printing how long a command took.
class Stopwatch {
public:
    // e.g. "12.4 µs" or "3.21 ms".
    [[nodiscard]] std::string elapsed() const {
        const std::chrono::duration<double, std::micro> micros = Clock::now() - start_;
        if (micros.count() < 1000) return std::format("{:.1f} µs", micros.count());
        return std::format("{:.2f} ms", micros.count() / 1000);
    }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point start_ = Clock::now();
};

}  // namespace seek::cli
