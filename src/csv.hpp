#pragma once
#include <string>
#include <algorithm>
#include <span>
#include "person.hpp"
#include <nlohmann/json.hpp>

inline std::string csv_escape(std::string s) {
  bool need_quotes = s.find_first_of(",\"\n") != std::string::npos;
  if (need_quotes) {
    std::string t; 
    t.reserve(s.size()+2);
    for (char c: s) { 
      t += (c=='"') ? std::string("\"\"") : std::string(1,c); 
    }
    return "\"" + t + "\"";
  }
  return s;
}

inline std::string write_polymorphic_csv(std::span<const Person*> people) {
  std::string out =
      "role,id,name,email,grad_year,office,courses,teaches\n";

  for (const Person* person : people) {
    if (person == nullptr)
      throw SerializationError("null Person pointer");

    auto j = person->to_json();

    auto get_field = [&](const char* key) -> std::string {
      if (!j.contains(key) || j.at(key).is_null())
        return "";

      const auto& value = j.at(key);

      if (value.is_string())
        return value.get<std::string>();

      if (value.is_number_integer())
        return std::to_string(value.get<long long>());

      if (value.is_array()) {
        std::string joined;

        for (size_t i = 0; i < value.size(); ++i) {
          if (i > 0)
            joined += ";";

          if (!value.at(i).is_string())
            throw SerializationError(
                "array field must contain strings");

          joined += value.at(i).get<std::string>();
        }

        return joined;
      }

      throw SerializationError("unsupported CSV field type");
    };

    out += csv_escape(get_field("role")) + ",";
    out += csv_escape(get_field("id")) + ",";
    out += csv_escape(get_field("name")) + ",";
    out += csv_escape(get_field("email")) + ",";
    out += csv_escape(get_field("grad_year")) + ",";
    out += csv_escape(get_field("office")) + ",";
    out += csv_escape(get_field("courses")) + ",";
    out += csv_escape(get_field("teaches")) + "\n";
  }

  return out;
}