#pragma once

#include <chrono>
#include <cstddef>
#include <random>
#include <string>
#include <vector>

namespace seek::bench {

// Results are added here so the compiler cannot optimize the measured work away.
inline std::size_t sink = 0;

// Runs `work` once and returns how long it took in microseconds.
template <typename Work>
double timeMicros(Work&& work) {
    const auto start = std::chrono::steady_clock::now();
    work();
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start)
        .count();
}

struct LatencySummary {
    double mean = 0;
    double p50 = 0;
    double p99 = 0;
};

[[nodiscard]] LatencySummary summarize(std::vector<double> samples);

// Every lower-case alphabetic word in a one-word-per-line file.
[[nodiscard]] std::vector<std::string> loadWords(const std::string& path);

// `count` random lower-case words of 3 to 12 letters.
[[nodiscard]] std::vector<std::string> syntheticWords(std::mt19937& rng, std::size_t count);

// `word` with one random substitution, insertion or deletion.
[[nodiscard]] std::string withRandomTypo(std::string word, std::mt19937& rng);

}  // namespace seek::bench
