#include "seek/index/inverted_index.hpp"

#include <limits>
#include <stdexcept>

using namespace std;

namespace seek {

DocId InvertedIndex::add(span<const string> terms) {
    if (nextId_ == numeric_limits<DocId>::max()) {
        throw length_error("InvertedIndex::add: ran out of document ids");
    }
    const DocId id = nextId_++;
    for (const string& term : terms) {
        PostingList& list = postings_[term];
        // `id` is the largest id in every list, so a repeated term can only match the last entry.
        if (list.empty() || list.back() != id) list.push_back(id);
    }
    return id;
}

const PostingList& InvertedIndex::postings(string_view term) const {
    static const PostingList empty;
    const auto it = postings_.find(term);
    return it != postings_.end() ? it->second : empty;
}

}  // namespace seek
