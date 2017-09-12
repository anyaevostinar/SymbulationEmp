// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
using namespace std;

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
  cout << "4 - Print reports." << endl;
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
  	 cout << "Reports not ready yet. I'm had to fix a seg fault!" << endl;
  }
  else if (ControlOption == 9) {
     continue;
  } else {
    cout << "Invalid choice!!" << endl;
  }
  
   } while (ControlOption != 9) ;
}
