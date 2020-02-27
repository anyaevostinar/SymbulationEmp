#include <stdlib.h>
#include <iostream>

using namespace std;

void* my_malloc(size_t size, const char *file, int line, const char *func)
{

    void *p = malloc(size);
    printf ("Allocated = %s, %i, %s, %p[%li]\n", file, line, func, p, size);

    /*Link List functionality goes in here*/

    return p;
}


#define malloc(X) my_malloc( X, __FILE__, __LINE__, __FUNCTION__)



// This is the main function for the NATIVE version of this project.

#include "../SymWorld.h"
#include "../../Empirical/source/config/ArgManager.h"


EMP_BUILD_CONFIG( SymConfigBase,
                 VALUE(SEED, int, 10, "What value should the random seed be?"),
                 VALUE(MUTATION_RATE, double, 0.002, "Standard deviation of the distribution to mutate by"),
                 VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
                 VALUE(VERTICAL_TRANSMISSION, double, 1, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
		  VALUE(HOST_INT, double, 0, "Interaction value from -1 to 1 that hosts should have initially, -2 for random"),
		  VALUE(SYM_INT, double, 0, "Interaction value from -1 to 1 that symbionts should have initially, -2 for random"),
                 VALUE(GRID_X, int, 5, "Width of the world, just multiplied by the height to get total size"),
                 VALUE(GRID_Y, int, 5, "Height of world, just multiplied by width to get total size"),
                 VALUE(UPDATES, int, 1, "Number of updates to run before quitting"),
		  VALUE(SYM_LIMIT, int, 1, "Number of symbiont allowed to infect a single host"),
		  VALUE(LYSIS, bool, 0, "Should lysis occur? 0 for no, 1 for yes"),
		  VALUE(HORIZ_TRANS, bool, 0, "Should non-lytic horizontal transmission occur? 0 for no, 1 for yes"),
		  VALUE(BURST_SIZE, int, 10, "If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update"),
		  VALUE(BURST_TIME, int, 10, "If lysis enabled, this is how many updates will pass before lysis occurs"),
		  VALUE(HOST_REPRO_RES, double, 1000, "How many resources required for host reproduction"),
		  VALUE(SYM_LYSIS_RES, double, 1, "How many resources required for symbiont to create offspring for lysis each update"),
		  VALUE(SYM_HORIZ_TRANS_RES, double, 100, "How many resources required for symbiont non-lytic horizontal transmission"),
		  VALUE(GRID, bool, 0, "Do offspring get placed immediately next to parents on grid, same for symbiont spreading"),
      VALUE(TRIALS, int, 1, "How many independent trials to repeat"),
      VALUE(FILE_PATH, string, "", "Output file path"),
      VALUE(FILE_NAME, string, "_data_", "Root output file name")
                 )
//TODO: add option for random host and sym int values
//TODO: add option for world structure, currently mixed only


	
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

  int numupdates = config.UPDATES();
  double POP_SIZE = config.GRID_X() * config.GRID_Y();
  bool random_phen_host = false;
  bool random_phen_sym = false;
  if(config.HOST_INT() == -2) random_phen_host = true;
  if(config.SYM_INT() == -2) random_phen_sym = true;

  cout << "Running Simbulation Trials [";
  for(int trial = 0; trial < config.TRIALS(); trial++) {
    emp::Random random(config.SEED()+trial);
        
    SymWorld world(random);
    if (config.GRID() == 0) world.SetPopStruct_Mixed();
    else world.SetPopStruct_Grid(config.GRID_X(), config.GRID_Y());

    world.SetVertTrans(config.VERTICAL_TRANSMISSION());
    world.SetMutRate(config.MUTATION_RATE());
    world.SetSymLimit(config.SYM_LIMIT());
    world.SetLysisBool(config.LYSIS());
    world.SetHTransBool(config.HORIZ_TRANS());
    world.SetBurstSize(config.BURST_SIZE());
    world.SetBurstTime(config.BURST_TIME());
    world.SetHostRepro(config.HOST_REPRO_RES());
    world.SetSymHRes(config.SYM_HORIZ_TRANS_RES());
    world.SetSymLysisRes(config.SYM_LYSIS_RES());

    //Set up files
    world.SetupPopulationFile().SetTimingRepeat(100);
    //world.SetupHostIntValFile("HostVals"+to_string(config.SEED())+"_"+to_string(config.VERTICAL_TRANSMISSION())+".data").SetTimingRepeat(10);
    //world.SetupSymIntValFile("SymVals"+to_string(config.SEED())+"_"+to_string(config.VERTICAL_TRANSMISSION())+".data").SetTimingRepeat(10);
    world.SetupHostIntValFile(config.FILE_PATH()+"HostVals"+config.FILE_NAME()+to_string(trial)+".data").SetTimingRepeat(100);
    world.SetupSymIntValFile(config.FILE_PATH()+"SymVals"+config.FILE_NAME()+to_string(trial)+".data").SetTimingRepeat(100);
    

    //inject organisms
    for (size_t i = 0; i < POP_SIZE; i++){
      Host *new_org;
      if (random_phen_host) new_org = new Host(random.GetDouble(-1, 1));
      else new_org = new Host(config.HOST_INT());
      world.Inject(*new_org);

      Symbiont new_sym; 
      if(random_phen_sym) new_sym = *(new Symbiont(random.GetDouble(-1, 1)));
      else new_sym = *(new Symbiont(config.SYM_INT()));
      world.InjectSymbiont(new_sym);
    }


    //Loop through updates
      
    if(trial != 0)
      cout << ", ";
    cout << "T-" << trial << ": ";
    cout.flush();
    for (int i = 0; i < numupdates; i++) {
      if(i%(numupdates/10)==0 && i/(numupdates/9) != 0 && i/(numupdates/9) != 10) {
        cout << i/(numupdates/9);
        cout.flush();
      }
      world.Update();
    }
  }
  cout << "]." << endl;
}
