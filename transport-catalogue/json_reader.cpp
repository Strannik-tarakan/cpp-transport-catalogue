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
		Dict routing_settings = json.at("routing_settings").AsMap();
		
		ProcessingBasicRequests(base_requests);
        ProcessingStatisticsRequests(stat_requests);
		ProcessingCardSettings(render_settings);
		ProcessingRoutingSettings(routing_settings);

		out_requests = ProcessRequest(add_query, directory_requests, transport_catalogue_, routing_settings_);
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
				PrintQueryBus(output,i);
			}
			else if (directory_requests[i].type == "Stop") {
				PrintQueryStop(output, i);
			}
			else if (directory_requests[i].type == "Map") {
				PrintQueryMap(output, i,stops_on_buses,buses);
			}
			else if (directory_requests[i].type == "Route") {     
				PrintQueryRoute(output, i);
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
			Dict request = stat_request.AsMap();
			DirectoryRequest directory_request;
			directory_request.id = request.at("id").AsInt();
			directory_request.type = request.at("type").AsString();
			if (directory_request.type == "Bus"|| directory_request.type == "Stop") {
				directory_request.name = request.at("name").AsString();
			}
			else if (directory_request.type == "Route") {
				directory_request.from_stop = request.at("from").AsString();
				directory_request.to_stop = request.at("to").AsString();
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
	void JSONReader::ProcessingRoutingSettings(Dict& routing_settings)
	{
		routing_settings_.bus_velocity = routing_settings.at("bus_velocity").AsDouble();
		routing_settings_.bus_wait_time = routing_settings.at("bus_wait_time").AsInt();

	}

	void JSONReader::PrintQueryBus(std::ostream& output, size_t i)
	{
		Bus* bus = std::get<Bus*>(out_requests[i]);
		std::unordered_set<Stop*> uniqueStops;
		for (const auto& stop : (*bus).stops) {
			uniqueStops.insert(stop);
		}

		Print(Document{ Builder{}
			.StartDict()
			.Key("curvature"s).Value(static_cast<double>(bus->distant_bus.second))
			.Key("request_id"s).Value(static_cast<int>(directory_requests[i].id))
			.Key("route_length"s).Value(static_cast<double>(bus->distant_bus.first))
			.Key("stop_count"s).Value(static_cast<double>((*bus).stops.size()))
			.Key("unique_stop_count").Value(static_cast<double>(uniqueStops.size()))
			.EndDict().Build()

			}, output);
		output << std::endl;
	}
	void JSONReader::PrintQueryStop(std::ostream& output, size_t i)
	{
		output << "{" << std::endl;
		output << "\"buses\": [" << std::endl;
		output << std::get<std::set<std::string_view>>(out_requests[i]);
		output << "]," << std::endl;
		output << "\"request_id\" : " << directory_requests[i].id << std::endl;
		output << "}" << std::endl;
	}
	void JSONReader::PrintQueryMap(std::ostream& output, size_t i, 
		std::map<std::string_view, Coordinates>& stops_on_buses, std::vector<Bus*>& buses){

		std::ifstream in;
		std::ofstream out;
		MapRenderer map_render(map_settings, stops_on_buses, buses);

		output << "{" << std::endl;
		output << "\"map\": " << std::endl;
		out.open("svg.text");

		map_render.DrawSvg(out);

		in.open("svg.text");
		Print(Document{
			Builder{}
			.StartDict()
			.Key("map")
			.Value(std::string((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>()))
			.Key("request_id")
			.Value(static_cast<double>(directory_requests[i].id))
			.EndDict()
			.Build()
			}, output);

		in.close();
		out.close();
		output << "," << std::endl;
		output << "\"request_id\" : " << directory_requests[i].id << std::endl;
		output << "}" << std::endl;

	}

	void JSONReader::PrintQueryRoute(std::ostream& output, size_t i){
		OptimalRoute route = std::get<OptimalRoute>(out_requests[i]);
		Builder build;
		build.StartDict()
			.Key("request_id"s).Value(static_cast<int>(directory_requests[i].id))
			.Key("total_time"s).Value(route.total_time)
			.Key("items"s).StartArray();
		for (int i = 0; i < route.elements.size(); ++i) {
			build
				.StartDict()
				.Key("type").Value(route.elements[i].type)
				.Key("stop_name").Value(std::string(route.elements[i].name))
				.Key("time").Value(route.elements[i].time)
				.EndDict();
			++i;
			build.StartDict()
				.Key("type").Value(route.elements[i].type)
				.Key("bus").Value(std::string(route.elements[i].name))
				.Key("span_count").Value(route.elements[i].count)
				.Key("time").Value(route.elements[i].time)
				.EndDict();
		}
		build.EndArray().EndDict();

		Print(Document{ build.Build() }, output);
		output << std::endl;
	}


