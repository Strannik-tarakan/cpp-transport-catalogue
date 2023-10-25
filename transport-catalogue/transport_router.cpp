#include "transport_router.h"

void TransportRouter::AddRoutingSetting(int bus_wait_time, double bus_velocity)
{
	bus_wait_time_ = bus_wait_time;
	bus_velocity_ = bus_velocity;
}

OptimalRoute TransportRouter::GetOptimalRoute(const std::string& stop_first, const std::string& stop_last, graph::Router<double>& router){
	
	auto route_info = router.BuildRoute(catalogue_.stopname_to_stop_.at(stop_first)->id, catalogue_.stopname_to_stop_.at(stop_last)->id);
	if (route_info) {
		OptimalRoute result;
		result.total_time = route_info.value().weight;
		if (result.total_time == 0) {
			return result;
		}
		ElementsOptimalRoute action;
		action.type = "Wait";
		action.name = catalogue_.stopname_to_stop_.at(stop_first)->name;
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
				action.name = catalogue_.stopname_to_stop_.at(stop_last)->name;
				action.time = bus_wait_time_;
				result.elements.push_back(action);
			}
		}
		return result;
	}
	throw NotFound{ "Route", stop_first, stop_last };
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const{
	return stop_graph_;
}


void TransportRouter::CreateGraph() {

	stop_graph_.IncreaseGraph(catalogue_.stopname_to_stop_.size());
	for (auto& [stop_name, stop] : catalogue_.stopname_to_stop_) {
		if (stop == nullptr)continue;
		stop->id = numb_stops_graph_;
		++numb_stops_graph_;
	}

	for (const auto& [name, bus] : catalogue_.busname_to_bus_) {
		size_t count_stops_bus = bus->stops.size();
		for (int i = 0; i < count_stops_bus; ++i) {
			double distance = 0;
			for (int j = i + 1; j < count_stops_bus; ++j) {
				Stop* stop1 = catalogue_.busname_to_bus_.at(name)->stops[i];
				Stop* stop2 = catalogue_.busname_to_bus_.at(name)->stops[j];
				distance += catalogue_.GetDistance(catalogue_.busname_to_bus_.at(name)->stops[j - 1]->name, stop2->name);
				stop_graph_.AddEdge({ static_cast<unsigned long long>(stop1->id)
					, static_cast<unsigned long long>(stop2->id),
					 distance / 1000.0 / bus_velocity_ * 60 + bus_wait_time_ ,
					catalogue_.busname_to_bus_.at(name)->name,
					j - i });
			}
		}
	}

}
