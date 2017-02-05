#pragma once

#include "Json.h"

#include <deque>
#include <map>
#include <set>
#include <type_traits>
#include <cassert>
#include <vector>
#include "variant"
#include <experimental/optional>
#include "JsonException.h"

namespace JSON{
namespace io{
namespace detail {

template <size_t dummy, size_t i, size_t N, typename... Args>
struct assignTupleExpanderHelper;

template <size_t i, size_t N, typename... Args>
struct assignTupleExpanderHelper<0, i, N, Args...> {
  static void expand_json_to_tuple(const Json& json, std::tuple<Args...>& t) {
    JSON::io::deserialize(json[i], std::get<i>(t));
    assignTupleExpanderHelper<0, i + 1, N, Args...>::expand_json_to_tuple(json, t);
  }
  static void assign_tuple_to_json(Json& json, const std::tuple<Args...>& t) {
    json[i] = JSON::io::serialize(std::get<i>(t));
    assignTupleExpanderHelper<0, i + 1, N, Args...>::assign_tuple_to_json(json, t);
  }
};

template <size_t i, typename... Args>
struct assignTupleExpanderHelper<0, i, i, Args...> {
  static void expand_json_to_tuple(const Json&, std::tuple<Args...>&) {}
  static void assign_tuple_to_json(Json&, const std::tuple<Args...>&) {}
};

}

template <typename... Args>
struct JsonSerializer<std::tuple<Args...>> {
  static const size_t size = std::tuple_size<std::tuple<Args...>>::value;

  static void deserialize(const Json& json, std::tuple<Args...>& t) {
    if (json.size() != size) {
      throw std::runtime_error("Json array size != " + std::to_string(size));
    }
    detail::assignTupleExpanderHelper<0, 0, size, Args...>::expand_json_to_tuple(json, t);
  }
  static Json serialize(const std::tuple<Args...>& t) {
    Json json(Json::Array{});
    json.get_array().resize(size);
    detail::assignTupleExpanderHelper<0, 0, size, Args...>::assign_tuple_to_json(json, t);
    return json;
  }
};


template <typename T1, typename T2>
struct JsonSerializer<std::pair<T1, T2>> {
  static void deserialize(const Json& json, std::pair<T1, T2>& v) {
//    Expects(json.is_array()); // todo reenable
//    Expects(json.size() == 2); // todo reenable
    JSON::deserialize(json[0], v.first);
    JSON::deserialize(json[1], v.second);
  }
  static Json serialize(const std::pair<T1, T2>& v) {
    Json json(Json::Array{});
    json.push_back(JSON::serialize(v.first));
    json.push_back(JSON::serialize(v.second));
    return json;
  }
};

template <typename T>
struct JsonSerializer<std::vector<T>> {
  static void deserialize(const Json& json, std::vector<T>& v) {
    v.clear();
    for (auto& x : json) {
      T t;
      JSON::deserialize(x, t);
      v.push_back(std::move(t));
    }
  }
  static Json serialize(const std::vector<T>& v) {
    Json json(Json::Array{});
    for (auto& x : v) {
      json.push_back(JSON::serialize(x));
    }
    return json;
  }
};

template <typename T, size_t N>
struct JsonSerializer<std::array<T,N>> {
  static void deserialize(const Json& json, std::array<T,N>& v) {
    for (size_t i=0;i<N;i++) {
      JSON::deserialize(json[i], v[i]);
    }
  }
  static Json serialize(const std::array<T,N>& v) {
    Json json(Json::Array{});
    for (auto& x : v) {
      json.push_back(JSON::serialize(x));
    }
    return json;
  }
};

template <typename T>
struct JsonSerializer<std::deque<T>> {
  static void deserialize(const Json& json, std::deque<T>& v) {
    v.clear();
    for (auto& x : json) {
      T t;
      JSON::deserialize(x, t);
      v.push_back(std::move(t));
    }
  }
  static Json serialize(const std::deque<T>& v) {
    Json json(Json::Array{});
    for (auto& x : v) {
      json.push_back(JSON::serialize(x));
    }
    return json;
  }
};


template <typename T>
struct JsonSerializer<std::set<T>> {
  static void deserialize(const Json& json, std::set<T>& v) {
    v.clear();
    for (auto& x : json) {
      T t;
      JSON::deserialize(x, t);
      v.insert(std::move(t));
    }
  }
  static Json serialize(const std::set<T>& v) {
    Json json(Json::Array{});
    for (auto& x : v) {
      json.push_back(JSON::serialize(x));
    }
    return json;
  }
};


template <typename T>
struct JsonSerializer<std::reference_wrapper<T>> {
  static void deserialize(const Json& json, std::reference_wrapper<T>& v) {
    JSON::deserialize(json, v.get());
  }
  static Json serialize(const std::reference_wrapper<T>& v) {
    return JSON::serialize(v.get());
    ;
  }
};



template <typename T>
struct JsonSerializer<estd::optional<T>>{
  static void deserialize(const Json& json, estd::optional<T>& v) {
    if (json.is_null()) {
      v = estd::optional<T>{};
    } else {
      T t;
      v = std::move(t);
      assert(v);
      JSON::deserialize(json, v.value());
    }
  }
  static Json serialize(const estd::optional<T>& v) {
    if (v) {
      return JSON::serialize(v.value());
    }
    return Json();
  }
};

namespace detail {

template<size_t dummy, size_t i, size_t N, typename... Args>
struct assignVariantExpanderHelper;

template<size_t i, size_t N, typename... Args>
struct assignVariantExpanderHelper<0, i, N, Args...> {
  static void expand_json_to_variant(const Json& json, estd::variant<Args...>& t)
  {
    try {
      using U = std::decay_t<decltype(estd::get<i>(t))>;
      U value;
      JSON::deserialize(json, value);
      t = std::move(value);
    }
    catch (JsonException& e) {
      assignVariantExpanderHelper<0, i+1, N, Args...>::expand_json_to_variant(json, t);
    }
  }
};

template<size_t i, typename... Args>
struct assignVariantExpanderHelper<0, i, i, Args...> {
  static void expand_json_to_variant(const Json&, estd::variant<Args...>&)
  {
    throw std::runtime_error("No variant matched");
  }
};
}

template <typename... Args>
struct JsonSerializer<estd::variant<Args...>> {
  static const size_t size = estd::variant_size<estd::variant<Args...>>::value;

