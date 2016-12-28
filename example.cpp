#include <iostream>
#include "concise_json_schema/Schema.h"

using namespace JSON;

int main()
{
  Schema schema;
  Json json;

  std::cin >> schema >> json;

  auto match = schema.match(json);
  std::cout << match << std::endl;
  if (!match){
    match.get_error().pretty_wordy_print(std::cout);
  }


  return 0;
}
