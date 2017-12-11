#include "concise_json_schema/Schema.h"
#include <cassert>
#include <cmath>
#include <iomanip>
#include "console_style/ConsoleSyle.h"

namespace cs = ConsoleStyle;
using namespace JSON;

namespace {

struct reference_visiter {
  bool is_extended;
  const Json& json;
  template <typename T>
  SchemaMatchResult operator()(const T& v) {
    return v.match(json);
  };
  SchemaMatchResult operator()(const Schema::ObjectSchema& o) { return o.match(json, is_extended); };
};

inline SchemaParseException unexpected_eof() { return SchemaParseException("unexpected EOF"); };
inline SchemaParseException unexpected_char(const char expected, char& got) {
  return SchemaParseException("expected `" + std::string(1, expected) + "`, got `" + std::string(1, got) + "`");
};

inline void expect_char(const char expected, char& got) {
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

void read_doc_comment(std::istream& in, std::vector<std::string>& docs) {
  char c;
  in.read(&c, 1);
  expect_char('*', c);
  in.read(&c, 1);
  throw_if_bad(in);
  if (c == '*') {
    // empty comment nothing to do
    if (in.peek() == '/') {
      in.read(&c, 1);
      return;
    }
    docs.push_back("");
    auto& str = docs.back();
    //    str.clear();
    in.read(&c, 1);
    while (in.good()) {
      if (c == '*') {
        if (in.peek() == '/') {
          in.read(&c, 1);
          break;
        }
      }
      str += c;
      in.read(&c, 1);
    }
  } else {
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
  }
  throw_if_bad(in);
}

void pretty_print(std::ostream& out, const SchemaMatchResult::MatchError& error, size_t offset) {
  const int tab_size = 4;
  if (offset == 0) {
    out << ". ";
  }
  out << error.what();
  auto it = error.nested.begin();
  auto end = error.nested.end();
  if (it != end) {
    out << std::endl;
    if (error.nested.size() > 1) {
      end--;
      for (; it != end; ++it) {
        out << std::string(offset, ' ') << "|-- ";
        pretty_print(out, *it, offset + tab_size);
        out << std::endl;
      }
    }
    out << std::string(offset, ' ') << "`-- ";
    pretty_print(out, *it, offset + tab_size);
  }
}

void pretty_print(
    const Json& json, std::ostream& out, int tab_size, int offset, bool first_line_offset,
    const std::map<const Json*, std::vector<std::pair<int, const SchemaMatchResult::MatchError*>>>& comments) {
  namespace cs = ConsoleStyle;
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  if (json.is_array()) {
    const Json::Array& array = json.get_array();
    out << "[";
    if (array.size() > 0) {
      out << "\n";
      pretty_print(array[0], out, tab_size, offset + tab_size, true, comments);
      for (size_t i = 1; i < array.size(); ++i) {
        out << ",\n";
        pretty_print(array[i], out, tab_size, offset + tab_size, true, comments);
      }
      out << "\n";
      out << std::string(offset, ' ');
    }
    out << ']';
  } else if (json.is_bool()) {
    out << cs::bright() << (json.get_bool() ? "true" : "false");
  } else if (json.is_integer()) {
    out << cs::white() << json.get_integer();
  } else if (json.is_null()) {
    out << cs::bright() << "null";
  } else if (json.is_object()) {
    const Json::Object& object = json.get_object();
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
      pretty_print(iter->second, out, tab_size, offset + tab_size + maxKeyLength + 4, false, comments);
      ++iter;
      for (; iter != object.end(); ++iter) {
        out << ",\n";
        out << std::string(offset + tab_size, ' ');
        out << cs::bright() << cs::magenta() << "\"" << iter->first << "\""
            << std::string(maxKeyLength - iter->first.size(), ' ');
        out << ": ";
        pretty_print(iter->second, out, tab_size, offset + tab_size + maxKeyLength + 4, false, comments);
      }
      out << "\n";
      out << std::string(offset, ' ');
    }
    out << '}';
  } else if (json.is_double()) {
    SET_SCOPED_CONSOLE_STYLE(out, cs::white());
    const double upper_fixed = 1e5;
    const double lower_fixed = 1.0 / upper_fixed;
    double d = json.get_double();
    if (std::isnan(d) || std::isinf(d)) {
      throw std::runtime_error("nan/inf are not representative in Json format");
    }
    if (d > upper_fixed || d < -upper_fixed || (d < lower_fixed && d > -lower_fixed)) {
      std::ostringstream stream;
      stream << std::scientific << std::setprecision(14) << json.get_double();
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
      stream << std::fixed << std::setprecision(14) << json.get_double();
      std::string value = stream.str();
      while (value.size() > 1 && value[value.size() - 2] != '.' && value[value.size() - 1] == '0') {
        value.resize(value.size() - 1);
      }
      out << value;
    }
  } else if (json.is_string()) {
    out << cs::bright() << cs::green() << '"' << json.get_string() << '"';
  }
  if (comments.count(&json)) {
    auto& lines = comments.at(&json);
    SET_SCOPED_CONSOLE_STYLE(out, cs::red())
    int base_level = lines.front().first;
    out << "\n" << std::string(offset, ' ');
    out << std::string(8, '^') << "\n";
    for (auto& comment : lines) {
      assert(comment.second->schema != nullptr);
      out << std::string(offset + 2 * tab_size * (comment.first - base_level), ' ');
      out << comment.second->what() << cs::yellow() << " //" << *comment.second->schema << std::endl;
    }
    out << std::string(offset, ' ');
  }
}
}

SchemaMatchResult::SchemaMatchResult() : m_match(MatchSuccess{}) {}
SchemaMatchResult::SchemaMatchResult(SchemaMatchResult::MatchError&& matchError) : m_match(std::move(matchError)){};

const SchemaMatchResult::MatchError& SchemaMatchResult::get_error() const& {
  if (m_match.index() == 0) {
    throw std::runtime_error("SchemaMatchResult: no match error");
  }
  return estd::get<MatchError>(m_match);
}

SchemaMatchResult::MatchError& SchemaMatchResult::get_error()& {
  if (m_match.index() == 0) {
    throw std::runtime_error("SchemaMatchResult: no match error");
  }
  return estd::get<MatchError>(m_match);
}

