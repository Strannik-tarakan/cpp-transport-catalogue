#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>


namespace svg {
    struct Rgb {
        Rgb(uint8_t red, uint8_t green, uint8_t blue)
            :red(red), green(green), blue(blue) {

        }
        Rgb() = default;
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba {
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
            :red(red), green(green), blue(blue), opacity(opacity) {

        }
        Rgba() = default;
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{"none"};
    using namespace std::literals;
    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {

            out << "none"sv;
        }
        void operator()(std::string color) const {
            out << color;
        }
        void operator()(svg::Rgb color) const {
            out << "rgb("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ")"sv;
        }
        void operator()(svg::Rgba color) const {
            out << "rgba("sv << static_cast<int>(color.red) << ","sv << static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity << ")"sv;
        }
    };


    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };
    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    inline std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }
    inline std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap) {
        switch (cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case  StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case  StrokeLineCap::SQUARE:
            out << "square"sv;
            break;

        }
        return out;
    }
    inline std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join) {
        switch (join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;

        }
        return out;
    }

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color);
        Owner& SetStrokeColor(Color color);
        Owner& SetStrokeWidth(double width);
        Owner& SetStrokeLineCap(StrokeLineCap line_cap);
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join);
    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const;

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:

        template <typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }


        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;


    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);
    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_ = { 0,0 };
        Point offset_ = { 0,0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class Document :public ObjectContainer {
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    private:
    };

    // ---------- PathProps------------------//

    template<typename Owner>
    inline Owner& PathProps<Owner>::SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    template<typename Owner>
    inline Owner& PathProps<Owner>::SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    template<typename Owner>
    inline Owner& PathProps<Owner>::SetStrokeWidth(double width)
    {
        width_ = std::move(width);
        return AsOwner();
    }
    template<typename Owner>
    inline Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap)
    {
        line_cap_ = std::move(line_cap);
        return AsOwner();
    }
    template<typename Owner>
    inline Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join)
    {
        line_join_ = std::move(line_join);
        return AsOwner();
    }

    template<typename Owner>
    inline void PathProps<Owner>::RenderAttrs(std::ostream& out) const
    {


        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(ColorPrinter{ out }, *fill_color_);
            out << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{ out }, *stroke_color_);
            out << "\""sv;
        }
        if (width_) {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

}  // namespace svg
