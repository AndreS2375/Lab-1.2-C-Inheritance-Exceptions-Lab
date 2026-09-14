#include <catch2/catch_test_macros.hpp>
#include "student.hpp"
#include "instructor.hpp"
#include <vector>
#include <span>

TEST_CASE("Student JSON serialization") {
  Student s(10,"Alice","alice@uni.edu",2027, {"CS101","HIST110"});
  auto j = s.to_json();

  CHECK(j.at("role") == "Student");
  CHECK(j.at("name") == "Alice");
  CHECK(j.at("grad_year") == 2027);
}

TEST_CASE("Instructor YAML serialization") {
  Instructor i(11,"Bob","bob@uni.edu","C-210", {"CS101"});
  auto y = i.to_yaml();

  CHECK(y["role"].as<std::string>() == "Instructor");
  CHECK(y["office"].as<std::string>() == "C-210");
}

TEST_CASE("CSV escaping handles commas and quotes") {
  Student s(12,"Eve, \"The Great\"","eve@uni.edu",2028,
            {"CS,101","AI\"Lab"});

  auto row = s.csv_row();

  REQUIRE(row.find("\"Eve, \"\"The Great\"\"\"") != std::string::npos);
}

TEST_CASE("Validation throws on bad email") {
  REQUIRE_THROWS_AS(
    Student(13,"Zed","not-an-email",2026),
    ValidationError
  );
}

TEST_CASE("Student JSON round trip") {
  Student original(
    20,
    "Alice",
    "alice@uni.edu",
    2027,
    {"CS101", "HIST110"}
  );

  auto j = original.to_json();
  Student restored = Student::from_json(j);

  CHECK(restored.to_json() == j);
}

TEST_CASE("Instructor JSON round trip") {
  Instructor original(
    21,
    "Bob",
    "bob@uni.edu",
    "C-210",
    {"CS101", "CS202"}
  );

  auto j = original.to_json();
  Instructor restored = Instructor::from_json(j);

  CHECK(restored.to_json() == j);
}

TEST_CASE("Student from_json rejects missing fields") {
  nlohmann::json j = {
    {"role", "Student"},
    {"id", 30},
    {"name", "Zed"},
    {"grad_year", 2027},
    {"courses", {"CS101"}}
  };

  REQUIRE_THROWS_AS(
    Student::from_json(j),
    ValidationError
  );
}

TEST_CASE("Student from_json rejects invalid email") {
  nlohmann::json j = {
    {"role", "Student"},
    {"id", 31},
    {"name", "Zed"},
    {"email", "not-an-email"},
    {"grad_year", 2027},
    {"courses", {"CS101"}}
  };

  REQUIRE_THROWS_AS(
    Student::from_json(j),
    ValidationError
  );
}

TEST_CASE("Instructor from_json rejects missing fields") {
  nlohmann::json j = {
    {"role", "Instructor"},
    {"id", 40},
    {"name", "Bob"},
    {"email", "bob@uni.edu"},
    {"teaches", {"CS101"}}
  };

  REQUIRE_THROWS_AS(
    Instructor::from_json(j),
    ValidationError
  );
}

TEST_CASE("Instructor from_json rejects empty office") {
  nlohmann::json j = {
    {"role", "Instructor"},
    {"id", 41},
    {"name", "Bob"},
    {"email", "bob@uni.edu"},
    {"office", ""},
    {"teaches", {"CS101"}}
  };

  REQUIRE_THROWS_AS(
    Instructor::from_json(j),
    ValidationError
  );
}

TEST_CASE("Polymorphic CSV writer uses superset header") {
  Student s(
    50,
    "Alice",
    "alice@uni.edu",
    2027,
    {"CS101", "HIST110"}
  );

  Instructor i(
    51,
    "Bob",
    "bob@uni.edu",
    "C-210",
    {"CS101"}
  );

  std::vector<const Person*> people = {&s, &i};

  auto csv = write_polymorphic_csv(
    std::span<const Person*>(people.data(), people.size())
  );

  CHECK(
    csv.find(
      "role,id,name,email,grad_year,office,courses,teaches\n"
    ) == 0
  );

  CHECK(
    csv.find(
      "Student,50,Alice,alice@uni.edu,2027,,CS101;HIST110,\n"
    ) != std::string::npos
  );

  CHECK(
    csv.find(
      "Instructor,51,Bob,bob@uni.edu,,C-210,,CS101\n"
    ) != std::string::npos
  );
}

TEST_CASE("Polymorphic CSV writer escapes fields") {
  Student s(
    60,
    "Eve, \"The Great\"",
    "eve@uni.edu",
    2028,
    {"CS,101", "AI\"Lab"}
  );

  std::vector<const Person*> people = {&s};

  auto csv = write_polymorphic_csv(
    std::span<const Person*>(people.data(), people.size())
  );

  CHECK(
    csv.find(
      "\"Eve, \"\"The Great\"\"\""
    ) != std::string::npos
  );

  CHECK(
    csv.find(
      "\"CS,101;AI\"\"Lab\""
    ) != std::string::npos
  );
}