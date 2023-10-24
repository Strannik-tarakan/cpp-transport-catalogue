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
Stop* TransportCatalogue::FindStop(const std::string& stop_name) {
	if (stopname_to_stop_.count(stop_name)) {
		return stopname_to_stop_[stop_name];
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
void TransportCatalogue::AddRoutingSetting(int bus_wait_time, double bus_velocity)
{
	bus_wait_time_ = bus_wait_time;
	bus_velocity_ = bus_velocity;
}

Bus TransportCatalogue::FindBus(const std::string& bus_name) {
	return *busname_to_bus_[bus_name];
}

void TransportCatalogue::CreateGraph(){
	
	stop_graph_.IncreaseGraph(stopname_to_stop_.size());
	for (auto& [stop_name,stop ]: stopname_to_stop_) {
		if (stop == nullptr)continue;
		stop->id = numb_stops_graph_;
		++numb_stops_graph_;
	}
	for(const auto& [name,bus]: busname_to_bus_){
		size_t count_stops_bus = bus->stops.size();
		for (int i = 0; i < count_stops_bus; ++i) {
			double distance=0;
			for (int j = i + 1; j < count_stops_bus; ++j) {
				Stop* stop1 = busname_to_bus_.at(name)->stops[i];
				Stop* stop2 = busname_to_bus_.at(name)->stops[j];
				distance += this->GetDistance(busname_to_bus_.at(name)->stops[j-1]->name, stop2->name);
				stop_graph_.AddEdge({ static_cast<unsigned long long>(stop1->id)
					, static_cast<unsigned long long>(stop2->id),
					 distance / 1000.0 / bus_velocity_ * 60 + bus_wait_time_ ,
					busname_to_bus_.at(name)->name,
					j - i });
			}
		}
	}
	
}

Bus* TransportCatalogue::GetInfoBus(const std::string& bus_name){
	if (busname_to_bus_.count(bus_name)) {
		return busname_to_bus_[bus_name];
	}
	throw NotFound{"Bus",bus_name};
}

const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetInfoAllBus()
{
	return busname_to_bus_;
}

std::set<std::string_view> TransportCatalogue::GetInfoStop(const std::string& stop_name)
{
	if (FindStop(stop_name) == nullptr) {
		throw NotFound{"Stop",stop_name};
	}
	
	if (stopname_to_stop_[stop_name]->passing_buses.size() != 0) {
		return stopname_to_stop_[stop_name]->passing_buses;
	}

	throw NoBuses{stop_name};
	
}

double TransportCatalogue::GetDistance(std::string& stop_first, std::string& stop_last)
{
	if (distant_.count({ stop_first,stop_last }) != 0) {
		return distant_[{stop_first, stop_last }];
	}
	else if (distant_.count({ stop_last, stop_first }) != 0) {
		return distant_[{stop_last, stop_first }];
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

 OptimalRoute TransportCatalogue::GetOptimalRoute(const std::string& stop_first, const std::string& stop_last, graph::Router<double>& router)
 {
	 
	 auto route_info = router.BuildRoute(stopname_to_stop_.at(stop_first)->id, stopname_to_stop_.at(stop_last)->id);
	 if (route_info) {
		 OptimalRoute result;
		 result.total_time = route_info.value().weight;
		 if(result.total_time==0){
			 return result;
		 }
		 ElementsOptimalRoute action;
		 action.type = "Wait";
		 action.name = stopname_to_stop_.at(stop_first)->name;
		 action.time = bus_wait_time_;
		 result.elements.push_back(action);
		 for (auto& id : route_info.value().edges) {
			 {
				 ElementsOptimalRoute action;
				 action.type = "Bus";
				 action.count = stop_graph_.GetEdge(id).count_stops;
				 action.name = stop_graph_.GetEdge(id).bus_name;
				 action.time = stop_graph_.GetEdge(id).weight - bus_wait_time_;
				 result.elements.push_back(action);
			 }
			 if (id == *route_info.value().edges.rbegin()) {
				 continue;
			 }
			 {
				 ElementsOptimalRoute action;
				 action.type = "Wait";
				 action.name = stopname_to_stop_.at(stop_last)->name;
				 action.time = bus_wait_time_;
				 result.elements.push_back(action);
			 }
		 }
		 return result;
	 }
	 throw NotFound{ "Route",stop_first,stop_last };
 }

 const graph::DirectedWeightedGraph<double>& TransportCatalogue::GetGraph() const
 {
	 return stop_graph_;
 }


 size_t Hash_distant::operator()(const std::pair<std::string, std::string>& stops) const {
	 size_t h1 = s_hasher_(stops.first);
	 size_t h2 = s_hasher_(stops.second);
	 
	 return h1 + h2 * 16;
 }