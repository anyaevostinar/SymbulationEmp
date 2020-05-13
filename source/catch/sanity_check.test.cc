unsigned int Cube( unsigned int number ) {
  return number*number*number;
}

TEST_CASE( "1+1=2" ) {
  REQUIRE(1+1 == 2);
}

/*TEST_CASE( "1+1=3" ) {
  REQUIRE(1+1 == 3);
}*/

TEST_CASE( "Cubes are computed") {
  SECTION("Positive values") {
    REQUIRE( Cube(1) == 1 );
    REQUIRE( Cube(2) == 8 );
    REQUIRE( Cube(30) == 27000 );
  }
    SECTION("Negative values") {
    REQUIRE( Cube(-1) == -1 );
    REQUIRE( Cube(-2) == -8 );
    REQUIRE( Cube(-30) == -27000 );
  }
    SECTION("Zero") {
    REQUIRE( Cube(0) == 0 );
  }
}
