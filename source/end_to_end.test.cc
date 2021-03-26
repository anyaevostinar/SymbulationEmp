#include <fstream>
#include <algorithm>
#include <array>
#include <string>
#include "native/symbulation.cc"

using namespace std;

void Test(std::string expected_result_file, int seed = 10, int data_int = 100, double mutation_rate = 0.002, 
    double synergy = 5, double vertical_transmission = 1, double host_int = 0, 
    double sym_int = 0,  int grid_x = 5, int grid_y = 5, int updates = 1, 
    int sym_limit = 1, bool lysis = 0, bool horiz_trans = 0, int burst_size = 10, 
    int burst_time = 10, double host_repro_res = 1000,double sym_lysis_res = 1, 
	  double sym_horiz_trans_res = 100, int start_moi = 1, bool grid = 0) {

  SymConfigBase config;

  config.SEED(seed);                 //What value should the random seed be? If seed <= 0, then it is randomly re-chosen.
  config.DATA_INT(data_int);      //How frequently data should be written
  config.MUTATION_RATE(mutation_rate);     //Standard deviation of the distribution to mutate by
  config.SYNERGY(synergy);               //Amount symbiont's returned resources should be multiplied by
  config.VERTICAL_TRANSMISSION(vertical_transmission); //Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces
  config.HOST_INT(host_int);              //Interaction value from -1 to 1 that hosts should have initially, -2 for random
  config.SYM_INT(sym_int);               //Interaction value from -1 to 1 that symbionts should have initially, -2 for random
  config.GRID_X(grid_x);                //Width of the world, just multiplied by the height to get total size
  config.GRID_Y(grid_y);                //Height of world, just multiplied by width to get total size
  config.UPDATES(updates);               //Number of updates to run before quitting
  config.SYM_LIMIT(sym_limit);             //Number of symbiont allowed to infect a single host
  config.LYSIS(lysis);                 //Should lysis occur? 0 for no, 1 for yes
  config.HORIZ_TRANS(horiz_trans);           //Should non-lytic horizontal transmission occur? 0 for no, 1 for yes
  config.BURST_SIZE(burst_size);           //If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update
  config.BURST_TIME(burst_time);           //If lysis enabled, this is how many updates will pass before lysis occurs
  config.HOST_REPRO_RES(host_repro_res);     //How many resources required for host reproduction
  config.SYM_LYSIS_RES(sym_lysis_res);         //How many resources required for symbiont to create offspring for lysis each update
  config.SYM_HORIZ_TRANS_RES(sym_horiz_trans_res); //How many resources required for symbiont non-lytic horizontal transmission
  config.START_MOI(start_moi);     //How many symbionts per host to start
  config.GRID(grid);                  //Do offspring get placed immediately next to parents on grid, same for symbiont spreading
  config.FILE_PATH("");            //Output file location, leave blank for current folder
  config.FILE_NAME("_Test");       //Root output file name

  GIVEN( string("The configuration {") +
          string(" seed = ") + to_string(seed) + 
	 string(" data_int = ") + to_string(data_int) +
          string(" mutation_rate = ") + to_string(mutation_rate) + 
          string(" synergy = ") + to_string(synergy) + 
          string(" vertical_transmission = ") + to_string(vertical_transmission) + 
          string(" host_int = ") + to_string(host_int) + 
          string(" sym_int = ") + to_string(sym_int) + 
          string(" grid_x = ") + to_string(grid_x) + 
          string(" grid_y = ") + to_string(grid_y) + 
          string(" updates = ") + to_string(updates) + 
          string(" sym_limit = ") + to_string(sym_limit) + 
          string(" lysis = ") + to_string(lysis) + 
          string(" horiz_trans = ") + to_string(horiz_trans) + 
          string(" burst_size = ") + to_string(burst_size) + 
          string(" burst_time = ") + to_string(burst_time) + 
          string(" host_repro_res = ") + to_string(host_repro_res) + 
          string(" sym_lysis_res = ") + to_string(sym_lysis_res) + 
          string(" sym_horiz_trans_res = ") + to_string(sym_horiz_trans_res) +
	 string(" start_moi = ") + to_string(start_moi) +
          string(" grid = ") + to_string(grid) + " }") {
    
    config.Write("SymSettings.cfg");

    THEN( "Symublation runs without error" ) {
      symbulation_main(0, NULL);
    }

    string type;
    for(int i = 0; i < 2; i++) {
      if(i == 0)
        type = "SymVals";
      else
        type = "HostVals";

      string path = "source/end_to_end_test_data/"+type+"_"+expected_result_file+".data";

      THEN( "Symbulation's actual "+type+" output found at \"" + type + "_Test.data\" matches the expected output at \"" + path + "\"" ) {
        ifstream actual, expected;
        actual.open(type + "_Test.data", ios::in);
        expected.open(path, ios::in);
	REQUIRE(actual.is_open());
        REQUIRE(expected.is_open());
      
        //Is this a good length of text to display to the user?
        const int chunk_length = 1000;

        char actual_buffer[chunk_length];
        char expected_buffer[chunk_length];

        while (!(actual.eof() && expected.eof())) {

          actual >> actual_buffer;
          expected >> expected_buffer;

          //This provides a nice error with context when they don't match.
          // array<char, chunk_length> actual_chunk;
          // move(begin(actual_buffer), end(actual_buffer), actual_chunk.begin());
          // array<char, chunk_length> expected_chunk;
          // move(begin(expected_buffer), end(expected_buffer), expected_chunk.begin());
          string actual_chunk = string(actual_buffer);
          string expected_chunk = string(expected_buffer);


          REQUIRE(actual_chunk == expected_chunk);

        }

        actual.close();
        expected.close();
      }//THEN
    }//type loop
  }//GIVEN
}//void Test(.........)

TEST_CASE( "End To End" ) {
  Test("vert_trans_0", 17, 100, 0.002, 5, 0, -2, -2, 100, 100, 1001, 1, 0, 1, 4, 2, 1000, 0.3, 100, 1, 0);
  Test("vert_trans_0.5", 17, 100, 0.002, 5, 0.5, -2, -2, 100, 100, 1001, 1, 0, 1, 4, 2, 1000, 0.3, 100, 1, 0);
  Test("vert_trans_1", 17, 100, 0.002, 5, 1, -2, -2, 100, 100, 1001, 1, 0, 1, 4, 2, 1000, 0.3, 100, 1, 0);
  Test("vert_trans_0.1", 17, 100, 0.002, 5, 0.1, -2, -2, 100, 100, 1001, 1, 0, 1, 4, 2, 1000, 0.3, 100, 1, 0);
}
