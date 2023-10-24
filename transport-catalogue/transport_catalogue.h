#pragma once
#include "domain.h"
#include "router.h"

#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
 


class Hash_distant {
public:
	size_t operator()(const std::pair<std::string,std::string>& stops) const;
private:
	std::hash<std::string> s_hasher_;
};



struct ElementsOptimalRoute {
	std::string type; 
	std::string_view name;
	int count;
	double time;
};

struct OptimalRoute {
	double total_time;
	std::vector<ElementsOptimalRoute> elements;
};

class TransportCatalogue {
public:
	

	void AddStop(const std::string& name, double lat, double lng,const std::vector < std::pair<std::string, int>>& distance_to);
	void AddBus(const std::string& name, const std::vector<std::string>& stops, std::string last_stop);
	void AddDistanceBetweenStops(std::string& stop_first, std::string& stop_last, const int distant);
	void AddRoutingSetting(int bus_wait_time, double bus_velocity);

	Stop* FindStop(const std::string& stop_name);
	Bus FindBus(const std::string& bus_name);

	void CreateGraph();

	Bus* GetInfoBus(const std::string& bus_name);
	const std::unordered_map<std::string_view, Bus*>& GetInfoAllBus();
	std::set<std::string_view> GetInfoStop(const std::string& stop_name);
	double GetDistance(std::string& stop_first, std::string& stop_last);
	static double GetBusDistantGeo(const Bus& bus);
	OptimalRoute GetOptimalRoute(const std::string& stop_first,const std::string& stop_last, graph::Router<double>& router);
	const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
	

	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Bus*> busname_to_bus_;

	std::unordered_map<std::pair<std::string, std::string>, int,Hash_distant> distant_;

	graph::DirectedWeightedGraph<double> stop_graph_;
	size_t numb_stops_graph_ = 0;

	int bus_wait_time_=0;
	double bus_velocity_=0;
};
