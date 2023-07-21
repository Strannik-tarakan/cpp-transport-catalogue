#pragma once
#include "transport_catalogue.h"

#include<iostream>


std::ostream& operator << (std::ostream& os, const Bus* bus);
std::ostream& operator<<(std::ostream& os, const std::set<std::string_view>& osunset);

void ExecuteRequests(std::istream& in, TransportCatalogue& transport_catalogue);
