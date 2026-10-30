#include "seek/query/or_node.hpp"

#include "seek/index/posting_list_union.hpp"

using namespace std;

namespace seek {

ResultSet OrNode::evaluate(const QueryContext& context) const {
    vector<ResultSet> results = evaluateChildren(context);
    if (results.empty()) return ResultSet::own({});

    ResultSet matched = std::move(results.front());
    for (size_t i = 1; i < results.size(); ++i) {
        matched = ResultSet::own(unionOf(matched.ids(), results[i].ids()));
    }
    return matched;
}

}  // namespace seek
