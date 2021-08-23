#pragma once
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include "string_processing.h"
#include "document.h"
#include "paginator.h"



const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);
    explicit SearchServer(const std::string& stop_words_text);
    
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;
    
    using const_iterator=typename std::set<int>::const_iterator;
    const_iterator begin() const  { return document_ids_.begin(); }
    const_iterator end() const  { return document_ids_.end(); }
    
    int GetDocumentCount() const;
    
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;
    
    const std::map<std::string, double>& GetWordFrequencies(int document_id) const {
        return id_word_to_document_freqs_.at(document_id);
    }
    
    void RemoveDocument(int document_id){
        documents_.erase(document_id);
        id_word_to_document_freqs_.erase(document_id);
        document_ids_.erase(document_id);
    }
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const std::set<std::string> stop_words_;
    std::map<int, std::map<std::string, double>> id_word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    
    bool IsStopWord(const std::string& word) const;
    static bool IsValidWord(const std::string& word);
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(const std::string& text) const;
    
    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };
    
    Query ParseQuery(const std::string& text) const;
    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

void PrintDocument(const Document& document);
void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);
void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) ;
void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) ;
void MatchDocuments(const SearchServer& search_server, const std::string& query);

template <typename Container>
auto Paginate(const Container& c, size_t page_size);

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
: stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{
    using namespace std;
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
    const auto query = ParseQuery(raw_query);
    
    auto matched_documents = FindAllDocuments(query, document_predicate);
    
    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    
    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    
    for (const std::string& word : query.plus_words) {
        for (const auto& [document_id, word_to_document_freqs ] : id_word_to_document_freqs_){
            if (word_to_document_freqs.count(word) == 0) {
                continue;
            }
            
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            const auto& document_data = documents_.at(document_id);
            
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += word_to_document_freqs.at(word) * inverse_document_freq;
            }
        }
    }
    
    for (const int document_id : document_ids_){
        for (const std::string& word : query.minus_words) {
            if (id_word_to_document_freqs_.at(document_id).count(word) == 0) {
                continue;
            }else{
                document_to_relevance.erase(document_id);
            }
        }
    }
    
    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

