#include "seek/index/two_pointer_intersection.hpp"

#include <algorithm>

using namespace std;

namespace seek {

PostingList TwoPointerIntersection::intersect(span<const DocId> a, span<const DocId> b) const {
    PostingList result;
    result.reserve(min(a.size(), b.size()));
    size_t i = 0;
    size_t j = 0;
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

}  // namespace seek
