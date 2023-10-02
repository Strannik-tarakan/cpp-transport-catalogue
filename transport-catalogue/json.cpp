#include "json.h"


using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw json::ParsingError("Array empty"s);
            }
            return Node(move(result));
        }

        class ParsingError : public std::runtime_error {
        public:
            using runtime_error::runtime_error;
        };

        using Number = std::variant<int, double>;

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw json::ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw json::ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw json::ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw json::ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw json::ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw json::ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw json::ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (!input) {
                throw json::ParsingError("Map empty"s);
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {

                input >> c;

                input >> c;

                if (input >> c) {
                    if (c == 'l') {
                        return Node(move(nullptr_t()));
                    }
                }


                throw json::ParsingError(" !=null");
            }
            else if (c == 't') {
                input >> c;
                input >> c;
                input >> c;
                if (c == 'e') {
                    return Node(move(true));
                }
                throw json::ParsingError(" !=true");
            }
            else if (c == 'f') {
                input >> c;
                input >> c;
                input >> c;
                input >> c;
                if (c == 'e') {
                    return Node(move(false));
                }
                throw json::ParsingError(" !=false");
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    //--------------------------------------Node-----------------------------------------------------//

    Node::Node(Array array)
        : value_(move(array)) {
    }
    Node::Node(Dict map)
        : value_(move(map)) {
    }
    Node::Node(int value)
        : value_(value) {
    }
    Node::Node(string value)
        : value_(move(value)) {
    }
    Node::Node(double value)
        : value_(move(value))
    {
    }
    Node::Node(nullptr_t value)
        : value_(move(value))
    {
    }
    Node::Node(bool value)
        : value_(move(value))
    {
    }

    bool Node::IsInt() const
    {
        return holds_alternative<int>(value_);
    }
    bool Node::IsString() const
    {
        return holds_alternative<string>(value_);
    }
    bool Node::IsArray() const
    {
        return holds_alternative<Array>(value_);;
    }
    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(value_);;
    }
    bool Node::IsDouble() const
    {
        return holds_alternative<int>(value_) || holds_alternative<double>(value_);;
    }
    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(value_);
    }
    bool Node::IsBool() const
    {
        return holds_alternative<bool>(value_);;
    }
    bool Node::IsNull() const
    {
        return holds_alternative<nullptr_t>(value_);;
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(value_);
        }
        throw logic_error("wrong type");
    }
    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return get<Dict>(value_);
        }
        throw logic_error("wrong type");
    }
    Array& Node::AsArray() {
        if (IsArray()) {
            return get<Array>(value_);
        }
        throw logic_error("wrong type");
    }
    Dict& Node::AsMap() {
        if (IsMap()) {
            return get<Dict>(value_);
        }
        throw logic_error("wrong type");
    }
    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(value_);
        }
        throw logic_error("wrong type");
    }
    const string& Node::AsString() const {
        if (IsString()) {
            return get<string>(value_);
        }
        throw logic_error("wrong type");
    }
    bool Node::AsBool() const
    {
        if (IsBool()) {
            return get<bool>(value_);
        }
        throw logic_error("wrong type");
    }
    double Node::AsDouble() const
    {
        if (IsDouble()) {
            if (IsPureDouble()) {
                return get<double>(value_);
            }
            return static_cast<double>(get<int>(value_));
        }
        throw logic_error("wrong type");
    }

    bool  Node::operator==(const Node& node) const {

        return this->value_ == node.value_;
    }
    bool  Node::operator!=(const Node& node) const {

        return !(*this == node);
    }

    //--------------------------------------NodePrinter--------------------------------------//
    void NodePrinter::operator()(std::string node) const {

        std::string str;
        for (auto it = node.begin(); it != node.end(); ++it) {
            switch (*it) {
            case '\n':
                str.push_back('\\');
                str.push_back('n');
                break;
            case '\t':
                str.push_back('\t');
                break;
            case '\r':
                str.push_back('\\');
                str.push_back('r');
                break;
            case '\"':
                str.push_back('\\');
                str.push_back('\"');
                break;
            case '\\':
                str.push_back('\\');
                str.push_back('\\');
                break;
            default:
                str.push_back(*it);
            }
        }
        os << "\"" << str << "\"";
    }
    void NodePrinter::operator()(int node) const {
        os << node;
    }
    void NodePrinter::operator()(double node) const {
        os << node;
    }
    void NodePrinter::operator()(nullptr_t) const {
        os << "null"sv;
    }
    void NodePrinter::operator()(bool node) const {
        os << std::boolalpha << node << std::noboolalpha;
    }
    void NodePrinter::operator()(Array node) const {
        os << "["sv;
        for (auto it = node.begin(); it != node.end(); ++it) {
            if (it == node.begin()) {
                std::visit(NodePrinter{ os }, (*it).GetValue());
                continue;
            }
            os << ", "sv;
            std::visit(NodePrinter{ os }, (*it).GetValue());
        }
        os << "]"sv;
    }
    void NodePrinter::operator()(Dict node) const {
        os << "{"sv;
        for (auto n : node) {
            if (n == *node.begin()) {
                os << "\"" << n.first << "\"" << ": ";
                std::visit(NodePrinter{ os }, n.second.GetValue());
                continue;
            }
            os << ", "sv;
            os << "\"" << n.first << "\"" << ": ";
            std::visit(NodePrinter{ os }, n.second.GetValue());
        }
        os << " }"sv;
    }

    //--------------------------------------Document--------------------------------------//

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& doc) const {

        return this->root_ == doc.root_;
    }
    bool Document::operator!=(const Document& doc) const {

        return this->root_ != doc.root_;
    }

    //----------------------------------------------------------------------------//

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {

        std::visit(NodePrinter{ output }, doc.GetRoot().GetValue());

    }

}  // namespace json