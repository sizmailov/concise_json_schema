#include <gtest/gtest.h>

#include "concise_json_schema/Json.h"
#include "concise_json_schema/JsonException.h"

using ::testing::Test;
using namespace JSON;

class JsonTests : public Test {

};

TEST_F(JsonTests, bad_inputs)
{
  EXPECT_THROW(""_json, JsonException);
  EXPECT_THROW("["_json, JsonException);
  EXPECT_THROW("{"_json, JsonException);
  EXPECT_THROW("{{}}"_json, JsonException);
  EXPECT_THROW("\'"_json, JsonException);
  EXPECT_THROW("("_json, JsonException);
  EXPECT_THROW("()"_json, JsonException);
  EXPECT_THROW(")"_json, JsonException);
  EXPECT_THROW("}"_json, JsonException);
  EXPECT_THROW("}{"_json, JsonException);
  EXPECT_THROW("]["_json, JsonException);
  EXPECT_THROW("False"_json, JsonException);
  EXPECT_THROW("fals"_json, JsonException);
  EXPECT_THROW("tru"_json, JsonException);
  EXPECT_THROW("True"_json, JsonException);
  EXPECT_THROW("TRUE"_json, JsonException);
  EXPECT_THROW("FALSE"_json, JsonException);
  EXPECT_THROW("Nill"_json, JsonException);
  EXPECT_THROW("nill"_json, JsonException);
  EXPECT_THROW("Null"_json, JsonException);
  EXPECT_THROW("NULL"_json, JsonException);

  EXPECT_THROW(".."_json, JsonException);
  EXPECT_THROW(".5."_json, JsonException);
  EXPECT_THROW("--5"_json, JsonException);
  EXPECT_THROW("+5"_json, JsonException);
  EXPECT_THROW("01"_json, JsonException);
  EXPECT_THROW("05"_json, JsonException);
  EXPECT_THROW("-01"_json, JsonException);
  EXPECT_THROW("-02"_json, JsonException);
  EXPECT_THROW("[0A]"_json, JsonException);
  EXPECT_THROW("1ef"_json, JsonException);
  EXPECT_THROW("{1e05}"_json, JsonException);
  EXPECT_THROW("[1e15f]"_json, JsonException);

  EXPECT_THROW("\"abra"_json, JsonException);
  EXPECT_THROW("\"ab\\\"ra"_json, JsonException);

}

TEST_F(JsonTests, good_inputs)
{
  EXPECT_NO_THROW("0e0"_json);
  EXPECT_NO_THROW("0.0"_json);
  EXPECT_NO_THROW("0."_json);
  EXPECT_NO_THROW("0"_json);
  EXPECT_NO_THROW("1"_json);
  EXPECT_NO_THROW("1e9"_json);
  EXPECT_NO_THROW("1e-9"_json);
  EXPECT_NO_THROW("1e+9"_json);
  EXPECT_NO_THROW("-1e+9"_json);
  EXPECT_NO_THROW("-1e+15"_json);
  EXPECT_NO_THROW("-0.2"_json);
  EXPECT_NO_THROW("0.00e0001"_json);
  EXPECT_NO_THROW("\"abra\""_json);

  EXPECT_NO_THROW("null"_json);
  EXPECT_NO_THROW("[]"_json);
  EXPECT_NO_THROW("[[]]"_json);
  EXPECT_NO_THROW("[{},{}]"_json);
  EXPECT_NO_THROW("{\"a\":1,\"b\":2}"_json);
  EXPECT_NO_THROW("true"_json);
  EXPECT_NO_THROW("false"_json);
  EXPECT_NO_THROW("null"_json);
  EXPECT_NO_THROW("\"\""_json);
  EXPECT_NO_THROW(R"({"\"":null})"_json);
  EXPECT_NO_THROW("\"str\""_json);
  EXPECT_NO_THROW("\"str with space\""_json);
  EXPECT_NO_THROW("\"   str begins with space\""_json);
  EXPECT_NO_THROW("/*comment*/2"_json);
  EXPECT_NO_THROW("/*comment*/[]"_json);
  EXPECT_NO_THROW("/*comment*/[1,2,3]"_json);
  EXPECT_NO_THROW("/*comment*/[1,/*comment2*/2,3]"_json);
  EXPECT_NO_THROW("/*comment0*/ /*comment1*/[1,/*comment2*/2,3]"_json);
  EXPECT_NO_THROW("/*comment0*//*consequent comment1*/[1,/*comment2*/2/*comment3*/,3]"_json);
}

