#pragma once

#include <cstddef>

#include "seek/index/galloping_intersection.hpp"
#include "seek/index/two_pointer_intersection.hpp"

namespace seek {

// Uses galloping when one list is much longer than the other, and the two-pointer merge
// otherwise. This is the strategy the search engine uses.
class AdaptiveIntersection final : public IntersectionStrategy {
public:
    // Galloping wins once the longer list is about this many times the shorter one
    // (crossover measured with bench/seek_bench; see README).
    static constexpr std::size_t kGallopingRatio = 32;

    [[nodiscard]] PostingList intersect(std::span<const DocId> a,
                                        std::span<const DocId> b) const override;

private:
    TwoPointerIntersection twoPointer_;
    GallopingIntersection galloping_;
};

}  // namespace seek
