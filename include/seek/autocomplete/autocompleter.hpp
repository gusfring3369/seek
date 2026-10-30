#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "seek/autocomplete/suggestion.hpp"

namespace seek {

// Interface: anything that can suggest completions for a prefix.
//
// Implementations:
//   TrieAutocompleter    walks the trie on every request
//   CachedAutocompleter  answers short prefixes from a precomputed table and forwards the rest
//                        to another Autocompleter (Decorator pattern)
class Autocompleter {
public:
    Autocompleter() = default;
    Autocompleter(const Autocompleter&) = delete;  // polymorphic: no copies (avoids slicing)
    Autocompleter& operator=(const Autocompleter&) = delete;
    virtual ~Autocompleter() = default;

    // Up to `k` words that start with `prefix`, best first (see Suggestion::ranksAbove).
    [[nodiscard]] virtual std::vector<Suggestion> complete(std::string_view prefix,
                                                           std::size_t k) const = 0;
};

}  // namespace seek
