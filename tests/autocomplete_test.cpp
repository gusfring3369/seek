#include "seek/autocomplete.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>

namespace seek {
namespace {

class AutocompleteTest : public ::testing::Test {
protected:
    void SetUp() override {
        trie_.insert("algorithm", 450);
        trie_.insert("algorithms", 820);
        trie_.insert("algebra", 120);
        trie_.insert("algo", 610);
        trie_.insert("alligator", 90);
        trie_.insert("alpha", 340);
    }

    Trie trie_;
};

TEST_F(AutocompleteTest, ReturnsTopKByWeightDescending) {
    const std::vector<Suggestion> expected = {
        {"algorithms", 820}, {"algo", 610}, {"algorithm", 450}};
    EXPECT_EQ(topKCompletions(trie_, "alg", 3), expected);
}

TEST_F(AutocompleteTest, PrefixThatIsItselfAWordIsIncluded) {
    const auto result = topKCompletions(trie_, "algo", 10);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[1].word, "algo");
}

TEST_F(AutocompleteTest, ReturnsFewerThanKWhenSubtreeIsSmall) {
    EXPECT_EQ(topKCompletions(trie_, "alp", 5).size(), 1u);
}

TEST_F(AutocompleteTest, UnknownPrefixAndZeroKReturnNothing) {
    EXPECT_TRUE(topKCompletions(trie_, "xyz", 5).empty());
    EXPECT_TRUE(topKCompletions(trie_, "al", 0).empty());
}

TEST(AutocompleteTieTest, EqualWeightsAreOrderedAlphabetically) {
    Trie trie;
    for (const char* word : {"cd", "ca", "cc", "cb"}) trie.insert(word, 7);
    const std::vector<Suggestion> expected = {{"ca", 7}, {"cb", 7}};
    EXPECT_EQ(topKCompletions(trie, "c", 2), expected);
}

// Property: the bounded heap agrees with "collect everything, sort, truncate".
TEST(AutocompletePropertyTest, MatchesFullSortOnRandomVocabulary) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> letter('a', 'd');
    std::uniform_int_distribution<std::size_t> length(1, 6);
    std::uniform_int_distribution<Weight> weight(1, 20);

    Trie trie;
    std::map<std::string, Weight> reference;
    for (int i = 0; i < 2000; ++i) {
        std::string word;
        for (std::size_t n = length(rng); n > 0; --n) {
            word.push_back(static_cast<char>(letter(rng)));
        }
        const Weight w = weight(rng);
        trie.insert(word, w);
        reference[word] += w;
    }

    for (const std::string prefix : {"", "a", "b", "ab", "dca", "ccc"}) {
        for (std::size_t k : {1u, 5u, 50u}) {
            std::vector<Suggestion> expected;
            for (const auto& [word, w] : reference) {
                if (word.starts_with(prefix)) expected.push_back({word, w});
            }
            std::sort(expected.begin(), expected.end(), [](const auto& a, const auto& b) {
                return a.weight != b.weight ? a.weight > b.weight : a.word < b.word;
            });
            if (expected.size() > k) expected.resize(k);
            EXPECT_EQ(topKCompletions(trie, prefix, k), expected) << "prefix=" << prefix;
        }
    }
}

}  // namespace
}  // namespace seek
