#include "concise_json_schema/Json.h"
#include "concise_json_schema/JsonException.h"
#include "console_style/ConsoleSyle.h"

#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace JSON;

using std::get;


namespace {

inline JSONParseException unexpected_eof() { return JSONParseException("unexpected EOF"); };
inline JSONParseException unexpected_char(const char expected, char& got) {
  return JSONParseException("expected `" + std::string(1, expected) + "`, got `" + std::string(1, got) + "`");
};
void expect_char(const char expected, char& got) {
  if (expected != got) {
    throw unexpected_char(expected, got);
  }
}
bool read_non_space(std::istream& in, char& c, bool skip_comments = true) {
  in.read(&c, 1);
  while (in.good()) {
    if (skip_comments && c == '/') {
      in.read(&c, 1);
      expect_char('*', c);
      in.read(&c, 1);
      while (in.good()) {
        if (c == '*') {
          if (in.peek() == '/') {
            in.read(&c, 1);
            break;
          }
        }
        in.read(&c, 1);
      }
      in.read(&c, 1);
    }
    if (!isspace(c)) {
      break;
    }
    in.read(&c, 1);
  }
  return in.good();
}
void throw_if_bad(std::istream& in) {
  if (!in.good()) {
    throw unexpected_eof();
  }
}
void read_non_space_or_throw(std::istream& in, char& c, bool skip_comments = true) {
  read_non_space(in, c, skip_comments);
  throw_if_bad(in);
}
}
Json::Json() {
  static_assert(sizeof(Variant) == sizeof(decltype(m_value)));
  static_assert(alignof(Variant) == alignof(decltype(m_value)));
  new (m_value.__data) Variant(Nil{});
}

Json::Json(Json&& other) : Json{} {
  new (m_value.__data) Variant(std::move(other.variant()));
}

Json::Json(const Json& other) : Json{} {
  new (m_value.__data) Variant(other.variant());
}

Json::~Json() { reinterpret_cast<Variant*>(m_value.__data)->~Variant(); }

bool Json::is_array() const { return variant().index() == 0; }

bool Json::is_bool() const { return variant().index() == 1; }

bool Json::is_integer() const { return variant().index() == 2; }

bool Json::is_null() const { return variant().index() == 3; }

bool Json::is_object() const { return variant().index() == 4; }

bool Json::is_double() const { return variant().index() == 5; }

bool Json::is_number() const { return is_double() || is_integer(); }

bool Json::is_string() const { return variant().index() == 6; }

Json::Boolean Json::get_bool() const {
  if (!is_bool()) {
    throw JsonGetException("not a bool");
  }
  return get<Boolean>(variant());
}

Json::Boolean& Json::get_bool() {
  if (!is_bool()) {
    throw JsonGetException("not a bool");
  }
  return get<Boolean>(variant());
}

Json::Integer Json::get_integer() const {
  if (!is_integer()) {
    throw JsonGetException("not an integer");
  }
  return get<Integer>(variant());
}

Json::Integer& Json::get_integer() {
  if (!is_integer()) {
    throw JsonGetException("not an integer");
  }
  return get<Integer>(variant());
}

const Json::Object& Json::get_object() const {
  if (!is_object()) {
    throw JsonGetException("not an object");
  }
  return get<Object>(variant());
}

Json::Object& Json::get_object() {
  if (!is_object()) {
    throw JsonGetException("not an object");
  }
  return get<Object>(variant());
}

Json::Double Json::get_double() const {
  if (!is_double()) {
    throw JsonGetException("not a double");
  }
  return get<Double>(variant());
}

Json::Double& Json::get_double() {
  if (!is_double()) {
    throw JsonGetException("not a double");
  }
  return get<Double>(variant());
}

Json::Double Json::get_number() const {
  if (is_integer()) return get_integer();
  if (!is_double()) {
    throw JsonGetException("not a number");
  }
  return get<Double>(variant());
}

