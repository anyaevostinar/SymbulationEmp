// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
#include "config/ArgManager.h"
using namespace std;

EMP_BUILD_CONFIG( SymConfigBase,
		  VALUE(SEED, double, 10, "What value should the random seed be?"),
		  VALUE(MUTATION_RATE, double, 0.001, "Standard deviation of the distribution to mutate by"),
		  VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
		  VALUE(VERTICAL_TRANSMISSION, double, 1, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
		  VALUE(GRID_X, int, 100, "Width of the world"),
		  VALUE(GRID_Y, int, 100, "Height of world"),
		  VALUE(UPDATES, int, 1000, "Number of updates to run before quitting")
		  )
		  


void DistribResources(int resources) { // might want to declare a remainingResources variable just to make this easier to maintain
	double hostIntVal = 0.0;
	double symIntVal = 0.0;
	
	
	double hostPortion = 0.0;
	double hostDonation = 0.0;
	double symPortion = 0.0;
	double symReturn = 0.0;
	
	cout << "Host interaction value (between -1.0 and 1.0): " << endl;
	cin >> hostIntVal;
	cout << "Symbiont interaction value (between -1.0 and 1.0): " << endl;
	cin >> symIntVal;
	
	cout << "Dividing resources: " << resources << endl;
	if (hostIntVal >= 0 && symIntVal >= 0)  {  
	    hostDonation = resources * hostIntVal;
	    hostPortion = resources - hostDonation;  // I may have shorted the hosts in the first version
	    
	    cout << "Host keeps " << hostPortion << " and gives " << hostDonation << " to symbiont." << endl;
	    
	    symReturn = (hostDonation * symIntVal) * 5;
	    symPortion = hostDonation - (hostDonation * symIntVal);
	    
	    cout << "Symbiont keeps " << symPortion << " and returns " << symReturn << " to host (with bonus)" << endl;
	    
	    hostPortion += symReturn;
	    
	    cout << "In the end, host has " << hostPortion << endl;
	    
	} else if (hostIntVal <= 0 && symIntVal < 0) {
	     double hostDefense = -1.0 * (hostIntVal * resources);
	     cout << "Host invests " << hostDefense << " in defense (which is lost), ";
	     resources = resources - hostDefense;
	     cout << "leaving " << resources << " available for reproduction. " << endl;
	     
	     double symSteals = (hostIntVal - symIntVal) * resources;
	     cout << "Symbiont steals " << symSteals << " resources." << endl;
	     symPortion = symSteals;
	     
	     hostPortion = resources - symSteals;
	     cout << "Leaving host with " << hostPortion << " resources." << endl;
	     
	
	} else if (hostIntVal > 0 && symIntVal < 0) {
		hostDonation = hostIntVal * resources;
		hostPortion = resources - hostDonation;
		cout << "Host donates " << hostDonation << " to symbiont." << endl;
		resources = resources - hostDonation;
		
		double symSteals = -1.0 * (resources * symIntVal);
		hostPortion = hostPortion - symSteals;
		symPortion = hostDonation + symSteals;
		cout << "Symbiont steals an additional " << symSteals << " resources, ";
		cout << "leaving host with " << hostPortion << " resources.  ";
		cout << "Symbiont has " << symPortion << " at end." << endl;
		
		
	} else if (hostIntVal < 0 && symIntVal >= 0) {
		double hostDefense = -1.0 * (hostIntVal * resources);
		hostPortion = resources - hostDefense;
		
		cout << "Host invests " << hostDefense << " in defense against a friendly symbiont." << endl;
		cout << "Host keeps " << hostPortion << " and symbiont gets nothing." << endl;
		// symbiont gets nothing from antagonistic host
	} else {
		cout << "Missed a logical case in distributing resources." << endl;
	}

}
	

int main(int argc, char * argv[])
{

 
  SymConfigBase config;

  //config settings can be accessed by config.KEYWORD_NAME()
  //ex: double seedf = config.SEED();

  SymWorld world; 

  int ControlOption = 0;
  int updateRounds = 1;
  auto args = emp::cl::ArgManager(argc, argv);
  if (args && args[0] == "r") {
    cout << "Research Mode Go!" << endl;
    config.Read("SymSettings.cfg");
    if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) exit(1);
    if (args.TestUnknown() == false) exit(2);  // If there are leftover args, throw an error.
    world.Update(config.UPDATES());
  } else if {
    do {
      cout << endl;
      cout << "Options: " << endl;
      cout << "1 - Configure settings." << endl;
      cout << "2 - Run updates for one (or more) generations." << endl;
      cout << "3 - Print the world." << endl;
      cout << "4 - Test resource behavior cases. " << endl;
      cout << "5 - Print reports." << endl;
      cout << "9 - Quit." << endl;
      cin >> ControlOption;
  
      if (ControlOption == 1) {
	cout << "No controls yet for you" << endl;
      } else if (ControlOption == 2) {
	cout << "How many generations? " << endl;
	cin >> updateRounds;
	for (int i = 0; i < updateRounds; i++) {
	  world.Update(10);
	}
      } else if (ControlOption == 3) {
	world.PrintIt();
      } else if (ControlOption == 4) {
	DistribResources(20);
      } else if (ControlOption == 5) {
	cout << "Reports not ready yet." << endl;
      }
      else if (ControlOption == 9) {
	continue;
      } else {
	cout << "Invalid choice!!" << endl;
      }
      
    } while (ControlOption != 9) ;
}
