#pragma once
#include <iostream>
#include <regex>
#include <type_traits>
#include "Json.h"

#include <experimental/optional>
#include "variant"

namespace JSON {

namespace estd = std::experimental;

class Schema;
std::istream& operator>>(std::istream& in, Schema& schema);
std::ostream& operator<<(std::ostream& out, const Schema& schema);

class SchemaMatchResult {
 public:
  struct MatchSuccess {};

  struct MatchError : std::runtime_error {
    MatchError(const Json& json, const std::string& what);
    MatchError(const Json& json, const std::string& what, MatchError&& matchError);
    // precondition: corresponding Schema and Json MUST still exist (otherwise you'll get SEGFAULT)
    void pretty_wordy_print(std::ostream& out, int tab_size=2, int offset=0) const;
    const Json* json;
    const Schema* schema=nullptr;
    std::vector<MatchError> nested;
  };
  SchemaMatchResult();
  SchemaMatchResult(MatchError&& matchError);

  operator bool() const { return m_match.index() == 0; }

  const MatchError& get_error() const&;
  MatchError& get_error() &;
  MatchError get_error() &&;

 private:
  estd::variant<MatchSuccess, MatchError> m_match;
  friend class JsonSchema;
};

class Schema {
 public:
  class AllOfSchema {
  public:
    AllOfSchema(){};
    AllOfSchema(const std::vector<Schema>& items) : items(items){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::vector<Schema> items;
    friend class Schema;
  };
  class AnySchema {
  public:
    AnySchema(){}
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
    friend class Schema;
  };
  class AnyOfSchema {
  public:
    AnyOfSchema(){};
    AnyOfSchema(const std::vector<Schema>& items) : items(items){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::vector<Schema> items;
    friend class Schema;
  };
  class ArraySchema {
  public:
    ArraySchema(){};
    ArraySchema(const Schema& items_schema, bool unique=false) : items_schema(std::make_shared<Schema>(items_schema)),unique(unique){};
    ArraySchema(const Schema& items_schema, estd::optional<int> min, estd::optional<int> max, bool unique=false) : items_schema(std::make_shared<Schema>(items_schema)), min(min),max(max), unique(unique){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::shared_ptr<Schema> items_schema;
    estd::optional<int> min;
    estd::optional<int> max;
    bool unique = false;
    friend class Schema;
  };
  class BoolSchema {
  public:
    BoolSchema(){}
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
    friend class Schema;
  };
  class DoubleSchema {
  public:
    DoubleSchema(){};
    DoubleSchema(estd::optional<double> min, estd::optional<double> max) : min(min),max(max){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    estd::optional<double> min;
    estd::optional<double> max;
    friend class Schema;
  };
  class EnumSchema {
  public:
    EnumSchema() {};
    EnumSchema(const std::vector<Json>& enumeration) : enumeration(enumeration){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::vector<Json> enumeration;
    friend class Schema;
  };
  class IntSchema {
  public:
    IntSchema(){};
    IntSchema(estd::optional<int> min, estd::optional<int> max) : min(min),max(max){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    estd::optional<int> min;
    estd::optional<int> max;
    friend class Schema;
  };
  class NotSchema {
  public:
    NotSchema(){};
    NotSchema(const Schema& items_schema) : sub(std::make_shared<Schema>(items_schema)){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::shared_ptr<Schema> sub;
    friend class Schema;
  };
  class NullSchema {
  public:
    NullSchema(){}
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
    friend class Schema;
  };
  class OneOfSchema {
  public:
    OneOfSchema(){};
    OneOfSchema(const std::vector<Schema>& items) : items(items){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::vector<Schema> items;
    friend class Schema;
  };
  class ObjectSchema {
  public:
    using Property =
    std::tuple<Schema /*schema*/, estd::optional<Json> /*default*/, bool /*is_optional*/>;

    ObjectSchema(){};
    ObjectSchema(const std::map<std::string, Property>& properties, bool is_extensible=false) : properties(properties),is_extensible(is_extensible){};
    ObjectSchema(const std::map<std::string, Property>& properties, std::vector<std::pair<std::string,Schema>>& pattern_properties, bool is_extensible=false) : properties(properties),is_extensible(is_extensible){
      for (auto&x: pattern_properties){
        this->pattern_properties.emplace_back(std::make_pair(std::make_pair(x.first,std::regex(x.first)),x.second));
      }
    };
    SchemaMatchResult match(const Json& json, bool allow_extensions=false) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
    static const int i_scheme=0;
    static const int i_default=1;
    static const int i_optional=2;
    void add_property(std::string key, Property p) { properties.emplace(std::move(key), std::move(p)); }
    void add_pattern_property(std::string pattern, Schema s) {
      pattern_properties.emplace_back(std::make_pair(pattern, std::regex(pattern)), std::move(s));
    }

