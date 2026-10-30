#pragma once

#include <span>

#include "seek/common/types.hpp"

namespace seek {

// Interface (Strategy pattern): how to intersect two sorted posting lists, i.e. find the
// documents that contain both terms of an AND.
//
// Implementations:
//   TwoPointerIntersection  walks both lists once                 O(n + m)
//   GallopingIntersection   searches the long list for each id    O(n log(m / n)), n <= m
//   AdaptiveIntersection    picks one of the two from the list sizes
class IntersectionStrategy {
public:
    IntersectionStrategy() = default;
    IntersectionStrategy(const IntersectionStrategy&) = delete;
    IntersectionStrategy& operator=(const IntersectionStrategy&) = delete;
    virtual ~IntersectionStrategy() = default;

    // Ids present in both `a` and `b`, in increasing order. Both inputs must be sorted.
    [[nodiscard]] virtual PostingList intersect(std::span<const DocId> a,
                                                std::span<const DocId> b) const = 0;
};

}  // namespace seek
