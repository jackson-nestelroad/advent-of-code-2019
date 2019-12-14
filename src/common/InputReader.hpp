// Template functions for recursively parsing data from a file

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <functional>
#include <string_view>
#include <any>

namespace in {
    // A thin layer to wrap around a piece of data
    template <typename T>
    struct wrapper {
        T data;
        wrapper() : data() { };
        wrapper(const wrapper &line) = default;
        inline wrapper &operator=(const wrapper &line) = default;
        inline operator T() const {
            return data;
        }
    };

    // An object that can produce a new input stream
    template <typename T>
    struct streamable {
        virtual std::unique_ptr<std::istream> stream() const = 0;
    };

    // Wrapper around a single data value
    template <typename T, char delim = 0>
    struct value : wrapper<T> {
        friend std::istream &operator>>(std::istream &input, value<T, delim> &value) {
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
    };

    // Specialization for no delimiter given
    template <typename T>
    struct value<T, 0> : wrapper<T> {
        friend std::istream &operator>>(std::istream &input, value<T, 0> &value) {
            input >> value.data;
            return input;
        }
    };

    // Wrapper around a single line of data
    template <typename T = std::string, char delim = '\n'>
    struct line;

    // Specialization for std::string, requires std::getline to get full line
    // Can produce a new stream to parse values from
    template <char delim>
    struct line<std::string, delim> : wrapper<std::string>, streamable<std::string> {
        friend std::istream &operator>>(std::istream &in, line<std::string, delim> &line) {
            std::getline(in, line.data, delim);
            return in;
        }

        std::unique_ptr<std::istream> stream() const override {
            return std::unique_ptr<std::istream>(new std::istringstream(data));
        }
    };

    namespace {
        // Opens a file stream directly
        std::ifstream openFile(std::string_view fileName) {
            std::ifstream file { fileName.data() };
            if (!file) {
                console::fatal("Failed to open file ", fileName, ".");
            }
            return file;
        }

        // Opens a file stream pointer to be moved across different functions
        std::unique_ptr<std::istream> openFilePtr(std::string_view fileName) {
            std::unique_ptr<std::ifstream> file(new std::ifstream(fileName.data()));
            if (!*file.get()) {
                console::fatal("Failed to open file ", fileName, ".");
            }
            return file;
        }
    }

    // Namespace for all compile-time constants and template checks
    namespace {
        // Statically check if class is derived from wrapper<T>
        std::false_type is_wrapper_impl(...);
        template <typename T>
        std::true_type is_wrapper_impl(wrapper<T> *);

        // Interface for checking if class is derived from wrapper<T>
        // Use pointer to account for multiple inheritance
        template <typename T>
        using is_wrapper = decltype(is_wrapper_impl(std::declval<T *>()));

        // Interface for checking if all given classes are derived from wrapper<T>
        template <typename... Ts>
        using all_wrappers = std::conjunction<is_wrapper<Ts>...>;

        // Statically check if class is derived from streamable<T>
        std::false_type is_streamable_impl(...);
        template <typename T>
        std::true_type is_streamable_impl(streamable<T> *);

        // Interface for checking if all given classes are derived from streamable<T>
        template <typename T>
        using is_streamable = decltype(is_streamable_impl(std::declval<T *>()));

        // Check if all classes are derived from streamable<T>, recursive case
        template <typename T, typename... Ts>
        struct all_streamable_impl {
            using type = typename std::conjunction<is_streamable<T>, typename all_streamable_impl<Ts...>::type>;
        };

        // Check if all classes are derived from streamable<T>, base case
        // Last class does NOT need to implement streamable since the parsing is finished,
        // so it is always true
        template <typename T>
        struct all_streamable_impl<T> {
            using type = std::true_type;
        };

        // Interface for checking if all given classes are derived from streamable<T>
        template <typename... Ts>
        using all_streamable = typename all_streamable_impl<Ts...>::type;

        // Interface for checking if all given classes can be used in reading a file
        template <typename... Ts>
        using all_parseable = typename std::conjunction<all_wrappers<Ts...>, all_streamable<Ts...>>;

        // Parsed vector, recursive case
        template <typename T, typename... Ts>
        struct parsed_vector_impl {
            using value_type = parsed_vector_impl<Ts...>;
            using type = std::vector<typename value_type::type>;
        };

        // Parsed vector, base case with wrapper
        template <template <typename, char> class C, typename T, char delim>
        struct parsed_vector_impl<C<T, delim>> {
            // If C<T, delim> is a wrapper, unwrap it for the vector
            using value_type = typename std::conditional<is_wrapper<C<T, delim>>::value, T, C<T, delim>>::type;
            using type = typename parsed_vector_impl<value_type>::type;
        };

        // Parsed vector, base case
        template <typename T>
        struct parsed_vector_impl<T> {
            using value_type = T;
            using type = std::vector<value_type>;
        };

        // Interface for nested parsed vectors
        template <typename T, typename... Ts>
        using parsed_vector = typename parsed_vector_impl<T, Ts...>::type;
    }

    namespace {
        // Recursively parse stream data, base case
        template <typename In>
        parsed_vector<In> parseStream(const std::unique_ptr<std::istream> &in) {
            parsed_vector<In> readInput;

            std::copy(std::istream_iterator<In>(*in), std::istream_iterator<In>(), std::back_inserter(readInput));
            return readInput;
        }

        // Recursively parse stream data, recursive case
        template <typename In1, typename In2, typename... Ins>
        parsed_vector<In1, In2, Ins...> parseStream(const std::unique_ptr<std::istream> &in) {
            parsed_vector<In1, In2, Ins...> readInput;

            std::transform(std::istream_iterator<In1>(*in), std::istream_iterator<In1>(), std::back_inserter(readInput), 
                [](auto in) {
                    return parseStream<In2, Ins...>(in.stream());
                });
            return readInput;
        }
    }

    // Reads a single value from a file
    template <typename Out>
    Out readValue(std::string_view fileName) {
        auto file = openFile(fileName);
        Out out;
        file >> out;
        return out;
    }

    // Read file for a single layer of values
    template <typename In>
    parsed_vector<In> readFile(std::string_view fileName) {
        auto file = openFile(fileName);

        return parsed_vector<In>(std::istream_iterator<In>(file), std::istream_iterator<In>());
    }

    // Read file for multiple layers of values
    template <typename In1, typename In2, typename... Ins>
    typename std::enable_if<
        all_parseable<In1, In2, Ins...>::value,
        parsed_vector<In1, In2, Ins...>
    >::type
    readFile(std::string_view fileName) {
        auto file = openFilePtr(fileName);
        return parseStream<In1, In2, Ins...>(file);
    }

    // Dump all of the file contents into a string to manually parse
    static std::string dumpFile(std::string_view fileName) {
        auto file = openFile(fileName);

        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}