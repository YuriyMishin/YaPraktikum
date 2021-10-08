#include "search_server.h"



SearchServer::SearchServer(const std::string& stop_words_text)
: SearchServer(std::string_view(stop_words_text))
{
}
SearchServer::SearchServer(std::string_view stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))
{
}


void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id");
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status, std::string(document)});
    document_ids_.insert(document_id);

    const auto words = SplitIntoWordsNoStop(documents_.at(document_id).str_words);
    const double inv_word_count = 1.0 / words.size();
    
    for (const std::string_view word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        id_word_to_document_freqs_[document_id][word]+= inv_word_count;
    }
}


std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
    });
}


std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}

SearchServer::const_iterator SearchServer::begin() const {
    return document_ids_.begin();
}

SearchServer::const_iterator SearchServer::end() const {
    return document_ids_.end();
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

SearchServer::MatchedDocument SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

SearchServer::MatchedDocument SearchServer::MatchDocument(const std::execution::sequenced_policy&,
                                                                                 std::string_view raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);
    
    std::vector<std::string_view> matched_words;
    for (const std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}

SearchServer::MatchedDocument SearchServer::MatchDocument(const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string_view> matched_words(query.plus_words.size());

    auto check_word_in_document = [this, document_id](std::string_view word) {
        return word_to_document_freqs_.at(word).count(document_id);
    };


    if (std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(), check_word_in_document)) {
        return {std::vector<std::string_view>(), documents_.at(document_id).status};
    }

    std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), check_word_in_document);

    return {matched_words, documents_.at(document_id).status};
}

const std::map<std::string_view , double>& SearchServer::GetWordFrequencies(int document_id) const {
    if (id_word_to_document_freqs_.count(document_id)){
        return id_word_to_document_freqs_.at(document_id);
    }
    static const std::map<std::string_view , double> empty_doc;
    return empty_doc;
}

void SearchServer::RemoveDocument(int document_id) {
    return RemoveDocument(std::execution::seq, document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id){
    if (document_ids_.count(document_id)){
        documents_.erase(document_id);
        document_ids_.erase(document_id);
        for (auto [word, _] : id_word_to_document_freqs_.at(document_id)){
            word_to_document_freqs_.at(word).erase(document_id);
            }
        id_word_to_document_freqs_.erase(document_id);
    }
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    if (document_ids_.count(document_id)){
        documents_.erase(document_id);
        document_ids_.erase(document_id);
        const auto &words_to_freqs = id_word_to_document_freqs_.at(document_id);

        for_each(std::execution::par, words_to_freqs.begin(), words_to_freqs.end(),
                 [this, &document_id](const auto& word_freq) {
                    word_to_document_freqs_.at(word_freq.first).erase(document_id);
                });
        id_word_to_document_freqs_.erase(document_id);
    }
}

bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}


bool SearchServer::IsValidWord(std::string_view word) {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}


std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
    std::vector<std::string_view> words;
    for (const std::string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word " + static_cast<std::string>(word) + " is invalid");
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}


int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}


SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty");
    }
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text.remove_prefix(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("Query word " + static_cast<std::string>(text) + " is invalid");
    }
    return {text, is_minus, IsStopWord(text)};
}


SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
    Query result;
    for (std::string_view word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}


double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}


void PrintDocument(const Document& document) {
    std::cout << "{ "
    << "document_id = " << document.id << ", "
    << "relevance = " << document.relevance << ", "
    << "rating = " << document.rating << " }" << std::endl;
}


void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view> words, DocumentStatus status) {
    std::cout << "{ "
    << "document_id = " << document_id << ", "
    << "status = " << static_cast<int>(status) << ", "
    << "words =";
    for (const std::string_view word : words) {
        std::cout << ' ' << static_cast <std::string> (word);
    }
    std::cout << "}" << std::endl;
}


void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const std::invalid_argument& e) {
        std::cout << "Ошибка добавления документа " << document_id << ": " << e.what() << std::endl;
    }
}


void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    std::cout << "Результаты поиска по запросу: " << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "Ошибка поиска: " << e.what() <<std:: endl;
    }
}


void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
        std::cout << "Матчинг документов по запросу: " << query << std::endl;
        for (const int document_id : search_server) {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "Ошибка матчинга документов на запрос " << query << ": " << e.what() << std::endl;
    }
}
