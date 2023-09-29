#include "json_reader.h"



#include <variant>
#include <iostream>
#include <set>
#include <string_view>
#include <map>
#include <fstream>

std::ostream& operator<<(std::ostream& os, const std::set<std::string_view>& osunset) {
	for (auto it = osunset.begin(); it != osunset.end(); ++it) {
		std::cout <<"\"" << *it << "\"";

		if (std::next(it) != osunset.end()) {
			std::cout << ", ";
		}
	}

	return os;
}

	void JSONReader::ReadJson(std::istream& input) {
		Dict json = Load(input).GetRoot().AsMap();
		Array base_requests = json.at("base_requests").AsArray();
		Array stat_requests = json.at("stat_requests").AsArray();
		Dict render_settings = json.at("render_settings").AsMap();

		ProcessingBasicRequests(base_requests);
        ProcessingStatisticsRequests(stat_requests);
		ProcessingCardSettings(render_settings);

		out_requests = ProcessRequest(add_query, directory_requests, transport_catalogue_);
	}


	void JSONReader::ConclusionJson(std::ostream& output)
	{
		if (out_requests.empty()) {
			return;
		}

		std::map<std::string_view, Coordinates> stops_on_buses;
		std::vector<Bus*> buses;
		for (const auto& [name, bus] : transport_catalogue_.GetInfoAllBus()) {
			buses.push_back(bus);
			for (const auto& stop : bus->stops) {
				stops_on_buses[stop->name] = stop->cordinat;
			}
		}

		output << "[" << std::endl;
		for (size_t i = 0; i < out_requests.size(); ++i) {

			if (std::holds_alternative<std::string>(out_requests[i])) {
				output << "{" << std::endl;
				output << "\"request_id\": " << directory_requests[i].id << "," << std::endl;
				output << "\"error_message\" : \"not found\"" << std::endl;
				output << "}" << std::endl;
				if (i != out_requests.size() - 1) {
					output << ",";
				}
				continue;
			}
			if (directory_requests[i].type == "Bus") {

				Bus* bus = std::get<Bus*>(out_requests[i]);
				std::unordered_set<Stop*> uniqueStops;
				for (const auto& stop : (*bus).stops) {
					uniqueStops.insert(stop);
				}

				Print(Document{ Dict{
					{"curvature"s, static_cast<double>(bus->distant_bus.second)},
					{"request_id"s,static_cast<int>(directory_requests[i].id)},
					{"route_length"s,static_cast<double>(bus->distant_bus.first)},
					{"stop_count"s, static_cast<double>((*bus).stops.size())},
					{"unique_stop_count"s,static_cast<double>(uniqueStops.size())},
				} }, output);
				output << std::endl;
			}
			else if (directory_requests[i].type == "Stop") {


				output << "{" << std::endl;
				output << "\"buses\": [" << std::endl;
				output << std::get<std::set<std::string_view>>(out_requests[i]);
				output << "]," << std::endl;
				output << "\"request_id\" : " << directory_requests[i].id << std::endl;
				output << "}" << std::endl;
			}
			else if (directory_requests[i].type == "Map") {
				std::ifstream in;
				std::ofstream out;
				MapRenderer map_render(map_settings, stops_on_buses, buses);

				output << "{" << std::endl;
				output << "\"map\": " << std::endl;
				out.open("svg.text");

				map_render.DrawSvg(out);

				in.open("svg.text");
				Print(Document{ Dict{
					{"map",std::string((std::istreambuf_iterator<char>(in)),
					std::istreambuf_iterator<char>())},
					{"request_id",static_cast<double>(directory_requests[i].id)}
				} }, output);
				
				in.close();
				out.close();
				output << "," << std::endl;
				output << "\"request_id\" : " << directory_requests[i].id << std::endl;
				output << "}" << std::endl;

			}
			if (i != out_requests.size() - 1) {
				output << ",";
			}

		}
		output << "]" << std::endl;
	}

	void JSONReader::ProcessingBasicRequests(Array& base_requests) {
		for (const auto& base_request : base_requests) {
			Dict stop_bus = base_request.AsMap();
			if (stop_bus.at("type").AsString() == "Stop") {
				StopQuery stop;
				stop.name = stop_bus.at("name").AsString();
				stop.lat = stop_bus.at("latitude").AsDouble();
				stop.lng = stop_bus.at("longitude").AsDouble();


				for (const auto& [name, distant] : stop_bus.at("road_distances").AsMap()) {
					stop.distance_to.push_back({ name,distant.AsDouble() });
				}

				add_query.stop_query.push_back(stop);
			}
			else {
				BusQuery bus;
				std::vector<Node> stops = stop_bus.at("stops").AsArray();

				bus.name = stop_bus.at("name").AsString();

				if (stops.empty()) {
					continue;
				}

				if (stop_bus.at("is_roundtrip").AsBool()) {
					for (const auto& stop : stops) {
						bus.stops.push_back(stop.AsString());
					}
				}
				else {

					for (const auto& stop : stops) {
						bus.stops.push_back(stop.AsString());
					}
					if (bus.stops.back() != *bus.stops.begin()) {
						bus.last_stop = bus.stops.back();
					}

					for (auto it = stops.end() - 2; it != stops.begin(); --it) {
						bus.stops.push_back((*it).AsString());
					}
					bus.stops.push_back(stops[0].AsString());
				}
				add_query.bus_query.push_back(bus);
			}
		}
	}
	void JSONReader::ProcessingStatisticsRequests(Array& stat_requests) {
		for (const auto& stat_request : stat_requests) {
			Dict stop_bus = stat_request.AsMap();
			DirectoryRequest directory_request;
			directory_request.id = stop_bus.at("id").AsInt();
			directory_request.type = stop_bus.at("type").AsString();
			if (directory_request.type != "Map") {
				directory_request.name = stop_bus.at("name").AsString();
			}

			directory_requests.push_back(directory_request);
		}
	}
	void JSONReader::ProcessingCardSettings(Dict& render_settings) {
		map_settings.width_ = render_settings.at("width").AsDouble();
		map_settings.height_ = render_settings.at("height").AsDouble();

		map_settings.padding_ = render_settings.at("padding").AsDouble();

		map_settings.line_width_ = render_settings.at("line_width").AsDouble();
		map_settings.stop_radius = render_settings.at("stop_radius").AsDouble();

		map_settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
		Array array = render_settings.at("bus_label_offset").AsArray(); //: [7.0, 15.0] ,
		map_settings.bus_label_offset = { array[0].AsDouble(),array[1].AsDouble() };

		map_settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();

		array = render_settings.at("stop_label_offset").AsArray();// : [7.0, -3.0] ,
		map_settings.stop_label_offset = { array[0].AsDouble(),array[1].AsDouble() };


		if (render_settings.at("underlayer_color").IsArray()) {
			array = render_settings.at("underlayer_color").AsArray();
			if (array.size() == 3) {

				map_settings.underlayer_color = svg::Rgb{ static_cast<uint8_t>(array[0].AsDouble()),static_cast<uint8_t>(array[1].AsDouble()),static_cast<uint8_t>(array[2].AsDouble()) };
			}
			else {
				map_settings.underlayer_color = svg::Rgba{ static_cast<uint8_t>(array[0].AsDouble()),static_cast<uint8_t>(array[1].AsDouble()),static_cast<uint8_t>(array[2].AsDouble()),array[3].AsDouble() };
			}
		}
		else if (render_settings.at("underlayer_color").IsString()) {
			map_settings.underlayer_color = render_settings.at("underlayer_color").AsString();
		}
		else {
			map_settings.underlayer_color = std::monostate{};
		}


		map_settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

		Array array2 = render_settings.at("color_palette").AsArray();
		for (const auto& a : array2) {

			if (a.IsArray()) {
				array = a.AsArray();
				if (array.size() == 3) {

					map_settings.color_palette.push_back(svg::Rgb{static_cast<uint8_t>(array[0].AsDouble()), static_cast<uint8_t>(array[1].AsDouble()), static_cast<uint8_t>(array[2].AsDouble())});
				}
				else {
					map_settings.color_palette.push_back(svg::Rgba{static_cast<uint8_t>(array[0].AsDouble()), static_cast<uint8_t>(array[1].AsDouble()), static_cast<uint8_t>(array[2].AsDouble()), array[3].AsDouble()});
				}
			}
			else if (a.IsString()) {
				map_settings.color_palette.push_back(a.AsString());
			}
			else {
				map_settings.color_palette.push_back(std::monostate{});
			}
		}
	}


