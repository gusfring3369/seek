#pragma once

#include <string>

#include "seek/autocomplete/autocompleter.hpp"
#include "seek/autocomplete/top_k_heap.hpp"
#include "seek/trie/trie.hpp"

namespace seek {

// Autocomplete by walking the trie:
//   1. Follow the prefix's characters down from the root.            O(L)
//   2. Visit every word in that node's subtree and offer it to a     O(M log k)
//      TopKHeap, which keeps only the k best.
// Total O(L + M log k), where M is the size of the prefix's subtree. M is large for short
// prefixes ("s" covers much of the dictionary), which is why CachedAutocompleter exists.
class TrieAutocompleter final : public Autocompleter {
public:
    explicit TrieAutocompleter(const Trie& trie) : trie_(trie) {}

    [[nodiscard]] std::vector<Suggestion> complete(std::string_view prefix,
                                                   std::size_t k) const override;

private:
    // Depth-first walk; `path` holds the word spelled so far and is restored before returning.
    static void collect(const TrieNode& node, std::string& path, TopKHeap& heap);

    const Trie& trie_;
};

}  // namespace seek
