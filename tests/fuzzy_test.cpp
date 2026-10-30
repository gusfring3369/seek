#include "seek/fuzzy.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <map>
#include <random>

namespace seek {
namespace {

// Textbook full-matrix Levenshtein distance, used as the reference implementation.
std::size_t levenshtein(std::string_view a, std::string_view b) {
    std::vector<std::vector<std::size_t>> dp(a.size() + 1, std::vector<std::size_t>(b.size() + 1));
    for (std::size_t i = 0; i <= a.size(); ++i) dp[i][0] = i;
    for (std::size_t j = 0; j <= b.size(); ++j) dp[0][j] = j;
    for (std::size_t i = 1; i <= a.size(); ++i) {
        for (std::size_t j = 1; j <= b.size(); ++j) {
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                                 dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1)});
        }
    }
    return dp[a.size()][b.size()];
}

std::vector<std::string> words(const std::vector<FuzzyMatch>& matches) {
    std::vector<std::string> out;
    for (const auto& m : matches) out.push_back(m.word);
    return out;
}

TEST(FuzzyTest, FindsMissingCharacterTypo) {
    Trie trie;
    trie.insert("distributed", 10);
    trie.insert("distribute", 3);
    trie.insert("database", 8);

    const auto matches = fuzzySearch(trie, "distrbuted", 2);
    ASSERT_EQ(matches.size(), 2u);
    EXPECT_EQ(matches[0], (FuzzyMatch{"distributed", 1, 10}));
    EXPECT_EQ(matches[1], (FuzzyMatch{"distribute", 2, 3}));
}

TEST(FuzzyTest, HandlesSubstitutionInsertionDeletion) {
    Trie trie;
    for (const char* w : {"cat", "cart", "at", "bat", "dog"}) trie.insert(w);
    EXPECT_EQ(words(fuzzySearch(trie, "cat", 0)), (std::vector<std::string>{"cat"}));
    EXPECT_EQ(words(fuzzySearch(trie, "cat", 1)),
              (std::vector<std::string>{"cat", "at", "bat", "cart"}));
}

TEST(FuzzyTest, OrdersByDistanceThenWeightThenWord) {
    Trie trie;
    trie.insert("bat", 1);
    trie.insert("cab", 5);
    trie.insert("hat", 5);
    const auto matches = fuzzySearch(trie, "cat", 1);
    EXPECT_EQ(words(matches), (std::vector<std::string>{"cab", "hat", "bat"}));
}

TEST(FuzzyTest, EmptyQueryMatchesShortWords) {
    Trie trie;
    trie.insert("a");
    trie.insert("ab");
    trie.insert("abc");
    EXPECT_EQ(words(fuzzySearch(trie, "", 2)), (std::vector<std::string>{"a", "ab"}));
}

TEST(FuzzyTest, PruningSkipsMostOfTheTrie) {
    Trie trie;
    trie.insert("zebra");
    for (char c = 'a'; c <= 'y'; ++c) trie.insert(std::string(1, c) + "qqqqqqqqqqqq");
    FuzzyStats stats;
    const auto matches = fuzzySearch(trie, "zebra", 1, &stats);
    EXPECT_EQ(words(matches), (std::vector<std::string>{"zebra"}));
    EXPECT_LT(stats.nodesVisited, trie.nodeCount() / 4);
    EXPECT_EQ(stats.dpCellsComputed, stats.nodesVisited * 5);
}

// Property: trie-pruned search returns exactly what brute force over every word returns.
TEST(FuzzyPropertyTest, MatchesBruteForceOnRandomVocabulary) {
    std::mt19937 rng(7);
    std::uniform_int_distribution<int> letter('a', 'e');
    std::uniform_int_distribution<std::size_t> length(1, 7);
    auto randomWord = [&] {
        std::string w;
        for (std::size_t n = length(rng); n > 0; --n) w.push_back(static_cast<char>(letter(rng)));
        return w;
    };

    Trie trie;
    std::map<std::string, Weight> vocabulary;
    for (int i = 0; i < 1500; ++i) {
        const std::string w = randomWord();
        trie.insert(w);
        ++vocabulary[w];
    }

    for (int q = 0; q < 60; ++q) {
        const std::string query = randomWord();
        for (std::size_t maxDistance : {0u, 1u, 2u, 3u}) {
            std::vector<FuzzyMatch> expected;
            for (const auto& [word, weight] : vocabulary) {
                const std::size_t d = levenshtein(word, query);
                if (d <= maxDistance) expected.push_back({word, d, weight});
            }
            std::sort(expected.begin(), expected.end(), [](const auto& a, const auto& b) {
                if (a.distance != b.distance) return a.distance < b.distance;
                if (a.weight != b.weight) return a.weight > b.weight;
                return a.word < b.word;
            });
            EXPECT_EQ(fuzzySearch(trie, query, maxDistance), expected)
                << "query=" << query << " maxDistance=" << maxDistance;
        }
    }
}

}  // namespace
}  // namespace seek