const Json::String& Json::get_string() const {
  if (!is_string()) {
    throw JsonGetException("not a string");
  }
  return get<String>(variant());
}

Json::String& Json::get_string() {
  if (!is_string()) {
    throw JsonGetException("not a string");
  }
  return get<String>(variant());
}

const Json::Array& Json::get_array() const {
  if (!is_array()) {
    throw JsonGetException("not an array");
  }
  return get<Array>(variant());
}

Json::Array& Json::get_array() {
  if (!is_array()) {
    throw JsonGetException("not an array");
  }
  return get<Array>(variant());
}

const Json& Json::operator()(const std::string& name) const {
  try {
    return get_object().at(name);
  } catch (std::out_of_range&) {
    throw JSONRangeException(*this, name);
  }
}

Json& Json::operator()(const std::string& name) {
  try {
    return get_object()[name];
  } catch (std::out_of_range&) {
    throw JSONRangeException(*this, name);
  }
}

size_t Json::count(const std::string& name) const { return get_object().count(name); }

const Json& Json::operator[](size_t index) const {
  try {
    return get_array().at(index);
  } catch (std::out_of_range&) {
    throw JSONRangeException(*this, index);
  }
}

Json& Json::operator[](size_t index) {
  try {
    return get_array().at(index);
  } catch (std::out_of_range&) {
    throw JSONRangeException(*this, index);
  }
}

size_t Json::size() const {
  if (is_object()) {
    return get_object().size();
  } else if (is_array()) {
    return get_array().size();
  } else {
    throw JsonGetException("size(): not Array nor Object");
  }
}

Json::Array::const_iterator Json::begin() const { return get_array().begin(); }

std::vector<Json>::iterator Json::begin() { return get_array().begin(); }

Json::Array::const_iterator Json::end() const { return get_array().end(); }

std::vector<Json>::iterator Json::end() { return get_array().end(); }

const Json::Variant& Json::variant() const { return *reinterpret_cast<const Variant*>(m_value.__data); }
Json::Variant& Json::variant() { return *reinterpret_cast<Variant*>(m_value.__data); }

void Json::readArray(std::istream& in) {
  char c;
  Json::Array value;

  read_non_space_or_throw(in,c);

  if (c != ']') {
    for (; in.good();) {
      in.unget();
      value.resize(value.size() + 1);
      in >> value.back();
      read_non_space_or_throw(in,c);

      if (c == ']') {
        break;
      }
      expect_char(',',c);
      read_non_space_or_throw(in,c);
    }
  }

  variant() = std::move(value);
}

void Json::readTrue(std::istream& in) {
  char data[3];
  in.read(data, 3);
  if (data[0] != 'r' || data[1] != 'u' || data[2] != 'e') {
    throw JSONParseException("bad `true` keyword");
  }

  variant() = true;
}

void Json::readFalse(std::istream& in) {
  char data[4];
  in.read(data, 4);
  if (data[0] != 'a' || data[1] != 'l' || data[2] != 's' || data[3] != 'e') {
    throw JSONParseException("bad `false` keyword");
  }

  variant() = false;
}

void Json::readNumber(std::istream& in, char c) {
  std::string text;
  text += c;
  size_t dots = 0;
  if (c == '.') {
    dots++;
  }
  bool exps = false;
  for (; in.good();) {
    int i = in.peek();
    if (i >= '0' && i <= '9') {
      in.read(&c, 1);
      text += c;
    } else if (i == '.') {
      in.read(&c, 1);
      text += '.';
      ++dots;
    } else {
      break;
    }
  }

  if (dots > 1 || text.size() - (int)(text[0] == '-') - dots == 0) {
    throw JSONParseException("invalid number");
  }

  int i = in.peek();
  if (in.peek() == 'e' || in.peek() == 'E') {
    exps = true;
    in.read(&c, 1);
    text += c;
    i = in.peek();
    if (i == '+') {
      in.read(&c, 1);
      text += c;
      i = in.peek();
    } else if (i == '-') {
      in.read(&c, 1);
      text += c;
      i = in.peek();
    }

    if (i < '0' || i > '9') {
      throw JSONParseException("invalid number");
    }

    do {
      in.read(&c, 1);
      text += c;
      i = in.peek();
    } while (i >= '0' && i <= '9');
  }

  if (exps || dots > 0) {
    double value;
    std::istringstream(text) >> value;
    variant() = value;
  } else {
    if (text.size() > 1 && ((text[0] == '0') || (text[0] == '-' && text[1] == '0'))) {
      throw JSONParseException("invalid number");
    }

    int64_t value;
    std::istringstream(text) >> value;
    variant() = value;
  }
}