   private:

    std::map<std::string /*key*/, Property> properties;
    std::vector<std::pair<std::pair<std::string, std::regex>, Schema>> pattern_properties;
    bool is_extensible=false;
    friend class Schema;
  };
  class ReferenceSchema {
  public:
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::string name;
    const Schema* ref;
    bool is_extended = false;
    friend class Schema;
  };
  class StringSchema {
  public:
    StringSchema(){};
    StringSchema(const std::string& pattern) : StringSchema(pattern,{},{}) {};
    StringSchema(const std::string& pattern, estd::optional<int> min, estd::optional<int> max) : pattern(std::make_shared<std::pair<std::string, std::regex>>(pattern,std::regex(pattern))), min(min), max(max){};
    StringSchema(estd::optional<int> min, estd::optional<int> max) : min(min),max(max){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::shared_ptr<std::pair<std::string, std::regex>> pattern;
    estd::optional<int> min;
    estd::optional<int> max;
    friend class Schema;
  };
  class TupleSchema {
  public:
    TupleSchema(){};
    TupleSchema(const std::vector<Schema>& items) : items(items){};
    SchemaMatchResult match(const Json& json) const;
    Json asJsonSchema() const;
    void pretty_print(std::ostream& out, int tab_size, int nest, bool first_line_offset) const;
  private:
    std::vector<Schema> items;
    friend class Schema;
  };

  SchemaMatchResult match(const Json& json) const;
  Json asJsonSchema() const;

  const std::vector<std::string>& doc_strings() const { return m_docstrings; }

  Schema() : m_schema(AnySchema{}){};
  Schema(const Schema& other) = default;
  Schema(Schema&& other) = default;
  Schema& operator=(const Schema& other) = default;
  Schema& operator=(Schema&& other) = default;

  void pretty_print(std::ostream& out, int tab_size=2, int offset=0, bool first_line_offset=true) const;

  template<typename T, typename = std::enable_if_t<
      std::is_same<std::decay_t<T>,AllOfSchema>::value ||
          std::is_same<std::decay_t<T>,AnySchema>::value ||
          std::is_same<std::decay_t<T>,AnyOfSchema>::value ||
          std::is_same<std::decay_t<T>,ArraySchema>::value ||
          std::is_same<std::decay_t<T>,BoolSchema>::value ||
          std::is_same<std::decay_t<T>,DoubleSchema>::value ||
          std::is_same<std::decay_t<T>,EnumSchema>::value ||
          std::is_same<std::decay_t<T>,IntSchema>::value ||
          std::is_same<std::decay_t<T>,NotSchema>::value ||
          std::is_same<std::decay_t<T>,NullSchema>::value ||
          std::is_same<std::decay_t<T>,OneOfSchema>::value ||
          std::is_same<std::decay_t<T>,ObjectSchema>::value ||
          std::is_same<std::decay_t<T>,ReferenceSchema>::value ||
          std::is_same<std::decay_t<T>,StringSchema>::value ||
          std::is_same<std::decay_t<T>,TupleSchema>::value
      >
  >
  Schema (T&& other) : m_schema(std::forward<T>(other)){
  };

  template<typename T, typename = std::enable_if_t<
      std::is_same<std::decay_t<T>,AllOfSchema>::value ||
      std::is_same<std::decay_t<T>,AnySchema>::value ||
      std::is_same<std::decay_t<T>,AnyOfSchema>::value ||
      std::is_same<std::decay_t<T>,ArraySchema>::value ||
      std::is_same<std::decay_t<T>,BoolSchema>::value ||
      std::is_same<std::decay_t<T>,DoubleSchema>::value ||
      std::is_same<std::decay_t<T>,EnumSchema>::value ||
      std::is_same<std::decay_t<T>,IntSchema>::value ||
      std::is_same<std::decay_t<T>,NotSchema>::value ||
      std::is_same<std::decay_t<T>,NullSchema>::value ||
      std::is_same<std::decay_t<T>,OneOfSchema>::value ||
      std::is_same<std::decay_t<T>,ObjectSchema>::value ||
      std::is_same<std::decay_t<T>,ReferenceSchema>::value ||
      std::is_same<std::decay_t<T>,StringSchema>::value ||
      std::is_same<std::decay_t<T>,TupleSchema>::value
      >
  >
  Schema& operator=(T&& other){
    m_docstrings.clear();
    definitions.reset();
    m_schema = std::forward<T>(other);
    return *this;
  };

