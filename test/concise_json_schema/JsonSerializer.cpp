#include <gtest/gtest.h>
#include "concise_json_schema/JsonSerializerStd.h"

using ::testing::Test;
using namespace JSON;
using namespace std::string_literals;

struct JsonSerializerTests : public Test{
};

namespace {
template<typename T>
void json_round_trip(const T& value, const char* comment){
    Json json;
    T restored;
    ASSERT_NO_THROW(json = io::serialize(value)) << comment;
    ASSERT_NO_THROW(io::deserialize(json,restored)) << comment;
    ASSERT_EQ(value, restored) << comment;
}
}

#define JSON_ROUND_TRIP(arg)  json_round_trip(arg,#arg)

TEST_F(JsonSerializerTests, round_trip){
  JSON_ROUND_TRIP("hello world"s);
  JSON_ROUND_TRIP("hello"s);
  JSON_ROUND_TRIP(""s);
  JSON_ROUND_TRIP("  "s);
  JSON_ROUND_TRIP(1);
  JSON_ROUND_TRIP(2);
  JSON_ROUND_TRIP(-1);
  JSON_ROUND_TRIP(42);
  JSON_ROUND_TRIP(52l);
  JSON_ROUND_TRIP(62ll);
  JSON_ROUND_TRIP(1e300);
  JSON_ROUND_TRIP(-1e300);
  JSON_ROUND_TRIP(1e-300);
  JSON_ROUND_TRIP(-1e-300);
  JSON_ROUND_TRIP(0.0e0);
  {
    auto value =std::map<int,int>{{1,1},{2,3}} ;
    JSON_ROUND_TRIP( value );
  }
  {
    auto value =std::map<std::string,int>{{"one",1},{"three",3}} ;
    JSON_ROUND_TRIP( value );
  }
  {
    auto value =std::map<std::string,std::string>{{"one","two"},{"three","twor"}} ;
    JSON_ROUND_TRIP( value );
  }
  {
    auto value =std::set<int>{1,1,2,3} ;
    JSON_ROUND_TRIP( value );
  }
  {
    auto value =std::vector<int>{1,1,2,3} ;
    JSON_ROUND_TRIP( value );
  }
  {
    auto value =std::vector<std::string>(10,"str");
    JSON_ROUND_TRIP( value );
  }
  {
    auto value =std::deque<std::string>(10,"str");
    JSON_ROUND_TRIP( value );
  }
  {
    estd::variant<std::string,int> value;
    value = "string"s;
    JSON_ROUND_TRIP( value );
    value = 1;
    JSON_ROUND_TRIP( value );
  }
  {
    estd::optional<std::string> value;
    JSON_ROUND_TRIP( value );
    value = "string"s;
    JSON_ROUND_TRIP( value );
  }
  {
    std::vector<std::vector<std::vector<std::string>>> value;
    JSON_ROUND_TRIP( value );
  }
}


/*
 * map<string,T>       serialized as   Json object
 * map<string-like,T>  serialized as   Json object
 *    otherwise
 * map<U,T>            serialized as   Json array
 * */
TEST_F(JsonSerializerTests, map_string){
  {
    auto value = std::map<int,int>{{1,2},{3,4}};
    ASSERT_TRUE(serialize(value).is_array());
    JSON_ROUND_TRIP(value);
  }
  {
    auto value = std::map<std::string,int>{{"string 1",2},{"string 2",4}};
    ASSERT_TRUE(serialize(value).is_object());
    JSON_ROUND_TRIP(value);
  }
  {
    auto value = std::map<int,std::string>{{2,"string 1"},{4,"string 2"}};
    ASSERT_TRUE(serialize(value).is_array());
    JSON_ROUND_TRIP(value);
  }


  struct StringLike{
    StringLike(){}
    explicit StringLike(const std::string& ){}
    explicit operator std::string () const {
      return "string like"s;
    }
    bool operator < (const StringLike&) const {
      return false;
    }
    bool operator == (const StringLike&) const {
      return true;
    }
  };

  {
    auto value = std::map<StringLike,int>{{StringLike("string 1"),1},{StringLike("string 2"),3}};
    ASSERT_TRUE(serialize(value).is_object());
    JSON_ROUND_TRIP(value);
  }
}