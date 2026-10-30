#include "seek/query/and_node.hpp"

#include <algorithm>

using namespace std;

namespace seek {

ResultSet AndNode::evaluate(const QueryContext& context) const {
    vector<ResultSet> results = evaluateChildren(context);
    if (results.empty()) return ResultSet::own({});

    sort(results.begin(), results.end(),
         [](const ResultSet& a, const ResultSet& b) { return a.size() < b.size(); });

    ResultSet matched = std::move(results.front());
    for (size_t i = 1; i < results.size() && !matched.empty(); ++i) {
        matched = ResultSet::own(context.intersection.intersect(matched.ids(), results[i].ids()));
    }
    return matched;
}

}  // namespace seek
