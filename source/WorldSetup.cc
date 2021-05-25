#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"
#include "Phage.h"
#include "ConfigSetup.h"

void worldSetup(emp::Ptr<SymWorld> world, emp::Ptr<SymConfigBase> my_config) {
// params
  emp::Random& random = world->GetRandom();

  int start_moi = my_config->START_MOI();
  int POP_SIZE = my_config->POP_SIZE();
  if (POP_SIZE == -1) POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  bool random_phen_host = false;
  bool random_phen_sym = false;
  if(my_config->HOST_INT() == -2) random_phen_host = true;
  if(my_config->SYM_INT() == -2) random_phen_sym = true;

  if (my_config->GRID() == 0) world->SetPopStruct_Mixed();
  else world->SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y());
// settings
  world->SetVertTrans(my_config->VERTICAL_TRANSMISSION());
  world->SetTotalRes(my_config->LIMITED_RES_TOTAL());

  world->SetResPerUpdate(my_config->RES_DISTRIBUTE());
  const bool STAGGER_STARTING_BURST_TIMERS = true;

  //inject organisms
  for (size_t i = 0; i < POP_SIZE; i++){
    emp::Ptr<Host> new_org;
    if (random_phen_host) new_org.New(&random, world, my_config, random.GetDouble(-1, 1));
    else new_org.New(&random, world, my_config, my_config->HOST_INT());
    //Currently hacked because there isn't an AddOrg function, but there probably should be
    if(my_config->GRID()) {
      world->AddOrgAt(new_org, emp::WorldPosition(world->GetRandomCellID()));
    } else {
      world->AddOrgAt(new_org, world->size());
    }
    //world.Inject(*new_org);
  }

  world->Resize(my_config->GRID_X(), my_config->GRID_Y()); //if the world wasn't full, creates room for more organisms

  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  int total_syms = POP_SIZE * start_moi;
  for (int j = 0; j < total_syms; j++){
      //TODO: figure out better way of doing the type

      double sym_int = 0;
      if (random_phen_sym) {sym_int = random.GetDouble(-1,1);}
      else {sym_int = my_config->SYM_INT();}

      if(my_config->LYSIS() == 1) { 
        emp::Ptr<Phage> new_sym = emp::NewPtr<Phage>(&random, world, my_config, 
           sym_int, 0);
        if(STAGGER_STARTING_BURST_TIMERS) {
          new_sym->SetBurstTimer(random.GetInt(-5,5));
        }
        world->InjectSymbiont(new_sym);
        
      } else {
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&random, world, my_config, 
          sym_int, 0); 
        world->InjectSymbiont(new_sym);
      }
      
  }

}