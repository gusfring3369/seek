#include "seek/query/composite_node.hpp"

using namespace std;

namespace seek {

void CompositeNode::collectTerms(vector<string>& terms) const {
    for (const auto& child : children_) child->collectTerms(terms);
}

vector<ResultSet> CompositeNode::evaluateChildren(const QueryContext& context) const {
    vector<ResultSet> results;
    results.reserve(children_.size());
    for (const auto& child : children_) results.push_back(child->evaluate(context));
    return results;
}

string CompositeNode::joinChildren(string_view separator) const {
    string text = "(";
    for (size_t i = 0; i < children_.size(); ++i) {
        if (i > 0) text += separator;
        text += children_[i]->toString();
    }
    return text + ")";
}

}  // namespace seek
