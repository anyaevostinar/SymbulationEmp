#include "evo/World.h"
#include "tools/Random.h"
#include <set>
#include "SymOrg.h"


class SymWorld {
 private:
  emp::Random random;
  emp::evo::GridWorld<Host> world;
  
  // declaring world configuration variables that will be initialized with the constructor
  const int dimX;
  const int dimY;
  const int seedf;
  const double muteRate; 
  const double synergy;
  const double vertTrans; 
  

 public:
  SymWorld(int gridx, int gridy, int randomSeed, double mutationProb, double bonus, double transProb)
  :dimX(gridx), dimY(gridy), seedf(randomSeed), muteRate(mutationProb), synergy(bonus), vertTrans(transProb) {
	// variables used only in the constructor
  	const int popSize = (dimX * dimY) / 2;  // number of organisms within the world

  	random.ResetSeed(seedf);
    world.ConfigPop(dimX,dimY);
    world.Insert( Host(0.0, Symbiont(), std::set<int>(), 0.0), popSize);
    // verify configuration
    std::cout << "Current configuration: " << std::endl;
    std::cout << "X: " << dimX << " Y: " << dimY << std::endl;
    std::cout << "Randomizer seed: " << seedf << std::endl;
    std::cout << "Mutation rate: " << muteRate << std::endl;
    std::cout << "Return bonus from symbionts: " << synergy << std::endl;
    std::cout << "Vertical transmission rate: " << vertTrans << std::endl;  
    world.Print(PrintOrg);
  }
  
  void PrintIt() {
     world.Print(PrintOrg);
  }
  
  double newIntVal(double _in = 0.0) {
        // get random deviation from original interaction values of host and symbiont 
  	 	double offset = random.GetRandNormal(0.0, muteRate); 
  	 	
  	 	double newVal = _in + offset;  
  	 	
  	 	 if (newVal > 1.0 ) {
  	 	      newVal = 1.0;
  	 	   } else if (newVal < -1.0) {
  	 	      newVal = -1.0;
  	 	   }
  	 	   
  	 	 return newVal;

}	

bool WillTransmit() {
	if (random.GetRandNormal(0.0, 1.0) <= vertTrans) {
		return true;
	}  else {
		return false;
	}
	

}
  
