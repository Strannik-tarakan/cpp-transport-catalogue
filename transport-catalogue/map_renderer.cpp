#include "map_renderer.h"

#include <algorithm>
#include <string>
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}


MapRenderer::MapRenderer(MapSettings& map_settings, std::map<std::string_view, Coordinates>& stops_on_buses, std::vector<Bus*>& buses)
    :map_settings(map_settings),
    stops_on_buses(stops_on_buses),
    buses(buses),
    proj(stops_on_buses.begin(), stops_on_buses.end(), map_settings.width_, map_settings.height_, map_settings.padding_)
{
    std::sort(buses.begin(), buses.end(), [](const auto& lhs, const auto& rhs) {return lhs->name < rhs->name; });
}

void MapRenderer::DrawSvg(std::ostream& output) {

    AddRouteLines();
    AddNamesBuses();
    AddBreakpoints();
    AddNamesStops();

    document.Render(output);
}

void MapRenderer::AddRouteLines() {
    size_t i = 0;
    for (const auto& bus : buses) {
        if (bus->stops.size() == 0) {
            continue;
        }
        svg::Polyline polyline;
        for (const auto& stop : bus->stops) {
            const svg::Point coord = proj({ stop->cordinat.lat,stop->cordinat.lng });
            polyline.AddPoint(coord);
        }
        polyline.SetFillColor("none");
        polyline.SetStrokeWidth(map_settings.line_width_);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        polyline.SetStrokeColor(map_settings.color_palette[i]);
        i = i == map_settings.color_palette.size() - 1 ? 0 : i + 1;

        document.Add(polyline);
    }
}
void MapRenderer::AddNamesBuses() {
    size_t i = 0;
    for (const auto& bus : buses) {
        if (bus->stops.size() == 0) {
            continue;
        }

        svg::Text underlayer_text;
        svg::Text text;

        underlayer_text.SetPosition(proj({ bus->stops[0]->cordinat.lat,bus->stops[0]->cordinat.lng }))
            .SetOffset({ map_settings.bus_label_offset.first,map_settings.bus_label_offset.second })
            .SetFontSize(map_settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(bus->name)
            .SetFillColor(map_settings.underlayer_color)
            .SetStrokeColor(map_settings.underlayer_color)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeWidth(map_settings.underlayer_width);
        text.SetPosition(proj({ bus->stops[0]->cordinat.lat,bus->stops[0]->cordinat.lng }))
            .SetOffset({ map_settings.bus_label_offset.first,map_settings.bus_label_offset.second })
            .SetFontSize(map_settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(bus->name)
            .SetFillColor(map_settings.color_palette[i]);

        document.Add(underlayer_text);
        document.Add(text);
        if (bus->last_stop != nullptr) {
            underlayer_text.SetPosition(proj({ bus->last_stop->cordinat.lat,bus->last_stop->cordinat.lng }))
                .SetOffset({ map_settings.bus_label_offset.first,map_settings.bus_label_offset.second })
                .SetFontSize(map_settings.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus->name)
                .SetFillColor(map_settings.underlayer_color)
                .SetStrokeColor(map_settings.underlayer_color)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeWidth(map_settings.underlayer_width);
            text.SetPosition(proj({ bus->last_stop->cordinat.lat,bus->last_stop->cordinat.lng }))
                .SetOffset({ map_settings.bus_label_offset.first,map_settings.bus_label_offset.second })
                .SetFontSize(map_settings.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus->name)
                .SetFillColor(map_settings.color_palette[i]);
            document.Add(underlayer_text);
            document.Add(text);
        }

        i = i == map_settings.color_palette.size() - 1 ? 0 : i + 1;
    }
}
void MapRenderer::AddBreakpoints() {
    for (const auto& [name, stop_cordinat] : stops_on_buses) {
        svg::Circle circle;
        circle.SetCenter(proj(stop_cordinat))
            .SetRadius(map_settings.stop_radius)
            .SetFillColor("white");
        document.Add(circle);
    }
}
void MapRenderer::AddNamesStops() {
    for (auto& [name, stop_cordinat] : stops_on_buses) {
        svg::Text underlayer_text;
        svg::Text text;

        underlayer_text.SetPosition(proj(stop_cordinat))
            .SetOffset({ map_settings.stop_label_offset.first,map_settings.stop_label_offset.second })
            .SetFontSize(map_settings.stop_label_font_size)
            .SetFontFamily("Verdana")

            .SetData(static_cast<std::string>(name))
            .SetFillColor(map_settings.underlayer_color)
            .SetStrokeColor(map_settings.underlayer_color)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeWidth(map_settings.underlayer_width);
        text.SetPosition(proj(stop_cordinat))
            .SetOffset({ map_settings.stop_label_offset.first,map_settings.stop_label_offset.second })
            .SetFontSize(map_settings.stop_label_font_size)
            .SetFontFamily("Verdana")

            .SetData(static_cast<std::string>(name))
            .SetFillColor("black");

        document.Add(underlayer_text);
        document.Add(text);
    }
}


