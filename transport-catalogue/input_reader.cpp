#include "input_reader.h"


#include <algorithm>

void ParsingStop(Query& query,size_t it,std::string& text)
{
    auto it_end = text.find(':');
    it = it + 5;
    StopQuery stop;
    stop.name = text.substr(it, it_end - it);
    for (int i = 0; i < 2; ++i) {
        ++it_end;
        it = text.find_first_not_of(' ', it_end);
        it_end = text.find(',', it);
        stop.y = std::stod(text.substr(it, it_end - it));
    }

    while (it_end != std::string::npos) {
        int distant;
        ++it_end;
        it = text.find_first_not_of(' ', it_end);
        it_end = text.find('m', it);
        distant = std::stoi(text.substr(it, it_end - it));

        it = text.find('o', it);
        ++it;
        it = text.find_first_not_of(' ', it);
        it_end = text.find(',', it);
        stop.distance_to.push_back({ text.substr(it,it_end - it),distant });
    }

    query.stop_query.push_back(stop);
}

void ParsingBus(Query& query, size_t it, std::string& text)
{
    auto it_end = text.find(':');
    it = it + 4;
    BusQuery bus;
    bus.name = text.substr(it, it_end - it);
    ++it_end;
    std::vector<std::string> stop;

    if (text.find('>') != std::string::npos) {
        it = text.find_first_not_of(' ', it_end);
        auto it_delimiter = text.find('>', it);
        it_end = text.find_last_not_of(' ', it_delimiter - 1);

        while (it_delimiter != std::string::npos) {
            ++it_end;
            stop.push_back(text.substr(it, it_end - it));
            it = text.find_first_not_of(' ', it_delimiter + 1);
            it_delimiter = text.find('>', it);
            it_end = text.find_last_not_of(' ', it_delimiter - 1);
        }

        ++it_end;
        stop.push_back(text.substr(it, it_end - it));
    }
    else {
        it = text.find_first_not_of(' ', it_end);
        auto it_delimiter = text.find('-', it);
        it_end = text.find_last_not_of(' ', it_delimiter - 1);

        while (it_delimiter != std::string::npos) {
            ++it_end;
            stop.push_back(text.substr(it, it_end - it));
            it = text.find_first_not_of(' ', it_delimiter + 1);
            it_delimiter = text.find('-', it);
            it_end = text.find_last_not_of(' ', it_delimiter - 1);
        }

        ++it_end;
        stop.push_back(text.substr(it, it_end - it));

        for (int i = stop.size() - 2; i >= 0; --i) {
            stop.push_back(stop[i]);
        }
    }

    bus.stops = { stop.begin(),stop.end() };
    query.bus_query.push_back(bus);
}

void ReadDatabase(std::istream& in, TransportCatalogue& transport_catalogue)
{
    Query query;
    int quantity_query;
    in >> quantity_query;
    in.ignore(); 

    for (int i = 0; i < quantity_query; ++i) {
        std::string text;
        std::getline(in, text);
        auto it = text.find_first_not_of(' ');

        if (text.substr(it, 4) == "Stop") {
            ParsingStop(query,it,text);
        }
        else if (text.substr(it, 3) == "Bus") {
            ParsingBus(query, it, text);
        }

    }

    for (auto& stop : query.stop_query) {
        transport_catalogue.AddStop(stop.name, stop.x, stop.y,stop.distance_to);

        for (auto& s : stop.distance_to) {
            transport_catalogue.AddDistanceBetweenStops(stop.name, s.first, s.second);
        }
    }
    for (auto& bus : query.bus_query) {
        transport_catalogue.AddBus(bus.name, bus.stops);
    }
}
