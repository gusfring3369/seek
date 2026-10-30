#include "seek/engine/document_store.hpp"

#include <utility>

using namespace std;

namespace seek {

DocId DocumentStore::add(string text) {
    texts_.push_back(std::move(text));
    return static_cast<DocId>(texts_.size() - 1);
}

}  // namespace seek
