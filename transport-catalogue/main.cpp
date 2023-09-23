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
	ReadJson(in, transport_catalogue);

	in.close();
	
}