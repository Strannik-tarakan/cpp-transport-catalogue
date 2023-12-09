#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    using namespace std::literals;

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(std::string value);
        Node(double value);
        Node(nullptr_t value);
        Node(bool value);

        bool IsInt() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsNull() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        Array& AsArray() ;
        Dict& AsMap() ;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        const Value& GetValue() const { return value_; }

        bool operator==(const Node& node) const;
        bool operator!=(const Node& node) const;

    private:
        Value value_;
    };

    struct NodePrinter {

        std::ostream& os;

        void operator()(std::string node) const;
        void operator()(int node) const;
        void operator()(double node) const;
        void operator()(nullptr_t) const;
        void operator()(bool node) const;
        void operator()(Array node) const;
        void operator()(Dict node) const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& doc) const;
        bool operator!=(const Document& doc) const;
    private:
        Node root_;
    };


    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
