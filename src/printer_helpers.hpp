#ifndef PRINTER_HELPERS_HPP
#define PRINTER_HELPERS_HPP

#include <iostream>
#include <nlohmann/json.hpp>
#include "defs.hpp"
#include "helper.hpp"

using json = nlohmann::json;

// @TODO: @API:
//              remove copy constructors (we shouldn't be copying these)
struct LineRange {
public:
  // @API: probably should expose these
  //       its just arguably bad style
  //       in particular the filename id
  int line_start;
  int line_end;
  int col_start;
  int col_end;
  std::string file_name;

  friend std::ostream &operator <<(std::ostream &out, const LineRange &pos);
  json GetMetaData() const;

  LineRange() {
    std::cerr << "Internal Error: Constructed an empty line range, FIXME"
              << std::endl;
    line_start = line_end = col_start = col_end = -1;
    file_name = "Internal Error; Invalid File Name";
  }

  LineRange(const LineRange &a, const LineRange &b) {
    Assert(a.file_name == b.file_name,
          "File names for merging ranges must be the same",
          a.file_name, b.file_name);

    this->line_start = std::min(a.line_start, b.line_start);
    this->line_end = std::max(a.line_end, b.line_end);
    this->col_start = std::min(a.col_start, b.col_start);
    this->col_end = std::max(a.col_end, b.col_end);
    this->file_name = a.file_name;
  }

  LineRange(const int line_start, const int line_end, const int col_start,
            const int col_end, std::string file) {
    // Assert(line_start >= 0, "Must be positive (or 0)", line_start);
    // Assert(line_end >= 0, "Must be positive (or 0)", line_end);
    // Assert(col_start >= 0, "Must be positive (or 0)", col_start);
    // Assert(col_end >= 0, "Must be positive (or 0)", col_end);

    this->line_start = line_start;
    this->line_end = line_end;
    this->col_start = col_start;
    this->col_end = col_end;
    // @TODO: somehow get this verified without exposing public vector
    //        probably just expose current file names.
    this->file_name = file;
  }
};

#endif