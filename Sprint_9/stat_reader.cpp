#include <algorithm>
#include <set>

#include "stat_reader.h"

namespace tc::detail {
    void AnswerBus(std::ostream &output, std::string &bus_name, tc::TransportCatalogue &transport_catalogue) {
        output << "Bus " << bus_name << ": ";
        const Bus *bus = transport_catalogue.GetBus(bus_name);
        if (!bus) {
            output << "not found" << std::endl;
        } else {
            auto bus_info = transport_catalogue.GetBusInfo(*bus);
            output << bus_info.stops_count << " stops on route, "
                   << bus_info.unique_stops_count << " unique stops, "
                   << bus_info.route_distance << " route length, "
                   << bus_info.curvature << " curvature"
                   << std::endl;
        }
    }

    void AnswerStop(std::ostream &output, std::string &stop_name, tc::TransportCatalogue &transport_catalogue) {
        output << "Stop " << stop_name << ": ";
        const Stop *stop = transport_catalogue.GetStop(stop_name);
        if (!stop) {
            output << "not found" << std::endl;
        } else {
            std::set<std::string_view> buses = transport_catalogue.GetStopInfo(stop_name);
            if (buses.size() == 0) {
                std::cout << "no buses" << std::endl;
            } else {
                output << "buses";
                for (const std::string_view bus: buses) {
                    std::cout << " " << bus;
                }
                output << std::endl;
            }
        }
    }
}

std::ostream& operator << (std::ostream& output, tc::TransportCatalogue& transport_catalogue){
    using namespace tc;
    int count = 0;

    std::cin >> count;
    std::vector <std::string> buffer_bus;
    getchar();

    for (int n = 0; n < count; ++n){
        std::string command, query;
        std::cin >> command;
        getline(std::cin, query);
        TrimStr(query);
        if (command == "Bus") {
            AnswerBus(output, query, transport_catalogue);
        }
        else if (command == "Stop") {
            AnswerStop(output, query, transport_catalogue);
        }
    }
    return output;
}