  static void deserialize(const Json& json, estd::variant<Args...>& t) {
    detail::assignVariantExpanderHelper<0, 0, size, Args...>::expand_json_to_variant(json, t);
  }
  static Json serialize(const estd::variant<Args...>& t) {
    Json json;
    estd::visit([&json](auto&& arg) { json = JSON::serialize(arg); }, t);
    return json;
  }
};


namespace detail{


template<bool is_string_like, typename Key, typename Value>
struct JsonSerializerHelper;

template<typename Key, typename Value>
struct JsonSerializerHelper<false, Key, Value> {

  static void deserialize(const Json& json, std::map<Key, Value>& v) {
    v.clear();
    for (auto& x : json.get_array()) {
      std::pair<Key, Value> t;
      JSON::deserialize(x, t);
      v.insert(t);
    }
  }

  static Json serialize(const std::map<Key, Value>& v) {
    Json json(Json::Array{});
    for (auto& x : v) {
      Json pair(Json::Array{});
      pair.push_back(JSON::serialize(x.first));
      pair.push_back(JSON::serialize(x.second));
      json.push_back(pair);
    }
    return json;
  }
};

template<typename Key, typename Value>
struct JsonSerializerHelper<true, Key, Value> {

  static void deserialize(const Json& json, std::map<Key, Value>& v) {
    v.clear();
    for (auto& x : json.get_object()) {
      std::pair<Key, Value> t;
      t.first = (Key)x.first;
      JSON::deserialize(x.second, t.second);
      v.insert(t);
    }
  }

  static Json serialize(const std::map<Key, Value>& v) {
    Json json(Json::Object{});
    for (auto& x : v) {
      json.insert((std::string)x.first, JSON::serialize(x.second));
    }
    return json;
  }
};
}

template<typename Key, typename Value>
struct JsonSerializer<std::map<Key, Value>> {

  static void deserialize(const Json& json, std::map<Key, Value>& v) {
    detail::JsonSerializerHelper<
        std::is_constructible<std::string,Key>::value
            && std::is_constructible<Key,std::string>::value,
        Key,
        Value
    >::deserialize(json,v);
  }

  static Json serialize(const std::map<Key, Value>& v) {
    return detail::JsonSerializerHelper<
        std::is_constructible<std::string,Key>::value
            && std::is_constructible<Key,std::string>::value,
        Key,
        Value
    >::serialize(v);
  }

};

}
}


