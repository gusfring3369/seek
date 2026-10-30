#include "seek/index/posting_list_union.hpp"

using namespace std;

namespace seek {

PostingList unionOf(span<const DocId> a, span<const DocId> b) {
    PostingList result;
    result.reserve(a.size() + b.size());
    size_t i = 0;
    size_t j = 0;
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
    // One list is used up; whatever is left in the other is larger than everything so far.
    for (; i < a.size(); ++i) result.push_back(a[i]);
    for (; j < b.size(); ++j) result.push_back(b[j]);
    return result;
}

}  // namespace seek
