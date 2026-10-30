#include "seek/autocomplete/top_k_heap.hpp"

#include <algorithm>
#include <string>

using namespace std;

namespace seek {

void TopKHeap::offer(string_view word, Weight weight) {
    if (k_ == 0) return;
    if (heap_.size() < k_) {
        heap_.push({string(word), weight});
        return;
    }
    const Suggestion& weakest = heap_.top();
    if (Suggestion::ranksAbove(weight, word, weakest.weight, weakest.word)) {
        heap_.pop();
        heap_.push({string(word), weight});
    }
}

vector<Suggestion> TopKHeap::takeBestFirst() {
    vector<Suggestion> result;
    result.reserve(heap_.size());
    while (!heap_.empty()) {  // pops weakest first...
        result.push_back(heap_.top());
        heap_.pop();
    }
    reverse(result.begin(), result.end());  // ...so reverse to get best first
    return result;
}

}  // namespace seek
