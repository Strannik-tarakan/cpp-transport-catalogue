#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "geo.h"

#include <iostream>
#include <string>
#include <fstream>

int main() {
	std::ifstream in;
	TransportCatalogue transport_catalogue;
	in.open("testjson.json");
	JSONReader json_reader(transport_catalogue);
	json_reader.ReadJson(in);
	json_reader.ConclusionJson(std::cout);

	in.close();
	
}