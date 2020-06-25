#include <iostream>
#include "../SymWorld.h"
#include "../../Empirical/source/config/ArgManager.h"
#include "../Web.h"

using namespace std;

EMP_BUILD_CONFIG(SymConfigBase,
    VALUE(SEED, int, 17, "What value should the random seed be? If seed <= 0, then it is randomly re-chosen."),
    VALUE(DATA_INT, int, 100, "How frequently, in updates, should data print?"),
    VALUE(MUTATION_RATE, double, 0.002, "Standard deviation of the distribution to mutate by"),
    VALUE(SYNERGY, double, 5, "Amount symbiont's returned resources should be multiplied by"),
    VALUE(VERTICAL_TRANSMISSION, double, 0, "Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces"),
    VALUE(HOST_INT, double, -2, "Interaction value from -1 to 1 that hosts should have initially, -2 for random"),
    VALUE(SYM_INT, double, -2, "Interaction value from -1 to 1 that symbionts should have initially, -2 for random"),
    VALUE(GRID_X, int, 50, "Width of the world, just multiplied by the height to get total size"),
    VALUE(GRID_Y, int, 50, "Height of world, just multiplied by width to get total size"),
    VALUE(UPDATES, int, 1001, "Number of updates to run before quitting"),
    VALUE(SYM_LIMIT, int, 1, "Number of symbiont allowed to infect a single host"),
    VALUE(LYSIS, bool, 0, "Should lysis occur? 0 for no, 1 for yes"),
    VALUE(HORIZ_TRANS, bool, 1, "Should non-lytic horizontal transmission occur? 0 for no, 1 for yes"),
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
SymConfigBase config;

class MyAnimate : public UI::Animate {
private:
  UI::Document doc;
  UI::Text em_vert_trans{"em_vert_trans"};
  UI::Text em_grid{"em_grid"};

  // Define world and population
  size_t POP_SIZE = config.GRID_X() * config.GRID_Y();
  size_t GENS = 10000;
  const size_t POP_SIDE = (size_t) std::sqrt(POP_SIZE);
  emp::Random random{config.SEED()};
  SymWorld world{random};
  int numupdates = config.UPDATES();
  double vert_transmit = config.VERTICAL_TRANSMISSION();
  bool grid = config.GRID();
  emp::vector<emp::Ptr<Host>> p;

  // Params for controlling petri dish
  int side_x = config.GRID_X();
  int side_y = config.GRID_Y();
  const int offset = 20;
  const int RECT_WIDTH = 10;
  int can_size = offset + RECT_WIDTH * POP_SIDE; // set canvas size to be just enough to incorporate petri dish

  // params for controlling textarea input
  bool empty_vert = false;
  bool empty_grid = false;

public:

  MyAnimate() : doc("emp_base") {
    initializeWorld();
    // ----------------------- Input field for modifying the vertical transmission rate -----------------------
    doc << "<b>See what happens at different vertical transmission rates!<br>Please type in a vertical transmisson rate between 0 and 1, then click Reset: </b><br>";
    doc.AddTextArea([this](const std::string & in){
      bool isValidInput = true;
      for (char c : in){
        if (c == 46) continue; // "." is part of a double, skip
        else if (c < 48 || c > 57){ isValidInput = false; break; } // check for valid input string (must be a double <= 1)
      }
      if (in.empty()) { 
        em_vert_trans.SetCSS("opacity", "0");  // set empty_vert so nothing's printed
        empty_vert = true; doc.Text("vert_trans_txt").Redraw();
      } 
      //TO DO: make stod(in) be called only once
      else if (isValidInput && stod(in) <= 1) {
        em_vert_trans.SetCSS("opacity", "0");
        vert_transmit = stod(in); 
        doc.Text("vert_trans_txt").Redraw(); 
        empty_vert = false;
      }
      else { em_vert_trans.SetCSS("opacity", "1"); } // turn on error message
    }, "update_vert_transmit");
    doc << em_vert_trans; 
    doc << "<br>";
    doc << UI::Text("vert_trans_txt") << "Vertical Transmission Rate = " << 
      UI::Live( [this](){ return empty_vert ? "" : std::to_string(vert_transmit); } );


    // ----------------------- Input field for changing the grid setting -----------------------
    doc << "<br>";
    doc << "<b>See how global versus local reproduction changes evolution! <br>Please type in 0 for global reproduction  or 1 for local reproduction, then click Reset: </b><br>";
    doc.AddTextArea([this](const std::string & in){
      bool isValidInput = (in.size() == 1 && (in == "0" || in == "1")); // input must be either "0" or "1"
      if (in.empty()) { 
        em_grid.SetCSS("opacity", "0"); 
        empty_grid = true; // set empty_grid so nothing's printed
        doc.Text("grid_txt").Redraw(); 
      } 
      else if (isValidInput) {
        em_grid.SetCSS("opacity", "0"); 
        grid = stoi(in); 
        doc.Text("grid_txt").Redraw(); 
        empty_grid = false;
      }
      else { em_grid.SetCSS("opacity", "1"); } // turn on error message
    }, "update_grid");
    doc << em_grid;
    doc << "<br>";
    doc << UI::Text("grid_txt") << "Reproduction location = " << 
      UI::Live( [this](){ return grid; } );


    // ----------------------- Error message settings -----------------------
    em_vert_trans << "Invalid Input!";
    em_vert_trans.SetCSS("color", "red");
    em_vert_trans.SetCSS("opacity", "0");

    em_grid << "Invalid Input!";
    em_grid.SetCSS("color", "red");
    em_grid.SetCSS("opacity", "0");

    // Add explanation for organism color:
    doc << "<br><br>Blue squares are hosts that are defensive against a parasitic symbiont" <<
      "<br> Yellow squares are hosts that are cooperating with a mutualistic symbiont <br>" <<
      "Blue dots are symbionts that steal resources from the host<br>" <<
      "Yellow dots are symbionts that are cooperating with a mutualistic host <br>";

    // ----------------------- Add a button that allows for pause and start toggle -----------------------
    doc << "<br>";
    doc.AddButton([this](){
      // animate up to the number of updates
      ToggleActive();
      auto but = doc.Button("toggle"); 
      if (GetActive()) but.SetLabel("Pause");
      else but.SetLabel("Start");
    }, "Start", "toggle");

    // ----------------------- Add a reset button to reset the animation/world -----------------------
    /* Note: Must first run world.Reset(), because Inject checks for valid position.
      If a position is occupied, new org is deleted and your world isn't reset.
      Also, canvas must be redrawn to let users see that it is reset */
    doc.AddButton([this](){
      world.Reset();
      doc.Text("update").Redraw();
      initializeWorld();
      p = world.getPop();
    
      if (GetActive()) { // If animation is running, stop animation and adjust button label
        ToggleActive();   
      }
      auto but = doc.Button("toggle"); 
      but.SetLabel("Start"); 

      // redraw petri dish
      auto mycanvas = doc.Canvas("can");
      drawPetriDish(mycanvas);
    }, "Reset");


    // ----------------------- Keep track of number of updates -----------------------
    doc << UI::Text("update") << "Update = " << UI::Live( [this](){ return world.GetUpdate(); } );
    doc << "<br>";

    // Add a canvas for petri dish and draw the initial petri dish
    auto mycanvas = doc.AddCanvas(can_size, can_size, "can");
    targets.push_back(mycanvas);
    drawPetriDish(mycanvas);
    doc << "<br>";

    doc << "If you'd like to learn more, please see the publication <a href=\"https://www.mitpressjournals.org/doi/abs/10.1162/artl_a_00273\">Spatial Structure Can Decrease Symbiotic Cooperation</a>.";

  }

  void initializeWorld(){
     // Reset the seed and the random machine of world to ensure consistent result (??)
    random.ResetSeed(config.SEED());
    world.SetRandom(random);

    // params
    int start_moi = config.START_MOI();
    bool random_phen_host = false;
    bool random_phen_sym = false;
    if(config.HOST_INT() == -2) random_phen_host = true;
    if(config.SYM_INT() == -2) random_phen_sym = true;

    if (grid == 0) world.SetPopStruct_Mixed();
    else world.SetPopStruct_Grid(config.GRID_X(), config.GRID_Y());

    // settings
    world.SetVertTrans(vert_transmit);
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
    world.SetResPerUpdate(100); 

    int TIMING_REPEAT = config.DATA_INT();
    const bool STAGGER_STARTING_BURST_TIMERS = true;

  // Initialize organisms by injecting them
  for (size_t i = 0; i < POP_SIZE; i++){
    Host *new_org;
    if (random_phen_host) new_org = new Host(random.GetDouble(-1, 1));
    else new_org = new Host(config.HOST_INT());
    world.Inject(*new_org);
  }

  //This loop must be outside of the host generation loop since otherwise
  //syms try to inject into mostly empty spots at first
  int total_syms = POP_SIZE * start_moi;
  for (int j = 0; j < total_syms; j++){ 
      Symbiont new_sym; 
      if(random_phen_sym) new_sym = *(new Symbiont(random.GetDouble(-1, 1)));
      else new_sym = *(new Symbiont(config.SYM_INT()));
      if(STAGGER_STARTING_BURST_TIMERS)
        new_sym.burst_timer = random.GetInt(-5,5);
      world.InjectSymbiont(new_sym); 
    }
    p = world.getPop();
  }

  // now draw a virtual petri dish with coordinate offset from the left frame
  void drawPetriDish(UI::Canvas & can){
        int i = 0;
        for (int x = 0; x < side_x; x++){ 
            for (int y = 0; y < side_y; y++){
                std::string color_host;
                std::string color_sym;
                // color setting for host
                if (p[i]->GetIntVal() < 0) color_host = "blue";
                else color_host = "yellow";
                // color setting for symbiont
                emp::vector<Symbiont>& syms = p[i]->GetSymbionts(); // retrieve all syms for this host (assume only 1 sym for each host)
                if (syms[0].GetIntVal() < 0) color_sym = "blue";
                else color_sym = "yellow";
                // Draw the host and symbiont
                can.Rect(offset + x * RECT_WIDTH, offset + y * RECT_WIDTH, RECT_WIDTH, RECT_WIDTH, color_host, "black");
                int radius = RECT_WIDTH / 4;
                can.Circle(offset + x * RECT_WIDTH + RECT_WIDTH/2, offset + y * RECT_WIDTH + RECT_WIDTH/2, radius, color_sym, "black");

                // Molly's code to increase colors
                // std::string color;
                // if (p[i]->GetIntVal() >= -1 && p[i]->GetIntVal() < -0.667) color = "navy";
                // if (p[i]->GetIntVal() >= -.667 && p[i]->GetIntVal() < -0.333) color = "blue";
                // if (p[i]->GetIntVal() >= -.0333 && p[i]->GetIntVal() < 0) color = "purple";
                // if (p[i]->GetIntVal() >= 0 && p[i]->GetIntVal() < 0.333) color = "pink";
                // if (p[i]->GetIntVal() >= 0.333 && p[i]->GetIntVal() < 0.667) color = "orange";
                // if (p[i]->GetIntVal() >= 0.667 && p[i]->GetIntVal() <= 1) color = "yellow";
                // can.Rect(offset + x * RECT_WIDTH, offset + y * RECT_WIDTH, RECT_WIDTH, RECT_WIDTH, color, "black");
                i++;
            }
        }
  }

  // create a box that will display more info about a host you select/hover over
  void makeInfoBox(UI::Canvas & can) {
      doc <<"The host intval is:";
      
  }

  void DoFrame() {
    if (world.GetUpdate() == numupdates && GetActive()) {
        ToggleActive();
    } else {
      auto mycanvas = doc.Canvas("can"); // get canvas by id
      mycanvas.Clear();

      // Update world and draw the new petri dish
      world.Update();
      p = world.getPop();
      drawPetriDish(mycanvas);
      doc.Text("update").Redraw();
    }
  }
};

MyAnimate anim;


int symbulation_main(int argc, char * argv[]){
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
    return 0;
}

#ifndef CATCH_CONFIG_MAIN
int main(int argc, char * argv[]) {
  return symbulation_main(argc, argv);
}
#endif
