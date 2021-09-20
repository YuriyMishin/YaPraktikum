#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
    std::set<std::set <std::string>> all_uniq_documents;
    std::vector<int> duplicated_documents_id;
    std::set<std::string> next_document_data;
    
    for (const int document_id : search_server){
        next_document_data = {};
        for (const auto& [word, _] : search_server.GetWordFrequencies(document_id)){
            next_document_data.insert(word);
        }
        if (all_uniq_documents.emplace(next_document_data).second) {
            continue;
        }
        duplicated_documents_id.push_back(document_id);
        std::cout << "Found duplicate document id "<< document_id << std::endl;
    }
    for (int duplicated_id : duplicated_documents_id)
    {
        search_server.RemoveDocument(duplicated_id);
    }
}

