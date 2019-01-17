#ifndef READER_HPP
#define READER_HPP

#include <iostream>
#include "defs.hpp"

/*
  A generic reader for reading into a buf in the style of fread.
*/
class Reader {
 public:
  using ItemType = char;
  const size_t kItemSize = sizeof(ItemType);

  /*
      Try to read *len* in groups of *kItemSize* bytes into *buf*.
      Return how many items were actually read.
  */
  virtual size_t Read(ItemType *buf, size_t len) = 0;

  /*
      In the case that you need to cleanup.
  */
  virtual ~Reader() {}
};

/*
  This is a cstyle reader using fread.
*/
class CFileReader : public Reader {
 private:
  FILE *fp;

 public:
  CFileReader(const char *&&file_name) {
    fp = fopen(file_name, "r");
  }

  ~CFileReader() {
    fclose(fp);
  }

  size_t Read(Reader::ItemType *buf, size_t len);
};

#endif