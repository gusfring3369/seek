#include "seek/posting_list.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <random>
#include <set>

namespace seek::postings {
namespace {

TEST(PostingListTest, IntersectLinear) {
    EXPECT_EQ(intersectLinear(PostingList{1, 3, 5, 7}, PostingList{3, 4, 5, 8}),
              (PostingList{3, 5}));
}

TEST(PostingListTest, IntersectGallopingIsSymmetric) {
    const PostingList small{10, 5000, 99999};
    PostingList large;
    for (DocId id = 0; id < 100000; id += 5) large.push_back(id);
    EXPECT_EQ(intersectGalloping(small, large), (PostingList{10, 5000}));
    EXPECT_EQ(intersectGalloping(large, small), (PostingList{10, 5000}));
}

TEST(PostingListTest, UniteKeepsEachIdOnce) {
    EXPECT_EQ(unite(PostingList{1, 3, 5}, PostingList{2, 3, 6}), (PostingList{1, 2, 3, 5, 6}));
}

TEST(PostingListTest, EmptyInputs) {
    const PostingList empty;
    const PostingList some{1, 2};
    EXPECT_TRUE(intersect(empty, some).empty());
    EXPECT_TRUE(intersectGalloping(some, empty).empty());
    EXPECT_EQ(unite(empty, some), some);
    EXPECT_EQ(unite(some, empty), some);
}

PostingList randomList(std::mt19937& rng, std::size_t size, DocId universe) {
    std::uniform_int_distribution<DocId> id(0, universe - 1);
    std::set<DocId> ids;
    while (ids.size() < size) ids.insert(id(rng));
    return {ids.begin(), ids.end()};
}

// Property: every strategy agrees with the standard library on random inputs of skewed sizes.
TEST(PostingListPropertyTest, MatchesStandardAlgorithms) {
    std::mt19937 rng(1234);
    const std::pair<std::size_t, std::size_t> sizes[] = {
        {0, 10}, {1, 1}, {5, 500}, {50, 60}, {200, 4000}, {1000, 1000}, {3, 9000}};
    for (const auto& [n, m] : sizes) {
        const PostingList a = randomList(rng, n, 10000);
        const PostingList b = randomList(rng, m, 10000);

        PostingList expectedAnd;
        std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                              std::back_inserter(expectedAnd));
        PostingList expectedOr;
        std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(expectedOr));

        EXPECT_EQ(intersectLinear(a, b), expectedAnd);
        EXPECT_EQ(intersectGalloping(a, b), expectedAnd);
        EXPECT_EQ(intersectGalloping(b, a), expectedAnd);
        EXPECT_EQ(intersect(a, b), expectedAnd);
        EXPECT_EQ(unite(a, b), expectedOr);
    }
}

}  // namespace
}  // namespace seek::postings
