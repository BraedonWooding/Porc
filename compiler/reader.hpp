#ifndef READER_HPP
#define READER_HPP

#include <iostream>
#include <string>
#include "defs.hpp"

/*
  A generic reader for reading into a buf in the style of fread.
*/
class Reader {
 public:
  using ItemType = char;

  const size_t kItemSize = sizeof(ItemType);

  const std::string file_name;

  Reader(std::string file_name) : file_name(file_name) {}

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
  CFileReader(const char *&&file_name) : Reader(file_name) {
    fp = fopen(file_name, "r");
  }

  ~CFileReader() {
    fclose(fp);
  }

  size_t Read(Reader::ItemType *buf, size_t len);
};

/*
  String buffer reader
*/
class StringReader : public Reader {
 private:
  std::string &buf;
  int i;

 public:
  StringReader(std::string &buf) : Reader("String"), buf(buf), i(0) {}

  size_t Read(Reader::ItemType *buf, size_t len);
};

#endif