SchemaMatchResult::MatchError SchemaMatchResult::get_error()&& {
  if (m_match.index() == 0) {
    throw std::runtime_error("SchemaMatchResult: no match error");
  }
  return estd::get<MatchError>(std::move(m_match));
}

SchemaMatchResult::MatchError::MatchError(const Json& json, const std::string& what)
    : runtime_error(what), json(&json) {}
SchemaMatchResult::MatchError::MatchError(const Json& json, const std::string& what,
                                          SchemaMatchResult::MatchError&& matchError)
    : runtime_error(what), json(&json), nested{std::move(matchError)} {}

void SchemaMatchResult::MatchError::pretty_wordy_print(std::ostream& out, int tab_size, int offset) const {
  std::vector<std::pair<int, const MatchError*>> queue;
  std::map<const Json*, std::vector<std::pair<int, const MatchError*>>> comments;
  queue.emplace_back(0, this);

  while (!queue.empty()) {
    int level = queue.back().first;
    const MatchError& m = *queue.back().second;
    queue.pop_back();
    comments[m.json].emplace_back(level, &m);
    for (auto& x : m.nested) {
      queue.emplace_back(level + 1, &x);
    }
  }
  pretty_print(*json, out, tab_size, offset, true, comments);
}

SchemaMatchResult Schema::AllOfSchema::match(const Json& json) const {
  for (size_t i = 0; i < items.size(); i++) {
    auto m = items[i].match(json);
    if (!m) {
      return SchemaMatchResult::MatchError(json, "allOf: schema[" + std::to_string(i) + "] fails",
                                           std::move(m).get_error());
    }
  }
  return SchemaMatchResult{};
}
SchemaMatchResult Schema::AnySchema::match(const Json& json) const { return SchemaMatchResult{}; }
SchemaMatchResult Schema::AnyOfSchema::match(const Json& json) const {
  SchemaMatchResult::MatchError result(json, "anyOf: no match");
  int count = 0;
  for (auto& x : items) {
    auto m = x.match(json);
    if (m) {
      return SchemaMatchResult();
    } else {
      result.nested.push_back(std::move(m).get_error());
    }
  }
  if (count == 0) {
    return std::move(result);
  }
  return result;
}
SchemaMatchResult Schema::ArraySchema::match(const Json& json) const {
  if (!json.is_array()) {
    return SchemaMatchResult::MatchError(json, "array: not an array");
  }
  if (min && json.size() < min.value()) {
    return SchemaMatchResult::MatchError(
        json, "array: size (" + std::to_string(json.size()) + ") < min items (" + std::to_string(min.value()) + ")");
  }
  if (max && json.size() > max.value()) {
    return SchemaMatchResult::MatchError(
        json, "array: size (" + std::to_string(json.size()) + ") < max items (" + std::to_string(max.value()) + ")");
  }

  if (items_schema) {
    for (size_t i = 0; i < json.size(); i++) {
      auto m = items_schema->match(json[i]);
      if (!m) {
        return SchemaMatchResult::MatchError(json, "array: bad item[ " + std::to_string(i) + " ]",
                                             std::move(m).get_error());
      }
    }
  }

  if (unique) {
    auto x = json.get_array();
    std::sort(x.begin(), x.end());
    auto e = std::unique(x.begin(), x.end());
    if (e != x.end()) {
      return SchemaMatchResult::MatchError(json, "array: items are not unique");
    }
  }

  return SchemaMatchResult{};
}
SchemaMatchResult Schema::BoolSchema::match(const Json& json) const {
  if (!json.is_bool()) {
    return SchemaMatchResult::MatchError(json, "bool: not a bool");
  }
  return SchemaMatchResult{};
}
SchemaMatchResult Schema::EnumSchema::match(const Json& json) const {
  for (auto& x : enumeration) {
    if (json == x) {
      return SchemaMatchResult{};
    }
  }
  return SchemaMatchResult::MatchError(json, "enum: not one of " + to_string(Json(Json::Array(enumeration))));
}
SchemaMatchResult Schema::DoubleSchema::match(const Json& json) const {
  if (!json.is_number()) {
    return SchemaMatchResult::MatchError(json, "double: not a double");
  }
  if (min && json.get_number() < min.value()) {
    return SchemaMatchResult::MatchError(
        json, "double: value (" + std::to_string(json.get_number()) + ")< min (" + std::to_string(min.value()) + ")");
  }
  if (max && json.get_number() > max.value()) {
    return SchemaMatchResult::MatchError(
        json, "double: value (" + std::to_string(json.get_number()) + ")> max (" + std::to_string(max.value()) + ")");
  }
  return SchemaMatchResult{};
}
SchemaMatchResult Schema::IntSchema::match(const Json& json) const {
  if (!json.is_integer()) {
    return SchemaMatchResult::MatchError(json, "int: not an integer");
  }
  if (min && json.get_integer() < min.value()) {
    return SchemaMatchResult::MatchError(
        json, "int: value (" + std::to_string(json.get_integer()) + ")< min (" + std::to_string(min.value()) + ")");
  }
  if (max && json.get_integer() > max.value()) {
    return SchemaMatchResult::MatchError(
        json, "int: value (" + std::to_string(json.get_integer()) + ")> max (" + std::to_string(max.value()) + ")");
  }
  return SchemaMatchResult{};
}
SchemaMatchResult Schema::NotSchema::match(const Json& json) const {
  auto m = sub->match(json);
  if (m) {
    return SchemaMatchResult::MatchError(json, "not: matches");
  } else {
    return SchemaMatchResult{};
  }
}
SchemaMatchResult Schema::NullSchema::match(const Json& json) const {
  if (json.is_null()) {
    return SchemaMatchResult{};
  } else {
    return SchemaMatchResult::MatchError(json, "null: not a null");
  }
}
SchemaMatchResult Schema::OneOfSchema::match(const Json& json) const {
  SchemaMatchResult::MatchError result(json, "oneOf: no match");
  int count = 0;
  for (auto& x : items) {
    auto m = x.match(json);
    if (m) {
      count++;
      if (count > 1) {
        return SchemaMatchResult::MatchError(json, "oneOf: more than one match");
      }
    } else {
      result.nested.push_back(std::move(m).get_error());
    }
  }
  if (count == 0) {
    return std::move(result);
  }
  return SchemaMatchResult();
}
SchemaMatchResult Schema::ObjectSchema::match(const Json& json, bool allow_extensions) const {
  if (!json.is_object()) {
    return SchemaMatchResult::MatchError(json, "object: not an object");
  }

  for (auto& x : json.get_object()) {
    bool isPatternProperty = false;

    for (auto& pattern : pattern_properties) {
      if (std::regex_match(x.first, pattern.first.second)) {
        isPatternProperty = true;
        auto m = pattern.second.match(x.second);
        if (!m) {
          return SchemaMatchResult::MatchError(json, "object: bad pattern property `" + x.first + "`",
                                               std::move(m).get_error());
        }
      }
    }

    if (properties.count(x.first)) {
      auto m = std::get<0>(properties.at(x.first)).match(x.second);
      if (!m) {
        return SchemaMatchResult::MatchError(json, "object: bad property `" + x.first + "`", std::move(m).get_error());
      }
    } else if (!isPatternProperty && !(is_extensible || allow_extensions)) {
      return SchemaMatchResult::MatchError(json, "object: unexpected property `" + x.first + "`");
    }
  }
  for (auto& p : properties) {
    if (!std::get<2>(p.second) && !json.count(p.first)) {
      return SchemaMatchResult::MatchError(json, "object: no property `" + p.first + "``");
    }
  }
  return SchemaMatchResult{};
}
SchemaMatchResult Schema::ReferenceSchema::match(const Json& json) const {
  if (!ref) {
    throw std::runtime_error("bad reference");
  }

  return estd::visit(reference_visiter{is_extended, json}, ref->m_schema);
}
SchemaMatchResult Schema::StringSchema::match(const Json& json) const {
  if (!json.is_string()) {
    return SchemaMatchResult::MatchError(json, "str: not a string");
  }
  auto& s = json.get_string();
  if (min && s.length() < min.value()) {
    return SchemaMatchResult::MatchError(
        json, "str: length (" + std::to_string(s.length()) + ") < minLength (" + std::to_string(min.value()) + ")");
  }
  if (max && json.get_string().length() > max.value()) {
    return SchemaMatchResult::MatchError(
        json, "str: length (" + std::to_string(s.length()) + ") > maxLength (" + std::to_string(max.value()) + ")");
  }
  if (pattern && !std::regex_match(json.get_string(), pattern->second)) {
    return SchemaMatchResult::MatchError(json, "str: pattern mismatch");
  }
  return SchemaMatchResult{};
}
SchemaMatchResult Schema::TupleSchema::match(const Json& json) const {
  if (!json.is_array()) {
    return SchemaMatchResult::MatchError(json, "tuple: is not an array");
  }
  if (json.size() != items.size()) {
    return SchemaMatchResult::MatchError(json, "tuple: size of tuple != " + std::to_string(items.size()));
  }

  for (size_t i = 0; i < items.size(); i++) {
    auto m = items[i].match(json[i]);
    if (!m) {
      return SchemaMatchResult::MatchError(json, "tuple: bad element [" + std::to_string(i) + "]",
                                           std::move(m).get_error());
    }
  }

  return SchemaMatchResult();
}
SchemaMatchResult Schema::match(const Json& json) const {
  SchemaMatchResult result = estd::visit([&json](auto&& v) { return v.match(json); }, m_schema);
  if (!result) {
    result.get_error().schema = this;
  }
  return result;
}

