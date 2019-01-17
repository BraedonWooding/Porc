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

#include <iostream>
#include <nlohmann/json.hpp>
#include "defs.hpp"
#include "helper.hpp"

using json = nlohmann::json;

struct LineRange {
public:
  int line_start;
  int line_end;
  int col_start;
  int col_end;

  friend std::ostream &operator <<(std::ostream &out, const LineRange &pos);
  json GetMetaData() const;

  LineRange(int line_start, int line_end, int col_start, int col_end) {
    Assert(line_start >= 0, "Must be positive (or 0)", line_start);
    Assert(line_end >= 0, "Must be positive (or 0)", line_end);
    Assert(col_start >= 0, "Must be positive (or 0)", col_start);
    Assert(col_end >= 0, "Must be positive (or 0)", col_end);

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