TEST_F(JsonTests, wrong_get_throws)
{
  {
    auto value = true;
    Json j_bool(value);
    ASSERT_TRUE(j_bool.is_bool());
    EXPECT_THROW(j_bool.get_array(), JsonGetException);
    EXPECT_THROW(j_bool.get_number(), JsonGetException);
    EXPECT_THROW(j_bool.get_integer(), JsonGetException);
    EXPECT_THROW(j_bool.get_double(), JsonGetException);
    EXPECT_THROW(j_bool.get_object(), JsonGetException);
    EXPECT_THROW(j_bool.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_bool.get_bool());
    ASSERT_TRUE(j_bool.get_bool()==value);
  }
  {
    auto value = 15;
    Json j_int(value);
    ASSERT_TRUE(j_int.is_integer());
    ASSERT_TRUE(j_int.is_number());
    EXPECT_THROW(j_int.get_array(), JsonGetException);
    EXPECT_THROW(j_int.get_bool(), JsonGetException);
    EXPECT_THROW(j_int.get_double(), JsonGetException);
    EXPECT_THROW(j_int.get_object(), JsonGetException);
    EXPECT_THROW(j_int.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_int.get_integer());
    EXPECT_NO_THROW(j_int.get_number());
    ASSERT_TRUE(j_int.get_integer()==value);
  }

  {
    auto value = 15.0;
    Json j_double(value);
    ASSERT_TRUE(j_double.is_double());
    ASSERT_TRUE(j_double.is_number());
    EXPECT_THROW(j_double.get_array(), JsonGetException);
    EXPECT_THROW(j_double.get_bool(), JsonGetException);
    EXPECT_THROW(j_double.get_integer(), JsonGetException);
    EXPECT_THROW(j_double.get_object(), JsonGetException);
    EXPECT_THROW(j_double.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_double.get_number());
    EXPECT_NO_THROW(j_double.get_double());
    ASSERT_TRUE(j_double.get_double()==value);
  }

  {
    std::vector<Json> value = {
        Json(1),
        Json(Json::Nil{}),
        Json("string")
    };
    Json j_array(value);
    ASSERT_TRUE(j_array.is_array());
    EXPECT_THROW(j_array.get_bool(), JsonGetException);
    EXPECT_THROW(j_array.get_integer(), JsonGetException);
    EXPECT_THROW(j_array.get_double(), JsonGetException);
    EXPECT_THROW(j_array.get_number(), JsonGetException);
    EXPECT_THROW(j_array.get_object(), JsonGetException);
    EXPECT_THROW(j_array.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_array.get_array());
    ASSERT_TRUE(j_array.get_array()==value);
  }

  {
    Json::Object value{
        {"a", Json(1)},
        {"b", {}},
        {"c", Json(42)}
    };
    Json j_object(value);
    ASSERT_TRUE(j_object.is_object());
    EXPECT_THROW(j_object.get_bool(), JsonGetException);
    EXPECT_THROW(j_object.get_integer(), JsonGetException);
    EXPECT_THROW(j_object.get_double(), JsonGetException);
    EXPECT_THROW(j_object.get_number(), JsonGetException);
    EXPECT_THROW(j_object.get_array(), JsonGetException);
    EXPECT_THROW(j_object.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_object.get_object());
    ASSERT_TRUE(j_object.get_object()==value);
  }

  {
    Json j_null;
    ASSERT_TRUE(j_null.is_null());
    EXPECT_THROW(j_null.get_bool(), JsonGetException);
    EXPECT_THROW(j_null.get_integer(), JsonGetException);
    EXPECT_THROW(j_null.get_double(), JsonGetException);
    EXPECT_THROW(j_null.get_number(), JsonGetException);
    EXPECT_THROW(j_null.get_array(), JsonGetException);
    EXPECT_THROW(j_null.get_string(), JsonGetException);
    EXPECT_THROW(j_null.get_object(), JsonGetException);
    ASSERT_TRUE(j_null==Json::Nil{});
  }
  {
    Json::String value{"Hello world!"};
    Json j_object(value);
    ASSERT_TRUE(j_object.is_string());
    EXPECT_THROW(j_object.get_bool(), JsonGetException);
    EXPECT_THROW(j_object.get_integer(), JsonGetException);
    EXPECT_THROW(j_object.get_double(), JsonGetException);
    EXPECT_THROW(j_object.get_number(), JsonGetException);
    EXPECT_THROW(j_object.get_array(), JsonGetException);
    EXPECT_NO_THROW(j_object.get_string());
    ASSERT_TRUE(j_object.get_string()==value);
  }

}

