#include "concise_json_schema/JsonSerializerStd.h"
#include <complex>

using namespace JSON::io;

namespace JSON{
namespace io{

template<>
void JsonSerializer<bool>::deserialize(const Json& json, bool& v) {
  v = json.get_bool();
}

template<>
Json JsonSerializer<bool>::serialize(const bool& t) {
  return Json(t);
}

#define INTEGER_SERIALIZATION(inttype)                                   \
                                                                         \
template<>                                                               \
void JsonSerializer<inttype>::deserialize(const Json& json, inttype& v) {\
  v = json.get_integer();                                                \
}                                                                        \
                                                                         \
template<>                                                               \
Json JsonSerializer<inttype>::serialize(const inttype& t) {              \
  return Json(Json::Integer(t));                                         \
}

INTEGER_SERIALIZATION(int8_t);
INTEGER_SERIALIZATION(int16_t);
INTEGER_SERIALIZATION(int);
INTEGER_SERIALIZATION(long);
INTEGER_SERIALIZATION(long long);

#undef INTEGER_SERIALIZATION


#define DOUBLE_SERIALIZATION(doubletype)                                       \
                                                                               \
template<>                                                                     \
void JsonSerializer<doubletype>::deserialize(const Json& json, doubletype& v) {\
  v = json.get_number();                                                       \
}                                                                              \
                                                                               \
template<>                                                                     \
Json JsonSerializer<doubletype>::serialize(const doubletype& t) {              \
  return Json(Json::Double(t));                                                \
}

DOUBLE_SERIALIZATION(double);
DOUBLE_SERIALIZATION(float);

#undef DOUBLE_SERIALIZATION

template<>
void JsonSerializer<std::complex<double>>::deserialize(const Json& json, std::complex<double>& v) {
  v = std::complex<double>(json[0].get_double(),json[1].get_double());
}


template<>
Json JsonSerializer<std::complex<double>>::serialize(const std::complex<double>& t) {
  return Json(Json::Array{Json(t.real()),Json(t.imag())});
}

template<>
void JsonSerializer<std::string>::deserialize(const Json& json, std::string& v) {
  v = json.get_string();
}

template<>
Json JsonSerializer<std::string>::serialize(const std::string& t) {
  return Json(std::string(t));
}

}
}