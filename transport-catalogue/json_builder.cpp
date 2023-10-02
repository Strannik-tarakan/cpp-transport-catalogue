#include "json_builder.h"

namespace json {

    json::Node Builder::Build() {
        if ((!nodes_stack_.empty() && root_ != nullptr) || (root_ == nullptr && _i_ == 0)) {
            throw std::logic_error("Invalid JSON - unready described object");
        }
        return root_;
    }
    Builder::KeyItemContext Builder::Key(std::string key) {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && !key_) {
            nodes_stack_.back()->AsMap()[key] = {};
            key_ = key;
        }
        else {
            throw std::logic_error("Invalid JSON - error KEY");
        }
        return *this;
    }
    Builder& Builder::Value(Node::Value&& value) {
       
        if (nodes_stack_.empty()) {
            if (_i_ == 1) {
                throw std::logic_error("Invalid JSON");
            }
            _i_ = 1;
            root_ = std::move(CreateNode(std::move(value)));

        }
        else if (nodes_stack_.back()->IsMap()) {
            if (!key_) {
                throw std::logic_error("Invalid JSON - first you need to call key");
            }
            nodes_stack_.back()->AsMap()[key_.value()] = std::move(CreateNode(std::move(value)));
            key_.reset();
        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().push_back(std::move(CreateNode(std::move(value))));
        }

        return *this;
    }
    Builder::DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty()) {

            root_ = CreateNode(Dict{});
            nodes_stack_.push_back(&root_);
        }
        else if (nodes_stack_.back()->IsMap()) {
            if (!key_) {
                throw std::logic_error("Invalid JSON - first you need to call key");
            }
            nodes_stack_.back()->AsMap()[key_.value()] = std::move(CreateNode(Dict{}));
            nodes_stack_.push_back(&nodes_stack_.back()->AsMap()[key_.value()]);
            key_.reset();
        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().push_back(std::move(CreateNode(Dict{})));
            nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
        }

        return  *this;
    }
    Builder::ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty()) {
            _i_ = 1;
            root_ = CreateNode(Array{});
            nodes_stack_.push_back(&root_);

        }
        else if (nodes_stack_.back()->IsMap()) {
            if (!key_) {
                throw std::logic_error("Invalid JSON - first you need to call key");
            }
            nodes_stack_.back()->AsMap()[key_.value()] = std::move(CreateNode(Array{}));
            nodes_stack_.push_back(&nodes_stack_.back()->AsMap()[key_.value()]);
            key_.reset();
        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArray().push_back(std::move(CreateNode(Array{})));
            nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
        }

        return  *this;

    }
    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap() || key_) {
            throw std::logic_error("Invalid JSON - map was not open");
        }
        /* if (nodes_stack_.size() == 1) {
             root_ = nodes_stack_.back();
         }*/
        nodes_stack_.pop_back();
        return *this;
    }
    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Invalid JSON - array was not open");
        }
        /* if (nodes_stack_.size() == 1) {
             root_ = nodes_stack_.back();
         }*/
        nodes_stack_.pop_back();
        return *this;
    }


    Node Builder::CreateNode(Node::Value&& value) {
        if (std::holds_alternative<double>(value)) {
            return Node(std::get<double>(value));
        }
        if (std::holds_alternative<int>(value)) {
            return Node(std::get<int>(value));
        }
        if (std::holds_alternative<std::string>(value)) {
            return Node(std::get<std::string>(value));
        }
        if (std::holds_alternative<bool>(value)) {
            return Node(std::get<bool>(value));
        }
        if (std::holds_alternative<Dict>(value)) {
            return Node(std::get<Dict>(value));
        }
        if (std::holds_alternative<Array>(value)) {
            return Node(std::get<Array>(value));
        }
        if (std::holds_alternative<nullptr_t>(value)) {
            _i_ = 1;
            return Node(std::get<std::nullptr_t>(value));
        }
        return Node{};
    }


}