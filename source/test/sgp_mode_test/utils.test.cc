
/// updated implementation of unit tests for refactor:
#include "../../source/sgp_mode/utils.h"
#include "../../catch/catch.hpp"
#include "emp/bits/Bits.hpp"

TEST_CASE("AnyMatch Test", "[sgp]") {

    SECTION("Matching bits", "[sgp]") {
        emp::BitSet<3> b1("001");
        emp::BitSet<3> b2("001");
        REQUIRE(sgpmode::utils::AnyMatch(b1, b2) == true);
    }

    SECTION("No matching bits", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("000");
        REQUIRE(sgpmode::utils::AnyMatch(bits_a, bits_b) == false);
    }
}

TEST_CASE("SimpleMatchCoeff Test", "[sgp]") {

    SECTION("Matching bits count - 2", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("110");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 2);
    }

    SECTION("Matching bits count - 1", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("100");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 1);
    }

    SECTION("No matching bits", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("000");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 0);
    }
}

TEST_CASE("ResizeClear Test - no ones", "[sgp]") {
    emp::BitVector bits(0, 5);
    REQUIRE(bits.CountOnes() == 0);


    SECTION("Resize and clear", "[sgp]") {
        sgpmode::utils::ResizeClear(bits, 5);
        REQUIRE(bits.size() == 5);
        REQUIRE(bits.CountOnes() == 0);
    }
}

TEST_CASE("ResizeFill Test", "[sgp]") {
    std::vector<int> container = {1, 2, 3, 4, 5};

    // Resize and fill with value
    sgpmode::utils::ResizeFill(container, 7, 0);
    REQUIRE(container.size() == 7);
    REQUIRE(container[0] == 0);
    REQUIRE(container[1] == 0);
    REQUIRE(container[2] == 0);
    REQUIRE(container[3] == 0);
    REQUIRE(container[4] == 0);
    REQUIRE(container[5] == 0);
    REQUIRE(container[6] == 0);

    // Resize and fill with another value
    sgpmode::utils::ResizeFill(container, 7, 10);
    REQUIRE(container.size() == 7);
    for (size_t i = 0; i < container.size(); ++i) {
        REQUIRE(container[i] == 10);
    }
}












