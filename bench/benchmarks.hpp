#pragma once

#include <random>
#include <string>
#include <vector>

#include "seek/autocomplete/cached_autocompleter.hpp"
#include "seek/autocomplete/trie_autocompleter.hpp"
#include "seek/trie/trie.hpp"

namespace seek::bench {

// Each benchmark prints one Markdown table.

void benchmarkAutocomplete(const TrieAutocompleter& trieWalk, const CachedAutocompleter& cached,
                           const std::vector<std::string>& words, std::mt19937& rng);

void benchmarkSpellCheck(const Trie& dictionary, const std::vector<std::string>& words,
                         std::mt19937& rng);

void benchmarkIntersection(std::mt19937& rng);

}  // namespace seek::bench
