#include "domain.h"

#include <unordered_set>
#include <iostream>

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