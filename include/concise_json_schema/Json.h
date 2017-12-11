#pragma once

#include "variant"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>


namespace JSON {

namespace estd = std::experimental;

class Json;

std::istream& operator>>(std::istream& in, Json& json);
std::ostream& operator<<(std::ostream& out, const Json& json);

class Json {
 public:
  using Array = std::vector<Json>;
  using Boolean = bool;
  using Integer = int64_t;
  struct Nil {};
  using Object = std::map<std::string, Json>;
  using Double = double;
  using String = std::string;
  using Variant = estd::variant<Array, Boolean, Integer, Nil, Object, Double, String>;

  Json();
  Json(Json&& other);
  Json(const Json& other);
  explicit Json(Array&& value);
  explicit Json(Boolean value);
  explicit Json(Integer value);
  explicit Json(int value);
  Json(Nil value);
  explicit Json(Object&& value);
  explicit Json(Double value);
  explicit Json(String&& value);
  explicit Json(const char* value);
  ~Json();

  Json& operator=(const Json& other);
  Json& operator=(Json&& other);

  Json& operator=(const Array& value);
  Json& operator=(Boolean value);
  Json& operator=(Integer value);
  Json& operator=(Nil value);
  Json& operator=(const Object& value);
  Json& operator=(Double value);
  Json& operator=(const String& value);
  Json& operator=(const char* value);

  bool operator==(const Json& other) const;
  bool operator!=(const Json& other) const;
  bool operator<(const Json& other) const;

  bool is_array() const;
  bool is_bool() const;
  bool is_integer() const;
  bool is_null() const;
  bool is_object() const;
  bool is_double() const;
  bool is_number() const;
  bool is_string() const;

  const Array& get_array() const;
  Boolean get_bool() const;
  Integer get_integer() const;
  const Object& get_object() const;
  Double get_double() const;
  Double get_number() const;
  const String& get_string() const;

  // no need in overloads for int/double/bool
  Array& get_array();
  Object& get_object();
  String& get_string();

  const Json& operator()(const std::string& name) const;
  Json& operator()(const std::string& name);

  const Json& operator[](size_t index) const;
  Json& operator[](size_t index);

  size_t size() const;
  size_t count(const std::string& name) const;

  Array::const_iterator begin() const;
  Array::const_iterator end() const;

  Array::iterator begin();
  Array::iterator end();

  Json& push_back(const Json& val);
  Json& push_back(Json&& val);

  Json& insert(const std::string key, const Json& value);
  Json& insert(const std::string key, Json&& value);

  friend std::istream& JSON::operator>>(std::istream& in, Json& json);
  void pretty_print(std::ostream& out, int tab_size=2, int offset=0, bool first_line_offset=true) const;
 private:
  using ObjectMimic = std::aligned_union_t<0, Object>;

  static_assert(sizeof(ObjectMimic) == sizeof(Object), "");
  static_assert(alignof(ObjectMimic) == alignof(Object), "");

  std::aligned_union_t<0,
                       std::experimental::variant<Array,        // 0
                                                  Boolean,      // 1
                                                  Integer,      // 2
                                                  Nil,          // 3
                                                  ObjectMimic,  // 4
                                                  Double,       // 5
                                                  String        // 6
                                                  >>
      m_value;

  void constructVariant();
  const Variant& variant() const;
  Variant& variant();

  void readArray(std::istream& in);
  void readTrue(std::istream& in);
  void readFalse(std::istream& in);
  void readNumber(std::istream& in, char c);
  void readNull(std::istream& in);
  void readObject(std::istream& in);
  void readString(std::istream& in);
};

bool operator==(const Json::Nil&, const Json::Nil&);
bool operator!=(const Json::Nil&, const Json::Nil&);
bool operator<(const Json::Nil&, const Json::Nil&);

std::string to_string(const Json& json);

inline namespace io{
template <class T, typename = void, typename=void>
struct JsonSerializer {
    static void deserialize(const Json& json, T& t);
    static Json serialize(const T& t);
};

template <typename T>
void deserialize(const Json& json, T& t) {
    static_assert(!std::is_const<std::remove_reference_t<T>>::value, "Can't deserialize to const object!");
    JsonSerializer<T>::deserialize(json, t);
}

template <typename T>
Json serialize(const T& t) {
    return JsonSerializer<T>::serialize(t);
}
}

inline namespace literals{
Json operator ""_json(const char*, size_t);
}

}
