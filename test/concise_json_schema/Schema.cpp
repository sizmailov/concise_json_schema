#include <gtest/gtest.h>

#include "concise_json_schema/Schema.h"

using ::testing::Test;
using namespace JSON;

class SchemaTests : public Test {

public:
  std::vector<std::tuple<Schema, Json, bool>> tests{
      {R"(any)"_schema, R"(null )"_json, true},
      {R"(any)"_schema, R"("str")"_json, true},
      {R"(any)"_schema, R"(1)"_json, true},
      {R"(allOf(str,any,str("he.*")))"_schema, R"("hello")"_json, true},
      {R"(allOf(int,any))"_schema, R"(1)"_json, true},
      {R"(int )"_schema, R"(1)"_json, true},
      {R"(int(1..10))"_schema, R"(1)"_json, true},
      {R"(int() )"_schema, R"(1)"_json, true},
      {R"(int(..) )"_schema, R"(1)"_json, true},
      {R"(anyOf(int,str,bool))"_schema, R"(true)"_json, true},
      {R"(anyOf(int,str,bool))"_schema, R"(3.14)"_json, false},
      {R"(bool)"_schema, R"(true)"_json, true},
      {R"(bool)"_schema, R"(false)"_json, true},
      {R"(enum(1,"2"))"_schema, R"(2)"_json, false},
      {R"(enum(1,"2"))"_schema, R"("2")"_json, true},
      {R"(not(int))"_schema, R"(5.5)"_json, true},
      {R"(null)"_schema, R"(null)"_json, true},
      {R"(double(1.5..10.0))"_schema, R"(1.5)"_json, true},
      {R"(oneOf(int,str,bool))"_schema, R"(true)"_json, true},
      {R"(oneOf(int,double))"_schema, R"(42)"_json, false},
      {R"({})"_schema, R"({})"_json, true},
      {R"({re"a":any,re"b":any,re"c":any})"_schema, R"({"a":1,"b":2,"c":3})"_json, true},
      {R"({ "x" : int})"_schema, R"({ })"_json, false},
      {R"({ "x" : int})"_schema, R"({ "x":2 })"_json, true},
      {R"({ ?"x" : int})"_schema, R"({ })"_json, true},
      {R"({ ?"x" : int})"_schema, R"({ "x":2})"_json, true},
      {R"({ ?"x" : int = 5})"_schema, R"({ })"_json, true},
      {R"({ ?"x" : int = 5})"_schema, R"({"x":2 })"_json, true},
      {R"({ re"dbl_.+" : double})"_schema, R"({"dbl_x": 2})"_json, true},
      {R"({ "x":str, re".*":double})"_schema, R"({"x": 2})"_json, false},
      {R"({ })"_schema, R"({"z":2 })"_json, false},
      {R"(str)"_schema, R"("foo")"_json, true},
      {R"(str{3})"_schema, R"("bar")"_json, true},
      {R"(str{,3})"_schema, R"("bar")"_json, true},
      {R"(str{3,})"_schema, R"("bar")"_json, true},
      {R"(str{3,10})"_schema, R"("foobar")"_json, true},
      {R"(str("[A-Z]+"))"_schema, R"("FOO")"_json, true},
      {R"(str("A"))"_schema, R"("AAA")"_json, false},
      {R"(str("A*"){,5})"_schema, R"("AAA")"_json, true},
      {R"([any])"_schema, R"([1,"s",{}])"_json, true},
      {R"([int]{1,5})"_schema, R"([1,"s",{}])"_json, false},
      {R"([ unique int]{,5})"_schema, R"([1,2,3])"_json, true},
      {R"([ unique int])"_schema, R"([1,2,3,4,1])"_json, false},
      {R"((int,int))"_schema, R"([1,2])"_json, true},
      {R"((int,int,str))"_schema, R"([1,2,"s"])"_json, true},
      {R"(not(null))"_schema, R"([])"_json, true},
      {R"(not(anyOf(bool,null)))"_schema, R"(12345)"_json, true},

  };
};

