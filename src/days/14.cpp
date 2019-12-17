#include "../AdventOfCode2019.hpp"

struct chemical {
    int quantity;
    std::string id;

    static const std::string parser;
};

const std::string chemical::parser = "(?:([0-9]+) ([A-Z]+))";

struct reaction {
    std::vector<chemical> input;
    chemical output;
};

std::istream &operator>>(std::istream &input, reaction &data) {
    std::string str;
    std::getline(input, str);
    std::regex chemicalRegex { chemical::parser };
    auto begin = std::sregex_iterator(str.begin(), str.end(), chemicalRegex);
    auto end = std::sregex_iterator();
    data.input.clear();
    for (auto it = begin; it != end; ++it) {
        auto match = *it;
        chemical next;
        next.quantity = std::stoi(match[1]);
        next.id = match[2];
        if (std::distance(it, end) == 1) {
            data.output = next;
        }
        else {
            data.input.push_back(next);
        }
    }
    return input;
}

void breakDownChemicals(const std::map<std::string, reaction> &reactionMap,
    std::map<std::string, std::pair<long long, long long>> &chemicalMap,
    const std::string &id, long long needed) {
    auto &amount = chemicalMap[id];
    amount.first += needed;
    // No further reaction
    if (id == "ORE") {
        return;
    }
    // Break down chemical by its reaction
    else {
        auto formula = reactionMap.find(id)->second;
        const int portion = formula.output.quantity;
        // Add excess chemicals from reaction
        if (long long mod = needed % portion; mod != 0) {
            amount.second += portion - mod;
        }
        // Have enough excess to reuse for this reaction
        if (amount.second >= portion) {
            long long mod = amount.second % portion;
            needed -= portion - mod;
            amount.second = mod;
        }
        // Carry out the reaction
        const long long multiple = static_cast<long long>(std::ceil(static_cast<double>(needed) / portion));
        for (const auto &input : formula.input) {
            breakDownChemicals(reactionMap, chemicalMap, input.id, multiple * input.quantity);
        }
    }
}

long long fuelToOre(const std::map<std::string, reaction> &reactionMap, long long n) {
    std::map<std::string, std::pair<long long, long long>> chemicalMap;
    breakDownChemicals(reactionMap, chemicalMap, "FUEL", n);
    return chemicalMap["ORE"].first;
}

long long AoC::A::day14() {
    auto reactions = in::readFile<in::value<reaction, '\n'>>("input/14.txt");
    std::map<std::string, reaction> reactionMap;
    for (const auto &reaction : reactions) {
        reactionMap[reaction.output.id] = reaction;
    }
    return fuelToOre(reactionMap, 1);
}

long long AoC::B::day14() {
    auto reactions = in::readFile<in::value<reaction, '\n'>>("input/14.txt");
    std::map<std::string, reaction> reactionMap;
    for (const auto &reaction : reactions) {
        reactionMap[reaction.output.id] = reaction;
    }
    constexpr long long goal = 1'000'000'000'000;
    long long step = static_cast<long long>(std::pow(2, 40));
    long long fuel = 0;
    long long ore = 0;
    while (step != 0) {
        ore = fuelToOre(reactionMap, fuel);
        if (ore > goal) {
            fuel -= step;
            step >>= 1;
        }
        else {
            fuel += step;
        }
    }
    return fuel;
}