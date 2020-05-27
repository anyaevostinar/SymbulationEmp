Hi there!

This file is supposed to help you understand what is going on with the test files among the source code.

There are two types of tests here:

1. Unit tests

2. End to end tests

Unit tests check the behavior of methods and collections of methods in individual files. For example, checking that when hostile hosts meet generous symbionts, the symbionts should die out. The unit tests are for source/SymOrg.h are in source/SymOrg.test.cc and the unit tests for source/SymWorld.h are in source/SymWorld.test.cc. There are no unit tests for symbulation.cc

End to end testing is the process of running the entire program on a known configuration and making sure the results match the expected results. This is done in source/end_to_end.test.cc. source/end_to_end.test.cc programmatically selects configuration parameters (not using SymSettings.cfg), and runs symbulation, generating datafile outputs. It then checks those output files against expected output files in source/end_to_end_test_data and requires that their contents match exactly.

Notably, the current (May 13, 2020) end to end testing data are not positively known to be bug free. While I (Lilith Orion Hafner on May 13, 2020) suspect they are free of major bugs, it is hard to know for sure, and they have _not_ been tested for consistency with previously published results.

There are some other files here too. source/catch/catch.hpp comes from https://github.com/catchorg/Catch2/ and is the library that defines the testing macros we use here. source/catch/main.cc is the main file for testing, it imports the library, and calls all the other tests. source/catch/sanity_check.test.cc is for testing the integrity of the testing system. For example, one can uncomment 
/*TEST_CASE( "1+1=3" ) {
  REQUIRE(1+1 == 3);
}*/
and check to make sure that the test suite reports failure.


The tests are written in standard C++ code, but rely heavily on macros defined in source/catch/catch.hpp