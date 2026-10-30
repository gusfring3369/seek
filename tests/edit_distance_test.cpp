#include "seek/spellcheck/edit_distance.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>

using namespace std;
using namespace seek;

TEST(EditDistanceTest, KnownDistances) {
    EXPECT_EQ(EditDistance::between("kitten", "sitting"), 3u);
    EXPECT_EQ(EditDistance::between("distrbuted", "distributed"), 1u);
    EXPECT_EQ(EditDistance::between("", "abc"), 3u);
    EXPECT_EQ(EditDistance::between("abc", ""), 3u);
    EXPECT_EQ(EditDistance::between("same", "same"), 0u);
}

TEST(EditDistanceTest, FirstRowCountsInsertions) {
    EXPECT_EQ(EditDistance::firstRow("abc"), (EditDistance::Row{0, 1, 2, 3}));
}

TEST(EditDistanceTest, NextRowReturnsTheRowMinimum) {
    EditDistance::Row next;
    const size_t smallest = EditDistance::nextRow(EditDistance::firstRow("abc"), 'b', "abc", next);
    EXPECT_EQ(next, (EditDistance::Row{1, 1, 1, 2}));
    EXPECT_EQ(smallest, 1u);
}

// Textbook full-table implementation, written independently as a reference.
static size_t referenceDistance(const string& a, const string& b) {
    vector<vector<size_t>> dp(a.size() + 1, vector<size_t>(b.size() + 1));
    for (size_t i = 0; i <= a.size(); ++i) dp[i][0] = i;
    for (size_t j = 0; j <= b.size(); ++j) dp[0][j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
        for (size_t j = 1; j <= b.size(); ++j) {
            dp[i][j] = min({dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                            dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1)});
        }
    }
    return dp[a.size()][b.size()];
}

TEST(EditDistancePropertyTest, MatchesTheFullTable) {
    mt19937 rng(3);
    uniform_int_distribution<int> letter('a', 'c');
    uniform_int_distribution<size_t> length(0, 8);
    auto randomWord = [&] {
        string word;
        for (size_t n = length(rng); n > 0; --n) word.push_back(static_cast<char>(letter(rng)));
        return word;
    };
    for (int i = 0; i < 500; ++i) {
        const string a = randomWord();
        const string b = randomWord();
        EXPECT_EQ(EditDistance::between(a, b), referenceDistance(a, b)) << a << " vs " << b;
    }
}
