#ifndef PGG_WORLD_SETUP_C
#define PGG_WORLD_SETUP_C

#include "PGGWorld.h"
#include "../ConfigSetup.h"
#include "PGGHost.h"
#include "PGGSymbiont.h"

void worldSetup(emp::Ptr<PGGWorld> world, emp::Ptr<SymConfigBase> my_config) {
// params
  emp::Random& random = world->GetRandom();

  double start_moi = my_config->START_MOI();
  long unsigned int POP_SIZE;
  if (my_config->POP_SIZE() == -1) {
    POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  } else {
    POP_SIZE = my_config->POP_SIZE();
  }
  bool random_phen_host = false;
  bool random_phen_sym = false;
  if(my_config->HOST_INT() == -2 && !my_config->COMPETITION_MODE()) random_phen_host = true;
  if(my_config->SYM_INT() == -2) random_phen_sym = true;

  if (my_config->GRID() == 0) {world->SetPopStruct_Mixed(false);}
  else world->SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);
// settings

  world->SetTotalRes(my_config->LIMITED_RES_TOTAL());
  world->SetFreeLivingSyms(my_config->FREE_LIVING_SYMS());
  world->SetMoveFreeSyms(my_config->MOVE_FREE_SYMS());
  world->SetTrackPhylogeny(my_config->PHYLOGENY());
  world->SetNumPhyloBins(my_config->NUM_PHYLO_BINS());
  world->SetResPerUpdate(my_config->RES_DISTRIBUTE());
  double comp_host_1 = 0;
  double comp_host_2 = 0.95;

  //inject pgg hosts
  for (size_t i = 0; i < POP_SIZE; i++){
    emp::Ptr<PGGHost> new_org;

    if (random_phen_host) {new_org.New(&random, world, my_config, random.GetDouble(-1, 1));
    } else if (my_config->COMPETITION_MODE() && i%2==0) {
        new_org.New(&random, world, my_config, comp_host_1);
    } else if (my_config->COMPETITION_MODE() && i%2==1) {
        new_org.New(&random, world, my_config, comp_host_2);
    } else { new_org.New(&random, world, my_config, my_config->HOST_INT());
    }
    //Currently hacked because there isn't an AddOrg function, but there probably should be
    if(my_config->GRID()) {
      world->AddOrgAt(new_org, emp::WorldPosition(world->GetRandomCellID()));
    } else {
      world->AddOrgAt(new_org, world->size());
    }
    //world.Inject(*new_org);
  }

  //sets up the world size
  world->Resize(my_config->GRID_X(), my_config->GRID_Y());

  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  int total_syms = POP_SIZE * start_moi;
  for (int j = 0; j < total_syms; j++){
      double sym_int = 0;
      if (random_phen_sym) {sym_int = random.GetDouble(-1,1);}
      else {sym_int = my_config->SYM_INT();}

      double sym_donation = my_config->PGG_DONATE();
      emp::Ptr<PGGSymbiont> new_sym = emp::NewPtr<PGGSymbiont>(&random, world, my_config,
          sym_int,sym_donation,0);
      world->InjectSymbiont(new_sym);
  }
}

#endif