void Json::readNull(std::istream& in) {
  char data[3];
  in.read(data, 3);
  if (data[0] != 'u' || data[1] != 'l' || data[2] != 'l') {
    throw JSONParseException("bad `null` keyword");
  }

  variant() = Nil{};
}

void Json::readObject(std::istream& in) {
  char c;
  Json::Object value;
  read_non_space_or_throw(in,c);

  if (c != '}') {
    std::string name;
    for (; in.good();) {
      expect_char('"',c);
      name.clear();
      for (; in.good();) {
        in.read(&c, 1);
        if (c == '"') {
          break;
        }

        if (c == '\\') {
          name += c;
          in.read(&c, 1);
        }

        name += c;
      }

      read_non_space_or_throw(in,c);
      expect_char(':',c);

      in >> value[name];
      read_non_space_or_throw(in,c);
      if (c == '}') {
        break;
      }

      expect_char(',',c);
      read_non_space_or_throw(in,c);
    }
  }

  variant() = std::move(value);
}

void Json::readString(std::istream& in) {
  char c;
  std::string value;

  for (; in.good();) {
    in.read(&c, 1);
    if (c == '"') {
      break;
    }

    if (c == '\\') {
      value += c;
      in.read(&c, 1);
    }

    value += c;
  }

  if (!in.good()) {
    throw JSONParseException("unexpected EOF");
  }

  variant() = std::move(value);
}

std::istream& JSON::operator>>(std::istream& in, Json& JSONValue) {
  char c;
  read_non_space_or_throw(in,c);
  while (c== '/'){
    in.unget();
    read_non_space_or_throw(in,c);
  }
  if (c == '[') {
    JSONValue.readArray(in);
  } else if (c == 't') {
    JSONValue.readTrue(in);
  } else if (c == 'f') {
    JSONValue.readFalse(in);
  } else if ((c == '-') || (c >= '0' && c <= '9') || ((c == '.'))) {
    JSONValue.readNumber(in, c);
  } else if (c == 'n') {
    JSONValue.readNull(in);
  } else if (c == '{') {
    JSONValue.readObject(in);
  } else if (c == '"') {
    JSONValue.readString(in);
  } else {
    throw JSONParseException("unexpected char `" + std::string(1, c) + "`");
  }
  return in;
}