//void ReadJson(std::istream& input, TransportCatalogue& transport_catalogue) {
//	AddQuery add_query;
//	std::vector<DirectoryRequest> directory_requests;
//	Dict json = Load(input).GetRoot().AsMap();
//	Array base_requests = json.at("base_requests").AsArray();
//	Array stat_requests = json.at("stat_requests").AsArray();
//	Dict render_settings = json.at("render_settings").AsMap();
//
//
//	for (const auto& base_request : base_requests) {
//		Dict stop_bus = base_request.AsMap();
//		if (stop_bus.at("type").AsString() == "Stop") {
//			StopQuery stop;
//			stop.name = stop_bus.at("name").AsString();
//			stop.lat = stop_bus.at("latitude").AsDouble();
//			stop.lng = stop_bus.at("longitude").AsDouble();
//			
//
//			for (const auto& [name, distant] : stop_bus.at("road_distances").AsMap()) {
//				stop.distance_to.push_back({ name,distant.AsDouble() });
//			}
//
//			add_query.stop_query.push_back(stop);
//		}
//		else {
//			BusQuery bus;
//			std::vector<Node> stops = stop_bus.at("stops").AsArray();
//
//			bus.name = stop_bus.at("name").AsString();
//
//			if (stops.empty()) {
//				continue;
//			}
//
//			if (stop_bus.at("is_roundtrip").AsBool()) {
//				for (const auto& stop : stops) {
//					bus.stops.push_back(stop.AsString());
//				}
//			}
//			else {
//
//				for (const auto& stop : stops) {
//					bus.stops.push_back(stop.AsString());
//				}
//				if (bus.stops.back() != *bus.stops.begin()) {
//					bus.last_stop = bus.stops.back();
//				}
//				
//				for (auto it = stops.end() - 2; it != stops.begin(); --it) {
//					bus.stops.push_back((*it).AsString());
//				}
//				bus.stops.push_back(stops[0].AsString());
//			}
//			add_query.bus_query.push_back(bus);
//		}
//	}
//
//	for (const auto& stat_request : stat_requests) {
//		Dict stop_bus = stat_request.AsMap();
//		DirectoryRequest directory_request;
//		directory_request.id = stop_bus.at("id").AsInt();
//		directory_request.type = stop_bus.at("type").AsString();
//		if (directory_request.type != "Map") {
//			directory_request.name = stop_bus.at("name").AsString();
//		}
//		
//		directory_requests.push_back(directory_request);
//	}
//
//	MapSettings map_settings;
//	{
//		map_settings.width_= render_settings.at("width").AsDouble();
//		map_settings.height_= render_settings.at("height").AsDouble();
//
//		map_settings.padding_= render_settings.at("padding").AsDouble();
//
//		map_settings.line_width_ = render_settings.at("line_width").AsDouble();
//		map_settings.stop_radius =render_settings.at("stop_radius").AsDouble();
//
//		map_settings.bus_label_font_size= render_settings.at("bus_label_font_size").AsInt();
//		Array array= render_settings.at("bus_label_offset").AsArray(); //: [7.0, 15.0] ,
//		map_settings.bus_label_offset = { array[0].AsDouble(),array[1].AsDouble() };
//
//		map_settings.stop_label_font_size= render_settings.at("stop_label_font_size").AsInt();
//
//		array = render_settings.at("stop_label_offset").AsArray();// : [7.0, -3.0] ,
//		map_settings.stop_label_offset= { array[0].AsDouble(),array[1].AsDouble() };
//
//
//		if (render_settings.at("underlayer_color").IsArray()) {
//			array = render_settings.at("underlayer_color").AsArray();
//			if (array.size() == 3) {
//				
//				map_settings.underlayer_color = svg::Rgb{ static_cast<uint8_t>(array[0].AsDouble()),static_cast<uint8_t>(array[1].AsDouble()),static_cast<uint8_t>(array[2].AsDouble()) };
//			}
//			else {
//				map_settings.underlayer_color = svg::Rgba{ static_cast<uint8_t>(array[0].AsDouble()),static_cast<uint8_t>(array[1].AsDouble()),static_cast<uint8_t>(array[2].AsDouble()),array[3].AsDouble() };
//			}
//		}
//		else if(render_settings.at("underlayer_color").IsString()){
//			map_settings.underlayer_color = render_settings.at("underlayer_color").AsString();
//		}
//		else {
//			map_settings.underlayer_color = std::monostate{};
//		}
//			
//
//		map_settings.underlayer_width= render_settings.at("underlayer_width").AsDouble();
//
//		Array array2 = render_settings.at("color_palette").AsArray(); 
//		for (const auto& a : array2) {
//			
//			if (a.IsArray()) {
//				array = a.AsArray();
//				if (array.size() == 3) {
//
//					map_settings.color_palette.push_back(svg::Rgb{static_cast<uint8_t>(array[0].AsDouble()),static_cast<uint8_t>(array[1].AsDouble()),static_cast<uint8_t>(array[2].AsDouble())});
//				}
//				else {
//					map_settings.color_palette.push_back(svg::Rgba{static_cast<uint8_t>(array[0].AsDouble()),static_cast<uint8_t>(array[1].AsDouble()),static_cast<uint8_t>(array[2].AsDouble()),array[3].AsDouble()});
//				}
//			}
//			else if (a.IsString()) {
//				map_settings.color_palette.push_back(a.AsString());
//			}
//			else {
//				map_settings.color_palette.push_back(std::monostate{});
//			}
//		}
//	}
//
//	OutRequests out_requests = ProcessRequest(add_query, directory_requests,  transport_catalogue);
//	std::map<std::string_view, Coordinates> stops_on_buses;
//	std::vector<Bus*> buses;
//	for (const auto& [name, bus] : transport_catalogue.GetInfoAllBus()) {
//		buses.push_back(bus);
//		for (const auto& stop : bus->stops) {
//			stops_on_buses[stop->name]=stop->cordinat;
//		}
//	}
//	
//	//DrawSvg(std::cout,  map_settings, stops_on_buses, buses);
//
//	ConclusionJson(std::cout, out_requests, directory_requests, map_settings, stops_on_buses, buses);
//	
//}

