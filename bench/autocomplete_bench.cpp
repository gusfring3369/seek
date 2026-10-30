#include <format>
#include <iostream>

#include "bench_utils.hpp"
#include "benchmarks.hpp"

using namespace std;

namespace seek::bench {

void benchmarkAutocomplete(const TrieAutocompleter& trieWalk, const CachedAutocompleter& cached,
                           const vector<string>& words, mt19937& rng) {
    constexpr size_t kQueriesPerLength = 2000;
    constexpr size_t kTopK = 10;

    cout << "\n### Top-10 autocomplete latency by prefix length\n\n"
         << "| prefix length | trie walk p50 (µs) | trie walk p99 (µs) "
            "| with top-k cache p50 (µs) | with top-k cache p99 (µs) |\n"
         << "|---:|---:|---:|---:|---:|\n";

    uniform_int_distribution<size_t> pick(0, words.size() - 1);
    for (size_t length = 1; length <= 5; ++length) {
        vector<string> prefixes;
        while (prefixes.size() < kQueriesPerLength) {
            const string& word = words[pick(rng)];
            if (word.size() >= length) prefixes.push_back(word.substr(0, length));
        }

        // Separate passes, so neither one runs on data the other just pulled into the CPU cache.
        vector<double> walkTimes;
        for (const string& prefix : prefixes) {
            walkTimes.push_back(
                timeMicros([&] { sink += trieWalk.complete(prefix, kTopK).size(); }));
        }
        vector<double> cachedTimes;
        for (const string& prefix : prefixes) {
            cachedTimes.push_back(
                timeMicros([&] { sink += cached.complete(prefix, kTopK).size(); }));
        }

        const LatencySummary walk = summarize(walkTimes);
        const LatencySummary withCache = summarize(cachedTimes);
        cout << format("| {} | {:.1f} | {:.1f} | {:.1f} | {:.1f} |\n", length, walk.p50, walk.p99,
                       withCache.p50, withCache.p99);
    }
}

}  // namespace seek::bench
