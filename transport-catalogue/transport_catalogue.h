#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
 
struct Stop
{
	std::string name;
	Coordinates cordinat;
	std::set<std::string_view> passing_buses;
};
struct Bus {
	std::string name;
	std::vector<Stop*> stops;
	std::pair<int, double> distant_bus;
};

class Hash_distant {
public:
	size_t operator()(const std::pair<std::string,std::string>& stops) const;
private:
	std::hash<std::string> s_hasher_;
};

class TransportCatalogue {
public:
	void AddStop(const std::string& name, double lat, double lng,const std::vector < std::pair<std::string, int>>& distance_to);
	void AddBus(const std::string& name, const std::vector<std::string>& stops);
	void AddDistanceBetweenStops(std::string& stop_first, std::string& stop_last, const int distant);

	Stop* FindStop(const std::string& stop_name);
	Bus FindBus(const std::string& bus_name);

	Bus* GetInfoBus(const std::string& bus_name);
	std::set<std::string_view> GetInfoStop(const std::string& stop_name);
	double GetDistance(std::string& stop_first, std::string& stop_last);
	static double GetBusDistantGeo(const Bus& bus);

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Stop*> stopname_to_stop_;

	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, Bus*> busname_to_bus_;

	std::unordered_map<std::pair<std::string, std::string>, int,Hash_distant> distant_;
};
