#ifndef PRINTER_HELPERS_HPP
#define PRINTER_HELPERS_HPP

/*
    This is a gigantic file!
*/

// This is predominantly based upon the EBNF
// of course changes will occur
// i.e. ArgumentExpressionList => Vector(Expression)

// @TODO: I'm a bit lazy rn so I've used template<typename T>
// in place of copy + pasting the constructor multiple times while in effect
// it shouldn't impact performance/codesize it will impact compile times
// (though I would presume a relatively negligible amount)
// regardless should be replaced as it isn't very explicit

// more just till I lose my laziness to not add proper lines
#define NULL_RANGE (lineRange){0}

#include <cstdint>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using uint = std::uint32_t;

struct LineRange {
public:
    uint32_t line_start;
    uint32_t line_end;
    uint32_t col_start;
    uint32_t col_end;

    friend std::ostream &operator <<(std::ostream &out, const LineRange &pos);
    json GetMetaData() const;

    LineRange(uint32_t line_start, uint32_t line_end, uint32_t col_start, uint32_t col_end) {
        this->line_start = line_start;
        this->line_end = line_end;
        this->col_start = col_start;
        this->col_end = col_end;
    }

    static LineRange NullRange() {
        return LineRange(0, 0, 0, 0);
    }
};

#endif