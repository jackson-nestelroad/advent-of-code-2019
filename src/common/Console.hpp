#pragma once

#include <iostream>

namespace console {
    static std::ostream &console = std::cout;

    inline void log(void) {
        console << std::endl;
    }

    template <class First>
    void log(const First &first) {
        console << first << std::endl;
    }

    template <class First, class... Rest>
    void log(const First &first, const Rest &... rest) {
        console << first << ' ';
        log(rest...);
    }

    inline void fatal(void) {
        console << std::endl;
        exit(-1);
    }

    template <class First>
    void fatal(const First &first) {
        console << first << std::endl;
        exit(-1);
    }

    template <class First, class... Rest>
    void fatal(const First &first, const Rest &... rest) {
        console << first;
        fatal(rest...);
    }
}