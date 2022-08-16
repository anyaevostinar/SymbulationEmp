#include "../../source/sgp_mode/Tasks.h"
#include <map>

TEST_CASE("GetSquareFrequencyData returns hostCalculationMap for host", "[sgp]") {
    bool hostCheck = 1;
    Task SQU1 = {"SQU1", OutputTask{[](uint32_t x) {
                    return sqrt(x) - floor(sqrt(x)) == 0 ? (0.5 * x) : 0.0;
                    }}};
    TaskSet SquareTest{SQU1};
    SquareTest.GetHostCalculationTable()->insert(std::pair<uint32_t, uint32_t>(1, 1));
    SquareTest.GetSymCalculationTable()->insert(std::pair<uint32_t, uint32_t>(2, 2));
    REQUIRE(SquareTest.GetSquareFrequencyData(hostCheck) == *(SquareTest.GetHostCalculationTable()));

}

TEST_CASE("GetSquareFrequencyData returns symCalculationMap for sym", "[sgp]") {
    bool symCheck = 0;
    Task SQU1 = {"SQU1", OutputTask{[](uint32_t x) {
                    return sqrt(x) - floor(sqrt(x)) == 0 ? (0.5 * x) : 0.0;
                    }}};
    TaskSet SquareTest{SQU1};
    SquareTest.GetHostCalculationTable()->insert(std::pair<uint32_t, uint32_t>(1, 1));
    SquareTest.GetSymCalculationTable()->insert(std::pair<uint32_t, uint32_t>(2, 2));
    REQUIRE(SquareTest.GetSquareFrequencyData(symCheck) == *(SquareTest.GetSymCalculationTable()));

}

TEST_CASE("IncrementSquareMap adds a new key-value pair if empty", "[sgp]") {
    Task PLACEHOLDER = {"PLACEHOLDER", OutputTask{[](uint32_t x) {
                    return 5;}}};
    TaskSet TestTaskSet{PLACEHOLDER};
    std::map<uint32_t, uint32_t> testMap;
    uint32_t testOutput = 4;
    TestTaskSet.IncrementSquareMap(testOutput, testMap);
    std::map<uint32_t, uint32_t>::iterator placemark;
    placemark = testMap.find(testOutput);
    REQUIRE(placemark != testMap.end());
    REQUIRE(placemark -> second == 1);
}

TEST_CASE("IncrementSquareMap adds new key-value pair to non-empty map", "[sgp]"){
      Task PLACEHOLDER = {"PLACEHOLDER", OutputTask{[](uint32_t x) {
                    return 5;}}};
    TaskSet TestTaskSet{PLACEHOLDER};
    std::map<uint32_t, uint32_t> testMap;
    uint32_t testOutput = 4;
    testMap.insert(std::pair<uint32_t, uint32_t>(5, 3));
    TestTaskSet.IncrementSquareMap(testOutput, testMap);
    std::map<uint32_t, uint32_t>::iterator placemark;
    placemark = testMap.find(testOutput);
    REQUIRE(testMap.size() == 2);
    REQUIRE(placemark != testMap.end());
    REQUIRE(placemark -> second == 1);
}

TEST_CASE("IncrementSquareMap increments frequency if key is present", "[sgp]"){
    Task PLACEHOLDER = {"PLACEHOLDER", OutputTask{[](uint32_t x) {
                    return 5;}}};
    TaskSet TestTaskSet{PLACEHOLDER};
    std::map<uint32_t, uint32_t> testMap;
    uint32_t testOutput = 4;
    testMap.insert(std::pair<uint32_t, uint32_t>(testOutput, 1));
    TestTaskSet.IncrementSquareMap(testOutput, testMap);
    std::map<uint32_t, uint32_t>::iterator placemark;
    placemark = testMap.find(testOutput);
    REQUIRE(placemark -> second == 2);
}

