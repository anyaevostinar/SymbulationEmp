// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
using namespace std;


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
	

int main()
{

  SymWorld world; 

  int ControlOption = 0;
  int updateRounds = 1;
  
  do {
  cout << endl;
  cout << "Options: " << endl;
  cout << "1 - Set configurations." << endl;
  cout << "2 - Run updates for one (or more) generations." << endl;
  cout << "3 - Print the world." << endl;
  cout << "4 - Test resource behavior cases. " << endl;
  cout << "5 - Print reports." << endl;
  cout << "9 - Quit." << endl;
  cin >> ControlOption;
  
  if (ControlOption == 1) {
  	cout << "Not ready for user configuration yet!" << endl;
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
