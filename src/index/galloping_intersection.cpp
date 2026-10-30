#include "seek/index/galloping_intersection.hpp"

#include <algorithm>

using namespace std;

namespace seek {

PostingList GallopingIntersection::intersect(span<const DocId> a, span<const DocId> b) const {
    span<const DocId> shorter = a.size() <= b.size() ? a : b;
    span<const DocId> longer = a.size() <= b.size() ? b : a;

    PostingList result;
    result.reserve(shorter.size());
    size_t start = 0;  // everything before longer[start] is smaller than the ids still to find

    for (DocId id : shorter) {
        if (start >= longer.size()) break;

        // Gallop: grow the step until longer[start + step] >= id or we run off the end.
        size_t step = 1;
        while (start + step < longer.size() && longer[start + step] < id) step *= 2;

        // The first element >= id is now somewhere in [start + step / 2, start + step].
        const size_t low = start + step / 2;
        const size_t high = min(start + step + 1, longer.size());
        const auto window = longer.subspan(low, high - low);
        start = low +
                static_cast<size_t>(lower_bound(window.begin(), window.end(), id) - window.begin());

        if (start < longer.size() && longer[start] == id) {
            result.push_back(id);
            ++start;
        }
    }
    return result;
}

}  // namespace seek
