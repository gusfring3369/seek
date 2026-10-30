#include "seek/autocomplete/top_k_heap.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace seek;

TEST(TopKHeapTest, KeepsTheKBestBestFirst) {
    TopKHeap heap(2);
    heap.offer("low", 1);
    heap.offer("high", 9);
    heap.offer("mid", 5);
    EXPECT_EQ(heap.takeBestFirst(), (vector<Suggestion>{{"high", 9}, {"mid", 5}}));
}

TEST(TopKHeapTest, BreaksTiesAlphabetically) {
    TopKHeap heap(2);
    for (const char* word : {"d", "a", "c", "b"}) heap.offer(word, 7);
    EXPECT_EQ(heap.takeBestFirst(), (vector<Suggestion>{{"a", 7}, {"b", 7}}));
}

TEST(TopKHeapTest, ZeroCapacityKeepsNothing) {
    TopKHeap heap(0);
    heap.offer("word", 3);
    EXPECT_TRUE(heap.takeBestFirst().empty());
}
