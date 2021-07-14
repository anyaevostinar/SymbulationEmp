#ifndef WORLD_SETUP_C
#define WORLD_SETUP_C

#include "SymWorld.h"
#include "Host.h"
#include "Symbiont.h"
#include "Phage.h"
#include "ConfigSetup.h"
#include "EfficientSymbiont.h"

void worldSetup(emp::Ptr<SymWorld> world, emp::Ptr<SymConfigBase> my_config) {
// params
  emp::Random& random = world->GetRandom();

  double start_moi = my_config->START_MOI();
  int POP_SIZE = my_config->POP_SIZE();
  if (POP_SIZE == -1) POP_SIZE = my_config->GRID_X() * my_config->GRID_Y();
  bool random_phen_host = false;
  bool random_phen_sym = false;
  if(my_config->HOST_INT() == -2 && !my_config->COMPETITION_MODE()) random_phen_host = true;
  if(my_config->SYM_INT() == -2) random_phen_sym = true;

  if(my_config->EFFICIENCY_MUT_RATE() == -1) my_config->EFFICIENCY_MUT_RATE(my_config->HORIZ_MUTATION_RATE());

  if (my_config->GRID() == 0) {world->SetPopStruct_Mixed(false);}
  else world->SetPopStruct_Grid(my_config->GRID_X(), my_config->GRID_Y(), false);
// settings
  world->SetVertTrans(my_config->VERTICAL_TRANSMISSION());
  world->SetTotalRes(my_config->LIMITED_RES_TOTAL());
  world->SetFreeLivingSyms(my_config->FREE_LIVING_SYMS());

  world->SetResPerUpdate(my_config->RES_DISTRIBUTE());
  const bool STAGGER_STARTING_BURST_TIMERS = true;
  double comp_host_1 = 0;
  double comp_host_2 = 0.95;



  //inject organisms
  for (size_t i = 0; i < POP_SIZE; i++){
    emp::Ptr<Host> new_org;
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
  double infection_chance = my_config->SYM_INFECTION_CHANCE();
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
        new_sym->SetInfectionChance(infection_chance);
        world->InjectSymbiont(new_sym);

      } else if (my_config->EFFICIENT_SYM()) {
        emp::Ptr<EfficientSymbiont> new_sym = emp::NewPtr<EfficientSymbiont>(&random, world, my_config, sym_int, 0, 1);
        new_sym->SetInfectionChance(infection_chance);
        world->InjectSymbiont(new_sym);
      } else {
        emp::Ptr<Symbiont> new_sym = emp::NewPtr<Symbiont>(&random, world, my_config,
          sym_int, 0);
        new_sym->SetInfectionChance(infection_chance);
        world->InjectSymbiont(new_sym);
      }

  }
}

#endif
