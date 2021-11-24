#pragma once

#include <string_view>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <set>

#include "geo.h"

namespace tc {
    using namespace detail;

    struct Stop {
        std::string name;
        Coordinates coordinates;
    };

    struct BusInfo {
        int stops_count = 0;
        int unique_stops_count = 0;
        int route_distance = 0;
        double curvature = 0.0;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> route;
        BusInfo info;
    };

    class DistanceHasher {
    public:
        size_t operator()(const std::pair<Stop*, Stop*>& stops) const{
            std::hash<const void*> hash_ptr;
            return hash_ptr(stops.first) + (hash_ptr(stops.second) * 37^3);
        }
    };

    class TransportCatalogue {

    public:
        void AddBus(const Bus& bus);
        void AddStop(const Stop& stop);

        Bus* GetBus(const std::string& bus)const;
        Stop* GetStop(const std::string& stop_name)const;

        const BusInfo GetBusInfo(const Bus& bus)const;
        const std::set<std::string_view> GetStopInfo(const std::string& stop_name) const;

        void AddDistanceBetweenStops(const std::string& from, const std::string& to, const int distance);
        int GetDistanceBetweenStops(const std::string& from, const std::string& to) const ;

        double CalculateGeographicalDistance (Bus& bus) const;
        int CalculateRouteDistance (Bus& bus) const;

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;
        std::unordered_map<std::string, Bus*> index_buses_;
        std::unordered_map<std::string, Stop*> index_stops_;
        std::unordered_map<Stop*, std::set<std::string_view>> buses_for_stops_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, DistanceHasher> distance_between_stops_;
    };

}
