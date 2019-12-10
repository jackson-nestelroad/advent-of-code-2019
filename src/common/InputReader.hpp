#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <functional>
#include <string_view>

class InputReader {
private:
    // Represents a single separated value in an input stream
    template <typename T, char delim = '\n'>
    class Value {
    private:
        T data;

    public:
        friend std::istream &operator>>(std::istream &input, Value<T, delim> &value) {
            if (!(input >> value.data)) {
                return input;
            }
            if (input.peek() == delim) {
                input.ignore();
            }
            else {
                input.clear();
            }
            return input;
        }

        operator T() const {
            return data;
        }
    };
    // Represents a single line in an input stream
    template <char delim = '\n'>
    class Line {
    private:
        std::string data;

    public:
        friend std::istream &operator>>(std::istream &input, Line<delim> &line) {
            std::getline(input, line.data, delim);
            return input;
        }

        operator std::string() const {
            return data;
        }
    };

    static std::ifstream openFile(std::string_view fileName) {
        std::ifstream file { fileName.data() };
        if (!file) {
            std::cerr << "Failed to open file " << fileName << std::endl;
            exit(-1);
        }
        return file;
    }

public:
    template <typename Out, char delim = '\n'>
    static std::vector<Out> readFile(std::string_view fileName) {
        auto file = openFile(fileName);

        return std::vector<Out>(std::istream_iterator<Value<Out, delim>>(file), std::istream_iterator<Value<Out, delim>>());
    }

    template <typename In, typename Out, char delim = '\n'>
    static std::vector<Out> readFile(std::string_view fileName, const std::function<Out(const In &)> &parser) {
        auto file = openFile(fileName);

        std::vector<Out> readInput;
        std::transform(std::istream_iterator<Value<Out, delim>>(file), std::istream_iterator<Value<Out, delim>>(), readInput.begin(), parser);
        return readInput;
    }

    template <char delim = '\n'>
    static std::vector<std::string> readLines(std::string_view fileName) {
        auto file = openFile(fileName);

        return std::vector<std::string>(std::istream_iterator<Line<delim>>(file), std::istream_iterator<Line<delim>>());
    }

    template <typename Out, char delim = '\n'>
    static std::vector<Out> readLines(std::string_view fileName, const std::function<Out(const std::string &)> &parser) {
        auto file = openFile(fileName);

        std::vector<Out> readInput;
        std::transform(std::istream_iterator<Line<delim>>(file), std::istream_iterator<Line<delim>>(), readInput.begin(), parser);
        return readInput;
    }
};