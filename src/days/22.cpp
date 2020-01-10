#include "../AdventOfCode2019.hpp"

namespace cards {
    using deck = std::vector<std::size_t>;

    deck factoryOrder(std::size_t size) {
        deck created(size);
        std::iota(created.begin(), created.end(), 0);
        return created;
    }

    deck blankDeck(std::size_t size) {
        return deck(size);
    }

    namespace shuffle {
        struct technique {
            enum class type {
                flip,
                cut,
                deal
            };

            type tech;
            int n;

            static const std::unordered_map<type, std::regex> parsers;
        };

        const std::unordered_map<technique::type, std::regex> technique::parsers {
            { type::flip, std::regex { "deal into new stack" } },
            { type::cut, std::regex { "cut (-?[0-9]+)" } },
            { type::deal, std::regex { "deal with increment ([0-9]+)" } }
        };

        std::istream &operator>>(std::istream &input, technique &data) {
            std::string str;
            std::getline(input, str);

            if (!input) {
                return input;
            }

            for (const auto &[type, parser] : technique::parsers) {
                std::smatch match;
                if (std::regex_match(str, match, parser)) {
                    data.tech = type;
                    auto num = match.str(1);
                    data.n = num.empty() ? 0 : std::stoi(num);
                    return input;
                }
            }
            console::fatal("Illegal shuffle technique: ", str);
            return input;
        }


        void apply(technique tech, deck &cards) {
            switch (tech.tech) {
                case technique::type::flip:
                    std::reverse(cards.begin(), cards.end());
                    break;
                case technique::type::cut:
                    {
                        int amount = tech.n < 0 ? static_cast<int>(cards.size()) + tech.n : tech.n;
                        std::rotate(cards.begin(), cards.begin() + amount, cards.end());
                    }
                    break;
                case technique::type::deal:
                    {
                        const std::size_t size = cards.size();
                        std::size_t inc = tech.n % size;
                        deck dealt = blankDeck(size);
                        for (std::size_t i = 0, j = 0; j < size; ++j, i += inc, i %= size) {
                            dealt[i] = cards[j];
                        }
                        cards = dealt;
                    }
                    break;
                default:
                    console::fatal("Illegal shuffle technique.");
            }
        }
    }
}

struct linear_modular_function {
    bigint a;
    bigint b;
    bigint m;

    bigint operator()(bigint x) const {
        return (a * x + b).mod(m);
    }

    linear_modular_function operator()(const linear_modular_function &g) const {
        if (m != g.m) {
            throw std::invalid_argument("Composed modular functions must have the same base m");
        }
        return { (a * g.a).mod(m), (a * g.b + b).mod(m), m };
    }

    linear_modular_function operator~() const {
        auto inv = a.modInverse(m);
        return { inv, m - ((b * inv) % m), m };
    }
};

template <std::size_t size>
linear_modular_function getShuffleEquation(std::vector<cards::shuffle::technique> shuffle) {
    const bigint zero { 0 };
    bigint a = 1;
    bigint b = 0;

     for (auto [tech, n] : shuffle) {
        long long c { }, d { };
        switch (tech) {
            case cards::shuffle::technique::type::flip:
                c = -1;
                d = -1;
                break;
            case cards::shuffle::technique::type::cut:
                c = 1;
                d = -n;
                break;
            case cards::shuffle::technique::type::deal:
                c = n;
                d = 0;
                break;
            default:
                console::fatal("Illegal shuffle technique.");
        }
        b *= c;
        b += d;
        a *= c;
    }
     a = a.mod(size);
     b = b.mod(size);
     std::size_t a2 = a.toULong();
     std::size_t b2 = b.toULong();
     return { a2, b2, size };
}
int AoC::A::day22() {
    auto shuffle = in::readFile<in::value<cards::shuffle::technique, '\n'>>("input/22.txt");
    
    static constexpr std::size_t size = 10'007;
    cards::deck deck = cards::factoryOrder(size);
    for (const auto &tech : shuffle) {
        cards::shuffle::apply(tech, deck);
    }

    auto res = std::find(deck.begin(), deck.end(), 2019);
    if (res == deck.end()) {
        console::fatal("Card 2019 has been lost!");
    }
    return static_cast<int>(std::distance(deck.begin(), res));
}

long long AoC::B::day22() {
    auto shuffle = in::readFile<in::value<cards::shuffle::technique, '\n'>>("input/22.txt");

    static constexpr std::size_t size = 119'315'717'514'047;
    static constexpr std::size_t timesShuffled = 101'741'582'076'661;

    // Get a linear modular function for a single shuffle
    linear_modular_function equation = getShuffleEquation<size>(shuffle);

    // Invert the function (uses modular inverses)
    auto inverted = ~equation;

    std::unordered_map<std::size_t, linear_modular_function> equationMap {
        { 1, inverted }
    };
    
    // Compose the inverted shuffle equation for powers of 2
    auto &currentEquation = inverted;
    std::size_t numInversions = 1;
    while (numInversions < timesShuffled) {
        numInversions <<= 1;
        auto next = currentEquation(currentEquation);
        equationMap.insert({ numInversions, next });
        currentEquation = next;
    }

    // Invert from the ending index using the powers of 2 for timesShuffled
    bigint result = 2020;
    std::size_t inversionsLeft = timesShuffled;
    std::size_t current = numInversions >> 1;
    while (inversionsLeft > 0 && current > 0) {
        if (current <= inversionsLeft) {
            result = equationMap[current](result);
            inversionsLeft -= current;
        }
        current >>= 1;
    }

    return result.toULong();
}