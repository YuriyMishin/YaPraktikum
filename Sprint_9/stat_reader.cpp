#include <algorithm>
#include <set>

#include "stat_reader.h"

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
            output << "Bus " << query << ": ";
            const Bus* bus = transport_catalogue.GetBus(query);
            if (!bus) {
                output << "not found" << std::endl;
            }
            else {
                auto bus_info = transport_catalogue.GetBusInfo(*bus);
                output << bus_info.stops_count << " stops on route, "
                       << bus_info.unique_stops_count << " unique stops, "
                       << bus_info.route_distance << " route length, "
                       << bus_info.curvature << " curvature"
                       << std::endl;
            }
        }
        else if (command == "Stop") {
            output << "Stop " << query << ": ";
            const Stop* stop = transport_catalogue.GetStop(query);
            if (!stop) {
                output << "not found" << std::endl;
            }
            else {
                std::set<std::string_view> buses = transport_catalogue.GetStopInfo(query);
                if (buses.size() == 0) {
                    std::cout << "no buses" << std::endl;
                }
                else {
                    output << "buses";
                    for (const std::string_view bus : buses){
                        std::cout <<" "<<bus;
                    }
                    output << std::endl;
                }
            }
        }
    }
    return output;
}


