#pragma once

#include <cstdint>
#include <vector>

namespace seek {

// Identifier of an indexed document. Assigned in order: 0, 1, 2, ...
using DocId = std::uint32_t;

// The ids of the documents that contain one term.
// Always sorted in increasing order with no duplicates; every algorithm on it relies on that.
using PostingList = std::vector<DocId>;

// How popular a word is (for example, how often it appears in the corpus).
using Weight = std::uint64_t;

}  // namespace seek
