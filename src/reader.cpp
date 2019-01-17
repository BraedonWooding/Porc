#include "reader.hpp"

size_t CFileReader::Read(Reader::ItemType *buf, size_t len) {
  return fread(buf, Reader::kItemSize, len, fp);
}
