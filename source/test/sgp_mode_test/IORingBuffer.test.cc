#include "../../sgp_mode/hardware/IORingBuffer.h"
#include "../../catch/catch.hpp"

TEST_CASE("IORingBuffer Push", "[sgp]") {
    sgpmode::IORingBuffer<int, 3> buffer;

    SECTION("Push Adds Elements Correctly") {
        buffer.push(1);
        buffer.push(2);
        buffer.push(3);

        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
    }

    SECTION("Push Overwrites the Oldest Element When Full") {
        buffer.push(1);
        buffer.push(2);
        buffer.push(3);
        buffer.push(4);

        REQUIRE(buffer[0] == 2); 
        REQUIRE(buffer[1] == 3);
        REQUIRE(buffer[2] == 4);
    }
}

TEST_CASE("IORingBuffer Indexing", "[sgp]") {
    sgpmode::IORingBuffer<int, 5> buffer;

    SECTION("Indexing Accesses Correct Element") {
        buffer.push(10);
        buffer.push(20);
        buffer.push(30);

        REQUIRE(buffer[0] == 10);
        REQUIRE(buffer[1] == 20);
        REQUIRE(buffer[2] == 30);
    }

    SECTION("Indexing Wraps Correctly") {
        buffer.push(1);
        buffer.push(2);
        buffer.push(3);
        buffer.push(4);
        buffer.push(5);

        REQUIRE(buffer[0] == 1);
        REQUIRE(buffer[1] == 2);
        REQUIRE(buffer[2] == 3);
        REQUIRE(buffer[3] == 4);
        REQUIRE(buffer[4] == 5);
        buffer.push(6); 
        REQUIRE(buffer[0] == 2);
        REQUIRE(buffer[1] == 3);
        REQUIRE(buffer[2] == 4);
        REQUIRE(buffer[3] == 5);
        REQUIRE(buffer[4] == 6);
    }
}

TEST_CASE("IORingBuffer Reset", "[sgp]") {
    sgpmode::IORingBuffer<int, 4> buffer(7); 

    SECTION("Reset with New Value") {
        buffer.Reset(0);
        for (size_t i = 0; i < buffer.size(); ++i) {
            REQUIRE(buffer[i] == 0);
        }
    }

    SECTION("Reset to a Different Value and Verify") {
        buffer.Reset(42);
        for (size_t i = 0; i < buffer.size(); ++i) {
            REQUIRE(buffer[i] == 42);
        }
    }
}

