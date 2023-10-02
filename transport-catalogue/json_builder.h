#pragma once
#include "json.h"
#include <vector>
#include <variant>
#include <optional>

namespace json {

    class Builder {
    private:
        class KeyItemContext;
        class ArrayItemContext;

        class ValueKeyItemContext {
        public:
            ValueKeyItemContext(Builder& bul) :bul_(bul) {}

            KeyItemContext Key(const std::string& key) {

                return bul_.Key(key);
            }
            Builder& EndDict() {
                return bul_.EndDict();
            }
        private:
            Builder& bul_;
        };

        class DictItemContext {
        public:
            DictItemContext(Builder& bul) :bul_(bul) {}

            KeyItemContext Key(const std::string& key) {
                return bul_.Key(key);
            }
            Builder& EndDict() {
                return bul_.EndDict();
            }
        private:
            Builder& bul_;
        };

        class KeyItemContext {
        public:
            KeyItemContext(Builder& bul) :bul_(bul) {}

            ValueKeyItemContext Value(Node::Value&& value) {
                return bul_.Value(std::move(value));
            }
            DictItemContext StartDict() {
                return bul_.StartDict();
            }
            ArrayItemContext StartArray() {
                return bul_.StartArray();
            }
        private:
            Builder& bul_;
        };

        class ValueArrayItemContext {
        public:
            ValueArrayItemContext(Builder& bul) :bul_(bul) {}

            Builder& EndArray() {
                return bul_.EndArray();
            }
            ValueArrayItemContext Value(Node::Value&& value) {
                return bul_.Value(std::move(value));
            }
            DictItemContext StartDict() {
                return bul_.StartDict();
            }
            ArrayItemContext StartArray() {
                return bul_.StartArray();
            }
        private:
            Builder& bul_;
        };

        class ArrayItemContext {
        public:
            ArrayItemContext(Builder& bul) :bul_(bul) {}

            Builder& EndArray() {
                return bul_.EndArray();
            }
            ValueArrayItemContext Value(Node::Value&& value) {
                return bul_.Value(std::move(value));
            }
            DictItemContext StartDict() {
                return bul_.StartDict();
            }
            ArrayItemContext StartArray() {
                return bul_.StartArray();
            }
        private:
            Builder& bul_;
        };


    public:
        json::Node Build();
        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value&& value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();

    private:
        Node root_;
        std::optional< std::string> key_;
        std::vector<Node*> nodes_stack_;
        int _i_ = 0;

        Node CreateNode(Node::Value&& value);
    };

}