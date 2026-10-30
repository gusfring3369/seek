// seek_bench: reproducible micro-benchmarks for each subsystem. Prints Markdown tables.
//
//   seek_bench [wordlist.txt]
//
// With a word list (one word per line), every lowercase alphabetic word is loaded and given a
// Zipf-distributed popularity. Without one, a deterministic synthetic vocabulary is generated.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "seek/autocomplete.hpp"
#include "seek/fuzzy.hpp"
#include "seek/posting_list.hpp"
#include "seek/top_k_cache.hpp"
#include "seek/trie.hpp"

namespace {

using Clock = std::chrono::steady_clock;

constexpr std::uint32_t kSeed = 2024;
std::size_t gSink = 0;  // consumes results so the optimizer cannot drop the measured work

template <typename Fn>
double timeMicros(Fn&& fn) {
    const auto start = Clock::now();
    fn();
    return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

struct Summary {
    double mean;
    double p50;
    double p99;
};

Summary summarize(std::vector<double> samples) {
    std::sort(samples.begin(), samples.end());
    const auto at = [&](double q) {
        return samples[static_cast<std::size_t>(q * static_cast<double>(samples.size() - 1))];
    };
    const double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
    return {sum / static_cast<double>(samples.size()), at(0.50), at(0.99)};
}

bool isLowerAlpha(const std::string& w) {
    return !w.empty() &&
           std::all_of(w.begin(), w.end(), [](char c) { return c >= 'a' && c <= 'z'; });
}

std::vector<std::string> loadWords(const char* path) {
    std::ifstream in(path);
    std::vector<std::string> words;
    std::string line;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        if (isLowerAlpha(line)) words.push_back(line);
    }
    return words;
}

std::vector<std::string> syntheticWords(std::mt19937& rng, std::size_t count) {
    std::uniform_int_distribution<int> letter('a', 'z');
    std::uniform_int_distribution<std::size_t> length(3, 12);
    std::vector<std::string> words(count);
    for (std::string& w : words) {
        for (std::size_t n = length(rng); n > 0; --n) w.push_back(static_cast<char>(letter(rng)));
    }
    return words;
}

std::string withRandomTypo(std::string word, std::mt19937& rng) {
    std::uniform_int_distribution<int> letter('a', 'z');
    std::uniform_int_distribution<std::size_t> pos(0, word.size() - 1);
    const auto i = static_cast<std::ptrdiff_t>(pos(rng));
    switch (rng() % 3) {
        case 0:
            word[static_cast<std::size_t>(i)] = static_cast<char>(letter(rng));
            break;
        case 1:
            word.insert(word.begin() + i, static_cast<char>(letter(rng)));
            break;
        default:
            word.erase(word.begin() + i);
            break;
    }
    return word;
}

// Plain two-row Levenshtein, the per-word baseline the trie search replaces.
std::size_t levenshtein(const std::string& a, const std::string& b, std::vector<std::size_t>& prev,
                        std::vector<std::size_t>& cur) {
    prev.resize(b.size() + 1);
    cur.resize(b.size() + 1);
    std::iota(prev.begin(), prev.end(), std::size_t{0});
    for (std::size_t i = 1; i <= a.size(); ++i) {
        cur[0] = i;
        for (std::size_t j = 1; j <= b.size(); ++j) {
            cur[j] = std::min(
                {prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1)});
        }
        std::swap(prev, cur);
    }
    return prev[b.size()];
}

void benchAutocomplete(const seek::Trie& trie, const seek::TopKCache& cache,
                       const std::vector<std::string>& words, std::mt19937& rng) {
    constexpr std::size_t kQueries = 2000;
    constexpr std::size_t kTopK = 10;
    std::cout << "\n### Top-10 autocomplete latency by prefix length\n\n"
              << "| prefix length | trie walk p50 (µs) | trie walk p99 (µs) "
                 "| with top-k cache p50 (µs) | with top-k cache p99 (µs) |\n"
              << "|---:|---:|---:|---:|---:|\n";
    std::uniform_int_distribution<std::size_t> pick(0, words.size() - 1);
    for (std::size_t length = 1; length <= 5; ++length) {
        std::vector<std::string> prefixes;
        while (prefixes.size() < kQueries) {
            const std::string& word = words[pick(rng)];
            if (word.size() >= length) prefixes.push_back(word.substr(0, length));
        }
        // Separate passes so neither strategy runs on data the other just pulled into cache.
        std::vector<double> walk;
        for (const std::string& prefix : prefixes) {
            walk.push_back(
                timeMicros([&] { gSink += seek::topKCompletions(trie, prefix, kTopK).size(); }));
        }
        std::vector<double> cached;  // same path as SearchEngine::autocomplete
        for (const std::string& prefix : prefixes) {
            cached.push_back(timeMicros([&] {
                auto hit = cache.lookup(prefix, kTopK);
                gSink += hit ? hit->size() : seek::topKCompletions(trie, prefix, kTopK).size();
            }));
        }
        const Summary w = summarize(walk);
        const Summary c = summarize(cached);
        std::cout << std::format("| {} | {:.1f} | {:.1f} | {:.1f} | {:.1f} |\n", length, w.p50,
                                 w.p99, c.p50, c.p99);
    }
}

