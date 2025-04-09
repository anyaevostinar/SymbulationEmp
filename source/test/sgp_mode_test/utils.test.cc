
/// updated implementation of unit tests for refactor: 
#include "../../source/sgp_mode/utils.h"
#include "../../catch/catch.hpp"
#include "emp/bits/Bits.hpp"

TEST_CASE("AnyMatch Test", "[sgp]") {
    
    SECTION("Matching bits - all", "[sgp]") {
        emp::BitSet<3> b1("000");
        emp::BitSet<3> b2("000"); 
        REQUIRE(sgpmode::utils::AnyMatch(b1, b2) == true);
    }

    SECTION("No matching bits - two ", "[sgp]") {
        emp::BitSet<3> bits_a("001");
        emp::BitSet<3> bits_b("000"); 
        REQUIRE(sgpmode::utils::AnyMatch(bits_a, bits_b) == true);
    }

    SECTION("No matching bits - one ", "[sgp]") {
        emp::BitSet<3> bits_a("110");
        emp::BitSet<3> bits_b("000"); 
        REQUIRE(sgpmode::utils::AnyMatch(bits_a, bits_b) == true);
    }

    SECTION("No matching bits", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("000"); 
        REQUIRE(sgpmode::utils::AnyMatch(bits_a, bits_b) == false);
    }
}

TEST_CASE("SimpleMatchCoeff Test", "[sgp]") {
  
    SECTION("Matching bits count - all", "[sgp]") {
        emp::BitSet<3> bits_a("101");
        emp::BitSet<3> bits_b("101");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 3);  
    }

    SECTION("Matching bits count - 2", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("101");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 2);  
    }

    SECTION("Matching bits count - 1", "[sgp]") {
        emp::BitSet<3> bits_a("111");
        emp::BitSet<3> bits_b("100");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 2);  
    }

    SECTION("No matching bits", "[sgp]") {
        emp::BitSet<3> bits_a("101");
        emp::BitSet<3> bits_b("010");
        REQUIRE(sgpmode::utils::SimpleMatchCoeff(bits_a, bits_b) == 0);  
    }
}

TEST_CASE("ResizeClear Test - no ones", "[sgp]") {
    emp::BitVector bits(10, 5); 
    REQUIRE(bits.CountOnes() == 0); 
 
    
    SECTION("Resize and clear", "[sgp]") {
        sgpmode::utils::ResizeClear(bits, 5);  
        REQUIRE(bits.size() == 5); 
        REQUIRE(bits.CountOnes() == 0);  
    }
}

TEST_CASE("ResizeFill Test", "[sgp]") {
    std::vector<int> container = {1,2,3,4,5};

    
    SECTION("Resize and fill with value", "[sgp]") {
        sgpmode::utils::ResizeFill(container, 7, 0);  
        REQUIRE(container.size() == 7);  
        REQUIRE(container[6] == 0); 
    }

    SECTION("Resize and fill with another value", "[sgp]") {
        sgpmode::utils::ResizeFill(container, 7, 10);  
        REQUIRE(container[6] == 10);  
    }
}












