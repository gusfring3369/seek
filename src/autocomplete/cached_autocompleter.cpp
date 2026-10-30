#include "seek/autocomplete/cached_autocompleter.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace std;

namespace seek {

CachedAutocompleter::CachedAutocompleter(const Autocompleter& fallback, size_t maxPrefixLength,
                                         size_t capacity)
    : fallback_(fallback), maxPrefixLength_(maxPrefixLength), capacity_(capacity) {
    if (capacity_ == 0) throw invalid_argument("CachedAutocompleter: capacity must be positive");
}

void CachedAutocompleter::onWeightChanged(string_view word, Weight newWeight) {
    // A word belongs to the lists of all its prefixes: "", "w", "wo", "wor", ...
    const size_t longest = min(maxPrefixLength_, word.size());
    for (size_t length = 0; length <= longest; ++length) {
        const string_view prefix = word.substr(0, length);
        auto it = table_.find(prefix);
        if (it == table_.end()) it = table_.emplace(string(prefix), RankedList{}).first;
        update(it->second, word, newWeight);
    }
}

void CachedAutocompleter::update(RankedList& list, string_view word, Weight newWeight) const {
    auto entry =
        find_if(list.begin(), list.end(), [&](const Suggestion& s) { return s.word == word; });

    if (entry != list.end()) {
        entry->weight = newWeight;  // already listed: just raise its weight
    } else if (list.size() < capacity_) {
        list.push_back({string(word), newWeight});  // room left: append
        entry = prev(list.end());
    } else if (Suggestion::ranksAbove(newWeight, word, list.back().weight, list.back().word)) {
        list.back() = {string(word), newWeight};  // beats the weakest: replace it
        entry = prev(list.end());
    } else {
        return;  // not good enough for this prefix
    }

    // Only `entry` changed, and it can only have moved up: bubble it into place.
    while (entry != list.begin() && entry->ranksAbove(*prev(entry))) {
        iter_swap(entry, prev(entry));
        --entry;
    }
}

vector<Suggestion> CachedAutocompleter::complete(string_view prefix, size_t k) const {
    if (!covers(prefix, k)) return fallback_.complete(prefix, k);

    const auto it = table_.find(prefix);
    if (it == table_.end()) return {};  // every word was reported, so none has this prefix
    const RankedList& list = it->second;
    const size_t count = min(k, list.size());
    return {list.begin(), list.begin() + static_cast<ptrdiff_t>(count)};
}

}  // namespace seek
