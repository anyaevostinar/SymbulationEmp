// This is the main function for the NATIVE version of this project.

#include <iostream>
#include "../SymWorld.h"
using namespace std;

int main()
{

  SymWorld world; 

  int ControlOption = 0;
  cout << "What do you want to do?" << endl;
  cout << "1 - Create a world" << endl;
  cout << "3 - Print the world" << endl;
  cout << "9 - End simulation" << endl;
  cin >> ControlOption;
  
  while (ControlOption != 9) {
  
  if (ControlOption == 1) {
     cout << "The world exists already." << endl;
  }
  else if (ControlOption == 3) {
  world.PrintIt();
  }
  else if (ControlOption == 5) {
//  	cout << "Trying to update resources for each host" << endl;
  	world.Update(10);
  }
  
  cout << endl;
  cout << "What do you want to do?" << endl;
  cout << "1 - Create a world." << endl;
  cout << "3 - Print the world." << endl;
  cout << "5 - Ok, fine, I'll update the world." << endl;
  cout << "6 - Figure out what to have for a snack!" << endl;
  cout << "9 - End simulation" << endl;
  cin >> ControlOption;
  
  }
}
