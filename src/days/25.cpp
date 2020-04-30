#include "../AdventOfCode2019.hpp"

void playManually(IntCode::Computer &comp) {
    while (true) {
        comp.run();
        std::cout << comp;

        if (comp.getStatus() == IntCode::Status::AwaitingInput) {
            std::string command;
            std::getline(std::cin, command);
            comp << command << '\n';
        }
        else {
            break;
        }
    }
}

enum class Direction {
    North,
    South,
    East,
    West,
    COUNT
};

constexpr auto DIRECTION_COUNT = static_cast<std::size_t>(Direction::COUNT);

std::string DIRECTION_NAMES[DIRECTION_COUNT] {
    "north",
    "south",
    "east",
    "west"
};

inline Direction invert(Direction dir) {
    return static_cast<Direction>(static_cast<int>(dir) ^ 1);
}

using room_id = std::size_t;

struct room {
    constexpr static room_id unknown = -1;
    constexpr static room_id none = -2;

    std::string name { };
    std::size_t dist { };
    room_id connections[DIRECTION_COUNT];

    room() {
        std::fill(connections, connections + DIRECTION_COUNT, none);
    }

    room(const std::string &name, std::size_t dist) : room() {
        this->name = name;
        this->dist = dist;
    }
};

struct state {
    room_id cameFrom = room::none;
    Direction dirTaken;
};

