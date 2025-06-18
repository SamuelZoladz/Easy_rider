#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#define private public
#define protected public
#include "Easy_rider/HelloWorldApp.h"
#undef private
#undef protected

TEST_CASE("add function behaves correctly", "[add]") {
  REQUIRE(add(0, 0) == 0);
  REQUIRE(add(1, 2) == 3);
  REQUIRE(add(-1, 1) == 0);
  REQUIRE(add(-5, -7) == -12);
}
