#include "printer_helpers.hpp"

std::ostream &operator <<(std::ostream &out, const LineRange &pos) {
  // 0:1 -> 1:2
  out << pos.line_start << ":" << pos.col_start
      << " -> "
      << pos.line_end << ":" << pos.col_end;
  return out;
}

json LineRange::GetMetaData() const {
  std::string line = std::to_string(line_start) + " -> " +
                     std::to_string(line_end);
  std::string col =  std::to_string(col_start) + " -> " +
                     std::to_string(col_end);
  return {
    { "file", this->file_name },
    { "line", line },
    { "col", col }
  };
}