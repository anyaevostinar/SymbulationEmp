#include "../../source/sgp_mode/Tasks.h"
#include <map>

TEST_CASE("GetSquareFrequencyData returns hostCalculationMap for host", "[sgp]") {
    bool hostCheck = 1;
    SquareTask PLACEHOLDER = {"PLACEHOLDER", [](uint32_t x) {
                    return 5;
                    }};
    TaskSet PlaceholderTasks{&PLACEHOLDER};
    PLACEHOLDER.hostCalculationTable.insert(std::pair<uint32_t, uint32_t>(1, 1));
    PLACEHOLDER.symCalculationTable.insert(std::pair<uint32_t, uint32_t>(2, 2));
    REQUIRE(PlaceholderTasks.GetSquareFrequencyData(hostCheck) == PLACEHOLDER.hostCalculationTable);

}

TEST_CASE("GetSquareFrequencyData returns symCalculationMap for sym", "[sgp]") {
    bool symCheck = 0;
    SquareTask PLACEHOLDER = {"PLACEHOLDER", [](uint32_t x) {
                    return 5;
                    }};
    TaskSet PlaceholderTasks{&PLACEHOLDER};
    PLACEHOLDER.hostCalculationTable.insert(std::pair<uint32_t, uint32_t>(1, 1));
    PLACEHOLDER.symCalculationTable.insert(std::pair<uint32_t, uint32_t>(2, 2));
    REQUIRE(PlaceholderTasks.GetSquareFrequencyData(symCheck) == PLACEHOLDER.symCalculationTable);

}

TEST_CASE("IncrementSquareMap adds a new key-value pair if empty", "[sgp]") {
    SquareTask PLACEHOLDER = {"PLACEHOLDER", [](uint32_t x) {
                    return 5;}};
    std::map<uint32_t, uint32_t>& testMap = PLACEHOLDER.hostCalculationTable;
    uint32_t testOutput = 4;
    PLACEHOLDER.IncrementSquareMap(testOutput, 1);
    std::map<uint32_t, uint32_t>::iterator placemark;
    placemark = testMap.find(testOutput);
    REQUIRE(placemark != testMap.end());
    REQUIRE(placemark -> second == 1);
}

TEST_CASE("IncrementSquareMap adds new key-value pair to non-empty map", "[sgp]"){
    SquareTask PLACEHOLDER = {"PLACEHOLDER", [](uint32_t x) {
                    return 5;}};
    std::map<uint32_t, uint32_t>& testMap = PLACEHOLDER.hostCalculationTable;
    uint32_t testOutput = 4;
    testMap.insert(std::pair<uint32_t, uint32_t>(5, 3));
    PLACEHOLDER.IncrementSquareMap(testOutput, 1);
    std::map<uint32_t, uint32_t>::iterator placemark;
    placemark = testMap.find(testOutput);
    REQUIRE(testMap.size() == 2);
    REQUIRE(placemark != testMap.end());
    REQUIRE(placemark -> second == 1);
}

TEST_CASE("IncrementSquareMap increments frequency if key is present", "[sgp]"){
    SquareTask PLACEHOLDER = {"PLACEHOLDER", [](uint32_t x) {
                    return 5;}};
    std::map<uint32_t, uint32_t>& testMap = PLACEHOLDER.hostCalculationTable;
    uint32_t testOutput = 4;
    testMap.insert(std::pair<uint32_t, uint32_t>(testOutput, 1));
    PLACEHOLDER.IncrementSquareMap(testOutput, 1);
    std::map<uint32_t, uint32_t>::iterator placemark;
    placemark = testMap.find(testOutput);
    REQUIRE(placemark -> second == 2);
}

TEST_CASE("ClearSquareFrequencyData successfully empties host and sym maps", "[sgp]"){
    SquareTask PLACEHOLDER = {"PLACEHOLDER", [](uint32_t x) {
                    return 5;}};
    TaskSet PlaceholderTasks{&PLACEHOLDER};
    PLACEHOLDER.hostCalculationTable.insert(std::pair<uint32_t, uint32_t>(4, 1));
    PLACEHOLDER.hostCalculationTable.insert(std::pair<uint32_t, uint32_t>(5, 2));
    PLACEHOLDER.symCalculationTable.insert(std::pair<uint32_t, uint32_t>(6, 3));
    PlaceholderTasks.ClearSquareFrequencyData(true);
    PlaceholderTasks.ClearSquareFrequencyData(false);
    REQUIRE(PLACEHOLDER.hostCalculationTable.empty());
    REQUIRE(PLACEHOLDER.symCalculationTable.empty());
}

