#pragma once
#include "transport_catalogue.h"

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

struct DirectoryRequest {
	int id;
	std::string type;
	std::string name;
};
using OutRequests = std::vector< std::variant<Bus*, std::set<std::string_view>,std::string,int>>;

OutRequests ProcessRequest(AddQuery query,std::vector<DirectoryRequest> directory_requests, TransportCatalogue& transport_catalogue);