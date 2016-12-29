#include <gtest/gtest.h>

#include "concise_json_schema/Json.h"
#include "concise_json_schema/JsonException.h"


using ::testing::Test;
using namespace JSON;


class JsonTests : public Test{

};


TEST_F(JsonTests, bad_inputs){
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
}

TEST_F(JsonTests, good_inputs){
  EXPECT_NO_THROW("0e0"_json);
  EXPECT_NO_THROW("0.0"_json);
  EXPECT_NO_THROW("0."_json);
  EXPECT_NO_THROW("0"_json);
  EXPECT_NO_THROW("1"_json);
  EXPECT_NO_THROW("1e9"_json);
  EXPECT_NO_THROW("1e-9"_json);
  EXPECT_NO_THROW("1e+9"_json);
  EXPECT_NO_THROW("-1e+9"_json);

  EXPECT_NO_THROW("null"_json);
  EXPECT_NO_THROW("[]"_json);
  EXPECT_NO_THROW("[[]]"_json);
  EXPECT_NO_THROW("[{},{}]"_json);
  EXPECT_NO_THROW("{\"a\":1,\"b\":2}"_json);
  EXPECT_NO_THROW("true"_json);
  EXPECT_NO_THROW("false"_json);
  EXPECT_NO_THROW("null"_json);
  EXPECT_NO_THROW("\"\""_json);
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
