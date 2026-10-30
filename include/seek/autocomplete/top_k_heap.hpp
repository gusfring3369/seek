#pragma once

#include <cstddef>
#include <queue>
#include <string_view>
#include <vector>

#include "seek/autocomplete/suggestion.hpp"

namespace seek {

// Keeps the k best suggestions out of a stream of candidates, using O(k) memory.
//
// Why a *min*-heap to find the *largest* weights? The heap's top is the weakest of the current
// top k. A new candidate only has to beat that one element to get in: one comparison, then an
// O(log k) pop + push. Sorting all M candidates would cost O(M log M) time and O(M) memory.
class TopKHeap {
public:
    explicit TopKHeap(std::size_t k) : k_(k) {}

    // Considers one candidate. The word is copied only if it makes it into the top k.
    void offer(std::string_view word, Weight weight);

    // Returns the kept suggestions, best first, and leaves the heap empty.
    [[nodiscard]] std::vector<Suggestion> takeBestFirst();

private:
    // Heap order: the element that ranks lowest ends up on top.
    struct WeakestOnTop {
        bool operator()(const Suggestion& a, const Suggestion& b) const { return a.ranksAbove(b); }
    };

    std::size_t k_;
    std::priority_queue<Suggestion, std::vector<Suggestion>, WeakestOnTop> heap_;
};

}  // namespace seek
