#include "seek/search_engine.hpp"

#include <cassert>

#include "seek/tokenizer.hpp"

namespace seek {

SearchEngine::SearchEngine(SearchEngineOptions options)
    : hotPrefixes_(options.cachedPrefixLength, options.cachedSuggestions) {}

DocId SearchEngine::addDocument(std::string text) {
    const std::vector<std::string> terms = tokenize(text);
    const DocId id = index_.add(terms);
    for (const std::string& term : terms) hotPrefixes_.update(term, vocabulary_.insert(term));
    documents_.push_back(std::move(text));
    assert(documents_.size() == index_.documentCount());
    return id;
}

std::vector<DocId> SearchEngine::search(std::string_view query) const {
    return search(parseQuery(query));
}

std::vector<DocId> SearchEngine::search(const Query& query) const { return execute(query, index_); }

std::vector<Suggestion> SearchEngine::autocomplete(std::string_view prefix, std::size_t k) const {
    const std::string normalized = toLowerAscii(prefix);
    if (auto cached = hotPrefixes_.lookup(normalized, k)) return *std::move(cached);
    return topKCompletions(vocabulary_, normalized, k);
}

std::vector<FuzzyMatch> SearchEngine::didYouMean(std::string_view word, std::size_t maxDistance,
                                                 std::size_t limit) const {
    std::vector<FuzzyMatch> matches = fuzzySearch(vocabulary_, toLowerAscii(word), maxDistance);
    if (matches.size() > limit) matches.resize(limit);
    return matches;
}

bool SearchEngine::hasTerm(std::string_view term) const {
    return vocabulary_.contains(toLowerAscii(term));
}

const std::string& SearchEngine::document(DocId id) const { return documents_.at(id); }

}  // namespace seek
