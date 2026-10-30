#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace seek {

// Splits text into terms: maximal runs of ASCII letters/digits, lowercased.
// Every other byte is a separator, so "Key-Value B+Tree" -> {"key", "value", "b", "tree"}.
[[nodiscard]] std::vector<std::string> tokenize(std::string_view text);

// Lowercases ASCII letters and leaves every other byte untouched.
[[nodiscard]] std::string toLowerAscii(std::string_view text);

}  // namespace seek
