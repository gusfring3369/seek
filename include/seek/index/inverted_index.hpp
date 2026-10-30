#pragma once

#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "seek/common/string_hash.hpp"
#include "seek/common/types.hpp"

namespace seek {

// Maps every term to the sorted list of documents that contain it (its "posting list"):
//
//     "distributed" -> [0, 1, 3]
//     "storage"     -> [0, 2, 3]
//
// The index hands out document ids itself, in increasing order. A new document therefore always
// has the largest id so far, so adding it is a push_back onto each of its terms' lists, and every
// list stays sorted without ever being re-sorted.
class InvertedIndex {
public:
    // Indexes one document's terms (repeats are fine) and returns the id given to it.
    DocId add(std::span<const std::string> terms);

    // The posting list of `term`, or an empty list if no document contains it.
    [[nodiscard]] const PostingList& postings(std::string_view term) const;

    [[nodiscard]] std::size_t documentCount() const noexcept { return nextId_; }
    [[nodiscard]] std::size_t termCount() const noexcept { return postings_.size(); }

private:
    std::unordered_map<std::string, PostingList, StringHash, std::equal_to<>> postings_;
    DocId nextId_ = 0;
};

}  // namespace seek
