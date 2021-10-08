#include "process_queries.h"

#include <algorithm>
#include <execution>

using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries) {
    vector<vector<Document>> searched_documents(queries.size());
    transform(execution::par, queries.begin(), queries.end(), searched_documents.begin(),
              [&search_server](const string& query) {
                return search_server.FindTopDocuments(query);
            });
    return searched_documents;
}

vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const vector<string>& queries) {
    vector<Document> documents;
    for (const auto& find_documents : ProcessQueries(search_server, queries)) {
        copy(execution::par, find_documents.begin(), find_documents.end(), back_inserter(documents));
    }
    return documents;
}