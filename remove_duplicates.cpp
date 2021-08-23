#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
    std::vector<std::map <std::string, double>> all_uniq_documents;
    std::vector<int> duplicated_documents_id;
    bool is_duplicate;
    
    for (const int document_id : search_server){
        if (all_uniq_documents.size() == 0) {
            all_uniq_documents.push_back(search_server.GetWordFrequencies(document_id));
            continue;
        }
        auto next_document_data = search_server.GetWordFrequencies(document_id);
        is_duplicate = false;
        for (int document_num = 0; document_num < all_uniq_documents.size(); ++document_num){
            const auto& document_data = all_uniq_documents.at(document_num);
            
            if ((document_data.size() == next_document_data.size()) && std::equal(document_data.begin(), document_data.end(), next_document_data.begin(), [](const auto& lhs, const auto& rhs) {
                return lhs.first == rhs.first;})) {
                    
                    duplicated_documents_id.push_back(document_id);
                    is_duplicate = true;
                    break;
                }
        }
        if (!is_duplicate) {
            all_uniq_documents.push_back(next_document_data);
        }
    }
    
    for (int duplicated_id : duplicated_documents_id)
    {
        std::cout << "Found duplicate document id "<< duplicated_id << std::endl;
        search_server.RemoveDocument(duplicated_id);
    }
}
