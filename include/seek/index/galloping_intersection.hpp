#pragma once

#include "seek/index/intersection_strategy.hpp"

namespace seek {

// Intersection for lists of very different sizes, e.g. a rare term AND a common one.
//
// For each id of the short list, search for it in the long list starting where the previous
// search stopped: first take steps of 1, 2, 4, 8, ... until we pass the id (galloping), then
// binary-search inside the last step. O(n log(m / n)) for n <= m, so intersecting 100 ids with
// 1,000,000 ids touches about a thousand elements instead of a million.
class GallopingIntersection final : public IntersectionStrategy {
public:
    [[nodiscard]] PostingList intersect(std::span<const DocId> a,
                                        std::span<const DocId> b) const override;
};

}  // namespace seek
