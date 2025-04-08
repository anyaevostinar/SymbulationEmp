

/// updated implementation: 

#include "../../sgp_mode/hardware/Stacks.h"
#include "../../catch/catch.hpp"
#include "emp/base/vector.hpp"
#include "emp/base/array.hpp"

#include <limits>
#include <optional>

TEST_CASE("Stack initialization", "[sgp]") {
    sgpmode::Stacks<int> stacks(3);  
    REQUIRE(stacks.GetNumStacks() == 3); 
    REQUIRE(stacks.GetActiveStack().size() == 0);  

    stacks.SetActive(1);
    REQUIRE(stacks.GetActiveStack().size() == 0); 

    stacks.SetActive(2);
    REQUIRE(stacks.GetActiveStack().size() == 0); 
    
}

TEST_CASE("Push elements onto stack", "[sgp]") {
    sgpmode::Stacks<int> stacks(2);  
    stacks.SetActive(0); 

    REQUIRE(stacks.Push(10) == true);  
    REQUIRE(stacks.Push(20) == true); 

    // Verify top of the stack
    REQUIRE(stacks.GetTop() == true);
    REQUIRE(stacks.GetTop() == 20);  

    stacks.SetStackLimit(3); 
    REQUIRE(stacks.Push(30) == true); 
    REQUIRE(stacks.Push(40) == false); 
}

TEST_CASE("Pop elements from stack", "[sgp]") {
    sgpmode::Stacks<int> stacks(1);  
    stacks.SetActive(0);  
   
    stacks.Push(10);
    stacks.Push(20);
    stacks.Push(30);

    // Pop and check top element
    auto pop_result = stacks.Pop();
    REQUIRE(pop_result.has_value() == true);  
    REQUIRE(pop_result.value() == 30);  

    pop_result = stacks.Pop();
    REQUIRE(pop_result.has_value() == true); 
    REQUIRE(pop_result.value() == 20);  

    pop_result = stacks.Pop();
    REQUIRE(pop_result.has_value() == true);  
    REQUIRE(pop_result.value() == 10); 

    pop_result = stacks.Pop();
    REQUIRE(pop_result.has_value() == false);  
}

TEST_CASE("Clear active stack", "[sgp]") {
    sgpmode::Stacks<int> stacks(2);  
    stacks.SetActive(0);  

    stacks.Push(10);
    stacks.Push(20);

    stacks.ClearActive();
    
    REQUIRE(stacks.GetActiveStack().size() == 0);
}

TEST_CASE("Change active stack", "[sgp]") {
    sgpmode::Stacks<int> stacks(3);  

    stacks.SetActive(0);
    REQUIRE(stacks.GetActiveStack().size() == 0);  

    stacks.Push(10);
    stacks.Push(20);

    stacks.SetActive(1);
    REQUIRE(stacks.GetActiveStack().size() == 0);  

    stacks.Push(30);
    stacks.Push(40);

    REQUIRE(stacks.GetActiveStack().size() == 2);
    REQUIRE(stacks.GetActiveStack()[0] == 30);
    REQUIRE(stacks.GetActiveStack()[1] == 40);

    stacks.SetActive(0);
    REQUIRE(stacks.GetActiveStack().size() == 2);
    REQUIRE(stacks.GetActiveStack()[0] == 10);
    REQUIRE(stacks.GetActiveStack()[1] == 20);
}

TEST_CASE("Set stack limit and resize stacks", "[sgp]") {
    sgpmode::Stacks<int> stacks(2); 

    stacks.SetStackLimit(2);

    stacks.SetActive(0);
    stacks.Push(10);
    stacks.Push(20);
    stacks.Push(30);  

    // Verify stack size limit
    REQUIRE(stacks.GetActiveStack().size() == 2);  
    REQUIRE(stacks.GetActiveStack()[0] == 10);
    REQUIRE(stacks.GetActiveStack()[1] == 20);
}

TEST_CASE("Stack pops after limit resize", "[sgp]") {
    sgpmode::Stacks<int> stacks(1);  
    stacks.SetActive(0);  

    stacks.SetStackLimit(3);
    stacks.Push(10);
    stacks.Push(20);
    stacks.Push(30);

    // Resize the stack limit and verify
    stacks.SetStackLimit(2);
    REQUIRE(stacks.GetActiveStack().size() == 2);  

    REQUIRE(stacks.GetActiveStack()[0] == 10);
    REQUIRE(stacks.GetActiveStack()[1] == 20);  
}

