#include "printer_helpers.hpp"

std::ostream &operator <<(std::ostream &out, const LineRange &pos) {
  // 0:1 -> 1:2
  out << pos.line_start << ":" << pos.col_start
      << " -> "
      << pos.line_end << ":" << pos.col_end;
  return out;
}

json LineRange::GetMetaData() const {
  return {
    { "file", this->file_name },
    { "line", json::array({line_start, line_end}) },
    { "col", json::array({col_start, col_end}) }
  };
}