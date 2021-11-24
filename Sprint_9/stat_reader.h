#pragma once

#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
namespace tc::detail {
    void PrintBusInfo(std::ostream &output, std::string &bus_name, tc::TransportCatalogue &transport_catalogue);
    void PrintStopInfo(std::ostream &output, std::string &stop_name, tc::TransportCatalogue &transport_catalogue);
}
std::ostream& operator << (std::ostream& output, tc::TransportCatalogue& transport_catalogue);
