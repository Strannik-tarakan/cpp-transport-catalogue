#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"

#include<iostream>
#include <vector>
#include <string>
#include <string_view>
#include <variant>


struct StopQuery
{
	std::string name;
	double lat;
	double lng;
	std::vector < std::pair<std::string, int>> distance_to;
};
struct BusQuery
{
	std::string name;
	std::vector<std::string> stops;
	std::string last_stop;

};
struct AddQuery
{
	std::vector<StopQuery> stop_query;
	std::vector<BusQuery> bus_query;
};
struct RoutSettings {
	int bus_wait_time;
	double bus_velocity;
};

struct DirectoryRequest {
	int id;
	std::string type;

	std::string name;

	std::string from_stop;
	std::string to_stop;
};
using OutRequests = std::vector< std::variant<Bus*, std::set<std::string_view>,std::string,int, OptimalRoute>>;

OutRequests ProcessRequest(AddQuery query,std::vector<DirectoryRequest> directory_requests, TransportCatalogue& transport_catalogue, RoutSettings rout_settings);