#pragma once
#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include "../Core.hpp"

struct String {
  UInt64 space;
  UInt64 size;
  Char8 *buffer;

  String(UInt64 space = 2);
  Void Destroy();
  Void Put(Char8 value);
  Bool operator==(const Char8 *comparee);
  Bool operator==(const String *comparee);
  Void Realloc();
  virtual Char8 *ToStr() = 0;
};

#endif  // UTILS_STRING_HPP
