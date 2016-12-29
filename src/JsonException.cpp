#include "concise_json_schema/JsonException.h"

using namespace JSON;

JSONRangeException::JSONRangeException(const Json& ref, const std::string& key)
    : JsonException("Json object has no key `" + key + "`") {}

JSONRangeException::JSONRangeException(const Json& ref, size_t index)
    : JsonException("index " + std::to_string(index) + " is out of range of Json array [0, .. , " +
                    std::to_string(ref.size())) {}

JSONParseException::JSONParseException(const std::string& what)
    : JsonException(what) {}

JSONParseException::JSONParseException(const char* what) : JsonException(what){}
