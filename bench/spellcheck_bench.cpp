#include <format>
#include <iostream>

#include "bench_utils.hpp"
#include "benchmarks.hpp"
#include "seek/spellcheck/edit_distance.hpp"
#include "seek/spellcheck/spell_checker.hpp"

using namespace std;

namespace seek::bench {

// The naive approach the SpellChecker replaces: a full edit-distance DP against every word.
// Rows are reused across words so the baseline is not slowed down by memory allocation.
static size_t countWithinTwoEdits(const vector<string>& words, const string& query) {
    const EditDistance::Row first = EditDistance::firstRow(query);
    EditDistance::Row row;
    EditDistance::Row scratch;
    size_t matches = 0;
    for (const string& word : words) {
        row = first;
        for (char c : word) {
            EditDistance::nextRow(row, c, query, scratch);
            swap(row, scratch);
        }
        if (row.back() <= 2) ++matches;
    }
    return matches;
}

void benchmarkSpellCheck(const Trie& dictionary, const vector<string>& words, mt19937& rng) {
    constexpr size_t kQueries = 300;
    constexpr size_t kBaselineQueries = 30;

    vector<string> queries;
    uniform_int_distribution<size_t> pick(0, words.size() - 1);
    while (queries.size() < kQueries) {
        const string& word = words[pick(rng)];
        if (word.size() >= 5) queries.push_back(withRandomTypo(word, rng));
    }
    size_t totalCharacters = 0;
    for (const string& word : words) totalCharacters += word.size();

    vector<double> baselineTimes;
    for (size_t q = 0; q < kBaselineQueries; ++q) {
        baselineTimes.push_back(
            timeMicros([&] { sink += countWithinTwoEdits(words, queries[q]); }));
    }
    const double baselineMean = summarize(baselineTimes).mean;

    cout << "\n### Fuzzy search (one random typo per query)\n\n"
         << format("Baseline (full DP against all {} words): {:.0f} µs/query\n\n", words.size(),
                   baselineMean)
         << "| max edits | mean (µs) | p99 (µs) | speedup vs baseline | trie nodes visited "
            "| DP cells vs baseline |\n"
         << "|---:|---:|---:|---:|---:|---:|\n";

    const SpellChecker checker(dictionary);
    for (size_t maxDistance : {1u, 2u}) {
        vector<double> times;
        double visitedShare = 0;
        double cellShare = 0;
        for (const string& query : queries) {
            SpellCheckStats stats;
            times.push_back(
                timeMicros([&] { sink += checker.suggest(query, maxDistance, &stats).size(); }));
            visitedShare += static_cast<double>(stats.nodesVisited) /
                            static_cast<double>(dictionary.nodeCount());
            cellShare += static_cast<double>(stats.dpCellsComputed) /
                         static_cast<double>(totalCharacters * query.size());
        }
        const LatencySummary summary = summarize(times);
        const auto n = static_cast<double>(queries.size());
        cout << format("| {} | {:.0f} | {:.0f} | {:.0f}x | {:.2f}% | {:.3f}% |\n", maxDistance,
                       summary.mean, summary.p99, baselineMean / summary.mean,
                       100 * visitedShare / n, 100 * cellShare / n);
    }
}

}  // namespace seek::bench
