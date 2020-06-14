#include <iostream>
#include "../SymWorld.h"
#include "../../Empirical/source/config/ArgManager.h"
#include "web/web.h"
#include "web/d3/visualizations.h"
#include "web/Document.h"

using namespace std;

EMP_BUILD_CONFIG(SymConfigBase,
    VALUE(SEED, int, 10, "What value should the random seed be? If seed <= 0, then it is randomly re-chosen."),
    VALUE(DATA_INT, int, 100, "How frequently, in updates, should data print?"),
    VALUE(MUTATION_RATE, double, 0.002, "Standard deviation of the distribution to mutate by"),
    VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
    VALUE(VERTICAL_TRANSMISSION, double, 1, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
    VALUE(HOST_INT, double, 0, "Interaction value from -1 to 1 that hosts should have initially, -2 for random"),
    VALUE(SYM_INT, double, 0, "Interaction value from -1 to 1 that symbionts should have initially, -2 for random"),
    VALUE(GRID_X, int, 5, "Width of the world, just multiplied by the height to get total size"),
    VALUE(GRID_Y, int, 5, "Height of world, just multiplied by width to get total size"),
    VALUE(UPDATES, int, 1, "Number of updates to run before quitting"),
    VALUE(SYM_LIMIT, int, 1, "Number of symbiont allowed to infect a single host"),
    VALUE(LYSIS, bool, 0, "Should lysis occur? 0 for no, 1 for yes"),
    VALUE(HORIZ_TRANS, bool, 0, "Should non-lytic horizontal transmission occur? 0 for no, 1 for yes"),
    VALUE(BURST_SIZE, int, 10, "If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update"),
    VALUE(BURST_TIME, int, 10, "If lysis enabled, this is how many updates will pass before lysis occurs"),
    VALUE(HOST_REPRO_RES, double, 1000, "How many resources required for host reproduction"),
    VALUE(SYM_LYSIS_RES, double, 1, "How many resources required for symbiont to create offspring for lysis each update"),
    VALUE(SYM_HORIZ_TRANS_RES, double, 100, "How many resources required for symbiont non-lytic horizontal transmission"),
    VALUE(START_MOI, int, 1, "Ratio of symbionts to hosts that experiment should start with"),
    VALUE(GRID, bool, 0, "Do offspring get placed immediately next to parents on grid, same for symbiont spreading"),
    VALUE(FILE_PATH, string, "", "Output file path"),
    VALUE(FILE_NAME, string, "_data_", "Root output file name")
)

namespace UI = emp::web;
UI::Table my_table(10, 2, "my_table");
UI::Document doc("emp_base");


int symbulation_main(int argc, char * argv[]){
    //n_objects(); // some line added to force it to call in order to fix the "js doesn't exist" problem.
    SymConfigBase config;
    config.Read("SymSettings.cfg");
    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(config, std::cout, "SymSettings.cfg") == false) {
      cerr << "There was a problem in processing the options file." << endl;
      exit(1);
    }
    if (args.TestUnknown() == false) {
      cerr << "Leftover args no good." << endl;
      exit(1);
    }
    if (config.BURST_SIZE()%config.BURST_TIME() != 0 && config.BURST_SIZE() < 999999999) {
      cerr << "BURST_SIZE must be an integer multiple of BURST_TIME." << endl;
      exit(1);
    }

    // params
    int numupdates = config.UPDATES();
    int start_moi = config.START_MOI();
    double POP_SIZE = config.GRID_X() * config.GRID_Y();
    bool random_phen_host = true;
    bool random_phen_sym = false;
    if(config.HOST_INT() == -2) random_phen_host = true;
    if(config.SYM_INT() == -2) random_phen_sym = true;

    emp::Random random(config.SEED());
      
    SymWorld world(random); // create the world
    if (config.GRID() == 0) world.SetPopStruct_Mixed(); // needed on UI. Assume it's mixed offspring
    else world.SetPopStruct_Grid(config.GRID_X(), config.GRID_Y());
    // settings
    world.SetVertTrans(config.VERTICAL_TRANSMISSION());
    world.SetMutRate(config.MUTATION_RATE());
    world.SetSymLimit(config.SYM_LIMIT());
    world.SetLysisBool(config.LYSIS());
    world.SetHTransBool(config.HORIZ_TRANS());
    world.SetBurstSize(config.BURST_SIZE());
    world.SetBurstTime(config.BURST_TIME());
    world.SetHostRepro(config.HOST_REPRO_RES());
    world.SetSymHRes(config.SYM_HORIZ_TRANS_RES());
    world.SetSymLysisRes(config.SYM_LYSIS_RES());
    world.SetSynergy(config.SYNERGY());
    world.SetResPerUpdate(100); // number of resources distributed per update

    int TIMING_REPEAT = config.DATA_INT();
    const bool STAGGER_STARTING_BURST_TIMERS = true;

    //inject organisms
    for (size_t i = 0; i < POP_SIZE; i++){
      Host *new_org;
      if (random_phen_host) new_org = new Host(random.GetDouble(-1, 1)); // Want random host genomes
      else new_org = new Host(config.HOST_INT()); // want predefined host genomes
          world.Inject(*new_org); // empirical-provided tool to put our host into the world

      for (int j = 0; j < start_moi; j++){ 
        Symbiont new_sym; 
        if(random_phen_sym) new_sym = *(new Symbiont(random.GetDouble(-1, 1)));
        else new_sym = *(new Symbiont(config.SYM_INT()));
        if(STAGGER_STARTING_BURST_TIMERS)
          new_sym.burst_timer = random.GetInt(-5,5);
        world.InjectSymbiont(new_sym); 
      }
    }

    // Drawing a virtual petri dish according to population size and color cells by intVal(interaction value)
    auto p = world.getPop();
    constexpr int RECT_WIDTH = 15;
    int side_x = config.GRID_X();
    int side_y = config.GRID_Y();
    for (size_t i = 0; i < p.size(); i++) doc << p[i]->GetIntVal() << " ";
    doc << "</br>";
    //auto hostCanvas = doc.AddCanvas(side_x * RECT_WIDTH, side_y * RECT_WIDTH, "can"); 
    auto hostCanvas = doc.AddCanvas(side_x * RECT_WIDTH, side_y * RECT_WIDTH, "can"); // weird behavior of canvas. Have to do +1
    for (int x = 0; x < side_x; x++){ // now draw a virtual petri dish. 20 is the starting coordinate
      for (int y = 0; y < side_y; y++){
        std::string color;
        if (p[y]->GetIntVal() < 0) color = "blue";
        else color = "yellow";
        hostCanvas.Rect(20 + x * RECT_WIDTH, 20 + y * RECT_WIDTH, RECT_WIDTH, RECT_WIDTH, color, "black");
      }
    }

    // for (Host *h : p){
    //     double k = h->GetPoints();
    // }
    // Testing
    // my_table.GetCell(0, 1) << !p[0]->HasSym();
    // my_table.GetCell(0, 0) << "1st org:  ";
    // std::cout << p[0]->GetPoints() << std::endl;
    // doc << "<h1>Symbulation Project</h1>";
    // doc << my_table;

    return 0;
}

#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif