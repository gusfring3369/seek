#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace seek {

// Levenshtein edit distance: the fewest single-character insertions, deletions and substitutions
// that turn one string into another. Example: "kitten" -> "sitting" is 3.
//
// The classic dynamic program fills a table where
//     dp[i][j] = distance between a[0..i) and b[0..j)
//     dp[0][j] = j                         (insert j characters)
//     dp[i][0] = i                         (delete i characters)
//     dp[i][j] = min(dp[i-1][j]   + 1,     (delete a[i-1])
//                    dp[i][j-1]   + 1,     (insert b[j-1])
//                    dp[i-1][j-1] + (a[i-1] == b[j-1] ? 0 : 1))   (keep or substitute)
//
// Row i depends only on row i-1, so this class works one row at a time. That is what lets the
// SpellChecker extend a row by one character each time it steps one level down the trie.
class EditDistance {
public:
    // One row of the table: entry j is the distance from some fixed string to target[0..j).
    using Row = std::vector<std::size_t>;

    // The row for the empty string: {0, 1, 2, ..., target.size()}.
    [[nodiscard]] static Row firstRow(std::string_view target);

    // Given `previous` (the row for some string s), writes the row for s + c into `next` and
    // returns the smallest value in it.
    static std::size_t nextRow(const Row& previous, char c, std::string_view target, Row& next);

    // Distance between two whole strings. O(|a| * |b|) time, O(|b|) memory.
    [[nodiscard]] static std::size_t between(std::string_view a, std::string_view b);
};

}  // namespace seek
