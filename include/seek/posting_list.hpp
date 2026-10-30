#pragma once

#include <cstddef>
#include <span>

#include "seek/types.hpp"

// Set operations over sorted, duplicate-free posting lists.
namespace seek::postings {

// When the longer list is at least this many times the shorter one, intersect() switches from
// the linear merge to galloping search. Chosen from bench/seek_bench (see README).
inline constexpr std::size_t kGallopingRatio = 32;

// Two-pointer merge. O(n + m).
[[nodiscard]] PostingList intersectLinear(std::span<const DocId> a, std::span<const DocId> b);

// For each id of the shorter list, exponential + binary search in the longer one, resuming
// from the previous match. O(n log(m / n)) for n <= m: far faster when one term is rare.
[[nodiscard]] PostingList intersectGalloping(std::span<const DocId> a, std::span<const DocId> b);

// Picks the cheaper strategy from the two list sizes.
[[nodiscard]] PostingList intersect(std::span<const DocId> a, std::span<const DocId> b);

// Two-pointer merge that keeps each id once. O(n + m).
[[nodiscard]] PostingList unite(std::span<const DocId> a, std::span<const DocId> b);

}  // namespace seek::postings
