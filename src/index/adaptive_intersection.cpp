#include "seek/index/adaptive_intersection.hpp"

#include <algorithm>

using namespace std;

namespace seek {

PostingList AdaptiveIntersection::intersect(span<const DocId> a, span<const DocId> b) const {
    const size_t shorter = min(a.size(), b.size());
    const size_t longer = max(a.size(), b.size());
    if (shorter == 0) return {};
    if (longer / shorter >= kGallopingRatio) return galloping_.intersect(a, b);
    return twoPointer_.intersect(a, b);
}

}  // namespace seek
