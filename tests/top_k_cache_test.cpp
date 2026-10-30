#include "seek/top_k_cache.hpp"

#include <gtest/gtest.h>

#include <random>
#include <stdexcept>

#include "seek/trie.hpp"

namespace seek {
namespace {

TEST(TopKCacheTest, KeepsBestFirstListsPerPrefix) {
    TopKCache cache(2, 2);
    cache.update("apple", 5);
    cache.update("apricot", 9);
    cache.update("avocado", 7);

    EXPECT_EQ(*cache.lookup("a", 2), (std::vector<Suggestion>{{"apricot", 9}, {"avocado", 7}}));
    EXPECT_EQ(*cache.lookup("ap", 2), (std::vector<Suggestion>{{"apricot", 9}, {"apple", 5}}));
    EXPECT_EQ(*cache.lookup("ap", 1), (std::vector<Suggestion>{{"apricot", 9}}));
}

TEST(TopKCacheTest, WeightIncreaseMovesWordUpAndCanEvict) {
    TopKCache cache(1, 2);
    cache.update("ant", 1);
    cache.update("ape", 2);
    cache.update("arc", 3);  // evicts "ant"
    cache.update("ape", 10);
    EXPECT_EQ(*cache.lookup("a", 2), (std::vector<Suggestion>{{"ape", 10}, {"arc", 3}}));
}

TEST(TopKCacheTest, DeclinesWhatItCannotAnswer) {
    TopKCache cache(2, 3);
    cache.update("abc", 1);
    EXPECT_FALSE(cache.lookup("abc", 1).has_value());             // prefix longer than cached
    EXPECT_FALSE(cache.lookup("a", 4).has_value());               // k above capacity
    EXPECT_EQ(cache.lookup("zz", 3), std::vector<Suggestion>{});  // covered, no words
}

TEST(TopKCacheTest, RejectsZeroCapacity) { EXPECT_THROW(TopKCache(3, 0), std::invalid_argument); }

// Property: after any sequence of weight increases, the cache agrees with a full trie walk.
TEST(TopKCachePropertyTest, MatchesTrieWalkUnderRepeatedInserts) {
    constexpr std::size_t kCapacity = 5;
    std::mt19937 rng(99);
    std::uniform_int_distribution<int> letter('a', 'c');
    std::uniform_int_distribution<std::size_t> length(1, 5);
    std::uniform_int_distribution<Weight> weight(1, 4);

    Trie trie;
    TopKCache cache(3, kCapacity);
    for (int i = 0; i < 3000; ++i) {
        std::string word;
        for (std::size_t n = length(rng); n > 0; --n) {
            word.push_back(static_cast<char>(letter(rng)));
        }
        cache.update(word, trie.insert(word, weight(rng)));
    }

    for (const std::string prefix : {"", "a", "b", "ab", "cab", "ccc"}) {
        for (std::size_t k = 1; k <= kCapacity; ++k) {
            EXPECT_EQ(*cache.lookup(prefix, k), topKCompletions(trie, prefix, k))
                << "prefix=" << prefix << " k=" << k;
        }
    }
}

}  // namespace
}  // namespace seek
