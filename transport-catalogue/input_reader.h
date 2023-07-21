#pragma once
#include "transport_catalogue.h"

#include<iostream>
#include <vector>
#include <string>
#include <string_view>

struct StopQuery
{
	std::string name;
	double x;
	double y;
	std::vector < std::pair<std::string, int>> distance_to;
};
struct BusQuery
{
	std::string name;
	std::vector<std::string> stops;
};
struct Query
{
	std::vector<StopQuery> stop_query;
	std::vector<BusQuery> bus_query;
};
void ParsingStop(Query& query, size_t it, std::string& text);
void ParsingBus(Query& query, size_t it, std::string& text);

void ReadDatabase(std::istream& in, TransportCatalogue& transport_catalogue);