Json Schema::AllOfSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("allOf") = std::vector<Json>{};
  for (auto& x : items) {
    result("allOf").push_back(x.asJsonSchema());
  }
  return std::move(result);
}
Json Schema::AnySchema::asJsonSchema() const { return Json(Json::Object{}); }
Json Schema::AnyOfSchema::asJsonSchema() const {
  Json result;
  result = std::map<std::string, Json>{};
  result("anyOf") = std::vector<Json>{};
  for (auto& x : items) {
    result("anyOf").push_back(x.asJsonSchema());
  }
  return std::move(result);
}
Json Schema::ArraySchema::asJsonSchema() const {
  Json result;
  result = std::map<std::string, Json>{};
  result("type") = "array";
  result("items") = items_schema->asJsonSchema();

  if (min) {
    result("minItems") = Json(min.value());
  }
  if (max) {
    result("maxItems") = Json(max.value());
  }
  if (unique) {
    result("uniqueItems") = true;
  }
  return std::move(result);
}
Json Schema::BoolSchema::asJsonSchema() const {
  Json result;
  result = std::map<std::string, Json>{};
  result("type") = "boolean";
  return std::move(result);
}
Json Schema::EnumSchema::asJsonSchema() const {
  Json result;
  result = std::map<std::string, Json>{};
  result("enum") = enumeration;
  return std::move(result);
}
Json Schema::DoubleSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("type") = "number";
  if (min) {
    result("minimum") = min.value();
  }
  if (max) {
    result("maximum") = max.value();
  }
  return std::move(result);
}
Json Schema::IntSchema::asJsonSchema() const {
  Json result;
  result = std::map<std::string, Json>{};
  result("type") = "integer";
  if (min) {
    result("minimum") = Json(min.value());
  }
  if (max) {
    result("maximum") = Json(max.value());
  }
  return std::move(result);
}
Json Schema::NotSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("not") = sub->asJsonSchema();
  return std::move(result);
}
Json Schema::NullSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("type") = "null";
  return std::move(result);
}
Json Schema::ObjectSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("type") = "object";
  result("additionalProperties") = is_extensible;
  if (!properties.empty()) {
    result("properties") = Json::Object{};
    result("required") = Json::Array{};
    for (auto& x : properties) {
      result("properties")(x.first) = std::get<i_scheme>(x.second).asJsonSchema();
      if (std::get<i_default>(x.second)) {
        result("properties")(x.first)("default") = std::get<i_default>(x.second).value();
      }
      if (!std::get<i_optional>(x.second)) {
        result("required").push_back(Json(std::string(x.first)));
      }
    }
  }
  if (!pattern_properties.empty()) {
    result("patternProperties") = Json::Object{};
    for (auto& x : pattern_properties) {
      result("patternProperties")("^" + x.first.first + "$") = x.second.asJsonSchema();
    }
  }
  return std::move(result);
}
Json Schema::OneOfSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("oneOf") = std::vector<Json>{};
  for (auto& x : items) {
    result("oneOf").push_back(x.asJsonSchema());
  }
  return std::move(result);
}
Json Schema::ReferenceSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("$ref") = name;
  return std::move(result);
}
Json Schema::StringSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("type") = "string";
  if (pattern) {
    result("pattern") = Json("^" + pattern->first + "$");
  }
  if (min) {
    result("minLength") = Json::Integer(min.value());
  }
  if (max) {
    result("maxLength") = Json::Integer(max.value());
  }
  return std::move(result);
}
Json Schema::TupleSchema::asJsonSchema() const {
  Json result;
  result = Json::Object{};
  result("type") = "array";
  result("items") = std::vector<Json>{};

  result("minItems") = Json::Integer(items.size());
  result("maxItems") = Json::Integer(items.size());

  for (auto& x : items) {
    result("items").push_back(x.asJsonSchema());
  }
  return std::move(result);
}

