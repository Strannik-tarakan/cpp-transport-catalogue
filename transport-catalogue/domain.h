#pragma once
#include "geo.h"

#include <string>
#include <set>
#include <vector>



struct Stop
{
	std::string name;
	Coordinates cordinat;
	std::set<std::string_view> passing_buses;
};
struct Bus {
	std::string name;
	std::vector<Stop*> stops;
	std::pair<double, double> distant_bus;   // ����������, ��������
	Stop* last_stop=nullptr;
};
struct NotFound {
	std::string type;
	std::string name;
};
struct NoBuses {
	std::string name;
};
std::ostream& operator << (std::ostream& os, const Bus* bus);