std::ostream& JSON::operator<<(std::ostream& out, const Json& json) {
  using std::isnan;

  if (json.is_array()) {
    const Json::Array& array = json.get_array();
    out << '[';
    if (array.size() > 0) {
      out << array[0];
      for (size_t i = 1; i < array.size(); ++i) {
        out << ',' << array[i];
      }
    }
    out << ']';
  } else if (json.is_bool()) {
    out << (json.get_bool() ? "true" : "false");
  } else if (json.is_integer()) {
    out << json.get_integer();
  } else if (json.is_null()) {
    out << "null";
  } else if (json.is_object()) {
    const Json::Object& object = json.get_object();
    out << '{';
    auto iter = object.begin();
    if (iter != object.end()) {
      out << "\"" << iter->first << "\":" << iter->second;
      ++iter;
      for (; iter != object.end(); ++iter) {
        out << ","
            << "\"" << iter->first << "\":" << iter->second;
      }
    }
    out << '}';
  } else if (json.is_double()) {
    const double upper_fixed = 1e5;
    const double lower_fixed = 1.0 / upper_fixed;
    double d = json.get_double();
    if (std::isnan(d) || std::isinf(d)) {
      throw std::runtime_error("NaN or Inf is not representative in Json format");
    }
    if (d > upper_fixed || d < -upper_fixed || (d < lower_fixed && d > -lower_fixed)) {
      std::ostringstream stream;
      stream << std::scientific << std::setprecision(17) << json.get_double();
      std::string value = stream.str();
      auto epos = value.rfind('e');
      auto pos = epos - 1;
      while (pos > 0 && value[pos - 1] == '0') {
        pos--;
      }
      if (value[pos - 1] == '.') {
        pos--;
      }
      out << value.substr(0, pos);
      out << value.substr(epos);
    } else {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(17) << json.get_double();
      std::string value = stream.str();
      while (value.size() > 1 && value[value.size() - 2] != '.' && value[value.size() - 1] == '0') {
        value.resize(value.size() - 1);
      }
      out << value;
    }
  } else if (json.is_string()) {
    out << '"' << json.get_string() << '"';
  } else {
    assert(false);
  }

  return out;
}

void Json::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  namespace cs = ConsoleStyle;
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  if (is_array()) {
    const Json::Array& array = get_array();
    out << "[";
    if (array.size() > 0) {
      out << "\n";
      array[0].pretty_print(out, tab_size, offset + tab_size);
      for (size_t i = 1; i < array.size(); ++i) {
        out << ",\n";
        array[i].pretty_print(out, tab_size, offset + tab_size);
      }
      out << "\n";
      out << std::string(offset, ' ');
    }
    out << ']';
  } else if (is_bool()) {
    out << cs::bright() << (get_bool() ? "true" : "false");
  } else if (is_integer()) {
    out << cs::white() << get_integer();
  } else if (is_null()) {
    out << cs::bright() << "null";
  } else if (is_object()) {
    const Json::Object& object = get_object();
    out << "{";
    auto iter = object.begin();
    if (iter != object.end()) {
      out << "\n";
      int maxKeyLength = object.empty() ? 0ul : std::max_element(object.begin(), object.end(), [](auto& a, auto& b) {
                                                  return a.first.size() < b.first.size();
                                                })->first.size();
      out << std::string(offset + tab_size, ' ');
      out << cs::bright() << cs::magenta() << "\"" << iter->first << "\""
          << std::string(maxKeyLength - iter->first.size(), ' ');
      out << ": ";
      iter->second.pretty_print(out, tab_size, offset + tab_size + maxKeyLength + 4, false);
      ++iter;
      for (; iter != object.end(); ++iter) {
        out << ",\n";
        out << std::string(offset + tab_size, ' ');
        out << cs::bright() << cs::magenta() << "\"" << iter->first << "\""
            << std::string(maxKeyLength - iter->first.size(), ' ');
        out << ": ";
        iter->second.pretty_print(out, tab_size, offset + tab_size + maxKeyLength + 4, false);
      }
      out << "\n";
      out << std::string(offset, ' ');
    }
    out << '}';
  } else if (is_double()) {
    SET_SCOPED_CONSOLE_STYLE(out, cs::white());
    const double upper_fixed = 1e5;
    const double lower_fixed = 1.0 / upper_fixed;
    double d = get_double();
    if (std::isnan(d) || std::isinf(d)) {
      throw std::runtime_error("NaN or Inf is not representative in Json format");
    }
    if (d > upper_fixed || d < -upper_fixed || (d < lower_fixed && d > -lower_fixed)) {
      std::ostringstream stream;
      stream << std::scientific << std::setprecision(14) << get_double();
      std::string value = stream.str();
      auto epos = value.rfind('e');
      auto pos = epos - 1;
      while (pos > 0 && value[pos - 1] == '0') {
        pos--;
      }
      if (value[pos - 1] == '.') {
        pos--;
      }
      out << value.substr(0, pos);
      out << value.substr(epos);
    } else {
      std::ostringstream stream;
      stream << std::fixed << std::setprecision(14) << get_double();
      std::string value = stream.str();
      while (value.size() > 1 && value[value.size() - 2] != '.' && value[value.size() - 1] == '0') {
        value.resize(value.size() - 1);
      }
      out << value;
    }
  } else if (is_string()) {
    out << cs::bright() << cs::green() << '"' << get_string() << '"';
  } else {
    assert(false);
  }
}

