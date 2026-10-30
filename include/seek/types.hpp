#pragma once

#include <cstdint>
#include <vector>

namespace seek {

// Dense, sequentially assigned document identifier.
using DocId = std::uint32_t;

// Document ids containing a term. Invariant: strictly increasing (sorted, no duplicates).
using PostingList = std::vector<DocId>;

// Popularity score of a vocabulary term (e.g. corpus frequency).
using Weight = std::uint64_t;

}  // namespace seek