Json Schema::asJsonSchema() const {
  Json json = estd::visit([](auto&& v) { return v.asJsonSchema(); }, m_schema);
  assert(json.is_object());
  if (!m_docstrings.empty()) {
    Json& s = json("description") = Json(Json::String(m_docstrings[0]));
    for (size_t i = 1; i < m_docstrings.size(); i++) {
      s.get_string() += "\n" + m_docstrings[i];
    }
  }
  if (definitions) {
    json("definitions") = Json::Object{};
    for (auto& x : *definitions) {
      json("definitions")(x.first) = x.second.second.asJsonSchema();
      json("definitions")(x.first)("id") = x.first;
    }
  }
  return std::move(json);
}

void Schema::AllOfSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "allOf";
  out << "(\n";
  auto it = items.begin();
  if (items.size() > 0) {
    it->pretty_print(out, tab_size, offset + tab_size);
    ++it;
  }
  for (; it != items.end(); ++it) {
    out << ",\n";
    it->pretty_print(out, tab_size, offset + tab_size);
  }
  if (items.size() > 0) {
    out << '\n';
  }
  out << std::string(offset, ' ') << ")";
}
void Schema::AnySchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "any";
}
void Schema::AnyOfSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  first_line_offset = false;
  out << std::string(offset, ' ') << cs::bright() << "anyOf";
  out << "(\n";
  auto it = items.begin();
  if (items.size() > 0) {
    it->pretty_print(out, tab_size, offset + tab_size);
    ++it;
  }
  for (; it != items.end(); ++it) {
    out << ",\n";
    it->pretty_print(out, tab_size, offset + tab_size);
  }
  if (items.size() > 0) {
    out << '\n';
  }
  out << std::string(offset, ' ') << ")";
}
void Schema::ArraySchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  first_line_offset = false;
  out << "[ ";
  if (unique) {
    out << cs::bright() << "unique ";
  }
  items_schema->pretty_print(out, tab_size, offset, false);
  out << "]";
  if (min || max) {
    out << "{";
    if (min) {
      out << min.value();
    }
    if (min != max) {
      out << ", ";
      if (max) {
        out << max.value();
      }
    }
    out << "}";
  };
}
void Schema::BoolSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  first_line_offset = false;
  out << cs::bright() << "bool";
}
void Schema::EnumSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "enum";
  out << "(";

  auto it = enumeration.begin();
  if (it != enumeration.end()) {
    out << *it;
    ++it;
    for (; it != enumeration.end(); ++it) {
      out << ", " << *it;
    }
  }
  out << ")";
}
void Schema::DoubleSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "double";
  if (min || max) {
    out << "(";
    if (min) {
      out << min.value();
    }
    out << " .. ";
    if (max) {
      out << max.value();
    }
    out << ")";
  };
}
void Schema::IntSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "int";
  if (min || max) {
    out << "(";
    if (min) {
      out << min.value();
    }
    out << " .. ";
    if (max) {
      out << max.value();
    }
    out << ")";
  };
}
void Schema::ObjectSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  if (is_extensible) {
    out << cs::bright() << "extensible ";
  }
  out << "{\n";
  int maxKeyLength =
      properties.empty() ? 0ul : std::max_element(properties.begin(), properties.end(), [](auto& a, auto& b) {
                                   return a.first.size() < b.first.size();
                                 })->first.size();
  int maxPatternLength =
      pattern_properties.empty()
          ? 0ul
          : std::max_element(pattern_properties.begin(), pattern_properties.end(), [](auto& a, auto& b) {
              return a.first.first.size() < b.first.first.size();
            })->first.first.size();
  int maxLength = std::max(maxKeyLength, maxPatternLength);
  int ololo = 5 + (maxPatternLength > 0 ? 1 : 0);
  {
    auto it = properties.begin();
    if (it != properties.end()) {
      out << std::string(offset + tab_size, ' ');
      out << (maxPatternLength ? " " : "");
      out << cs::bright() << cs::blue() << ((std::get<2>(it->second)) ? "?" : " ");
      out << cs::bright() << cs::magenta() << "\"" << it->first << "\""
          << std::string(maxLength - it->first.size(), ' ');
      out << ": ";
      std::get<0>(it->second).pretty_print(out, tab_size, offset + tab_size + maxLength + ololo, false);
      if (std::get<1>(it->second)) {
        out << " = ";
        out << cs::underline() << std::get<1>(it->second).value();
      }
      ++it;
    }
    for (; it != properties.end(); ++it) {
      out << ",\n";
      out << std::string(offset + tab_size, ' ');
      out << (maxPatternLength ? " " : "");
      out << cs::bright() << cs::blue() << ((std::get<2>(it->second)) ? "?" : " ");
      out << cs::bright() << cs::magenta() << "\"" << it->first << "\""
          << std::string(maxLength - it->first.size(), ' ');
      out << ": ";
      std::get<0>(it->second).pretty_print(out, tab_size, offset + tab_size + maxLength + ololo, false);
      if (std::get<1>(it->second)) {
        out << " = ";
        out << cs::underline() << std::get<1>(it->second).value();
      }
    }
  }
  {
    auto it = pattern_properties.begin();
    if (properties.empty() && it != pattern_properties.end()) {
      out << std::string(tab_size + offset, ' ');
      out << cs::bright() << cs::blue() << "re";
      out << cs::bright() << cs::magenta() << "\"" << it->first.first << "\""
          << std::string(maxLength - it->first.first.size(), ' ');
      out << ": ";
      it->second.pretty_print(out, tab_size, offset + tab_size + maxLength + ololo, false);
      ++it;
    }
    for (; it != pattern_properties.end(); ++it) {
      out << ",\n";
      out << std::string(tab_size + offset, ' ');
      out << cs::bright() << cs::blue() << "re";
      out << cs::bright() << cs::magenta() << "\"" << it->first.first << "\""
          << std::string(maxLength - it->first.first.size(), ' ');
      out << ": ";
      it->second.pretty_print(out, tab_size, offset + tab_size + maxLength + ololo, false);
    }
  }
  if (!pattern_properties.empty() || !properties.empty()) {
    out << "\n";
  }

  out << std::string(offset, ' ') << "}";
}
void Schema::OneOfSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "oneOf";
  out << "(\n";
  auto it = items.begin();
  if (items.size() > 0) {
    it->pretty_print(out, tab_size, offset + tab_size);
    ++it;
  }
  for (; it != items.end(); ++it) {
    out << ",\n";
    it->pretty_print(out, tab_size, offset + tab_size);
  }
  if (items.size() > 0) {
    out << '\n';
  }
  out << std::string(offset, ' ') << ")";
}
void Schema::NotSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  first_line_offset = false;
  out << cs::bright() << "not";
  out << "(";
  sub->pretty_print(out, tab_size, offset + tab_size, false);
  out << ")";
}
void Schema::NullSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  first_line_offset = false;
  out << cs::bright() << "null";
}
void Schema::ReferenceSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  if (is_extended) {
    out << cs::bright() << "extended ";
  }
  out << cs::yellow() << "@" << name;
}
void Schema::StringSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << cs::bright() << "str";
  if (pattern) {
    out << "(\"" << cs::bright() << cs::green() << pattern->first;
    out << "\")";
  }
  if (min || max) {
    out << "{";
    if (min) {
      out << min.value();
    }
    if (min != max) {
      out << ", ";
      if (max) {
        out << max.value();
      }
    }
    out << "}";
  };
}
void Schema::TupleSchema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (first_line_offset) {
    out << std::string(offset, ' ');
  }
  out << "(\n";
  auto it = items.begin();
  if (items.size() > 0) {
    it->pretty_print(out, tab_size, offset + tab_size);
    ++it;
    for (; it != items.end(); ++it) {
      out << ",\n";
      it->pretty_print(out, tab_size, offset + tab_size);
    }
    out << '\n';
  }
  out << std::string(offset, ' ') << ")";
}
void Schema::pretty_print(std::ostream& out, int tab_size, int offset, bool first_line_offset) const {
  if (definitions) {
    std::vector<std::pair<const std::string, std::pair<int, Schema>>*> ptrs;
    for (auto& d : *definitions) {
      ptrs.push_back(&d);
    }
    std::sort(ptrs.begin(), ptrs.end(), [](auto&& a, auto&& b) { return a->second.first < b->second.first; });

    for (auto& d : ptrs) {
      if (first_line_offset) {
        out << std::string(offset, ' ');
      }
      first_line_offset = true;
      out << "#" << cs::yellow() << d->first << "\n";
      d->second.second.pretty_print(out, tab_size, offset + tab_size);
      out << "\n";
      out << std::string(offset, ' ') << "#" << cs::italic() << "/*" << d->first << "*/\n";
    }
  }

  if (!m_docstrings.empty()) {
    if (first_line_offset) {
      out << std::string(offset, ' ');
    }
    first_line_offset = true;
    auto it = m_docstrings.begin();
    out << cs::italic() << "/**" << *it << "*/\n";
    ++it;
    for (; it != m_docstrings.end(); ++it) {
      out << std::string(offset, ' ');
      out << cs::italic() << "/**" << *it << "*/\n";
    }
  }

  visit([&out, tab_size, offset,
         first_line_offset](auto&& v) { return v.pretty_print(out, tab_size, offset, first_line_offset); },
        m_schema);
}

