#pragma once

#include "seek/index/intersection_strategy.hpp"

namespace seek {

// Classic merge-style intersection of two sorted lists:
//   a[i] == b[j]  -> match, advance both
//   a[i] <  b[j]  -> advance i
//   a[i] >  b[j]  -> advance j
// O(n + m). The best choice when both lists have similar sizes.
class TwoPointerIntersection final : public IntersectionStrategy {
public:
    [[nodiscard]] PostingList intersect(std::span<const DocId> a,
                                        std::span<const DocId> b) const override;
};

}  // namespace seek
