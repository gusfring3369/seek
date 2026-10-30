#pragma once

#include <cstddef>
#include <span>
#include <utility>

#include "seek/common/types.hpp"

namespace seek {

// The documents matched by one node of a query.
//
// A term's result is its posting list in the index. That list is *borrowed* (read in place), not
// copied: copying a million-id list just to gallop through a few of its entries would cost more
// than the intersection itself. Results produced by AND / OR are new lists and are *owned*.
class ResultSet {
public:
    static ResultSet borrow(const PostingList& list) { return ResultSet(&list, {}); }
    static ResultSet own(PostingList list) { return ResultSet(nullptr, std::move(list)); }

    [[nodiscard]] std::span<const DocId> ids() const {
        return borrowed_ != nullptr ? std::span<const DocId>(*borrowed_) : std::span(owned_);
    }
    [[nodiscard]] std::size_t size() const { return ids().size(); }
    [[nodiscard]] bool empty() const { return ids().empty(); }

    // The ids as a standalone list (copied only if they were borrowed).
    [[nodiscard]] PostingList toPostingList() && {
        if (borrowed_ != nullptr) return *borrowed_;
        return std::move(owned_);
    }

private:
    ResultSet(const PostingList* borrowed, PostingList owned)
        : borrowed_(borrowed), owned_(std::move(owned)) {}

    const PostingList* borrowed_;  // set when borrowing; points into the index
    PostingList owned_;            // used when borrowed_ is null
};

}  // namespace seek
