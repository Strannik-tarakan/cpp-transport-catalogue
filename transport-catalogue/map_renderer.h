#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <unordered_set>
#include <string_view>

struct MapSettings {
    double width_;
    double height_;
    double padding_;
    double line_width_;
    double stop_radius;
    int bus_label_font_size;
    std::pair<double,double> bus_label_offset;
    int stop_label_font_size;
    std::pair<double, double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;

bool IsZero(double value);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) 
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.second.lng < rhs.second.lng; });
        min_lon_ = left_it->second.lng;
        const double max_lon = right_it->second.lng;
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.second.lat < rhs.second.lat; });
        const double min_lat = bottom_it->second.lat;
        max_lat_ = top_it->second.lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    MapRenderer(MapSettings& map_settings, std::map<std::string_view, Coordinates>& stops_on_buses, std::vector<Bus*>& buses);

    void DrawSvg(std::ostream& output);

private:
    const MapSettings& map_settings;
    const std::map<std::string_view, Coordinates>& stops_on_buses;
    std::vector<Bus*>& buses;
    const SphereProjector proj;
    svg::Document document;

    void AddRouteLines();
    void AddNamesBuses();
    void AddBreakpoints();
    void AddNamesStops();
};

