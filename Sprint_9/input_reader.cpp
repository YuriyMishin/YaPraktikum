#include <iostream>
#include <algorithm>
#include <numeric>

#include "input_reader.h"

namespace tc::detail {

    void TrimStr(std::string &str) {
        str.erase(str.find_last_not_of(' ') + 1);
        str.erase(0, str.find_first_not_of(' '));
    }

    std::pair<int, std::string> ParseDescriptionDistance(std::istream &input) {
        int distance = 0;
        std::string name, no_data;
        input >> distance >> no_data >> no_data;
        std::getline(input, name, ',');
        TrimStr(name);
        return std::make_pair(distance, name);
    }

    std::string ParseNameFromQuery(std::string &query) {
        std::string name;
        name = query.substr(1, query.find(':') - 1);
        query.erase(0, query.find(':') + 2);
        return name;
    }

    Stop ParseQueryStop(std::string &query, TransportCatalogue &transport_catalogue) {
        Stop stop;
        std::string no_data;
        stop.name = ParseNameFromQuery(query);

        std::istringstream in_str_stream(std::move(query));
        in_str_stream >> stop.coordinates.lat >> no_data >> stop.coordinates.lng >> no_data;

        transport_catalogue.AddStop(std::move(stop));

        while (!in_str_stream.eof()) {
            auto[distance, to_stop] = ParseDescriptionDistance(in_str_stream);
            transport_catalogue.AddDistanceBetweenStops(stop.name, to_stop, distance);
        }
        return stop;
    }

    std::vector<std::string> ParseDescriptionRoute(std::string &query_description_route) {
        std::string delimiter = " - ";
        std::vector<std::string> route;
        bool is_ring = false;

        if (query_description_route.find('>') != std::string::npos) {
            is_ring = true;
            delimiter = " > ";
        }

        size_t pos = 0;
        while ((pos = query_description_route.find(delimiter)) != std::string::npos) {
            route.push_back(query_description_route.substr(0, pos));
            query_description_route.erase(0, pos + 3);
        }
        route.push_back(query_description_route);

        if (!is_ring) {
            for (int i = route.size() - 2; i >= 0; --i) {
                route.push_back(route[i]);
            }
        }
        return route;
    }

    Bus ParseQueryBus(std::string &query, TransportCatalogue &transport_catalogue) {
        Bus bus;
        bus.name = ParseNameFromQuery(query);

        for (auto &stop_name: ParseDescriptionRoute(query)) {
            Stop *stop_ptr = transport_catalogue.GetStop(stop_name);
            bus.route.push_back(stop_ptr);
        }

        double geographical_distance = transport_catalogue.CalculateGeographicalDistance(bus);
        bus.info.route_distance = transport_catalogue.CalculateRouteDistance(bus);
        bus.info.curvature = bus.info.route_distance / geographical_distance;

        std::unordered_set<std::string> unique_stops_tmp;
        for_each(bus.route.begin(), bus.route.end(),
                 [&unique_stops_tmp](auto &stop) { unique_stops_tmp.insert(stop->name); });

        bus.info.unique_stops_count = unique_stops_tmp.size();
        bus.info.stops_count = bus.route.size();
        return bus;
    }
}

std::istream &operator>>(std::istream &input, tc::TransportCatalogue &transport_catalogue) {
    using namespace tc;
    int count = 0;

    input >> count;
    std::vector<std::string> buffer_bus;
    getchar();

    for (int n = 0; n < count; ++n) {
        std::string command, query;
        input >> command;
        getline(input, query);
        if (command == "Bus") {
            buffer_bus.push_back(query);
            continue;
        } else if (command == "Stop") {
            transport_catalogue.AddStop(ParseQueryStop(query, transport_catalogue));
        }
    }

    for (auto &bus_query: buffer_bus) {
        transport_catalogue.AddBus(ParseQueryBus(bus_query, transport_catalogue));
    }
    return input;
}

