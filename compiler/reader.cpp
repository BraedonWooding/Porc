#include "reader.hpp"

size_t CFileReader::Read(Reader::ItemType *buf, size_t len) {
  auto len_read = fread(buf, Reader::kItemSize, len, fp);
  buf[len_read] = '\0';
  return len_read;
}
