#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "seek/autocomplete/autocompleter.hpp"
#include "seek/common/string_hash.hpp"

namespace seek {

// Decorator that adds a cache of precomputed answers for short prefixes to another Autocompleter.
//
// Short prefixes are the slow case for a trie walk (huge subtrees) and also the most common case
// (every query starts as a short prefix). This class stores the best `capacity` words for every
// prefix of up to `maxPrefixLength` characters, so those requests become one hash lookup.
// Longer prefixes, or k > capacity, are forwarded to the wrapped `fallback`.
//
// The table is kept exact on every update: a word's weight only ever grows, and when one word's
// weight grows the only possible change to any top-k list is that word moving into it or up it.
class CachedAutocompleter final : public Autocompleter {
public:
    CachedAutocompleter(const Autocompleter& fallback, std::size_t maxPrefixLength,
                        std::size_t capacity);

    // Must be called every time a word's total weight changes. `newWeight` must not be lower
    // than any weight reported for that word before.
    void onWeightChanged(std::string_view word, Weight newWeight);

    [[nodiscard]] std::vector<Suggestion> complete(std::string_view prefix,
                                                   std::size_t k) const override;

    // Whether complete(prefix, k) is answered from the table rather than the fallback.
    [[nodiscard]] bool covers(std::string_view prefix, std::size_t k) const noexcept {
        return prefix.size() <= maxPrefixLength_ && k <= capacity_;
    }

    [[nodiscard]] std::size_t cachedPrefixCount() const noexcept { return table_.size(); }

private:
    using RankedList = std::vector<Suggestion>;  // best first, at most capacity_ entries

    void update(RankedList& list, std::string_view word, Weight newWeight) const;

    const Autocompleter& fallback_;
    std::size_t maxPrefixLength_;
    std::size_t capacity_;
    std::unordered_map<std::string, RankedList, StringHash, std::equal_to<>> table_;
};

}  // namespace seek
