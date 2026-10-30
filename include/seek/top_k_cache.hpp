#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "seek/autocomplete.hpp"
#include "seek/types.hpp"

namespace seek {

// Precomputed top-k completions for short prefixes.
//
// Short prefixes are the slow autocomplete case (the prefix "s" covers a large part of the
// vocabulary) and also the most frequent one, since every query starts as a short prefix. This
// cache stores the best `capacity` words for every prefix of up to `maxPrefixLength` characters,
// turning those lookups into a hash probe. Longer prefixes have small subtrees and are served
// by the trie walk.
//
// The cache is updated on every insert in O(maxPrefixLength * capacity). That is exact because
// weights only grow: raising one word's weight can only move that word into or up a top-k list.
class TopKCache {
public:
    TopKCache(std::size_t maxPrefixLength, std::size_t capacity);

    // Records that `word` now has total weight `weight`. Precondition: `weight` is not lower
    // than any weight previously recorded for `word`.
    void update(std::string_view word, Weight weight);

    // Best-first top-k for `prefix`, or nullopt if the prefix is too long or k exceeds the
    // capacity (the caller then falls back to the trie).
    [[nodiscard]] std::optional<std::vector<Suggestion>> lookup(std::string_view prefix,
                                                                std::size_t k) const;

    [[nodiscard]] std::size_t prefixCount() const noexcept { return entries_.size(); }

private:
    struct PrefixHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view s) const noexcept {
            return std::hash<std::string_view>{}(s);
        }
    };

    void updateList(std::vector<Suggestion>& list, std::string_view word, Weight weight) const;

    std::size_t maxPrefixLength_;
    std::size_t capacity_;
    // prefix -> up to capacity_ suggestions, best first
    std::unordered_map<std::string, std::vector<Suggestion>, PrefixHash, std::equal_to<>> entries_;
};

}  // namespace seek
