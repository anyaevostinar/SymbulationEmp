# Guide to Testing in Symbulation 
 
This document details how testing works in Symbulation, both for writing
and understanding tests.
## Running Tests
 
In the root directory of Symbulation, use the maketarget `test`, like so:
 
```shell
make test
```
 
The tests will compile and execute automatically, and you should see
output that looks something like this:
 
```shell
g++ -O3 -DNDEBUG -Wall -Wno-unused-function -std=c++17 -I../Empirical/include/ source/catch/main.cc -o symbulation.test
# Execute tests
./symbulation.test
===============================================================================
All tests passed (592 assertions in 70 test cases)
```
 
## Writing Tests
 
It is required that contributions to the Symbulation library have test
coverage. Though writing tests can be a complex task in some cases, it can also be easy to do.
 
In general the best way to understand how to write tests is to look at
the existing tests. Each header file in source/ has a file full of tests ending with “class_name.test.cc”. We recommend browsing through those files. 
 
 
The test cases should have the following layout:
 
```cpp
TEST_CASE("Test name goes here", "[test classification here]")
{
        // body of test
}
```
 
Within a test case you can use the `REQUIRE` macro like an assert, to
require certain conditions within the test:
 
```cpp
REQUIRE(1==1); // will pass, obviously
REQUIRE(1==0); // will fail, and Catch will complain
```
 
If a `REQUIRE` fails, the compiler will give an error when you run “make test”.

