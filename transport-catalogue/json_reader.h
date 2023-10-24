#pragma once

#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>

using namespace json;



class JSONReader {
public:

	JSONReader(TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue) {
	}

	void ReadJson(std::istream& input);

	void ConclusionJson(std::ostream& output);

private:
	
	TransportCatalogue& transport_catalogue_;
	AddQuery add_query;
	std::vector<DirectoryRequest> directory_requests;
	OutRequests out_requests;
	MapSettings map_settings;
	RoutSettings routing_settings_;


	void ProcessingBasicRequests(Array& base_requests);
	void ProcessingStatisticsRequests(Array& stat_requests);
	void ProcessingCardSettings(Dict& render_settings);
	void ProcessingRoutingSettings(Dict& routing_settings);

	void PrintQueryBus(std::ostream& output, size_t i);
	void PrintQueryStop(std::ostream& output, size_t i);
	void PrintQueryMap(std::ostream& output, size_t i,
		std::map<std::string_view, Coordinates>& stops_on_buses, std::vector<Bus*>& buses);
	void PrintQueryRoute(std::ostream& output, size_t i);
};