const Schema* Schema::resolveReference(const std::string& name, const std::vector<Schema*>& parents) {
  for (auto it = parents.rbegin(); it != parents.rend(); ++it) {
    if ((*it)->definitions) {
      auto x = (*it)->definitions->find(name);
      if (x != (*it)->definitions->end()) {
        return &x->second.second;
      }
    }
  }
  return nullptr;
}

void Schema::readAllOf(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = AllOfSchema{};
  auto& result = estd::get<AllOfSchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  expect_char('(', c);

  readCSV(in, result.items, parents);
}
void Schema::readAnyOf(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = AnyOfSchema{};
  AnyOfSchema& result = estd::get<AnyOfSchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  expect_char('(', c);
  readCSV(in, result.items, parents);
}
void Schema::readArray(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = ArraySchema{};
  ArraySchema& result = estd::get<ArraySchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  in.unget();

  if (c == 'u') {
    char buf[6];
    in.read(buf, 6);
    if (strncmp(buf, "unique", 6) != 0) {
      throw SchemaParseException("bad `unique` keyword");
    }
    in.read(&c, 1);
    if (!isspace(c)) {
      throw SchemaParseException("space character required after `unique` keyword");
    }
    throw_if_bad(in);
    result.unique = true;
  }
  result.items_schema = std::make_shared<Schema>();
  result.items_schema->readSchema(in, parents);
  read_non_space_or_throw(in, c);
  expect_char(']', c);
  if (!read_non_space(in, c)){
    return;
  }
  if (c == '{') {
    read_non_space_or_throw(in, c);
    if (c >= '0' && c <= '9') {
      in.unget();
      int min;
      in >> min;
      result.min = min;
      read_non_space_or_throw(in, c);
    }
    if (c == '}') {
      result.max = result.min;
    } else if (c == ',') {
      read_non_space_or_throw(in, c);
      if (c >= '0' && c <= '9') {
        in.unget();
        int max;
        in >> max;
        result.max = max;
        read_non_space_or_throw(in, c);
      }
      expect_char('}', c);
    }
  } else {
    in.unget();
  }
}
void Schema::readDefinition(std::istream& in, std::vector<Schema*>& parents) {
  char c;
  std::string key;
  in.read(&c, 1);
  throw_if_bad(in);
  if (!std::isalpha(c)) {
    throw SchemaParseException("first symbol of define must be alphabetic ([A-Za-z])");
  }
  while (in.good() && (std::isalnum(c) || c == '_')) {
    key += c;
    in.read(&c, 1);
  }
  if (!std::isspace(c)) {
    throw SchemaParseException("expected space character");
  }
  Schema schema;
  schema.readSchema(in, parents);

  read_non_space_or_throw(in, c);
  expect_char('#', c);

  if (!definitions) {
    definitions = std::make_shared<std::map<std::string, std::pair<int, Schema>>>();
  }
  int ndef = definitions->size();
  definitions->emplace(std::move(key), std::make_pair(ndef, std::move(schema)));
}
void Schema::readDouble(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = DoubleSchema{};
  DoubleSchema& result = estd::get<DoubleSchema>(m_schema);
  char c;
  if (!read_non_space(in, c)){
    return;
  }

  if (c == '(') {
    read_non_space_or_throw(in, c);
    char next = in.peek();
    char lastc = '\0';
    if ((c == '.' && next >= '0' && next <= '9') || c == '-' || c >= '0' && c <= '9') {
      in.unget();
      double min;
      in >> min;
      result.min = min;

      in.unget();
      in.read(&c, 1);
      if (c == '.' && in.peek() == '.') {
        in.unget();
        lastc = '.';
      }
      read_non_space_or_throw(in, c);
    }
    if (c == ')') {
      return;
    }
    expect_char('.', c);
    read_non_space_or_throw(in, c);
    expect_char('.', c);
    read_non_space_or_throw(in, c);
    next = in.peek();
    if (((c == '.' && lastc != '.') && next >= '0' && next <= '9') || c == '-' || c >= '0' && c <= '9') {
      in.unget();
      double max;
      in >> max;
      result.max = max;
      read_non_space_or_throw(in, c);
    }
    expect_char(')', c);
  } else {
    in.unget();
  }
}
void Schema::readEnum(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = EnumSchema{};
  EnumSchema& result = estd::get<EnumSchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  expect_char('(', c);

  read_non_space_or_throw(in, c);
  while (true) {
    if (c == ')') {
      break;
    }
    Json json;
    in.unget();
    in >> json;
    result.enumeration.push_back(std::move(json));
    read_non_space_or_throw(in, c);
    if (c == ',') {
      read_non_space_or_throw(in, c);
    } else {
      expect_char(')', c);
    }
  }
}
void Schema::readNot(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = NotSchema{};
  auto& result = estd::get<NotSchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  expect_char('(', c);
  if (!result.sub) {
    result.sub = std::make_shared<Schema>();
  }
  result.sub->readSchema(in, parents);
  read_non_space_or_throw(in, c);
  expect_char(')', c);
}
void Schema::readInt(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = IntSchema{};
  IntSchema& result = estd::get<IntSchema>(m_schema);

  char c;
  if (!read_non_space(in, c)){
    return;
  }

  if (c == '(') {
    read_non_space_or_throw(in, c);
    if (c == '-' || c >= '0' && c <= '9') {
      in.unget();
      int min;
      in >> min;
      result.min = min;
      read_non_space_or_throw(in, c);
    }
    if (c == ')') {
      return;
    }
    expect_char('.', c);
    read_non_space_or_throw(in, c);
    expect_char('.', c);
    read_non_space_or_throw(in, c);
    if (c == '-' || c >= '0' && c <= '9') {
      in.unget();
      int max;
      in >> max;
      result.max = max;
      read_non_space_or_throw(in, c);
    }
    expect_char(')', c);
  } else {
    in.unget();
  }
}
void Schema::readOneOf(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = OneOfSchema{};
  OneOfSchema& result = estd::get<OneOfSchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  expect_char('(', c);
  readCSV(in, result.items, parents);
}
void Schema::readObject(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = ObjectSchema{};
  ObjectSchema& result = estd::get<ObjectSchema>(m_schema);

  char c;
  read_non_space_or_throw(in, c);
  while (true) {
    if (c == '}') {
      break;
    }

    Schema::ObjectSchema::Property aProperty;
    bool is_regex = false;
    std::get<2>(aProperty) = false;
    if (c == '?') {
      read_non_space_or_throw(in, c);
      std::get<2>(aProperty) = true;
    } else if (c == 'r') {
      read_non_space_or_throw(in, c);
      expect_char('e', c);
      read_non_space_or_throw(in, c);
      is_regex = true;
    }

    // reading string
    std::string key;
    expect_char('"', c);
    for (; in.good();) {
      in.read(&c, 1);
      if (c == '"') {
        break;
      }

      if (c == '\\') {
        key += c;
        in.read(&c, 1);
      }
      key += c;
    }
    read_non_space_or_throw(in, c);
    expect_char(':', c);

    std::get<0>(aProperty).readSchema(in, parents);
    read_non_space_or_throw(in, c);

    if (c == '=') {
      if (!std::get<2>(aProperty)) {
        throw SchemaParseException("only optional properties can be defaulted");
      }
      std::get<1>(aProperty) = Json{};
      in >> std::get<1>(aProperty).value();
      read_non_space_or_throw(in, c);
    }

    if (is_regex) {
      result.pattern_properties.emplace_back(
          std::make_pair(std::make_pair(key, std::regex(key)), std::move(std::move(std::get<0>(aProperty)))));
    } else {
      result.properties.emplace(std::move(key), std::move(std::move(aProperty)));
    }

    if (c == ',') {
      read_non_space_or_throw(in, c);
    } else {
      expect_char('}', c);
    }
  }
}
void Schema::readReference(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = ReferenceSchema{};
  ReferenceSchema& result = estd::get<ReferenceSchema>(m_schema);

  char c;
  in.read(&c, 1);
  if (!std::isalpha(c)) {
    throw SchemaParseException("First symbol of define must be alphabetic ([A-Za-z])");
  }
  while (in.good() && (std::isalnum(c) || c == '_')) {
    result.name += c;
    in.read(&c, 1);
  }
  in.unget();
  result.ref = resolveReference(result.name, parents);
  if (!result.ref) {
    throw SchemaParseException("bad reference `@" + result.name + "`");
  }
}
void Schema::readString(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = StringSchema{};
  StringSchema& result = estd::get<StringSchema>(m_schema);

  char c;
  if (!read_non_space(in, c)){
    return;
  }
  if (c == '(') {
    result.pattern = std::make_shared<std::pair<std::string, std::regex>>();
    read_non_space_or_throw(in, c);
    expect_char('"', c);
    for (; in.good();) {
      in.read(&c, 1);
      if (c == '"') {
        break;
      }

      if (c == '\\') {
        result.pattern->first += c;
        in.read(&c, 1);
      }

      result.pattern->first += c;
    }

    read_non_space_or_throw(in, c);
    try {
      result.pattern->second = std::regex(result.pattern->first);
    } catch (std::regex_error& e) {
      throw SchemaParseException("bad regex `" + result.pattern->first + "`");
    }
    expect_char(')', c);
    if (!read_non_space(in, c)){
      return;
    }
  }

  if (c == '{') {
    read_non_space_or_throw(in, c);
    if (c >= '0' && c <= '9') {
      in.unget();
      int min;
      in >> min;
      result.min = min;
      read_non_space_or_throw(in, c);
    }
    if (c == '}') {
      result.max = result.min;
    } else if (c == ',') {
      read_non_space_or_throw(in, c);
      if (c >= '0' && c <= '9') {
        in.unget();
        int max;
        in >> max;
        result.max = max;
        read_non_space_or_throw(in, c);
      }
      expect_char('}', c);
    }
    if (!read_non_space(in, c)){
      return;
    }
  }
  in.unget();
}
void Schema::readTuple(std::istream& in, std::vector<Schema*>& parents) {
  m_schema = TupleSchema{};
  TupleSchema& result = estd::get<TupleSchema>(m_schema);
  readCSV(in, result.items, parents);
}
void Schema::readCSV(std::istream& in, std::vector<Schema>& values, std::vector<Schema*>& parents) {
  char c;
  read_non_space_or_throw(in, c, false);
  while (true) {
    if (c == ')') {
      break;
    }
    values.push_back({});
    in.unget();
    values.back().readSchema(in, parents);
    read_non_space_or_throw(in, c);
    if (c == ',') {
      read_non_space_or_throw(in, c);
    } else {
      expect_char(')', c);
    }
  }
}

