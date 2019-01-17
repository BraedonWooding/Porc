#ifndef PRINTER_HELPERS_HPP
#define PRINTER_HELPERS_HPP

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