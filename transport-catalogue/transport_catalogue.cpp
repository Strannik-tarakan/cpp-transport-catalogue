#include "transport_catalogue.h"


#include <iostream>
#include <iterator>
#include <algorithm>

void TransportCatalogue::AddStop(const std::string& name, double lat, double lng, const std::vector < std::pair<std::string, int>>& distance_to) {
	Stop stop = { name,{lat,lng},{} };
	auto stop_point = &stops_.emplace_back(stop);
	stopname_to_stop_[stops_.back().name] =stop_point;

	for ( auto& distant_to : distance_to) {
		const std::pair<std::string, std::string> stops = { name,distant_to.first };
		distant_.insert({ stops,distant_to.second });
	}
}
Stop* TransportCatalogue::FindStop(const std::string& stop_name)const {
	if (stopname_to_stop_.count(stop_name)) {
		return stopname_to_stop_.at(stop_name);
	}

	return nullptr;

}
void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, std::string last_stop=nullptr) {
	std::vector<Stop*> stop_signs;
	Stop* last_stop_=stopname_to_stop_[last_stop];

	for (const auto& stop : stops) {
		stop_signs.push_back(stopname_to_stop_[stop]);
	}

	auto point_bus = &(buses_.emplace_back(Bus{ name,stop_signs,{0 ,0 },last_stop_ }));
	busname_to_bus_[buses_.back().name] = point_bus;

	for (const auto& stop : stops) {
		stopname_to_stop_[stop]->passing_buses.insert(buses_.back().name);
	}

	double result_geo = 0;
	double distant = 0;
	auto it = buses_.back().stops.begin();
	auto next_it = std::next(it);
	for (; next_it != buses_.back().stops.end(); ++it, ++next_it) {
		distant += GetDistance((*it)->name, (*next_it)->name);
		result_geo += ComputeDistance((*it)->cordinat, (*next_it)->cordinat);
	}
	buses_.back().distant_bus.first = distant;
	buses_.back().distant_bus.second =  distant/ result_geo;
}
void TransportCatalogue::AddDistanceBetweenStops(std::string& stop_first, std::string& stop_last, const int distant)
{
	distant_[{ stop_first, stop_last }] = distant;
}


Bus TransportCatalogue::FindBus(const std::string& bus_name) {
	return *busname_to_bus_[bus_name];
}


Bus* TransportCatalogue::GetInfoBus(const std::string& bus_name){
	if (busname_to_bus_.count(bus_name)) {
		return busname_to_bus_[bus_name];
	}
	throw NotFound{"Bus",bus_name};
}
const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetInfoAllBus()const
{
	return busname_to_bus_;
}
std::set<std::string_view> TransportCatalogue::GetInfoStop(const std::string& stop_name)const
{
	if (FindStop(stop_name) == nullptr) {
		throw NotFound{"Stop",stop_name};
	}
	
	if (stopname_to_stop_.at(stop_name)->passing_buses.size() != 0) {
		return stopname_to_stop_.at(stop_name)->passing_buses;
	}

	throw NoBuses{stop_name};
	
}
double TransportCatalogue::GetDistance(std::string& stop_first, std::string& stop_last) const
{
	if (distant_.count({ stop_first,stop_last }) != 0) {
		return distant_.at({stop_first, stop_last });
	}
	else if (distant_.count({ stop_last, stop_first }) != 0) {
		return distant_.at({stop_last, stop_first });
	}
	return 0;
}
 double TransportCatalogue::GetBusDistantGeo(const Bus& bus)
{
	double result=0;
	auto it = bus.stops.begin();
	auto next_it = std::next(it);
	for (; next_it != bus.stops.end();++it,++next_it) {
		result+=ComputeDistance((*it)->cordinat, (*next_it)->cordinat);
	}
	return result;
}


 size_t Hash_distant::operator()(const std::pair<std::string, std::string>& stops) const {
	 size_t h1 = s_hasher_(stops.first);
	 size_t h2 = s_hasher_(stops.second);
	 
	 return h1 + h2 * 16;
 }