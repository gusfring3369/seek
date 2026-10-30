#include "seek/engine/search_engine.hpp"

#include <cassert>

using namespace std;

namespace seek {

SearchEngine::SearchEngine(SearchEngineOptions options)
    : parser_(tokenizer_),
      trieAutocompleter_(vocabulary_),
      autocompleter_(trieAutocompleter_, options.cachedPrefixLength, options.cachedSuggestions),
      spellChecker_(vocabulary_) {}

DocId SearchEngine::addDocument(string text) {
    const vector<string> terms = tokenizer_.tokenize(text);

    const DocId id = index_.add(terms);
    for (const string& term : terms) {
        const Weight newFrequency = vocabulary_.insert(term);
        autocompleter_.onWeightChanged(term, newFrequency);
    }
    const DocId storedId = documents_.add(std::move(text));
    assert(storedId == id);  // the index and the store hand out ids in the same order
    (void)storedId;
    return id;
}

vector<DocId> SearchEngine::search(string_view query) const { return search(*parse(query)); }

vector<DocId> SearchEngine::search(const QueryNode& query) const {
    const QueryContext context{index_, intersection_};
    return query.evaluate(context).toPostingList();
}

unique_ptr<QueryNode> SearchEngine::parse(string_view query) const { return parser_.parse(query); }

vector<Suggestion> SearchEngine::autocomplete(string_view prefix, size_t k) const {
    return autocompleter_.complete(tokenizer_.normalize(prefix), k);
}

vector<Correction> SearchEngine::didYouMean(string_view word, size_t maxDistance,
                                            size_t limit) const {
    vector<Correction> corrections = spellChecker_.suggest(tokenizer_.normalize(word), maxDistance);
    if (corrections.size() > limit) corrections.resize(limit);
    return corrections;
}

bool SearchEngine::hasTerm(string_view term) const {
    return vocabulary_.contains(tokenizer_.normalize(term));
}

}  // namespace seek
