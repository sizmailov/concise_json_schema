#pragma once

#include "Json.h"

#include <stdexcept>


namespace JSON {
class JsonException : public std::runtime_error {
 public:
  JsonException(const std::string& what) : runtime_error(what){};

  JsonException(const char* what) : runtime_error(what){};
};

class JSONLogicException : public JsonException {
 public:
  JSONLogicException(const std::string& what) : JsonException(what){};

  JSONLogicException(const char* what) : JsonException(what){};
};

class JSONParseException : public JsonException {
 public:
  JSONParseException(std::istream& in, const std::string& what);
  JSONParseException(std::istream& in, const char* what);
  std::iostream::pos_type pos;
};

class JsonGetException : public JsonException {
 public:
  JsonGetException(const std::string& what) : JsonException(what){};
  JsonGetException(const char* what) : JsonException(what){};
};

class JSONRangeException : public JsonException {
 public:
  JSONRangeException(const Json& ref, const std::string& key);
  JSONRangeException(const Json& ref, size_t index);
  const Json violated_copy;
};
}
