#include "evo/World.h"
#include "tools/Random.h"
#include <set>
#include "SymOrg.h"

class SymWorld {
 private:
  emp::Random random;
  emp::evo::GridWorld<Host> world;

 public:
  SymWorld() {
  	const int dimX = 3;
  	const int dimY = 3;
  	const int popSize = dimX * dimY;  // full world
    world.ConfigPop(dimX,dimY);
    world.Insert( Host(0.5, Symbiont(), std::set<int>(), 0.0), popSize);
    world.Print(PrintOrg);
  }
  
  void PrintIt() {
     world.Print(PrintOrg);
  }
  
  void Update(size_t new_resources=10) {
  	 // divvy up and distribute resources to host and symbiont in each cell
  	 for (size_t i = 0; i < world.GetSize(); i++) {
  	   if (world.IsOccupied(i) == false) continue;  // no organism at that cell
  	   
  	   // initial disbursement to host and symbiont
  	   size_t sym_portion = world[i].GetIntVal() * new_resources;
//  	   std::cout << "Symbiont gets: " << sym_portion;
  	   size_t host_portion = new_resources - sym_portion;
//  	   std::cout << " and host keeps: " << host_portion << std::endl;
  	   
  	   world[i].AddPoints(host_portion);
	   world[i].GiveSymPoints(sym_portion);
  	   
  	   // symbiont trades value back based on its interaction value
  	   world[i].GetBackPoints(sym_portion);
  	   
  	   }

  	 // test print of resource values
  	 std::cout << std::endl;
  	 std::cout << "Resource values of hosts" << std::endl;
  	 for (size_t i = 0; i < world.GetSize(); i++) {
  	 	if (world.IsOccupied(i) == false) {
  	 	    std::cout << " - "; // no resources to print
  	 	}
  	 	else {
  	 	   double checkPoints;
  	 	   checkPoints = world[i].GetPoints();
  	 		std::cout << " " << checkPoints << " ";
  	    }
  	   
	}  	
	
	// test print of sym resource values
	std::cout << std::endl;
	std::cout << "Resource values of symbionts" << std::endl;
  	 for (size_t i = 0; i < world.GetSize(); i++) {
  	 	if (world.IsOccupied(i) == false) {
  	 	    std::cout << " - "; // no resources to print
  	 	}
  	 	else {
  	 	   double checkPoints;
  	 	   checkPoints = world[i].GetSymbiont().GetPoints();
  	 		std::cout << " " << checkPoints << " ";
  	    }
  	   
	} 

      std::cout << std::endl;
  	 
  	 // host reproduces if it can
  	 // assuming 100% vertical transmission rate right now
  	 for (size_t i = 0; i < world.GetSize(); i++) {
  	 	if (world.IsOccupied(i) == false) continue;  // nothing to replicate!
  	 	
  	 	if (world[i].GetPoints() >= 100 ) {
  	 	   // will replicate & mutate a random offset from parent values
  	 	   // while resetting resource points for host and symbiont to zero
  	 	
           // get random deviation from original interaction values of host and symbiont
           // not sure how large the standard deviation value should be
  	 	   double host_offset = random.GetRandNormal(0.0, 0.5);
  	 	   double sym_offset = random.GetRandNormal(0.0, 0.5);
  	 	//   std::cout << "Testing host offset of: " << host_offset << std::endl;
  	 	//   std::cout << "Testing sym offset of: " << sym_offset << std::endl;
  	 	   double newHostIntVal = world[i].GetIntVal() + host_offset;
  	 	   double newSymIntVal = world[i].GetSymbiont().GetIntVal() + sym_offset;
  	 	   if (newHostIntVal > 1 ) {
  	 	      newHostIntVal = 1.0;
  	 	   } else if (newHostIntVal < 0) {
  	 	      newHostIntVal = 0.0;
  	 	   }
  	 	   
  	 	    if (newSymIntVal > 1 ) {
  	 	      newSymIntVal = 1.0;
  	 	   } else if (newSymIntVal < 0) {
  	 	      newSymIntVal = 0.0;
  	 	   }
  	 	   
  	 	   world[i] = Host(newHostIntVal, Symbiont(), std::set<int>(), 0.0);
  	 	   
  	 	   world[i].SetSymIntVal(newSymIntVal);  
  	 	
  	 	// pick a new world[i] to get a new Host & symbiont with 
  	 	// random deviation from original interaction values of host and symbiont 
  	 	
  	 	// this should work, but it doesn't seem to ever include the end index for the world  
 // 	 int newLoc = random.GetInt(0, world.GetSize() - 1);
 		 int newLoc = random.GetInt(0, world.GetSize());  // try this but guard against going past end
 		 if (newLoc > world.GetSize() - 1) {
 		 	newLoc = world.GetSize() - 1;
 		 	}
  	 	   std::cout << "New Location: " << newLoc << std::endl;
  	 	   world[newLoc] = Host(1.0, Symbiont(), std::set<int>(), 0.0);
  	 	   world[newLoc].SetSymIntVal(1.0);
  	 	   
  	 	}
  	 	
  	 	 // symbiont reproduces independently if it has >= max resources
  	 	
  	 }
  	 
  	
  	 
  	 
  }

};
