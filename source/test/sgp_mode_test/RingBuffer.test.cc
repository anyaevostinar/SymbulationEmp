#include "../../sgp_mode/hardware/RingBuffer.h"
#include "../../catch/catch.hpp"

/// new implementation 

TEST_CASE("RingBuffer initialization", "[sgp]") {
    sgpmode::RingBuffer<int> buffer(5, 0);  
    REQUIRE(buffer.size() == 5);  
    for (size_t i = 0; i < buffer.size(); ++i) {
        REQUIRE(buffer[i] == 0);  
    }
}

TEST_CASE("Push elements into RingBuffer", "[sgp]") {
    sgpmode::RingBuffer<int> buffer(3, 0);  

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);

    REQUIRE(buffer[0] == 10);  
    REQUIRE(buffer[1] == 20); 
    REQUIRE(buffer[2] == 30);  

    buffer.push(40); 
    REQUIRE(buffer[0] == 40);  
    REQUIRE(buffer[1] == 20); 
    REQUIRE(buffer[2] == 30);

    // additional pushing 

    buffer.push(10);
    REQUIRE(buffer[0] == 10);  
    REQUIRE(buffer[1] == 40); 
    REQUIRE(buffer[2] == 20);

    buffer.push(50);
    REQUIRE(buffer[0] == 50);  
    REQUIRE(buffer[1] == 10); 
    REQUIRE(buffer[2] == 40);


}

TEST_CASE("Read from RingBuffer", "[sgp]") {
    sgpmode::RingBuffer<int> buffer(3, 0);  

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);

    REQUIRE(buffer.read() == 10);  
    REQUIRE(buffer.read() == 20);  
    REQUIRE(buffer.read() == 30);  

    
    buffer.push(40);  
    REQUIRE(buffer.read() == 40); 
}

TEST_CASE("Reset RingBuffer", "[sgp]") {
    sgpmode::RingBuffer<int> buffer(5, 0);  

    buffer.push(10);
    buffer.push(20);
    buffer.push(30);

    REQUIRE(buffer[0] == 10);
    REQUIRE(buffer[1] == 20);
    REQUIRE(buffer[2] == 30);

    buffer.Reset(5, 0);  

    REQUIRE(buffer[0] == 0);
    REQUIRE(buffer[1] == 0);
    REQUIRE(buffer[2] == 0);
    REQUIRE(buffer[3] == 0);
    REQUIRE(buffer[4] == 0);
}


