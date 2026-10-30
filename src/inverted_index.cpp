#include "seek/inverted_index.hpp"

#include <limits>
#include <stdexcept>

namespace seek {

DocId InvertedIndex::add(std::span<const std::string> terms) {
    if (nextId_ == std::numeric_limits<DocId>::max()) {
        throw std::length_error("InvertedIndex::add: document id space exhausted");
    }
    const DocId id = nextId_++;
    for (const std::string& term : terms) {
        PostingList& list = index_[term];
        // `id` is the largest id in every list, so a repeat can only be at the back.
        if (list.empty() || list.back() != id) list.push_back(id);
    }
    return id;
}

const PostingList& InvertedIndex::postings(std::string_view term) const {
    static const PostingList kEmpty;
    const auto it = index_.find(term);
    return it != index_.end() ? it->second : kEmpty;
}

}  // namespace seek
