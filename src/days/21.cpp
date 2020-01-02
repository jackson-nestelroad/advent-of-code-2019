#include "../AdventOfCode2019.hpp"

namespace SpringScript {
    enum class Op {
        AND,
        OR,
        NOT,
        WALK,
        RUN
    };

    template <Op op>
    constexpr std::string getOperationName() {
        switch (op) {
            case Op::AND: return "AND";
            case Op::OR: return "OR";
            case Op::NOT: return "NOT";
            case Op::WALK: return "WALK";
            case Op::RUN: return "RUN";
            default: return "UNK";
        }
    }

    enum class Write {
        Jump = 'J',
        Temporary = 'T'
    };

    enum class Read {
        OneTile = 'A',
        TwoTiles = 'B',
        ThreeTiles = 'C',
        FourTiles = 'D',
        FiveTiles = 'E',
        SixTiles = 'F',
        SevenTiles = 'G',
        EightTiles = 'H',
        NineTiles = 'I'
    };

    template <Op op, Write src, Write dest>
    constexpr std::string Instruction() {
        auto res = getOperationName<op>();
        res += ' ';
        res += static_cast<char>(src);
        res += ' ';
        res += static_cast<char>(dest);
        return res;
    }

    template <Op op, Read src, Write dest>
    constexpr std::string Instruction() {
        auto res = getOperationName<op>();
        res += ' ';
        res += static_cast<char>(src);
        res += ' ';
        res += static_cast<char>(dest);
        return res;
    }

    template <Op op>
    constexpr typename std::enable_if<op == Op::WALK || op == Op::RUN, std::string>::type Instruction() {
        return getOperationName<op>();
    }
}

static IntCode::Computer &operator<<(IntCode::Computer &comp, const std::string &str) {
    for (auto it = str.begin(); it != str.end(); ++it) {
        comp << *it;
    }
    return comp;
}

static IntCode::Computer &operator<<(IntCode::Computer &comp, const std::vector<std::string> &strs) {
    for (auto it = strs.begin(); it != strs.end(); ++it) {
        comp << *it << '\n';
    }
    return comp;
}

int AoC::A::day21() {
    // Droid jumps 4 positions each time
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/21.txt");
    IntCode::Computer comp { input };

    using namespace SpringScript;
    comp << IntCode::Output::ASCII;

    // J = ~A | (~C & D)
    std::vector<std::string> instructions {
        Instruction<Op::NOT, Read::ThreeTiles, Write::Jump>(),      // Three tiles away is a hole
        Instruction<Op::AND, Read::FourTiles, Write::Jump>(),       // Four tiles away is solid (can jump)

        Instruction<Op::NOT, Read::OneTile, Write::Temporary>(),    // Next tile is a hole -- MUST jump 
        Instruction<Op::OR, Write::Temporary, Write::Jump>(),
        
        Instruction<Op::WALK>()
    };

    comp << instructions;
    comp.run();

    return static_cast<int>(comp.flushOutput().back());
}

int AoC::B::day21() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/21.txt");
    IntCode::Computer comp { input };

    using namespace SpringScript;
    comp << IntCode::Output::ASCII;

    // J = ~A | (~C & D & H) | (~B & D)
    // J = ~A | (D & (~B | (~C & H)))
    std::vector<std::string> instructions {
        Instruction<Op::NOT, Read::ThreeTiles, Write::Jump>(),      // Three tiles away is a hole
        Instruction<Op::AND, Read::FourTiles, Write::Jump>(),       // Four tiles away is solid (can jump)
        Instruction<Op::AND, Read::EightTiles, Write::Jump>(),      // Eight tiles away is solid (can jump again so we don't get stranded)
        
        Instruction<Op::NOT, Read::TwoTiles, Write::Temporary>(),   // Two tiles away is a hole
        Instruction<Op::AND, Read::FourTiles, Write::Temporary>(),  // Four tiles away is solid (can jump)
        Instruction<Op::OR, Write::Temporary, Write::Jump>(),       // If we get stranded here, then the next jump is impossible
        
        Instruction<Op::NOT, Read::OneTile, Write::Temporary>(),    // Next tile is a hole  -- MUST jump
        Instruction<Op::OR, Write::Temporary, Write::Jump>(),
        
        Instruction<Op::RUN>()
    };

    comp << instructions;
    comp.run();

    return static_cast<int>(comp.flushOutput().back());
}