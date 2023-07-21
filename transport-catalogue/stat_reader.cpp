#include "stat_reader.h"
#include "transport_catalogue.h"

#include <unordered_set>
#include <iostream>
#include <algorithm>

std::ostream& operator<<(std::ostream& os, const Bus* bus)
{
	if (bus == nullptr) {
		return os;
	}

	std::unordered_set<Stop*> uniqueStops;
	for (const auto& stop : (*bus).stops) {
		uniqueStops.insert(stop);
	}

	os << "Bus " << (*bus).name << ": "
		<< (*bus).stops.size() << " stops on route, "
		<< uniqueStops.size() << " unique stops, "
		<< bus->distant_bus.first << " route length, " << bus->distant_bus.second << " curvature";

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<std::string_view>& osunset) {
	for (auto it = osunset.begin(); it != osunset.end(); ++it) {
		std::cout << *it;

		if (std::next(it) != osunset.end()) {
			std::cout << ' ';
		}
	}

	return os;
}

void ExecuteRequests(std::istream& in, TransportCatalogue& transport_catalogue) {

	int quantity_query;
	in >> quantity_query;
	in.ignore();

	for (int i = 0; i < quantity_query; ++i) {
		std::string text;
		std::getline(in, text);
		auto it = text.find_first_not_of(' ');

		if (text.substr(it, 3) == "Bus") {
			try {
				std::cout << transport_catalogue.GetInfoBus(text.substr(it + 4, text.find_last_not_of(' ') - it - 2)) << std::endl;
			}
			catch(std::string error){
				std::cout << error << std::endl;
			}
			
		}
		else if (text.substr(it, 4) == "Stop") {
			try {
				std::set<std::string_view> stops = transport_catalogue.GetInfoStop(text.substr(it + 5, text.find_last_not_of(' ') - it - 3));
				if (stops.size() != 0) {
					std::cout << "Stop " << text.substr(it + 5, text.find_last_not_of(' ') - it - 3)
						<< ": buses " << stops
						<< std::endl;
				}
			}
			catch (std::string error) {
				std::cout << error << std::endl;
			}
			
			

		}
	}
}
