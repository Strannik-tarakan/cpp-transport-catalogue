#include "request_handler.h"
#include "json.h"

#include <unordered_set>
#include <iostream>
#include <algorithm>

OutRequests ProcessRequest(AddQuery add_query, std::vector<DirectoryRequest> directory_requests, TransportCatalogue& transport_catalogue) {

    for (auto& stop : add_query.stop_query) {
        transport_catalogue.AddStop(stop.name, stop.lat, stop.lng, stop.distance_to);

        for (auto& s : stop.distance_to) {
            transport_catalogue.AddDistanceBetweenStops(stop.name, s.first, s.second);
        }
    }
    for (auto& bus : add_query.bus_query) {
        transport_catalogue.AddBus(bus.name, bus.stops,bus.last_stop);
    }
     
    OutRequests out_requests;

    for (const auto directory_request : directory_requests) {
        try {
            if (directory_request.type == "Bus") {
                out_requests.push_back(transport_catalogue.GetInfoBus(directory_request.name));
            }
            else if(directory_request.type == "Stop") {
                out_requests.push_back(transport_catalogue.GetInfoStop(directory_request.name));
            }
            else if (directory_request.type == "Map") {
                out_requests.push_back(0);
            }
        }
        catch (NoBuses error) {
            out_requests.push_back(std::set<std::string_view>{});
        }
        catch (NotFound error) {
            out_requests.push_back("not found");
        }
       
    }

    return out_requests;
}