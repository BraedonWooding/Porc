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
    { "line", this->line_start, this->line_end },
    { "col", this->col_start, this->col_end }
  };
}