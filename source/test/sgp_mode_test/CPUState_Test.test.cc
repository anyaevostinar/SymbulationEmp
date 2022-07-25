#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../../Organism.h"
#include "../../sgp_mode/CPU.h"
#include "../../sgp_mode/SGPHost.h"
#include "../../sgp_mode/SGPSymbiont.h"
#include "../../sgp_mode/SGPWorld.h"
#include "../../sgp_mode/Tasks.h"
#include "../../sgp_mode/SymbiontImpact.h"
#include "../../native/symbulation_sgp.cc"
#include <set>
#include <math.h>
#include <tuple>

TEST_CASE("Host Symbiont Test Chamber", "[sgp]"){
        emp::Ptr<emp::Random> random = new emp::Random(-1);
        SymConfigBase config;
        TaskSet TestTasks{
  {"DUMMY", OutputTask{[](uint32_t x) { return x ? 4.0 : 0.0; } }}//Bit weird because it's not testing squares
};   
        SGPWorld w(*random, &config, TestTasks);
        SGPWorld *world = &w;
        //Host Symbiont Preprocessing
        worldSetup(world, &config);
        int hostIndexNoResources = PreProcessCheckSymbiont(world);
        REQUIRE(hostIndexNoResources == -1);
        world->Update();

        int hostIndexWithResources = PreProcessCheckSymbiont(world);
        REQUIRE(hostIndexWithResources >= 0);
        //emp::Ptr<SGPHost> host = get<0>(hostSymbiontPair);
        emp::Ptr<SGPSymbiont> symbiont = get<1>(hostSymbiontPair);

        //REQUIRE(host->GetPoints() > 0);


        //Actual CheckSymbiont tests
        emp::Ptr<SGPHost> testHost = emp::NewPtr<SGPHost>(random, world, &config);
        emp::Ptr<SGPSymbiont> testSymbiont = emp::NewPtr<SGPSymbiont>(random, world, &config);
        testHost->AddSymbiont(testSymbiont);
        std::tuple<float, float> hostSymbiontFinalPoints = CheckSymbiont(*testHost, *testSymbiont);
        float hostPointsWithSymbiont = std::get<0>(hostSymbiontFinalPoints);
        float symbiontPoints = std::get<1>(hostSymbiontFinalPoints);
        REQUIRE(hostPointsWithSymbiont > 0);
        REQUIRE(symbiontPoints > 0);

}



        
