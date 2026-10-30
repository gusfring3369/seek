#include "seek/top_k_cache.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace seek {

TopKCache::TopKCache(std::size_t maxPrefixLength, std::size_t capacity)
    : maxPrefixLength_(maxPrefixLength), capacity_(capacity) {
    if (capacity_ == 0) throw std::invalid_argument("TopKCache: capacity must be positive");
}

void TopKCache::update(std::string_view word, Weight weight) {
    const std::size_t longest = std::min(maxPrefixLength_, word.size());
    for (std::size_t length = 0; length <= longest; ++length) {
        const std::string_view prefix = word.substr(0, length);
        auto it = entries_.find(prefix);
        if (it == entries_.end()) {
            it = entries_.emplace(std::string(prefix), std::vector<Suggestion>{}).first;
        }
        updateList(it->second, word, weight);
    }
}

void TopKCache::updateList(std::vector<Suggestion>& list, std::string_view word,
                           Weight weight) const {
    auto it =
        std::find_if(list.begin(), list.end(), [&](const Suggestion& s) { return s.word == word; });
    if (it != list.end()) {
        it->weight = weight;
    } else if (list.size() < capacity_) {
        list.push_back({std::string(word), weight});
        it = std::prev(list.end());
    } else if (ranksAbove(weight, word, list.back().weight, list.back().word)) {
        list.back() = {std::string(word), weight};
        it = std::prev(list.end());
    } else {
        return;  // not good enough for this prefix
    }
    // Only this entry changed, and only upwards: bubble it into place.
    while (it != list.begin() && ranksAbove(*it, *std::prev(it))) {
        std::iter_swap(it, std::prev(it));
        --it;
    }
}

std::optional<std::vector<Suggestion>> TopKCache::lookup(std::string_view prefix,
                                                         std::size_t k) const {
    if (prefix.size() > maxPrefixLength_ || k > capacity_) return std::nullopt;
    const auto it = entries_.find(prefix);
    if (it == entries_.end()) return std::vector<Suggestion>{};  // no word has this prefix
    const std::vector<Suggestion>& list = it->second;
    return std::vector<Suggestion>(
        list.begin(), list.begin() + static_cast<std::ptrdiff_t>(std::min(k, list.size())));
}

}  // namespace seek
