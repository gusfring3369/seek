#include "seek/posting_list.hpp"

#include <algorithm>
#include <utility>

namespace seek::postings {

PostingList intersectLinear(std::span<const DocId> a, std::span<const DocId> b) {
    PostingList result;
    result.reserve(std::min(a.size(), b.size()));
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return result;
}

PostingList intersectGalloping(std::span<const DocId> a, std::span<const DocId> b) {
    if (a.size() > b.size()) std::swap(a, b);  // iterate the shorter list, search the longer
    PostingList result;
    result.reserve(a.size());

    std::size_t lo = 0;  // every id before b[lo] is smaller than the ids still to come from a
    for (DocId id : a) {
        if (lo >= b.size()) break;
        // Exponential search: double the step until b[lo + bound] >= id or we run off the end.
        std::size_t bound = 1;
        while (lo + bound < b.size() && b[lo + bound] < id) bound *= 2;

        // The first element >= id lies in [lo + bound / 2, lo + bound].
        const std::size_t first = lo + bound / 2;
        const std::size_t last = std::min(lo + bound + 1, b.size());
        const auto window = b.subspan(first, last - first);
        const auto it = std::lower_bound(window.begin(), window.end(), id);
        lo = first + static_cast<std::size_t>(it - window.begin());

        if (lo < b.size() && b[lo] == id) {
            result.push_back(id);
            ++lo;
        }
    }
    return result;
}

PostingList intersect(std::span<const DocId> a, std::span<const DocId> b) {
    const std::size_t shorter = std::min(a.size(), b.size());
    const std::size_t longer = std::max(a.size(), b.size());
    if (shorter == 0) return {};
    if (longer / shorter >= kGallopingRatio) return intersectGalloping(a, b);
    return intersectLinear(a, b);
}

PostingList unite(std::span<const DocId> a, std::span<const DocId> b) {
    PostingList result;
    result.reserve(a.size() + b.size());
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            result.push_back(a[i++]);
        } else {
            result.push_back(b[j++]);
        }
    }
    result.insert(result.end(), a.begin() + static_cast<std::ptrdiff_t>(i), a.end());
    result.insert(result.end(), b.begin() + static_cast<std::ptrdiff_t>(j), b.end());
    return result;
}

}  // namespace seek::postings
