#pragma once

namespace Templates {
    template <typename T>
    T compare(const T &a, const T &b) {
        if (a < b) {
            return -1;
        }
        else if (a == b) {
            return 0;
        }
        return 1;
    }
}