  void Update(size_t new_resources=10) {
  	 // divvy up and distribute resources to host and symbiont in each cell 
  	 for (size_t i = 0; i < world.GetSize(); i++) {
  	   if (world.IsOccupied(i) == false) continue;  // no organism at that cell
  	   
  	  	double hostIntVal = world[i].GetIntVal();
 // 	  	std::cout << std::endl << "Host interaction: " << hostIntVal << std::endl;
  	  	double symIntVal = 0.0;
  	  	
  	  	if (world[i].HasSym()) {
  	    	symIntVal = world[i].GetSymbiont().GetIntVal();
  	    } else {
  	    	symIntVal = 0.0;
  	    }
  	  	
//  	  	std::cout << "Symbiont interaction: " << symIntVal << std::endl;
		world[i].DistribResources(new_resources, hostIntVal, symIntVal, synergy); // --- USING NEW FUNCTION!!
		
//		std::cout << "Host has: " << world[i].GetPoints() << " resources." << std::endl;
//		std::cout << "Symbiont has: " << world[i].GetSymbiont().GetPoints() << " resources." << std::endl;
  	   
  	   }

 
  	 
  	 // host reproduces if it can
  	 for (size_t i = 0; i < world.GetSize(); i++) {
  	 	if (world.IsOccupied(i) == false) continue;  // nothing to replicate!
  	 	
  	 	if (world[i].GetPoints() >= 100 ) {  // host replication
  	// 	   std::cout << "Testing replication at: " << i << std::endl;
  	 	   // will replicate & mutate a random offset from parent values
  	 	   // while resetting resource points for host and symbiont to zero
 		   double curHostIntVal = world[i].GetIntVal();  // save current host interaction value as basis for 2 new organisms
 		   double curSymIntVal = 0.0;  
 		   bool doVertTrans = false;
 		   bool hadSym = false;   // did the original host have a symbiont?

  	 	   double newHostIntVal = newIntVal(curHostIntVal);
  	 	   double newSymIntVal = 0.0;
  	 	   
  	 	   if (world[i].HasSym()) {
  	 	   		hadSym = true;
  	 	   		curSymIntVal = world[i].GetSymbiont().GetIntVal();
  	 	   		newSymIntVal = newIntVal(curSymIntVal);
  	 	   		doVertTrans = WillTransmit();
  	 	   	}

  	 	   world[i] = Host(newHostIntVal, Symbiont(), std::set<int>(), 0.0); // Possibly safer to use InsertAt()???
  	 	   
  	 	   if (doVertTrans) {
  		 	   world[i].SetSymIntVal(newSymIntVal);  // vertical transmission with possible mutation
  	 	   } else {
  	 	   		world[i].DeleteSym();  // vertical transmission failed
  	 	   }
  	 	
  	 	// pick a new world[i] to get a new Host & symbiont with 
  	 	// random deviation from original interaction values of host and symbiont 
  	 	
  	 	// this should work, but it doesn't seem to ever include the end index for the last possible cell 
 // 	 int newLoc = random.GetInt(0, world.GetSize() - 1);
 		 int newLoc = random.GetInt(0, world.GetSize());  // try this but guard against going past end
 		// std::cout << "New location: " << newLoc << std::endl;
 		 if (newLoc == world.GetSize()) {
 		 	newLoc = world.GetSize() - 1;
// 		 	std::cout << "Tried to go out of bounds." << std::endl;
 		 }
 		 	
 		  // std::cout << "New Location: " << newLoc << std::endl;
 		   newHostIntVal = newIntVal(curHostIntVal);
  	 	   world.InsertAt(Host(newHostIntVal, Symbiont(), std::set<int>(), 0.0), newLoc);
  	 	    		   
 		   if (hadSym) {
 		   		doVertTrans = WillTransmit();  // test vertical transmission of symbiont
 		   		if (doVertTrans) {
  	 	   			newSymIntVal = newIntVal(curSymIntVal);
  	  	 		    world[newLoc].SetSymIntVal(newSymIntVal); 	
  	  	 		    }
  	  	 		else {
  	  	 			world[newLoc].DeleteSym();  // transmission failed
  	  	 		}   		
  	 	   	}
  	 	   	else {
  	 	   		world[newLoc].DeleteSym();   // there was nothing to transmit
  	 	   	}	


  	 	   
  	 	}
  	 	else if (world[i].HasSym() && world[i].GetSymbiont().GetPoints() >= 100) {  
  	 	 // symbiont reproduces independently (horizontal transmission) if it has >= 100 resources
  	 	 // new symbiont in this host with mutated value
  	 	 double newSymIntVal = newIntVal(world[i].GetSymbiont().GetIntVal());
  	 	 world[i].SetSymIntVal(newSymIntVal);
  	 	 world[i].ResetSymPoints();
  	 	 
  	 	 // pick new host to infect, if one exists at the new location and does NOT already have a symbiont
  	 	 int newLoc = random.GetInt(0, world.GetSize());  // try this but guard against going past end
 		 if (newLoc == world.GetSize()) {
 		 	newLoc = world.GetSize() - 1;
// 		 	std::cout << "Tried to go out of bounds." << std::endl;
 		 }
 		 
// 		 std::cout << "Symbiont tried to replicate on its own from " << i << " to " << newLoc;
 		 if (world.IsOccupied(newLoc) == true) {
 		   
 		   if (world[newLoc].HasSym()) {
// 		   	  std::cout << " and failed because host already has a symbiont." << std::endl;
 		   } else {
 		     newSymIntVal = newIntVal(world[i].GetSymbiont().GetIntVal());
 		     world[newLoc].SetSymIntVal(newSymIntVal);
 		     world[newLoc].ResetSymPoints();
// 		     std::cout << " and succeeded." << std::endl;
 		   }
  	 	 } else {
//  	 	 	std::cout << "Symbiont tried to replicate on its own but failed to find a host at " << newLoc << std::endl;
  	 	 }
  	 	 
 }
  	 	
  	 	
  	 }
  	 
  	 
  }
  

};


