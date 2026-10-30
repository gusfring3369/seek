// seek_bench: reproducible benchmarks for each subsystem. Prints Markdown tables.
//
//   ./build/seek_bench data/words_alpha.txt   (a real dictionary; see scripts/fetch_wordlist.sh)
//   ./build/seek_bench                        (a synthetic one)

#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>

#include "bench_utils.hpp"
#include "benchmarks.hpp"

using namespace std;
using namespace seek;
using namespace seek::bench;

int main(int argc, char** argv) {
    mt19937 rng(2024);  // fixed seed: every run measures the same workload
    const vector<string> words = argc > 1 ? loadWords(argv[1]) : syntheticWords(rng, 300'000);
    if (words.empty()) {
        cerr << "seek_bench: no words loaded\n";
        return 1;
    }

    // Zipf-like popularity: the word with (random) rank r gets weight ~ 1 / r.
    vector<size_t> ranks(words.size());
    iota(ranks.begin(), ranks.end(), size_t{1});
    shuffle(ranks.begin(), ranks.end(), rng);

    Trie dictionary;
    const TrieAutocompleter trieWalk(dictionary);
    CachedAutocompleter cached(trieWalk, 3, 10);  // the SearchEngine defaults
    const double buildMicros = timeMicros([&] {
        for (size_t i = 0; i < words.size(); ++i) {
            const Weight weight = max<Weight>(1, 10'000'000 / ranks[i]);
            cached.onWeightChanged(words[i], dictionary.insert(words[i], weight));
        }
    });
    cout << format(
        "## seek_bench\n\nVocabulary: {} words, {} trie nodes, {} cached prefixes, "
        "built in {:.0f} ms\n",
        dictionary.wordCount(), dictionary.nodeCount(), cached.cachedPrefixCount(),
        buildMicros / 1000);

    benchmarkAutocomplete(trieWalk, cached, words, rng);
    benchmarkSpellCheck(dictionary, words, rng);
    benchmarkIntersection(rng);

    cerr << "(checksum " << sink << ")\n";
    return 0;
}
