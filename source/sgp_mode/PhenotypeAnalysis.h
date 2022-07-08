#ifndef PHENOTYPE_ANALYSIS_H
#define PHENOTYPE_ANALYSIS_H

#include "../../Empirical/include/emp/Evolve/World.hpp"
#include "../../Empirical/include/emp/data/DataFile.hpp"
#include "../../Empirical/include/emp/Evolve/Systematics.hpp"
#include "../../Empirical/include/emp/math/random_utils.hpp"
#include "../../Empirical/include/emp/math/Random.hpp"
#include "../Organism.h"
#include "Tasks.h"
#include "CPU.h"
#include "SGPHost.h"
#include "SGPWorld.h"
#include <set>
#include <math.h>



void CheckSymbiont(SGPHost host, SGPSymbiont symbiont){
    
    //set the host point to zero
    host.SetPoints(0.0);
    host.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,100);
    std::cout <<"without sym: " << host.GetPoints() << std::endl;
    //set the host point to zero
    host.SetPoints(0.0);
    symbiont.SetPoints(0.0);
    for (int i=0; i<100; i++){
        host.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,1);
        symbiont.GetCPU().RunCPUStep(emp::WorldPosition::invalid_id,1);
    }
    std::cout <<"with sym: " << host.GetPoints() << std::endl;
    std::cout << "=====" << std::endl;
    host.SetPoints(0.0);
    symbiont.SetPoints(0.0);
}





#endif