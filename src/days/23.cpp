#include "../AdventOfCode2019.hpp"

namespace Network {
    using address = std::size_t;
    struct packet {
        IntCode::Int x;
        IntCode::Int y;
    };

    struct NAT;

    template <std::size_t N>
    struct InterfaceController {

        static constexpr address size = N;
        static constexpr address natAddress = 255;

        std::vector<IntCode::Computer> network;
        std::unordered_map<address, std::queue<packet>> packets;

        bool throttleSent;
        bool useThrottle;
        NAT throttle;


        InterfaceController(const IntCode::Integers &program, bool useThrottle = false) 
            : network(N), packets(N), throttleSent(false), useThrottle(useThrottle), throttle() {
            for (int i = 0; i < N; ++i) {
                network[i].load(program);
                network[i] << i << -1;
            }
        }

        // Collect all sent packets from computer
        void collectAll() {
            for (auto it = network.begin(); it != network.end(); ++it) {
                it->run();
                auto output = it->flushOutput();
                IntCode::Int dest;
                packet sent;
                while (!output.empty()) {
                    auto it = output.begin();
                    dest = *it++;
                    sent.x = *it++;
                    sent.y = *it++;
                    output.erase(output.begin(), it);

                    if (dest == natAddress) {
                        if (useThrottle) {
                            throttle.send(sent);
                            throttleSent = true;
                        }
                        else {
                            packets[natAddress].push(sent);
                        }
                    }
                    else {
                        // Queue up the packet
                        packets[dest].push(sent);
                    }
                }
            }
        }

        // Distribute all queued packets to their destination
        void distributeAll() {
            for (int i = 0; i < N; ++i) {
                auto &pending = packets[i];
                if (pending.empty()) {
                    network[i] << -1;
                }
                else {
                    while (!pending.empty()) {
                        auto &next = pending.front();
                        network[i] << next.x << next.y;
                        pending.pop();
                    }
                }
            }
        }

        IntCode::Int firstTo255() {
            while (true) {
                collectAll();
                if (!packets[natAddress].empty()) {
                    return packets[natAddress].front().y;
                }
                distributeAll();
            }
        }

        IntCode::Int firstToBeSentTwiceByNAT() {
            while (true) {
                collectAll();
                if (auto res = throttle.checkNetwork(*this); throttleSent && res) {
                    return res.value();
                }
                distributeAll();
            }
        }
    };

    struct NAT {
        packet lastSent;
        packet last;

        void send(const packet &newPacket) {
            last = newPacket;
        }

        template <std::size_t N>
        std::optional<IntCode::Int> checkNetwork(InterfaceController<N> &nic) {
            for (int i = 0; i < N; ++i) {
                if (nic.network[i].hasOutput() || !nic.packets[i].empty()) {
                    return { };
                }
            }
            if (last.y == lastSent.y) {
                return last.y;
            }
            else {
                nic.network[0] << last.x << last.y;
                lastSent = last;
                return { };
            }
        }
    };
}


int AoC::A::day23() {
    auto program = in::readFile<in::value<IntCode::Int, ','>>("input/23.txt");
    static constexpr std::size_t numComputers = 50;
    Network::InterfaceController<numComputers> nic(program);

    return static_cast<int>(nic.firstTo255());
}

int AoC::B::day23() {
    auto program = in::readFile<in::value<IntCode::Int, ','>>("input/23.txt");
    static constexpr std::size_t numComputers = 50;
    Network::InterfaceController<numComputers> nic(program, true);

    return static_cast<int>(nic.firstToBeSentTwiceByNAT());
}