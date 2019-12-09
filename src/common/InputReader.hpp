#ifndef INPUTREADER_HPP
#define INPUTREADER_HPP

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
    // Represents a single line in an input stream
    class Line {
    private:
        std::string data;

    public:
        friend std::istream &operator>>(std::istream &input, Line &line) {
            std::getline(input, line.data);
            return input;
        }

        operator std::string() const {
            return data;
        }
    };

public:
    template <typename T>
    static std::vector<T> readFile(std::string_view fileName, const std::function<T(const std::string &)> parser) {
        std::ifstream file { fileName.data() };
        if (!file) {
            std::cerr << "Failed to open file " << fileName << std::endl;
            exit(-1);
        }

        std::vector<T> readInput;
        std::transform(std::istream_iterator<Line>(file), std::istream_iterator<Line>(), std::back_inserter(readInput), parser);
        return readInput;
    }

    static std::vector<std::string> readFile(std::string_view fileName) {
        std::ifstream file { fileName.data() };
        if (!file) {
            std::cerr << "Failed to open file " << fileName << std::endl;
            exit(-1);
        }

        return std::vector<std::string>(std::istream_iterator<Line>(file), std::istream_iterator<Line>());
    }
};

#endif