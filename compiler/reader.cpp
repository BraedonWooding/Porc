#include "reader.hpp"

#include <iostream>

size_t CFileReader::Read(Reader::ItemType *buf, size_t len) {
  auto len_read = fread(buf, Reader::kItemSize, len, fp);
  buf[len_read] = '\0';
  return len_read;
}

size_t StringReader::Read(Reader::ItemType *buf, size_t len) {
  if (this->i < this->buf.size()) {
    int to_read = this->i + len;
    if (to_read > this->buf.size()) to_read = this->buf.size() - this->i;
    memcpy(buf, this->buf.c_str() + i, to_read + 1);
    i += to_read;
    return to_read;
  } else {
    return 0;
  }
}
