#include "../SymWorld.h"
#include "../Phage.h"
#include "../Symbiont.h"
#include "../Host.h"
#include "../../../Empirical/include/emp/config/ArgManager.hpp"
#include <iostream>

using namespace std;

// This is the main function for the NATIVE version of this project.



int symbulation_main(int argc, char * argv[])
{    
  SymConfigBase config;

  bool success = config.Read("SymSettings.cfg");
  if(!success) {
    std::cout << "You didn't have a SymSettings.cfg, so one is being written, please try again" << std::endl;
    config.Write("SymSettings.cfg");
  }

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) {
    cerr << "There was a problem in processing the options file." << endl;
    exit(1);
  }
  if (args.TestUnknown() == false) {
    cerr << "Leftover args no good." << endl;
    exit(1);
  }
  if (config.BURST_SIZE()%config.BURST_TIME() != 0 && config.BURST_SIZE() < 999999999) {
  	cerr << "BURST_SIZE must be an integer multiple of BURST_TIME." << endl;
  	exit(1);
  }
// params
  int numupdates = config.UPDATES();
  int start_moi = config.START_MOI();
  int POP_SIZE = config.POP_SIZE();
  if (POP_SIZE == -1) POP_SIZE = config.GRID_X() * config.GRID_Y();
  bool random_phen_host = false;
  bool random_phen_sym = false;
  if(config.HOST_INT() == -2) random_phen_host = true;
  if(config.SYM_INT() == -2) random_phen_sym = true;

  emp::Random random(config.SEED());
      
  SymWorld world(random);

  if (config.GRID() == 0) world.SetPopStruct_Mixed();
  else world.SetPopStruct_Grid(config.GRID_X(), config.GRID_Y());
// settings
  world.SetVertTrans(config.VERTICAL_TRANSMISSION());
  world.SetTotalRes(config.LIMITED_RES_TOTAL());

  world.SetResPerUpdate(config.RES_DISTRIBUTE());

  int TIMING_REPEAT = config.DATA_INT();
  const bool STAGGER_STARTING_BURST_TIMERS = true;

  //Set up files
  //world.SetupPopulationFile().SetTimingRepeat(TIMING_REPEAT);

  if (config.LYSIS() == 1) {
    world.SetupLysisFile(config.FILE_PATH()+"Lysis"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  }
  world.SetupHostIntValFile(config.FILE_PATH()+"HostVals"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);
  world.SetupSymIntValFile(config.FILE_PATH()+"SymVals"+config.FILE_NAME()+".data").SetTimingRepeat(TIMING_REPEAT);

  
  //inject organisms
  for (size_t i = 0; i < POP_SIZE; i++){
    emp::Ptr<Host> new_org;
    if (random_phen_host) new_org.New(&random, &world, &config, random.GetDouble(-1, 1));
    else new_org.New(&random, &world, &config, config.HOST_INT());
    //Currently hacked because there isn't an AddOrg function, but there probably should be
    if(config.GRID()) {
      world.AddOrgAt(new_org, emp::WorldPosition(world.GetRandomCellID()));
    } else {
      world.AddOrgAt(new_org, world.size());
    }
    //world.Inject(*new_org);
  }

  world.Resize(config.GRID_X(), config.GRID_Y()); //if the world wasn't full, creates room for more organisms

  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  int total_syms = POP_SIZE * start_moi;
  for (int j = 0; j < total_syms; j++){
      //TODO: figure out better way of doing the type

      double sym_int = 0;
      if (random_phen_sym) {sym_int = random.GetDouble(-1,1);}
      else {sym_int = config.SYM_INT();}

      if(config.LYSIS() == 1) { 
        emp::Ptr<Phage> new_sym = emp::NewPtr<Phage>(&random, &world, &config, 
           sym_int, 0);
        if(STAGGER_STARTING_BURST_TIMERS) {
          new_sym->SetBurstTimer(random.GetInt(-5,5));
        }
        world.InjectSymbiont(new_sym);
        
      } else {
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&random, &world, &config, 
          sym_int, 0); 
        world.InjectSymbiont(new_sym);
      }
      
  }

  //Loop through updates
  for (int i = 0; i < numupdates; i++) {
    if((i%TIMING_REPEAT)==0) {
      cout <<"Update: "<< i << endl;
      cout.flush();
    }
    world.Update();
  }

  return 0;
}

/*
This definition guard prevents main from being defined twice during testing.
In testing, Catch will define a main function which will initiate tests
(including testing the symbulation_main function above).
*/
#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif
