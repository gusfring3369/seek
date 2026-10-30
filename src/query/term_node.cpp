#include "seek/query/term_node.hpp"

using namespace std;

namespace seek {

ResultSet TermNode::evaluate(const QueryContext& context) const {
    return ResultSet::borrow(context.index.postings(term_));
}

}  // namespace seek
