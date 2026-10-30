#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "seek/common/types.hpp"

namespace seek {

// Keeps the original text of every document so search results can be displayed.
class DocumentStore {
public:
    // Stores `text` and returns its id (0, 1, 2, ... in order of arrival).
    DocId add(std::string text);

    // Throws std::out_of_range for an id that was never returned by add().
    [[nodiscard]] const std::string& get(DocId id) const { return texts_.at(id); }

    [[nodiscard]] std::size_t size() const noexcept { return texts_.size(); }

private:
    std::vector<std::string> texts_;  // texts_[id] is document `id`
};

}  // namespace seek
