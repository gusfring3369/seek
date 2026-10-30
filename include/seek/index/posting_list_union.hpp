#pragma once

#include <span>

#include "seek/common/types.hpp"

namespace seek {

// Ids present in `a` or `b` (each once), in increasing order: the result of an OR.
// Two-pointer merge of two sorted lists, O(n + m).
[[nodiscard]] PostingList unionOf(std::span<const DocId> a, std::span<const DocId> b);

}  // namespace seek
