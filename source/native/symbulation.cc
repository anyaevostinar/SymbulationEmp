// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
#include "source/config/ArgManager.h"

using namespace std;

EMP_BUILD_CONFIG( SymConfigBase,
                 VALUE(SEED, double, 10, "What value should the random seed be?"),
                 VALUE(MUTATION_RATE, double, 0.002, "Standard deviation of the distribution to mutate by"),
                 VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
                 VALUE(VERTICAL_TRANSMISSION, double, 1, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
		  VALUE(HOST_INT, double, 0, "Interaction value from -1 to 1 that hosts should have initially"),
		  VALUE(SYM_INT, double, 0, "Interaction value from -1 to 1 that symbionts should have initially"),
                 VALUE(GRID_X, int, 5, "Width of the world"),
                 VALUE(GRID_Y, int, 5, "Height of world"),
                 VALUE(UPDATES, int, 1, "Number of updates to run before quitting"),

                 )
//TODO: add option for random host and sym int values


	
int main(int argc, char * argv[])
{    
    SymConfigBase config;
    
    config.Read("SymSettings.cfg");

    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) {
	cout << "There was a problem in processing the options file." << endl;
	exit(0);
      }
    if (args.TestUnknown() == false) exit(0); //Leftover args no good

    double numupdates = config.UPDATES();
    double POP_SIZE = config.GRID_X() * config.GRID_Y();


    emp::Random random(config.SEED());
        
    SymWorld world(random);
    world.SetGrid(config.GRID_X(), config.GRID_Y());
    world.SetVertTrans(config.VERTICAL_TRANSMISSION());
    world.SetMutRate(config.MUTATION_RATE());
    //Set up files
    world.SetupPopulationFile().SetTimingRepeat(10);
    world.SetupHostIntValFile("HostVals"+to_string(config.SEED())+"_"+to_string(config.VERTICAL_TRANSMISSION())+".data").SetTimingRepeat(10);
    world.SetupSymIntValFile("SymVals"+to_string(config.SEED())+"_"+to_string(config.VERTICAL_TRANSMISSION())+".data").SetTimingRepeat(10);

    //inject organisms
    for (size_t i = 0; i < POP_SIZE; i++){
      Host *new_org = new Host(config.HOST_INT(), *(new Symbiont(config.SYM_INT())));
      cout << new_org->GetIntVal() << endl;
      world.Inject(*new_org);
    }

    //Loop through updates
      
    for (int i = 0; i < numupdates; i++) {
      cout << i << endl;
      world.Update();
    }

}
