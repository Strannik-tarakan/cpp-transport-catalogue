#pragma once

#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <iostream>

using namespace json;


void ReadJson(std::istream& input, TransportCatalogue& transport_catalogue);

void ConclusionJson(std::ostream& output, OutRequests& out_requests, std::vector<DirectoryRequest>& directory_requests, MapSettings& map_settings, std::map<std::string_view, Coordinates>& stops_on_buses, std::vector<Bus*>& buses);