void Schema::readSchema(std::istream& in, std::vector<Schema*>& parents) {
  struct ParentGuard {
    ParentGuard(std::vector<Schema*>& parents, Schema& current) : parents(parents) { parents.push_back(&current); }
    ~ParentGuard() {
      assert(!parents.empty());
      parents.pop_back();
    }
    std::vector<Schema*>& parents;
  };
  ParentGuard parent_guard(parents, *this);

  char c;
  read_non_space_or_throw(in, c, false);

  while (c == '/' || c == '#') {
    if (c == '/') {
      read_doc_comment(in, m_docstrings);
    } else if (c == '#') {
      readDefinition(in, parents);
    }
    read_non_space_or_throw(in, c, false);
  }
  if (c == '{') {
    readObject(in, parents);
  } else if (c == '[') {
    readArray(in, parents);
  } else if (c == '(') {
    readTuple(in, parents);
  } else if (c == '@') {
    readReference(in, parents);
  } else {
    std::string word;
    int maxWordLength = 16;
    word.reserve(maxWordLength);
    while (std::isalpha(c) && in.good() && maxWordLength) {
      word.push_back(c);
      in.read(&c, 1);
      maxWordLength--;
    }
    in.unget();
    if (word == "allOf") {
      readAllOf(in, parents);
    } else if (word == "any") {
      m_schema = Schema::AnySchema{};
    } else if (word == "anyOf") {
      readAnyOf(in, parents);
    } else if (word == "bool") {
      m_schema = Schema::BoolSchema{};
    } else if (word == "double") {
      readDouble(in, parents);
    } else if (word == "enum") {
      readEnum(in, parents);
    } else if (word == "int") {
      readInt(in, parents);
    } else if (word == "not") {
      readNot(in, parents);
    } else if (word == "null") {
      m_schema = Schema::NullSchema{};
    } else if (word == "extensible") {
      read_non_space_or_throw(in, c);
      expect_char('{', c);
      readObject(in, parents);
      estd::get<ObjectSchema>(m_schema).is_extensible = true;
    } else if (word == "extended") {
      read_non_space_or_throw(in, c);
      expect_char('@', c);
      readReference(in, parents);
      estd::get<ReferenceSchema>(m_schema).is_extended = true;
    } else if (word == "oneOf") {
      readOneOf(in, parents);
    } else if (word == "str") {
      readString(in, parents);
    } else {
      if (!word.empty()) {
        throw SchemaParseException("unexpected word `" + word + "`");
      } else {
        throw SchemaParseException("unexpected character `" + std::string(1, c) + "`");
      }
    }
  }
}