Json& Json::push_back(const Json& val) {
  auto& array = get_array();
  array.push_back(val);
  return array.back();
}

Json& Json::push_back(Json&& val) {
  auto& array = get_array();
  array.push_back(std::move(val));
  return array.back();
}

Json& Json::insert(const std::string key, const Json& value) {
  auto& obj = get_object();
  auto& res = obj[key] = value;
  return res;
}

Json& Json::insert(const std::string key, Json&& value) {
  auto& obj = get_object();
  auto& res = obj[key] = std::move(value);
  return res;
}

Json& Json::operator=(const Json& value) {
  variant() = value.variant();
  return *this;
}

Json& Json::operator=(Json&& value) {
  variant() = std::move(value.variant());
  return *this;
}

Json& Json::operator=(const Json::Array& value) {
  variant() = value;
  return *this;
}

Json& Json::operator=(Json::Array&& value) {
  variant() = std::move(value);
  return *this;
}

Json& Json::operator=(Json::Boolean value) {
  variant() = value;
  return *this;
}

Json& Json::operator=(Json::Integer value) {
  variant() = value;
  return *this;
}

Json& Json::operator=(Json::Nil) {
  variant() = Nil{};
  return *this;
}

Json& Json::operator=(const Object& value) {
  variant() = value;
  return *this;
}

Json& Json::operator=(Object&& value) {
  variant() = std::move(value);
  return *this;
}

Json& Json::operator=(Json::Double value) {
  variant() = value;
  return *this;
}

Json& Json::operator=(const Json::String& value) {
  variant() = value;
  return *this;
}

Json& Json::operator=(std::string&& value) {
  variant() = std::move(value);
  return *this;
}


Json& Json::operator=(const char* value) { return operator=(Json::String(value)); }

Json::Json(const Json::Array& value) {
  new (m_value.__data) Variant(std::move(value));
}

Json::Json(Json::Array&& value) {
  new (m_value.__data) Variant(std::move(value));
}

Json::Json(bool value) {
  new (m_value.__data) Variant(value);
}

Json::Json(int64_t value) {
  new (m_value.__data) Variant(value);
}

Json::Json(int value) {
  new (m_value.__data) Variant(Integer(value));
}

Json::Json(Json::Nil) : Json() {
}

Json::Json(const Json::Object& value) {
  new (m_value.__data) Variant(value);
}

Json::Json(Json::Object&& value) {
  new (m_value.__data) Variant(std::move(value));
}

Json::Json(double value) {
  new (m_value.__data) Variant(value);
}

Json::Json(const Json::String& value) {
  new (m_value.__data) Variant(value);
}

Json::Json(Json::String&& value) {
  new (m_value.__data) Variant(std::move(value));
}

Json::Json(const char* value) {
  new (m_value.__data) Variant(Json::String(value));
}

bool Json::operator!=(const Json& other) const { return variant() != other.variant(); }

bool Json::operator<(const Json& other) const { return variant() < other.variant(); }

bool Json::operator==(const Json& other) const { return variant() == other.variant(); }

std::string JSON::to_string(const Json& json) {
  std::ostringstream ostr;
  ostr << json;
  return ostr.str();
}

bool JSON::operator==(const Json::Nil&, const Json::Nil&) { return true; };
bool JSON::operator!=(const Json::Nil&, const Json::Nil&) { return false; };
bool JSON::operator<(const Json::Nil&, const Json::Nil&) { return false; };

Json JSON::literals::operator""_json(const char* input, size_t) {
  Json json;
  std::istringstream(input) >> json;
  return std::move(json);
}
