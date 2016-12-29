#include <gtest/gtest.h>

#include "concise_json_schema/Schema.h"


using ::testing::Test;
using namespace JSON;


class SchemaTests : public Test{

};


TEST_F(SchemaTests, bad_inputs){
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


TEST_F(SchemaTests, good_inputs){

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

  EXPECT_NO_THROW( R"xxx({"a":int})xxx"_schema);
  EXPECT_NO_THROW( R"xxx({?"b":int})xxx"_schema);
  EXPECT_NO_THROW( R"xxx({?"b":int=4})xxx"_schema);

}

TEST_F(SchemaTests, comments){
  EXPECT_NO_THROW("/**/any"_schema);
  EXPECT_NO_THROW("/*comment*/any"_schema);
  EXPECT_NO_THROW("/*comment*/any"_schema);
  EXPECT_NO_THROW("/*comment*//*comment 2*/any"_schema);
  EXPECT_NO_THROW("/*comment*/ /*comment 2*//* comment 3*/any"_schema);
}
