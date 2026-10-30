#pragma once

#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "seek/types.hpp"

namespace seek {

// Term -> posting list of the documents that contain it.
//
// The index assigns document ids itself, sequentially. Because a new document always has the
// largest id so far, adding it is a push_back on each of its terms' lists, and every posting
// list stays sorted and duplicate-free by construction, which the set operations rely on.
class InvertedIndex {
public:
    // Indexes one document's terms (repeats allowed) and returns the id assigned to it.
    DocId add(std::span<const std::string> terms);

    // Posting list of `term`, or an empty list if the term never occurred.
    [[nodiscard]] const PostingList& postings(std::string_view term) const;

    [[nodiscard]] std::size_t documentCount() const noexcept { return nextId_; }
    [[nodiscard]] std::size_t termCount() const noexcept { return index_.size(); }

private:
    // Transparent hash so lookups by string_view do not allocate a std::string.
    struct TermHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view term) const noexcept {
            return std::hash<std::string_view>{}(term);
        }
    };

    std::unordered_map<std::string, PostingList, TermHash, std::equal_to<>> index_;
    DocId nextId_ = 0;
};

}  // namespace seek
