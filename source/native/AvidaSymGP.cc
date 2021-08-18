#include <iostream>
#include <vector>

#include "../../Empirical/include/emp/hardware/AvidaGP.hpp"
#include "../../Empirical/include/emp/hardware/InstLib.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../AvidaGPHost.hpp"
#include "../AvidaGPSymbio.hpp"

//is this SymWorld.h?
#include "../AvidaGPWorld.h"



void Print(const GPHost & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 100;

int main()
{
  emp::Random random;
  AvidaGPWorld world(random);
  world.SetPopStruct_Mixed(true);

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    GPHost cpu;
    cpu.PushRandom(random, GENOME_SIZE);
    world.Inject(cpu.GetGenome(),1);
    emp::vector<emp::Ptr<GPSymbiont>> symbiont_vec;
    GPSymbiont sb;
    sb.PushRandom(random, GENOME_SIZE);
    symbiont_vec.push_back(&sb);
    world.GetOrg(i).SetSymbio(symbiont_vec);
  }

  // Setup the mutation function.
  world.SetMutFun( [](GPHost & org, emp::Random & random) {

      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      uint32_t num_muts_for_syms = random.GetUInt(4);
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org.RandomizeInst(pos, random);
        for (int i = 0; i < org.GetSymbio().size(); i++){
          emp::vector<emp::Ptr<GPSymbiont>> symbiont_vec = org.GetSymbio();
          symbiont_vec[i]->RandomizeInst(pos, random);
        }
      }
      return num_muts;
    } );

  world.GetOrg(0).GetSymbio()[0]->PrintGenome();
  // setting symbioint in affecting hosts' fitness!!!!!!!!!!!!!!!!!!!
  // Setup the fitness function.
  std::function<double(const GPHost &)> fit_fun =
    [](const GPHost &org) {
    int count = 0; 

    //points is inherited from the AvidaGPHost class, but this function didn't have a variable previously defined with the host class, so I used symbiont_vec but I also understand this is incorrect and is giving me an error 
    //add points to count
    count += org.GetPoints();

    /*//add symbio to count
    //addsymbio void function, but adds one symbiont to count
    if (org.AddSymbio(org)){
      count += 1;
      cout << "Count: " << count;
    };*/
    

    for (int i = 0; i < 16; i++) {
      if (org.GetOutput(i) == (double) (i*i)) count++;
    }
    return (double) count;
  };
  world.SetFitFun(fit_fun);

/*
  emp::vector< std::function<double(const Host &)> > fit_set(16);
  for (size_t out_id = 0; out_id < 16; out_id++) {
    // Setup the fitness function.
    fit_set[out_id] = [out_id](const Host & org) {
      return (double) -std::abs(org.GetOutput((int)out_id) - (double) (out_id * out_id));
    };
  }
*/

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Update the status of all organisms.
    world.ResetHardware();
    world.Process(200);
    double fit0 = world.CalcFitnessID(0);

    std::cout << (ud+1) << " : " << 0 << " : " << fit0 << std::endl;
    // Keep the best individual.
    AvidaGPEliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    AvidaGPTournamentSelect(world, 5 , POP_SIZE-1);
    //LexicaseSelect(world, fit_set, POP_SIZE-1);
    // EcoSelect(world, fit0_fun, fit_set, 100, 5, POP_SIZE-1);

    world.Update();

    // // Mutate all but the first organism.
    world.DoMutations(1);
  }

  std::cout << "next line is: print func: " << std::endl;
  world.GetOrg(0).GetSymbio()[0]->PrintGenome();
  world.GetOrg(0).Process(200);
  std::cout << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;
  std::cout << world.GetOrg(0).GetSymbio().size() << std::endl;
  return 0;
}