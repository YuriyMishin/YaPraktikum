#include <algorithm>
#include <numeric>

#include "transport_catalogue.h"

namespace tc {

    void TransportCatalogue::AddStop(const Stop& stop) {
        Stop* finded_stop = GetStop(stop.name);
        if (!finded_stop) {
            auto ptr_stop = &*stops_.emplace(stops_.end(), stop);
            index_stops_[ptr_stop->name] = ptr_stop;
            buses_for_stops_[ptr_stop];
        }
        else {
            *finded_stop = { stop.name, stop.coordinates };
        }
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        auto ptr_bus = &*buses_.emplace(buses_.end(), bus);
        index_buses_[ptr_bus->name] = ptr_bus;

        for (auto& stop : ptr_bus->route) {
            buses_for_stops_[stop].emplace(ptr_bus->name);
        }
    }

    Bus* TransportCatalogue::GetBus(const std::string& bus) const {
        if (index_buses_.count(bus)) {
            return index_buses_.at(bus);
        }
        return nullptr;
    }

    Stop* TransportCatalogue::GetStop(const std::string& stop_name) const {
        if (index_stops_.count(stop_name)) {
            return index_stops_.at(stop_name);
        }
        return nullptr;
    }

    const BusInfo TransportCatalogue::GetBusInfo(const Bus& bus)const{
        if (index_buses_.count(bus.name)) {
            return bus.info;
        }
        return {};
    }

    const std::set<std::string_view> TransportCatalogue::GetStopInfo(const std::string& stop_name) const {
        Stop* stop = GetStop(stop_name);
        if (!stop) {
            static std::set<std::string_view> empty;
            return empty;
        }
        return buses_for_stops_.at(stop);
    }

    void TransportCatalogue::AddDistanceBetweenStops(const std::string& from, const std::string& to, const int distance) {
        if (!GetStop(from)) {
            AddStop(std::move(Stop({ from, {0.0, 0.0} })));
        }
        if (!GetStop(to)) {
            AddStop(std::move(Stop({ to, {0.0, 0.0} })));
        }

        auto forward = std::make_pair(GetStop(from), GetStop(to));
        auto backward = std::make_pair(GetStop(to), GetStop(from));

        distance_between_stops_[forward] = distance;
        if (!distance_between_stops_.count(backward)) {
            distance_between_stops_.emplace((backward), distance);
        }
    }

    int TransportCatalogue::GetDistanceBetweenStops(const std::string& stop1, const std::string& stop2) const {

        return distance_between_stops_.at({GetStop(stop1), GetStop(stop2)});
    }

    double TransportCatalogue::CalculateGeographicalDistance (Bus& bus ) const {
        return std::transform_reduce(bus.route.begin(), bus.route.end()-1, bus.route.begin()+1, 0.0, std::plus<>{}, [](const Stop* stop_from, const Stop* stop_to) {
                                         return ComputeDistance(stop_from->coordinates, stop_to->coordinates);
                                     }
        );
    }

    int TransportCatalogue::CalculateRouteDistance (Bus& bus) const {
        return std::transform_reduce(bus.route.begin(), bus.route.end()-1,
                                     bus.route.begin()+1, 0, std::plus<>{}, [this](const Stop* stop_from, const Stop* stop_to) {
                    return GetDistanceBetweenStops(stop_from->name, stop_to->name);
                }
        );
    }
}