void benchFuzzy(const seek::Trie& trie, const std::vector<std::string>& words, std::mt19937& rng) {
    constexpr std::size_t kQueries = 300;
    constexpr std::size_t kBaselineQueries = 30;

    std::vector<std::string> queries;
    std::uniform_int_distribution<std::size_t> pick(0, words.size() - 1);
    while (queries.size() < kQueries) {
        const std::string& word = words[pick(rng)];
        if (word.size() >= 5) queries.push_back(withRandomTypo(word, rng));
    }
    std::size_t totalChars = 0;
    for (const std::string& w : words) totalChars += w.size();

    // Baseline: full DP against every word.
    std::vector<std::size_t> prev;
    std::vector<std::size_t> cur;
    std::vector<double> baseline;
    for (std::size_t q = 0; q < kBaselineQueries; ++q) {
        baseline.push_back(timeMicros([&] {
            for (const std::string& w : words) gSink += levenshtein(w, queries[q], prev, cur) <= 2;
        }));
    }
    const double baselineMean = summarize(baseline).mean;

    std::cout << "\n### Fuzzy search (one random typo per query)\n\n"
              << std::format("Baseline (full DP against all {} words): {:.0f} µs/query\n\n",
                             words.size(), baselineMean)
              << "| max edits | mean (µs) | p99 (µs) | speedup vs baseline | trie nodes visited "
                 "| DP cells vs baseline |\n"
              << "|---:|---:|---:|---:|---:|---:|\n";
    for (std::size_t maxDistance : {1u, 2u}) {
        std::vector<double> samples;
        double visitedFraction = 0;
        double cellFraction = 0;
        for (const std::string& query : queries) {
            seek::FuzzyStats stats;
            samples.push_back(timeMicros(
                [&] { gSink += seek::fuzzySearch(trie, query, maxDistance, &stats).size(); }));
            visitedFraction +=
                static_cast<double>(stats.nodesVisited) / static_cast<double>(trie.nodeCount());
            cellFraction += static_cast<double>(stats.dpCellsComputed) /
                            static_cast<double>(totalChars * query.size());
        }
        const Summary s = summarize(samples);
        const auto n = static_cast<double>(queries.size());
        std::cout << std::format("| {} | {:.0f} | {:.0f} | {:.0f}x | {:.2f}% | {:.3f}% |\n",
                                 maxDistance, s.mean, s.p99, baselineMean / s.mean,
                                 100 * visitedFraction / n, 100 * cellFraction / n);
    }
}

seek::PostingList randomPostings(std::mt19937& rng, std::size_t size, seek::DocId universe) {
    // Sample without replacement by keeping each id with probability size / universe.
    std::bernoulli_distribution keep(static_cast<double>(size) / static_cast<double>(universe));
    seek::PostingList list;
    for (seek::DocId id = 0; id < universe; ++id) {
        if (keep(rng)) list.push_back(id);
    }
    return list;
}

void benchIntersection(std::mt19937& rng) {
    constexpr seek::DocId kUniverse = 10'000'000;
    constexpr std::size_t kLongSize = 1'000'000;
    constexpr int kRepetitions = 15;

    const seek::PostingList longList = randomPostings(rng, kLongSize, kUniverse);
    std::cout << std::format("\n### AND intersection against a {}-id posting list\n\n",
                             longList.size())
              << "| short list | size ratio | two-pointer (µs) | galloping (µs) | winner |\n"
              << "|---:|---:|---:|---:|:---|\n";
    for (std::size_t shortSize : {1'000'000u, 250'000u, 100'000u, 50'000u, 30'000u, 20'000u,
                                  15'000u, 10'000u, 1'000u, 100u}) {
        const seek::PostingList shortList = randomPostings(rng, shortSize, kUniverse);
        std::vector<double> linear;
        std::vector<double> galloping;
        for (int r = 0; r < kRepetitions; ++r) {
            linear.push_back(timeMicros(
                [&] { gSink += seek::postings::intersectLinear(shortList, longList).size(); }));
            galloping.push_back(timeMicros(
                [&] { gSink += seek::postings::intersectGalloping(shortList, longList).size(); }));
        }
        const double l = summarize(linear).p50;
        const double g = summarize(galloping).p50;
        std::cout << std::format(
            "| {} | {:.0f}x | {:.0f} | {:.0f} | {} |\n", shortList.size(),
            static_cast<double>(longList.size()) / static_cast<double>(shortList.size()), l, g,
            g < l ? "galloping" : "two-pointer");
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::mt19937 rng(kSeed);
    std::vector<std::string> words = argc > 1 ? loadWords(argv[1]) : syntheticWords(rng, 300'000);
    if (words.empty()) {
        std::cerr << "seek_bench: no words loaded\n";
        return 1;
    }

    // Zipf-like popularity: the word at random rank r gets weight ~ 1 / r.
    std::vector<std::size_t> ranks(words.size());
    std::iota(ranks.begin(), ranks.end(), std::size_t{1});
    std::shuffle(ranks.begin(), ranks.end(), rng);

    seek::Trie trie;
    seek::TopKCache cache(3, 10);  // SearchEngine defaults
    const double buildMicros = timeMicros([&] {
        for (std::size_t i = 0; i < words.size(); ++i) {
            const seek::Weight weight = std::max<seek::Weight>(1, 10'000'000 / ranks[i]);
            cache.update(words[i], trie.insert(words[i], weight));
        }
    });
    std::cout << std::format(
        "## seek_bench\n\nVocabulary: {} words, {} trie nodes, {} cached prefixes, "
        "built in {:.0f} ms\n",
        trie.wordCount(), trie.nodeCount(), cache.prefixCount(), buildMicros / 1000);

    benchAutocomplete(trie, cache, words, rng);
    benchFuzzy(trie, words, rng);
    benchIntersection(rng);

    std::cerr << "(sink " << gSink << ")\n";
    return 0;
}
