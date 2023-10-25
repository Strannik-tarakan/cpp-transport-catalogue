#pragma once

#include "router.h"
#include "transport_catalogue.h"

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

class TransportRouter {
public:
	TransportRouter(const TransportCatalogue& cataloge) :catalogue_(cataloge) {};

	void AddRoutingSetting(int bus_wait_time, double bus_velocity);

	const graph::DirectedWeightedGraph<double>& GetGraph() const;
	void CreateGraph();
	OptimalRoute GetOptimalRoute(const std::string& stop_first, const std::string& stop_last, graph::Router<double>& router);
private:
	graph::DirectedWeightedGraph<double> stop_graph_;
	size_t numb_stops_graph_ = 0;

	int bus_wait_time_ = 0;
	double bus_velocity_ = 0;

	const TransportCatalogue& catalogue_;
};