#include "concise_json_schema/JsonException.h"

using namespace JSON;

JSONRangeException::JSONRangeException(const Json& ref, const std::string& key)
    : JsonException("Json object has no key `" + key + "`"), violated_copy(ref) {}

JSONRangeException::JSONRangeException(const Json& ref, size_t index)
    : JsonException("index " + std::to_string(index) + " is out of range of Json array [0, .. , " +
                    std::to_string(ref.size())),
      violated_copy(ref) {}

JSONParseException::JSONParseException(std::istream& in, const std::string& what)
    : JsonException(what), pos(in.tellg()) {}

JSONParseException::JSONParseException(std::istream& in, const char* what) : JsonException(what), pos(in.tellg()) {}
