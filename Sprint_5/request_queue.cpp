#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
:   server(search_server)
{
    empty_request_count = 0;
    requests_.clear();
}

int RequestQueue::GetNoResultRequests() const {
    return empty_request_count;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
    });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}




