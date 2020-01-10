#pragma once

#include <string>
#include <string_view>

namespace string {
    using size = std::size_t;
    using static_string = const std::string_view &;

    namespace impl {
        template <static_string, typename, static_string, typename>
        struct concat_strings;

        template <static_string S1, size... I1, static_string S2, size... I2>
        struct concat_strings<S1, std::index_sequence<I1...>, S2, std::index_sequence<I2...>> {
            static constexpr const char value[] { S1[I1]..., S2[I2]..., 0 };
        };

        template <static_string...>
        struct join;

        template <>
        struct join<> {
            static constexpr static_string value = "";
        };

        template <static_string S1>
        struct join<S1> {
            static constexpr static_string value = S1;
        };

        template <static_string S1, static_string S2>
        struct join<S1, S2> {
            static constexpr static_string value =
                concat_strings<S1, std::make_index_sequence<S1.size()>, S2, std::make_index_sequence<S2.size()>>::value;
        };

        template <static_string S1, static_string... Rest>
        struct join<S1, Rest...> {
            static constexpr static_string value = join<S1, join<Rest...>::value>::value;
        };
    }

    template <static_string... Strs>
    struct join {
        static constexpr static_string value = impl::join<Strs...>::value;
    };
};