#pragma once

#include <iostream>
#include <sstream>

#include "transport_catalogue.h"

namespace tc::detail {
    void TrimStr(std::string &str);
    std::pair<int, std::string> ParseDescriptionDistance(std::istream& ist) ;
    std::string ParseNameFromQuery(std::string& query);
    std::vector <std::string> ParseDescriptionRoute (std::string &query_description_route);
    Stop ParseQueryStop (std::string& query, TransportCatalogue& transport_catalogue);
    Bus ParseQueryBus (std::string& query, TransportCatalogue& transport_catalogue);
}

std::istream& operator >> (std::istream& input, tc::TransportCatalogue& transport_catalogue);
