#pragma once
#include <deque>
#include "search_server.h"


class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(std::string_view raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(std::string_view raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(std::string_view raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        std::vector <Document> result_request;
        int is_empty;
    };
    const SearchServer& server;
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int empty_request_count;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(std::string_view raw_query, DocumentPredicate document_predicate)  {
    QueryResult query_result;
    query_result.result_request = server.FindTopDocuments(raw_query,document_predicate);
    query_result.is_empty = query_result.result_request.empty();
    empty_request_count+=query_result.is_empty;
    requests_.push_back(query_result);
    
    if (requests_.size() > min_in_day_) {
        empty_request_count-=requests_.front().is_empty;
        requests_.pop_front();
    }
    return query_result.result_request;
}

