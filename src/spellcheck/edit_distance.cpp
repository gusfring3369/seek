#include "seek/spellcheck/edit_distance.hpp"

#include <algorithm>
#include <numeric>

using namespace std;

namespace seek {

EditDistance::Row EditDistance::firstRow(string_view target) {
    Row row(target.size() + 1);
    iota(row.begin(), row.end(), size_t{0});
    return row;
}

size_t EditDistance::nextRow(const Row& previous, char c, string_view target, Row& next) {
    next.resize(target.size() + 1);
    next[0] = previous[0] + 1;  // delete every character so far
    size_t smallest = next[0];
    for (size_t j = 1; j <= target.size(); ++j) {
        const size_t deleteCost = previous[j] + 1;
        const size_t insertCost = next[j - 1] + 1;
        const size_t substituteCost = previous[j - 1] + (target[j - 1] == c ? 0 : 1);
        next[j] = min({deleteCost, insertCost, substituteCost});
        smallest = min(smallest, next[j]);
    }
    return smallest;
}

size_t EditDistance::between(string_view a, string_view b) {
    Row row = firstRow(b);
    Row scratch;
    for (char c : a) {
        nextRow(row, c, b, scratch);
        swap(row, scratch);
    }
    return row.back();
}

}  // namespace seek