TEST_F(SchemaTests, bad_inputs)
{
  EXPECT_THROW(""_schema, SchemaParseException);
  EXPECT_THROW("["_schema, SchemaParseException);
  EXPECT_THROW("{"_schema, SchemaParseException);
  EXPECT_THROW("{{}"_schema, SchemaParseException);
  EXPECT_THROW("{{}}"_schema, SchemaParseException);
  EXPECT_THROW("\'"_schema, SchemaParseException);
  EXPECT_THROW("("_schema, SchemaParseException);
  EXPECT_THROW("("_schema, SchemaParseException);
  EXPECT_THROW(")"_schema, SchemaParseException);
  EXPECT_THROW("(()"_schema, SchemaParseException);
  EXPECT_THROW("-)"_schema, SchemaParseException);
  EXPECT_THROW("-("_schema, SchemaParseException);
  EXPECT_THROW("/"_schema, SchemaParseException);
  EXPECT_THROW("#"_schema, SchemaParseException);
  EXPECT_THROW("##"_schema, SchemaParseException);
  EXPECT_THROW("}"_schema, SchemaParseException);
  EXPECT_THROW("}{"_schema, SchemaParseException);
  EXPECT_THROW("]["_schema, SchemaParseException);
  EXPECT_THROW("{\"x\": int =4}"_schema, SchemaParseException);
}

TEST_F(SchemaTests, good_inputs)
{

  EXPECT_NO_THROW("any"_schema);

  EXPECT_NO_THROW("null"_schema);

  EXPECT_NO_THROW("bool"_schema);

  EXPECT_NO_THROW("int"_schema);
  EXPECT_NO_THROW("int()"_schema);
  EXPECT_NO_THROW("int(1..)"_schema);
  EXPECT_NO_THROW("int(1..2)"_schema);
  EXPECT_NO_THROW("int(..1)"_schema);
  EXPECT_NO_THROW("int(..)"_schema);

  EXPECT_NO_THROW("double()"_schema);
  EXPECT_NO_THROW("double(1..)"_schema);
  EXPECT_NO_THROW("double(1..2)"_schema);
  EXPECT_NO_THROW("double(..1)"_schema);
  EXPECT_NO_THROW("double(..)"_schema);

  EXPECT_NO_THROW("double(..)"_schema);

  EXPECT_NO_THROW("[any]"_schema);
  EXPECT_NO_THROW("[any]{}"_schema);
  EXPECT_NO_THROW("[any]{,}"_schema);
  EXPECT_NO_THROW("[any]{1,}"_schema);
  EXPECT_NO_THROW("[any]{1,2}"_schema);
  EXPECT_NO_THROW("[any]{,2}"_schema);
  EXPECT_NO_THROW("[[[[any]]]]{,2}"_schema);
  EXPECT_NO_THROW("[ unique [[[any]]]]{,2}"_schema);
  EXPECT_NO_THROW("[unique any]{,2}"_schema);

  EXPECT_NO_THROW(R"xxx({"a":int})xxx"_schema);
  EXPECT_NO_THROW(R"xxx({?"b":int})xxx"_schema);
  EXPECT_NO_THROW(R"xxx({?"b":int=4})xxx"_schema);

}

TEST_F(SchemaTests, comments)
{
  EXPECT_NO_THROW("/**/any"_schema);
  EXPECT_NO_THROW("/***/any"_schema);
  EXPECT_NO_THROW("/*****/any"_schema);
  EXPECT_NO_THROW("/*****//**/any"_schema);
  EXPECT_NO_THROW("/*comment*/any"_schema);
  EXPECT_NO_THROW("/*comment*/any"_schema);
  EXPECT_NO_THROW("/*comment*//*comment 2*/any"_schema);
  EXPECT_NO_THROW("/*comment*/ /*comment 2*//* comment 3*/any"_schema);
}

TEST_F(SchemaTests, schema_match)
{
  for (auto&&[schema, json, result]: tests) {
    EXPECT_EQ(schema.match(json), result) << schema << " << "<< json;
  }

  ASSERT_TRUE(R"({re".*":double})"_schema.match("{\"a\":3.14}"_json));
  ASSERT_FALSE(R"({re".*":int})"_schema.match("{\"a\":3.14}"_json));
  ASSERT_TRUE(R"(double)"_schema.match("1"_json));
}

TEST_F(SchemaTests, concise_to_json_schema){
  for (auto&&[schema, json, result]: tests) {
    schema.asJsonSchema();
  }
}

TEST_F(SchemaTests, round_trip){
  for (auto&&[schema, json, result]: tests) {
    auto json_schema = schema.asJsonSchema();
    {
      std::stringstream ss;
      ss << schema;
      Schema schema2;
      ss >> schema2;
      ASSERT_EQ(json_schema, schema2.asJsonSchema());
    }
    {
      std::stringstream ss;
      schema.pretty_print(ss);
      Schema schema2;
      ss >> schema2;
      ASSERT_EQ(json_schema, schema2.asJsonSchema());
    }
  }
}