int AoC::A::day25() {
    static const std::string destination = "Pressure-Sensitive Floor";
    
    auto program = in::readFile<in::value<IntCode::Int, ','>>("input/25.txt");
    IntCode::Computer comp { program };
    comp << IntCode::Output::ASCII;

    std::regex roomMatcher { "== (.+) ==" };
    std::regex optionMatcher { "- (.+)" };
    std::regex numberMatcher { "[0-9]+" };

    // Run through the entire map, grabbing all items (BFS)

    std::vector<room> shipMap;
    std::vector<std::string> inventory;

    // These items disrupt the program in unique ways, so we just blacklist them here

    // Electromagnet could be handled by running comp.run() after collecting the item
    // and seeing if you are stuck in place
    std::unordered_set<std::string> badItems {
        "infinite loop",
        "giant electromagnet"
    };

    std::stack<state> history;
    history.push({ });

    while (true) {
        comp.run();

        std::stringstream stream;
        stream << comp;
        std::string output = stream.str();

        state &prev = history.top();

        std::smatch matches;
        if (std::regex_search(output, matches, roomMatcher)) {
            const std::string &name = matches[1];
            
            // Check if room has already been visited
            auto roomData = std::find_if(shipMap.begin(), shipMap.end(), [&name](const auto &r) { return r.name == name; });
            room_id roomId;

            // New room found, parse new data
            if (roomData == shipMap.end()) {
                // Add new room data
                auto newDist = prev.cameFrom != room::none ? shipMap[prev.cameFrom].dist + 1 : 0;
                shipMap.push_back({ name, newDist });
                roomId = shipMap.size() - 1;
                shipMap[roomId].connections[static_cast<std::size_t>(invert(prev.dirTaken))] = prev.cameFrom;
                if (prev.cameFrom != room::none) {
                    shipMap[prev.cameFrom].connections[static_cast<std::size_t>(prev.dirTaken)] = roomId;
                }

                // We are most likely immediately kicked out of this room due to 
                // having the wrong weight, so no need to parse data
                if (name == destination) {
                    // We somehow won on our very first try
                    if (std::regex_search(output, matches, numberMatcher)) {
                        return std::stoi(matches[0]);
                    }
                    // The more likely scenario, move back a room and keep searching
                    else {
                        roomId = prev.cameFrom;
                        history.pop();
                        prev = history.top();
                    }
                }
                else {
                    // Parse options
                    auto options_begin = std::sregex_iterator(output.begin(), output.end(), optionMatcher);
                    auto options_end = std::sregex_iterator();
                    for (auto it = options_begin; it != options_end; ++it) {
                        const auto &option = (*it)[1];

                        // Check if this is a direction
                        auto dirId = std::find(DIRECTION_NAMES, DIRECTION_NAMES + DIRECTION_COUNT, option);
                        // Mark unexplored directions
                        if (dirId != DIRECTION_NAMES + DIRECTION_COUNT) {
                            auto dir = std::distance(DIRECTION_NAMES, dirId);
                            if (prev.cameFrom == room::none || dir != static_cast<std::size_t>(invert(prev.dirTaken))) {
                                shipMap[roomId].connections[dir] = room::unknown;
                            }
                        }
                        // We found an item
                        else {
                            // We may be allowed to pick this up
                            if (std::find(badItems.begin(), badItems.end(), option) == badItems.end()) {
                                // Make a backup incase we are killed by this item
                                IntCode::Computer backup = comp;
                                comp << "take " << option << '\n';
                                comp.run();
                                // We died, blacklist the item
                                if (comp.isFinished() && comp.getStatus() != IntCode::Status::AwaitingInput) {
                                    badItems.insert(option);
                                    comp = backup;
                                }
                                // We lived, keep the item
                                else {
                                    inventory.push_back(option);
                                    comp.clearOutput();
                                }
                            }
                        }
                    }
                }
            }
            // Existing room, no need to parse options
            else {
                // Save potentially new connection (if there are cycles)
                roomId = std::distance(shipMap.begin(), roomData);
                shipMap[roomId].connections[static_cast<std::size_t>(invert(prev.dirTaken))] = prev.cameFrom;
                if (prev.cameFrom != room::none) {
                    shipMap[prev.cameFrom].connections[static_cast<std::size_t>(prev.dirTaken)] = roomId;
                }
            }

            // Get next direction to explore
            std::optional<Direction> nextDir { };
            for (int i = 0; i < DIRECTION_COUNT; ++i) {
                if (shipMap[roomId].connections[i] == room::unknown) {
                    nextDir = static_cast<Direction>(i);
                    break;
                }
            }
            // Move forward
            if (nextDir) {
                history.push({ roomId, nextDir.value() });
                comp << DIRECTION_NAMES[static_cast<std::size_t>(nextDir.value())] << '\n';
            }
            else {
                // We are back at the beginning
                if (prev.cameFrom == room::none) {
                    break;
                }
                // Move backwards
                history.pop();
                comp << DIRECTION_NAMES[static_cast<std::size_t>(invert(prev.dirTaken))] << '\n';
            }
        }
        else {
            console::fatal("Failed to parse room name.");
        }
    }

    // At this point, we are back at the start with all items
    // Use the shipMap to travel to the Security Checkpoint

    auto destData = std::find_if(shipMap.begin(), shipMap.end(), [](const room &r) { return r.name == destination; });
    if (destData == shipMap.end()) {
        console::fatal(destination, " was never discovered.");
    }

    // Get room preceding destination (Security Checkpoint)
    auto beforeDest = std::find_if_not(destData->connections, destData->connections + DIRECTION_COUNT,
        [](const auto &id) { return id == room::none; });

    // Get path to destination by using distance attributes recorded during BFS
    Direction prevDir = invert(static_cast<Direction>(std::distance(destData->connections, beforeDest)));
    Direction finalDir = prevDir;
    room_id currRoom = *beforeDest;
    std::list<Direction> pathToDestination;
    while (currRoom != 0) {
        for (int i = 0; i < DIRECTION_COUNT; ++i) {
            auto nextRoom = shipMap[currRoom].connections[i];
            if (nextRoom != room::none &&
                i != static_cast<int>(prevDir) &&
                shipMap[nextRoom].dist < shipMap[currRoom].dist) {
                prevDir = invert(static_cast<Direction>(i));
                currRoom = nextRoom;
                pathToDestination.push_front(prevDir);
            }
        }
    }

    // Travel to room preceding destination
    for (Direction dir : pathToDestination) {
        comp << DIRECTION_NAMES[static_cast<std::size_t>(dir)] << '\n';
    }

    // Drop all items
    for (const auto &str : inventory) {
        comp << "drop " << str << '\n';
    }
    
    comp.run();
    comp.clearOutput();

    // Try every possible combination of item on the destination room
    // Generate gray codes, find position of changed bit, take or drop the item, and go to the room
    // Program finishes when we are successful

    // This could possibly run better by analyzing
    // the "heavier" and "lighter" responses to narrow
    // down which items make sense

    const auto numItems = inventory.size();
    const auto numCombos = (1ULL << numItems) - 1;
    auto prev = 0;
    for (unsigned int i = 1; i < numCombos; ++i) {
        // Gray Code ==> Toggled Bit ==> Bit Index for inventory vector
        auto grayCode = i ^ (i >> 1);
        auto toggledBit = grayCode ^ prev;
        bool take = grayCode & toggledBit;
        int bitIndex = -1;
        while (toggledBit) {
            toggledBit >>= 1;
            ++bitIndex;
        }

        // Make inventory change
        comp << (take ? "take " : "drop ") << inventory[bitIndex] << '\n';
        comp << DIRECTION_NAMES[static_cast<std::size_t>(finalDir)] << '\n';
        comp.run();

        // We finished the program, and the entire Advent of Code!
        if (comp.isFinished()) {
            std::stringstream stream;
            stream << comp;
            std::string success = stream.str();
            std::smatch matches;
            if (std::regex_search(success, matches, numberMatcher)) {
                return std::stoi(matches[0]);
            }
            else {
                console::fatal("No number was found in exit message.");
            }
        }

        comp.clearOutput();
        prev = grayCode;
    }

    console::fatal("No item combination was correct!");
    return 0;
}

int AoC::B::day25() {
    return 0;
}