//void ConclusionJson(std::ostream& output, OutRequests& out_requests, std::vector<DirectoryRequest>& directory_requests, MapSettings& map_settings, std::map<std::string_view, Coordinates>& stops_on_buses, std::vector<Bus*>& buses)
//{
//	if (out_requests.empty()) {
//		return;
//	}
//	output << "[" << std::endl;
//	for (size_t i = 0; i < out_requests.size(); ++i) {
//
//		if (std::holds_alternative<std::string>(out_requests[i])) {
//			output << "{" << std::endl;
//			output << "\"request_id\": " << directory_requests[i].id << "," << std::endl;
//			output << "\"error_message\" : \"not found\"" << std::endl;
//			output << "}" << std::endl;
//			if (i != out_requests.size() - 1) {
//				output << ",";
//			}
//			continue;
//		}
//		if (directory_requests[i].type == "Bus") {
//
//			Bus* bus = std::get<Bus*>(out_requests[i]);
//			std::unordered_set<Stop*> uniqueStops;
//			for (const auto& stop : (*bus).stops) {
//				uniqueStops.insert(stop);
//			}
//
//			Print(Document{ Dict{
//				{"curvature"s, static_cast<double>(bus->distant_bus.second)},
//				{"request_id"s,static_cast<int>(directory_requests[i].id)},
//				{"route_length"s,static_cast<double>(bus->distant_bus.first)},
//				{"stop_count"s, static_cast<double>((*bus).stops.size())},
//				{"unique_stop_count"s,static_cast<double>(uniqueStops.size())},
//			} }, output);
//			output << std::endl;
//		}
//		else if (directory_requests[i].type == "Stop") {
//
//
//			output << "{" << std::endl;
//			output << "\"buses\": [" << std::endl;
//			output << std::get<std::set<std::string_view>>(out_requests[i]);
//			output << "]," << std::endl;
//			output << "\"request_id\" : " << directory_requests[i].id << std::endl;
//			output << "}" << std::endl;
//		}
//		else if (directory_requests[i].type == "Map") {
//			std::ifstream in;
//			std::ofstream out;
//			
//			output << "{" << std::endl;
//			output << "\"map\": " << std::endl;
//			out.open("svg.text");
//			
//			DrawSvg(out, map_settings, stops_on_buses, buses);
//			
//			in.open("svg.text");
//			Print(Document{ Dict{
//				{"map",std::string((std::istreambuf_iterator<char>(in)),
//				std::istreambuf_iterator<char>())},
//				{"request_id",static_cast<double>(directory_requests[i].id)}
//			} }, output);
//			/*Print(Load(in), output);*/
//			in.close();
//			out.close();
//			output << "," << std::endl;
//			output << "\"request_id\" : " << directory_requests[i].id << std::endl;
//			output << "}" << std::endl;
//
//
//
//
//			
//			
//		}
//		if (i != out_requests.size() - 1) {
//			output << ",";
//		}
//
//	}
//	output << "]" << std::endl;
//}

