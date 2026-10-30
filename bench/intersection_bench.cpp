#include <format>
#include <iostream>

#include "bench_utils.hpp"
#include "benchmarks.hpp"
#include "seek/index/galloping_intersection.hpp"
#include "seek/index/two_pointer_intersection.hpp"

using namespace std;

namespace seek::bench {

// A sorted list of about `size` distinct random ids below `universe`.
static PostingList randomPostingList(mt19937& rng, size_t size, DocId universe) {
    bernoulli_distribution keep(static_cast<double>(size) / static_cast<double>(universe));
    PostingList list;
    for (DocId id = 0; id < universe; ++id) {
        if (keep(rng)) list.push_back(id);
    }
    return list;
}

void benchmarkIntersection(mt19937& rng) {
    constexpr DocId kUniverse = 10'000'000;
    constexpr size_t kLongListSize = 1'000'000;
    constexpr int kRepetitions = 15;

    const TwoPointerIntersection twoPointer;
    const GallopingIntersection galloping;
    const PostingList longList = randomPostingList(rng, kLongListSize, kUniverse);

    cout << format("\n### AND intersection against a {}-id posting list\n\n", longList.size())
         << "| short list | size ratio | two-pointer (µs) | galloping (µs) | faster |\n"
         << "|---:|---:|---:|---:|:---|\n";

    for (size_t shortSize : {1'000'000u, 250'000u, 100'000u, 50'000u, 30'000u, 20'000u, 15'000u,
                             10'000u, 1'000u, 100u}) {
        const PostingList shortList = randomPostingList(rng, shortSize, kUniverse);
        vector<double> twoPointerTimes;
        vector<double> gallopingTimes;
        for (int r = 0; r < kRepetitions; ++r) {
            twoPointerTimes.push_back(
                timeMicros([&] { sink += twoPointer.intersect(shortList, longList).size(); }));
            gallopingTimes.push_back(
                timeMicros([&] { sink += galloping.intersect(shortList, longList).size(); }));
        }
        const double twoPointerMedian = summarize(twoPointerTimes).p50;
        const double gallopingMedian = summarize(gallopingTimes).p50;
        const double ratio =
            static_cast<double>(longList.size()) / static_cast<double>(shortList.size());
        cout << format("| {} | {:.0f}x | {:.0f} | {:.0f} | {} |\n", shortList.size(), ratio,
                       twoPointerMedian, gallopingMedian,
                       gallopingMedian < twoPointerMedian ? "galloping" : "two-pointer");
    }
}

}  // namespace seek::bench
