#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace seek {

// Turns raw text into search terms.
//
// A term is a run of ASCII letters or digits, lowercased. Every other character separates terms:
//     "Key-Value B+Tree" -> {"key", "value", "b", "tree"}
// Documents and queries go through the same tokenizer, so they always agree on what a term is.
class Tokenizer {
public:
    // Splits `text` into terms.
    [[nodiscard]] std::vector<std::string> tokenize(std::string_view text) const;

    // Lowercases a single word, e.g. an autocomplete prefix or a word to spell-check.
    [[nodiscard]] std::string normalize(std::string_view word) const;

private:
    static bool isTermCharacter(char c);
    static char toLower(char c);
};

}  // namespace seek
