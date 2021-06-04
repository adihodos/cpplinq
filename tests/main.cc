#include "cpplinq/cpplinq.hpp"
#include "cpplinq/ext/collect.hpp"
#include "cpplinq/ext/collect_map.hpp"
#include "cpplinq/ext/collect_set.hpp"
#include "cpplinq/ext/collect_unordered_map.hpp"
#include "cpplinq/ext/collect_unordered_set.hpp"
#include "cpplinq/ext/collect_vector.hpp"

#include <cstdint>
#include <forward_list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace std;
using namespace cpplinq;

vector<int32_t>
make_a_vector_with_capacity(const size_t cap)
{
  vector<int32_t> v{};
  v.reserve(cap);
  return v;
}

TEST_CASE("Cpplinq extensions - collect", "[cpplinq-collect-basic]")
{
  const vector<int32_t> numbers{ 0, 1, 2, 3, 4, 5 };
  const vector<int32_t> numbers_id =
    from(numbers) >> select([](const int32_t num) { return num; }) >> collect<vector<int32_t>>();

  REQUIRE(numbers == numbers_id);
}

TEST_CASE("Cpplinq extensions", "[cpplinq-collect-with-args]")
{
  const vector<int32_t> numbers{ 0, 1, 2, 3, 4, 5 };

  SECTION("works using lambda")
  {
    const vector<int32_t> numbers0 =
      from(numbers) >> select([](const int32_t num) { return num; }) >> collect(
                                                                          [](const size_t cap) {
                                                                            vector<int32_t> v;
                                                                            v.reserve(cap);
                                                                            return v;
                                                                          },
                                                                          16);

    REQUIRE(numbers == numbers0);
  }

  SECTION("works using function")
  {
    const vector<int32_t> numbers0 =
      from(numbers) >> select([](const int32_t num) { return num; }) >> collect(make_a_vector_with_capacity, 32);
    REQUIRE(numbers == numbers0);
  }
}

TEST_CASE("collect - associative containers", "[cpplinq-extensions-collect]")
{
  const vector<int32_t> numbers{ 0, 1, 2, 3, 4, 5 };

  SECTION("works with unordered_map")
  {
    const unordered_map<string, int32_t> expected_results{ { "0", 0 }, { "1", 1 }, { "2", 2 },
                                                           { "3", 3 }, { "4", 4 }, { "5", 5 } };

    const unordered_map<string, int32_t> actual_results =
      from(numbers) >> select([](const int32_t num) { return make_pair(to_string(num), num); }) >>
      collect<unordered_map<string, int32_t>>();

    REQUIRE(expected_results == actual_results);
  }

  SECTION("works with map")
  {
    const map<string, int32_t> expected_results{
      { "0", 0 }, { "1", 1 }, { "2", 2 }, { "3", 3 }, { "4", 4 }, { "5", 5 }
    };

    const map<string, int32_t> actual_results =
      from(numbers) >> select([](const int32_t num) { return make_pair(to_string(num), num); }) >>
      collect<map<string, int32_t>>();

    REQUIRE(expected_results == actual_results);
  }

  SECTION("works with unordered_set")
  {
    const unordered_set<string> expected_results{ "0", "1", "2", "3", "4", "5" };

    const unordered_set<string> actual_results =
      from(numbers) >> select([](const int32_t num) { return to_string(num); }) >> collect<unordered_set<string>>();
    REQUIRE(expected_results == actual_results);
  }

  SECTION("works with set")
  {
    const set<string> expected_results{ "0", "1", "2", "3", "4", "5" };

    const set<string> actual_results =
      from(numbers) >> select([](const int32_t num) { return to_string(num); }) >> collect<set<string>>();
    REQUIRE(expected_results == actual_results);
  }
}

TEST_CASE("cpplinq", "[cpplinq-from]")
{
  SECTION("works with containers")
  {
    const unordered_set<int32_t> numbers{ 0, 1, 2, 3, 4, 5 };
    const unordered_set<int32_t> even_numbers =
      from(numbers) >> where([](const int32_t x) { return x % 2 == 0; }) >> collect<unordered_set<int32_t>>();

    REQUIRE(even_numbers == unordered_set<int32_t>{ 0, 2, 4 });
  }

  SECTION("works with initializer list")
  {
    const vector<int32_t> even_numbers =
      from({ 0, 1, 2, 3, 4, 5 }) >> where([](const int32_t x) { return x % 2 == 0; }) >> collect<vector<int32_t>>();

    REQUIRE(even_numbers == vector<int32_t>{ 0, 2, 4 });
  }

  SECTION("works with arrays")
  {
    const int32_t numbers[] = { 0, 1, 2, 3, 4, 5 };
    const vector<int32_t> even_numbers =
      from(numbers) >> where([](const int32_t x) { return x % 2 == 0; }) >> collect<vector<int32_t>>();

    REQUIRE(even_numbers == vector<int32_t>{ 0, 2, 4 });
  }

  SECTION("works with rvalues")
  {
    const vector<int32_t> even_numbers = from(vector<int32_t>{ 0, 1, 2, 3, 4, 5 }) >>
                                         where([](const int32_t x) { return x % 2 == 0; }) >>
                                         collect<vector<int32_t>>();

    REQUIRE(even_numbers == vector<int32_t>{ 0, 2, 4 });
  }
}