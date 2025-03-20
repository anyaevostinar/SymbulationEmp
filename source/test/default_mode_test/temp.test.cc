#include "../../default_mode/SymWorld.h"

//TODO: Remove this test! This is demonstration that the randomness works for default World
//Once the randomness is fixed for SGPWorld, this file should be removed

TEST_CASE("Testing randomness after experiment", "[test]") {
    emp::Random random(10);
    //TODO: The random number seed doesn't seem to be working, different values for the same seed
  
    SymConfigBase config;
    config.SEED(10);

    size_t world_size = config.GRID_X() * config.GRID_Y();
    SymWorld world(random, &config);


    size_t run_updates = 20000;
    WHEN("The world runs") {    
        world.Setup();
    
        REQUIRE(world.GetNumOrgs() == world_size);
        for (size_t i = 0; i < run_updates; i++) {
        world.Update();
        }
        //std::cout << "Random: " << random.GetSeed() << std::endl;
        //std::cout << "Random number: " << random.GetUInt() << std::endl;
        THEN("The random number is....") {
        REQUIRE(random.GetUInt() == 0);
        }
    }
}