std::istream& JSON::operator>>(std::istream& in, Schema& schema) {
  std::vector<Schema*> parents;
  schema.readSchema(in, parents);
  assert(parents.empty());
  return in;
}
std::ostream& JSON::operator<<(std::ostream& out, const Schema& schema) {
  if (schema.definitions) {
    std::vector<std::pair<const std::string, std::pair<int, Schema>>*> ptrs;
    for (auto& d : *schema.definitions) {
      ptrs.push_back(&d);
    }
    std::sort(ptrs.begin(), ptrs.end(), [](auto&& a, auto&& b) { return a->second.first < b->second.first; });
    for (auto& d : ptrs) {
      out << "#" << d->first << " " << d->second.second << "#";
    }
  }
  if (!schema.m_docstrings.empty()) {
    for (auto& d : schema.m_docstrings) {
      out << "/**" << d << "*/";
    }
  }
  visit([&out](auto&& x) { Schema::write_to(out, x); }, schema.m_schema);
  return out;
}

void JSON::Schema::write_to(std::ostream& out, const Schema::AllOfSchema& schema) {
  out << "allOf(";
  auto it = schema.items.begin();
  if (schema.items.size() > 0) {
    out << *it;
    ++it;
  }
  for (; it != schema.items.end(); ++it) {
    out << ", " << *it;
  }
  out << ")";
}
void JSON::Schema::write_to(std::ostream& out, const Schema::AnySchema& schema) { out << "any"; }
void JSON::Schema::write_to(std::ostream& out, const Schema::AnyOfSchema& schema) {
  auto it = schema.items.begin();
  out << "anyOf(";
  if (schema.items.size() > 0) {
    out << *it;
    ++it;
  }
  for (; it != schema.items.end(); ++it) {
    out << ", " << *it;
  }
  out << ")";
}
void JSON::Schema::write_to(std::ostream& out, const Schema::ArraySchema& schema) {
  out << "[";
  if (schema.unique) {
    out << "unique ";
  }
  out << *schema.items_schema << "]";
  if (schema.min || schema.max) {
    out << "{";
    if (schema.min) {
      out << schema.min.value();
    }
    if (schema.min != schema.max) {
      out << ", ";
      if (schema.max) {
        out << schema.max.value();
      }
    }
    out << "}";
  };
}
void JSON::Schema::write_to(std::ostream& out, const Schema::BoolSchema& schema) { out << "bool"; }
void JSON::Schema::write_to(std::ostream& out, const Schema::DoubleSchema& schema) {
  out << "double";
  if (schema.min || schema.max) {
    out << "(";
    if (schema.min) {
      out << schema.min.value();
    }
    out << " .. ";
    if (schema.max) {
      out << schema.max.value();
    }
    out << ")";
  };
}
void JSON::Schema::write_to(std::ostream& out, const Schema::EnumSchema& schema) {
  auto it = schema.enumeration.begin();
  out << "enum(";
  if (schema.enumeration.size() > 0) {
    out << *it;
    ++it;
  }
  for (; it != schema.enumeration.end(); ++it) {
    out << ", " << *it;
  }
  out << ")";
}
void JSON::Schema::write_to(std::ostream& out, const Schema::IntSchema& schema) {
  out << "int";
  if (schema.min || schema.max) {
    out << "(";
    if (schema.min) {
      out << schema.min.value();
    }
    out << " .. ";
    if (schema.max) {
      out << schema.max.value();
    }
    out << ")";
  };
}
void JSON::Schema::write_to(std::ostream& out, const Schema::ObjectSchema& schema) {
  if (schema.is_extensible) {
    out << "extensible ";
  }
  out << "{";
  {
    auto it = schema.properties.begin();
    if (it != schema.properties.end()) {
      if (std::get<2>(it->second)) {
        out << "?";
      }
      out << "\"" << it->first << "\":";
      out << std::get<0>(it->second);
      if (std::get<1>(it->second)) {
        out << "=" << std::get<1>(it->second).value();
      }
      ++it;
    }
    for (; it != schema.properties.end(); ++it) {
      out << ", ";
      if (std::get<2>(it->second)) {
        out << "?";
      }
      out << "\"" << it->first << "\":";
      out << std::get<0>(it->second);
      if (std::get<1>(it->second)) {
        out << "=" << std::get<1>(it->second).value();
      }
    }
  }
  {
    auto it = schema.pattern_properties.begin();
    if (schema.properties.empty() && it != schema.pattern_properties.end()) {
      out << "re\"" << it->first.first << "\":";
      out << it->second;
      ++it;
    }
    for (; it != schema.pattern_properties.end(); ++it) {
      out << ", ";
      out << ", re\"" << it->first.first << "\":";
      out << it->second;
    }
  }

  out << "}";
}
void JSON::Schema::write_to(std::ostream& out, const Schema::OneOfSchema& schema) {
  out << "oneOf(";
  auto it = schema.items.begin();
  if (schema.items.size() > 0) {
    out << *it;
    ++it;
  }
  for (; it != schema.items.end(); ++it) {
    out << ", " << *it;
  }
  out << ")";
}
void JSON::Schema::write_to(std::ostream& out, const Schema::NotSchema& schema) { out << "not(" << *schema.sub << ")"; }
void JSON::Schema::write_to(std::ostream& out, const Schema::NullSchema& schema) { out << "null"; }
void JSON::Schema::write_to(std::ostream& out, const Schema::ReferenceSchema& schema) {
  if (schema.is_extended) {
    out << "extended ";
  }
  out << "@" << schema.name;
}
void JSON::Schema::write_to(std::ostream& out, const Schema::StringSchema& schema) {
  out << "str";
  if (schema.pattern) {
    out << "(\"" << schema.pattern->first << "\")";
  }
  if (schema.min || schema.max) {
    out << "{";
    if (schema.min) {
      out << schema.min.value();
    }
    if (schema.min != schema.max) {
      out << ", ";
      if (schema.max) {
        out << schema.max.value();
      }
    }
    out << "}";
  };
}
void JSON::Schema::write_to(std::ostream& out, const Schema::TupleSchema& schema) {
  out << "(";
  auto it = schema.items.begin();
  if (schema.items.size() > 0) {
    out << *it;
    ++it;
  }
  for (; it != schema.items.end(); ++it) {
    out << ", " << *it;
  }
  out << ")";
}

std::ostream& JSON::operator<<(std::ostream& out, const SchemaMatchResult::MatchError& matchError) {
  pretty_print(out, matchError, 0);
  return out;
}
std::ostream& JSON::operator<<(std::ostream& out, const SchemaMatchResult& match) {
  if (match) {
    out << "match";
  } else {
    out << match.get_error();
  }
  return out;
}

Schema JSON::literals::operator""_schema(const char* input, size_t size) {
  Schema schema;
  std::istringstream(input) >> schema;
  return std::move(schema);
}
