#include "svg.h"



namespace svg {
    // ---------- PathProps------------------


    // ---------- Object ------------------
    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------сделан

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ----------  Polyline ------------------сделан

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (auto it = points_.begin(); it != points_.end(); ++it) {
            if (it == points_.begin()) {
                out << it->x << "," << it->y;
                continue;
            }
            out << " " << it->x << "," << it->y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ----------  Text ------------------ сделано

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        std::string escaped_data;
        for (const auto& c : data) {
            switch (c) {
            case '\"':
                escaped_data += "&quot;";
                break;
            case '\'':
                escaped_data += "&apos;";
                break;
            case '<':
                escaped_data += "&lt;";
                break;
            case '>':
                escaped_data += "&gt;";
                break;
            case '&':
                escaped_data += "&amp;";
                break;
            default:
                escaped_data += c;
                break;
            }
        }
        data_ = escaped_data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;

        out << "<text";
        RenderAttrs(out);
        out<<" x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (font_family_.size() != 0) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_.size() != 0) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        
        out << ">"sv << data_ << "<"sv;
        out << "/text>"sv;
    }

    // ----------  Document ------------------


    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));

    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
        for (const auto& object : objects_) {
            object->Render(out);
        }
        out << "</svg>" << std::endl;
    }





}  // namespace svg