  const ObjectSchema& get_object() const { return estd::get<ObjectSchema>(m_schema); }
  ObjectSchema& get_object() { return estd::get<ObjectSchema>(m_schema); }

  Schema& prepend_docstring(std::string s) {
    m_docstrings.push_back(std::move(s));
    return *this;
  }

 private:
  estd::variant<
          AllOfSchema,
          AnySchema,
          AnyOfSchema,
          ArraySchema,
          BoolSchema,
          DoubleSchema,
          EnumSchema,
          IntSchema,
          NotSchema,
          NullSchema,
          OneOfSchema,
          ObjectSchema,
          ReferenceSchema,
          StringSchema,
          TupleSchema
  >
      m_schema;

  std::vector<std::string> m_docstrings;
  std::shared_ptr<std::map<std::string,std::pair<int,Schema>>> definitions;

  static const Schema* resolveReference(const std::string& name, const std::vector<Schema*>& parents);

  void readAllOf(std::istream& in, std::vector<Schema*>& parents);
  void readCSV(std::istream& in, std::vector<Schema>& result, std::vector<Schema*>& parents);
  void readAnyOf(std::istream& in, std::vector<Schema*>& parents);
  void readArray(std::istream& in, std::vector<Schema*>& parents);
  void readDouble(std::istream& in, std::vector<Schema*>& parents);
  void readEnum(std::istream& in, std::vector<Schema*>& parents);
  void readInt(std::istream& in, std::vector<Schema*>& parents);
  void readNot(std::istream& in, std::vector<Schema*>& parents);
  void readOneOf(std::istream& in, std::vector<Schema*>& parents);
  void readObject(std::istream& in, std::vector<Schema*>& parents);
  void readReference(std::istream& in, std::vector<Schema*>& parents);
  void readString(std::istream& in, std::vector<Schema*>& parents);
  void readDefinition(std::istream& in, std::vector<Schema*>& parents);
  void readSchema(std::istream& in, std::vector<Schema*>& parents);
  void readTuple(std::istream& in, std::vector<Schema*>& parents);


  static void write_to(std::ostream& out, const Schema::AllOfSchema& schema);
  static void write_to(std::ostream& out, const Schema::AnySchema& schema);
  static void write_to(std::ostream& out, const Schema::AnyOfSchema& schema);
  static void write_to(std::ostream& out, const Schema::ArraySchema& schema);
  static void write_to(std::ostream& out, const Schema::BoolSchema& schema);
  static void write_to(std::ostream& out, const Schema::EnumSchema& schema);
  static void write_to(std::ostream& out, const Schema::IntSchema& schema);
  static void write_to(std::ostream& out, const Schema::NotSchema& schema);
  static void write_to(std::ostream& out, const Schema::NullSchema& schema);
  static void write_to(std::ostream& out, const Schema::DoubleSchema& schema);
  static void write_to(std::ostream& out, const Schema::ObjectSchema& schema);
  static void write_to(std::ostream& out, const Schema::OneOfSchema& schema);
  static void write_to(std::ostream& out, const Schema::ReferenceSchema& schema);
  static void write_to(std::ostream& out, const Schema::StringSchema& schema);
  static void write_to(std::ostream& out, const Schema::TupleSchema& schema);


  friend std::istream& operator>>(std::istream& in, Schema& schema);
  friend std::ostream& operator<<(std::ostream& out, const Schema& schema);
};

std::ostream& operator<<(std::ostream& out, const SchemaMatchResult::MatchError& error);
std::ostream& operator<<(std::ostream& out, const SchemaMatchResult& match);

class SchemaParseException : public std::runtime_error{
public:
  explicit SchemaParseException(const std::string& what ) : runtime_error(what){};
  explicit SchemaParseException(const char* what ) : runtime_error(what){};
};

inline namespace literals{

Schema operator "" _schema(const char* , std::size_t );

}
}
