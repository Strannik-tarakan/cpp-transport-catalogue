#include "transport_catalogue.h"
#include "stat_reader.h"
#include "input_reader.h"

#include <iostream>
#include <string>
#include <string_view>
#include <fstream>

int main() {
	std::ifstream in;
	in.open("test.txt");
	TransportCatalogue transport_catalogue;
	ReadDatabaseCreation(in,transport_catalogue);
	ExecutionRequests(in, transport_catalogue);
	in.close();
	
}