#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <iostream>

namespace console {

    static std::ostream &console = std::cout;

    inline void log(void) {
        std::cout << std::endl;
    }

    template<class First>
    void log(const First &first) {
        console << first << std::endl;
    }

    template<class First, class... Rest>
    void log(const First &first, const Rest &... rest) {
        console << first << std::endl;
        log(rest...);
    }

    template<class First>
    void string(const First &first) {
        console << first << std::endl;
    }

    template<class First, class... Rest>
    void string(const First &first, const Rest &... rest) {
        console << first;
        string(rest...);
    }
}

#endif