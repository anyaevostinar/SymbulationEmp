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
                 VALUE(UPDATES, int, 1000, "Number of updates to run before quitting"),
                 VALUE(UPDATE_RESOURCES, int, 10, "Number of resources to distribute between host and symbiont at each update"),
                 VALUE(MODE, char, 'r', "Simulation mode. To read from config file, use -MODE r")
                 )


int main(int argc, char * argv[])
{
  
    
    SymConfigBase config;
    
    //config settings can be accessed by config.KEYWORD_NAME()
    //ex: double seedf = config.SEED();
    
    // getting the default settings
    double seedf = config.SEED();
    double mutrate = config.MUTATION_RATE();
    double symsyn = config.SYNERGY();
    double vertrans = config.VERTICAL_TRANSMISSION();
    double gridx = config.GRID_X();
    double gridy = config.GRID_Y();
    double numupdates = config.UPDATES();
    int updateresources = config.UPDATE_RESOURCES();
    char simmode = config.MODE();
    
  char ChangeSetting = 'N';
  int ControlOption = 0;
  int updateRounds = numupdates;
  
  
    auto args = emp::cl::ArgManager(argc, argv);
    
    if (argc < 2) { // interactive mode
            cout << endl << endl;
            cout << "Initializing Symbulation (Testing)!" << endl << endl;
            cout << "Current settings are: " << endl;
            cout << "Seed for randomizer: " << seedf << endl;
            cout << "Mutation rate: " << mutrate << endl;
            cout << "The multiplier for resource amount symbiont returns to host: " << symsyn << endl;
            cout << "Vertical transmission probability (between 0 and 1): " << vertrans << endl;
            cout << "Width of the world: " << gridx << endl;
            cout << "Height of the world: " << gridy << endl << endl;
            cout << "Number of resources given to each host at each update: " << updateresources << endl;
            cout << "Do you want to change a setting? (Y/N) " << endl;
            
            cin >> ChangeSetting;
            
            if (ChangeSetting == 'y' || ChangeSetting == 'Y') {
  	         	 do {
  	         	 	cout << "Which option do you want to change??" << endl;
  	         	 	cout << "1 - Seed for randomizer (" << seedf << ")"<< endl;
          			cout << "2 - Mutation rate (" << mutrate << ")" << endl;
           			cout << "3 - The MULTIPLIER for resource amount symbiont returns to host: " << symsyn << endl;
         			cout << "4 - Vertical transmission probability (between 0 and 1): " << vertrans << endl;
          			cout << "5 - Width of the world: " << gridx << endl;
       			 	cout << "6 - Height of the world: " << gridy << endl;
       			 	cout << "7 - Number of resources per update per host: " << updateresources << endl;
       			 	cout << "9 - Done changing settings." << endl << endl;
            			
            	   cin >> ControlOption;
            	   
            	   if (ControlOption == 1) {
            	   		cout << "Enter new seed for randomizer: " << endl;
            	   		cin >> seedf;
            	   } else if (ControlOption == 2) {
            	   		cout << "Enter mutation rate: " << endl;
            	   		cin >> mutrate;
            	   } else if (ControlOption == 3) {
            	   		cout << "Enter returned resource multiplier: " << endl;
            	   		cin >> symsyn;
            	   } else if (ControlOption == 4) {
            	   		cout << "Enter vertical transmission probability: " << endl;
            	   		cin >> vertrans;
            	   } else if (ControlOption == 5) {
            	   		cout << "Enter new width size: " << endl;
            	   		cin >> gridx;
            	   } else if (ControlOption == 6) {
            	   		cout << "Enter new height size: " << endl;
            	   		cin >> gridy;
            	   } else if (ControlOption == 7 ) {
            	   		cout << "Enter new resource amount: " << endl;
            	   		cin >> updateresources;
            	   } else if (ControlOption == 9) {
            	   		continue;
            	   } else {
            	   		cout << "Please enter a valid option." << endl;
            	   }
            	   
	           	 } while (ControlOption != 9);
            
            }
            
            ControlOption = 0;
            cout << "Creating World!!" << endl << endl;
            SymWorld world(gridx, gridy, seedf, mutrate, symsyn, vertrans); 
    
    	do {  

            cout << "Options: " << endl;
            cout << "1 - Configure settings and create world." << endl;
            cout << "2 - Run updates for one (or more) generations." << endl;
            cout << "3 - Print the world." << endl;
            cout << "5 - Print reports." << endl;
            cout << "9 - Quit." << endl;
            cin >> ControlOption;
            
            if (ControlOption == 1) {
                cout << "Actually, I think this is too late to configure." << endl;
                
            } else if (ControlOption == 2) {
                cout << "How many generations?? " << endl;
                cin >> updateRounds;
                for (int i = 0; i < updateRounds; i++) {
                    world.Update(updateresources);
                }
            } else if (ControlOption == 3) {
                world.PrintIt();
            } else if (ControlOption == 5) {
                cout << "Reports not ready yet." << endl;
            }
            else if (ControlOption == 9) {
                continue;
            } else {
                cout << "Invalid choice!" << endl;
            }
            
        } while (ControlOption != 9) ;
    
    } else if (argc > 1) {  // research mode
        cout << "Research Mode Go!" << endl;
        config.Read("SymSettings.cfg");
        if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) {
         	cout << "There was a problem in processing the options file." << endl;
         	exit(1);
         }
        if (args.TestUnknown() == false) {
        	cout << "Issue with leftover arguments." << endl;
        	exit(2); 
        } // If there are leftover args, throw an error.
        
        // good to go
        
        // get updated settings from the file 
		seedf = config.SEED();
 		mutrate = config.MUTATION_RATE();
		symsyn = config.SYNERGY();
		vertrans = config.VERTICAL_TRANSMISSION();
		gridx = config.GRID_X();
		gridy = config.GRID_Y();
		numupdates = config.UPDATES();
		updateresources = config.UPDATE_RESOURCES();
    	simmode = config.MODE();
        
            cout << endl << endl;
            cout << "Initializing Symbulation!" << endl << endl;
            cout << "Current settings are:" << endl;
            cout << "Seed for randomizer: " << seedf << endl;
            cout << "Mutation rate: " << mutrate << endl;
            cout << "The multiplier for resource amount symbiont returns to host: " << symsyn << endl;
            cout << "Vertical transmission probability (between 0 and 1): " << vertrans << endl;
            cout << "Width of the world: " << gridx << endl;
            cout << "Height of the world: " << gridy << endl;
            cout << "Number of resources per host per update: " << updateresources << endl;
        
        cout << endl << "Initial world population:" << endl << endl;
        SymWorld world(gridx, gridy, seedf, mutrate, symsyn, vertrans); 
        
        for (int i = 0; i < numupdates; i++) {
            world.Update(updateresources);
            }
        cout << endl << "World after " << numupdates << " generations." << endl << endl;
        world.PrintIt();
    } else {
		cout << "Unexpected command line argument.  Exiting now."  << endl;
    }
    
    }
