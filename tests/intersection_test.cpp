#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <random>
#include <set>

#include "seek/index/adaptive_intersection.hpp"
#include "seek/index/galloping_intersection.hpp"
#include "seek/index/posting_list_union.hpp"
#include "seek/index/two_pointer_intersection.hpp"

using namespace std;
using namespace seek;

TEST(IntersectionTest, TwoPointer) {
    EXPECT_EQ(TwoPointerIntersection().intersect(PostingList{1, 3, 5, 7}, PostingList{3, 4, 5, 8}),
              (PostingList{3, 5}));
}

TEST(IntersectionTest, GallopingWorksWithEitherArgumentOrder) {
    const PostingList shortList{10, 5000, 99999};
    PostingList longList;
    for (DocId id = 0; id < 100000; id += 5) longList.push_back(id);
    const GallopingIntersection galloping;
    EXPECT_EQ(galloping.intersect(shortList, longList), (PostingList{10, 5000}));
    EXPECT_EQ(galloping.intersect(longList, shortList), (PostingList{10, 5000}));
}

TEST(IntersectionTest, EmptyInputs) {
    const PostingList empty;
    const PostingList some{1, 2};
    EXPECT_TRUE(AdaptiveIntersection().intersect(empty, some).empty());
    EXPECT_TRUE(GallopingIntersection().intersect(some, empty).empty());
    EXPECT_EQ(unionOf(empty, some), some);
    EXPECT_EQ(unionOf(some, empty), some);
}

TEST(UnionTest, KeepsEachIdOnce) {
    EXPECT_EQ(unionOf(PostingList{1, 3, 5}, PostingList{2, 3, 6}), (PostingList{1, 2, 3, 5, 6}));
}

static PostingList randomPostingList(mt19937& rng, size_t size, DocId universe) {
    uniform_int_distribution<DocId> id(0, universe - 1);
    set<DocId> ids;
    while (ids.size() < size) ids.insert(id(rng));
    return {ids.begin(), ids.end()};
}

// Property test: every strategy agrees with the standard library, for list sizes from equal to
// very skewed.
TEST(IntersectionPropertyTest, AllStrategiesMatchTheStandardLibrary) {
    const TwoPointerIntersection twoPointer;
    const GallopingIntersection galloping;
    const AdaptiveIntersection adaptive;
    const IntersectionStrategy* strategies[] = {&twoPointer, &galloping, &adaptive};

    mt19937 rng(1234);
    const pair<size_t, size_t> sizes[] = {{0, 10},     {1, 1},       {5, 500}, {50, 60},
                                          {200, 4000}, {1000, 1000}, {3, 9000}};
    for (const auto& [n, m] : sizes) {
        const PostingList a = randomPostingList(rng, n, 10000);
        const PostingList b = randomPostingList(rng, m, 10000);

        PostingList expectedAnd;
        set_intersection(a.begin(), a.end(), b.begin(), b.end(), back_inserter(expectedAnd));
        PostingList expectedOr;
        set_union(a.begin(), a.end(), b.begin(), b.end(), back_inserter(expectedOr));

        for (const IntersectionStrategy* strategy : strategies) {
            EXPECT_EQ(strategy->intersect(a, b), expectedAnd);
            EXPECT_EQ(strategy->intersect(b, a), expectedAnd);
        }
        EXPECT_EQ(unionOf(a, b), expectedOr);
    }
}