TEST_F(JsonTests, wrong_const_get_throws)
{
  {
    auto value = true;
    const Json j_bool(value);
    ASSERT_TRUE(j_bool.is_bool());
    EXPECT_THROW(j_bool.get_array(), JsonGetException);
    EXPECT_THROW(j_bool.get_number(), JsonGetException);
    EXPECT_THROW(j_bool.get_integer(), JsonGetException);
    EXPECT_THROW(j_bool.get_double(), JsonGetException);
    EXPECT_THROW(j_bool.get_object(), JsonGetException);
    EXPECT_THROW(j_bool.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_bool.get_bool());
    ASSERT_TRUE(j_bool.get_bool()==value);
  }
  {
    auto value = 15;
    const Json j_int(value);
    ASSERT_TRUE(j_int.is_integer());
    ASSERT_TRUE(j_int.is_number());
    EXPECT_THROW(j_int.get_array(), JsonGetException);
    EXPECT_THROW(j_int.get_bool(), JsonGetException);
    EXPECT_THROW(j_int.get_double(), JsonGetException);
    EXPECT_THROW(j_int.get_object(), JsonGetException);
    EXPECT_THROW(j_int.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_int.get_integer());
    EXPECT_NO_THROW(j_int.get_number());
    ASSERT_TRUE(j_int.get_integer()==value);
  }

  {
    auto value = 15.0;
    const Json j_double(value);
    ASSERT_TRUE(j_double.is_double());
    ASSERT_TRUE(j_double.is_number());
    EXPECT_THROW(j_double.get_array(), JsonGetException);
    EXPECT_THROW(j_double.get_bool(), JsonGetException);
    EXPECT_THROW(j_double.get_integer(), JsonGetException);
    EXPECT_THROW(j_double.get_object(), JsonGetException);
    EXPECT_THROW(j_double.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_double.get_number());
    EXPECT_NO_THROW(j_double.get_double());
    ASSERT_TRUE(j_double.get_double()==value);
  }

  {
    std::vector<Json> value = {
        Json(1),
        Json(Json::Nil{}),
        Json("string")
    };
    const Json j_array(value);
    ASSERT_TRUE(j_array.is_array());
    EXPECT_THROW(j_array.get_bool(), JsonGetException);
    EXPECT_THROW(j_array.get_integer(), JsonGetException);
    EXPECT_THROW(j_array.get_double(), JsonGetException);
    EXPECT_THROW(j_array.get_number(), JsonGetException);
    EXPECT_THROW(j_array.get_object(), JsonGetException);
    EXPECT_THROW(j_array.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_array.get_array());
    ASSERT_TRUE(j_array.get_array()==value);
  }

  {
    Json::Object value{
        {"a", Json(1)},
        {"b", {}},
        {"c", Json(42)}
    };
    const Json j_object(value);
    ASSERT_TRUE(j_object.is_object());
    EXPECT_THROW(j_object.get_bool(), JsonGetException);
    EXPECT_THROW(j_object.get_integer(), JsonGetException);
    EXPECT_THROW(j_object.get_double(), JsonGetException);
    EXPECT_THROW(j_object.get_number(), JsonGetException);
    EXPECT_THROW(j_object.get_array(), JsonGetException);
    EXPECT_THROW(j_object.get_string(), JsonGetException);
    EXPECT_NO_THROW(j_object.get_object());
    ASSERT_TRUE(j_object.get_object()==value);
  }

  {
    const Json j_null;
    ASSERT_TRUE(j_null.is_null());
    EXPECT_THROW(j_null.get_bool(), JsonGetException);
    EXPECT_THROW(j_null.get_integer(), JsonGetException);
    EXPECT_THROW(j_null.get_double(), JsonGetException);
    EXPECT_THROW(j_null.get_number(), JsonGetException);
    EXPECT_THROW(j_null.get_array(), JsonGetException);
    EXPECT_THROW(j_null.get_string(), JsonGetException);
    EXPECT_THROW(j_null.get_object(), JsonGetException);
    ASSERT_TRUE(j_null==Json::Nil{});
  }
  {
    Json::String value{"Hello world!"};
    const Json j_object(value);
    ASSERT_TRUE(j_object.is_string());
    EXPECT_THROW(j_object.get_bool(), JsonGetException);
    EXPECT_THROW(j_object.get_integer(), JsonGetException);
    EXPECT_THROW(j_object.get_double(), JsonGetException);
    EXPECT_THROW(j_object.get_number(), JsonGetException);
    EXPECT_THROW(j_object.get_array(), JsonGetException);
    EXPECT_NO_THROW(j_object.get_string());
    ASSERT_TRUE(j_object.get_string()==value);
  }

}

TEST_F(JsonTests, read_object_with_comments)
{
  Json::Array array{
      R"({})"_json,
      R"(/*{}}*/{})"_json,
      R"(/**/{})"_json,
      R"({/*][}*/})"_json,
      R"({/**/})"_json,
      R"(/*{{*/
{})"_json,
      R"({
/*}}*/
})"_json,
  };
  for (auto& json: array) {
    ASSERT_TRUE(json.is_object());
    ASSERT_TRUE(json.size()==0);
  }

}

TEST_F(JsonTests, write_read_json)
{
  Json::Array array{
      R"({})"_json,
      R"([true,false,null])"_json,
      R"([[true,false],null])"_json,
      R"([[true,null],5, 12, 4.3, 1e99])"_json,
      R"([[null,true],{"a":"b","c":"d","e":"f"}])"_json,
      R"([[null,true],{"a":{"b":["x",1,null,false]}}])"_json,
  };
  for (auto& json: array) {
    {
      std::stringstream ss;
      ss << json;
      Json json_2;
      ss >> json_2;
      ASSERT_EQ(json, json_2);
    }
    {
      std::stringstream ss;
      json.pretty_print(ss, 4);
      Json json_2;
      ss >> json_2;
      ASSERT_EQ(json, json_2);
    }
  }

}
