#include "SGPHost.h"
#include "SGPSymbiont.h"
#include "SGPWorld.h"


void CheckSymbiont(emp::WorldPosition location, SGPHost host, SGPSymbiont symbiont){
    host.SetPoints(0.0);
    host.getCpu().runCpuStep(emp::WorldPosition::invalid_id/*what is this actually*/,100);
    std::cout <<"without sym: " << host.GetPoints() << std::endl;
    host.SetPoints(0.0);
    for (int i=0; i<100; i++){
        host.getCpu().runCpuStep(location,1);
        symbiont.getCpu().runCpuStep(location,1);
    }
    std::cout <<"with sym: " << host.GetPoints() << std::endl;
    std::cout << "=====" << std::endl;
}