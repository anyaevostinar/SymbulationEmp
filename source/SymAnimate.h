// This file contains all operations related to the web interface
#ifndef SYM_ANIMATE_H
#define SYM_ANIMATE_H

#include <iostream>
#include "SymWorld.h"
#include "SymConfig.h"
#include "SymJS.h"
#include "../../Empirical/source/web/Document.h"
#include "../../Empirical/source/web/Canvas.h"
#include "../../Empirical/source/web/web.h"
#include "../../Empirical/source/config/ArgManager.h"

namespace UI = emp::web;

class SymAnimate : public UI::Animate {
private:
  SymConfigBase config; // load the default configuration
  UI::Document animation;
  UI::Document settings;
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

  // Params for controlling textarea input
  bool empty_vert = false;
  bool empty_grid = false;

  // Params for controlling game mode
  bool game_mode = false;
  int challenge_ind = 0;
  std::vector<std::string> bg_colors{ "transparent", "yellow"}; // bg color of settings to indicate whether it is in game mode
  bool passed; // whether player passed the challenge
  int num_mutualistic = 0;
  int num_parasitic = 0;
  std::vector<std::string> challenges{ "Make all organisms mutualistic", "Make all organisms parasitic" };
  int challenge_number = challenges.size();

public:

  SymAnimate() : animation("emp_animate"), settings("emp_settings") {
    // Set parameters for "animation" and "settings" div to enable flex box
    animation.SetCSS("flex-grow", "1");
    animation.SetCSS("max-width", "500px");
    settings.SetCSS("flex-grow", "1");
    settings.SetCSS("max-width", "600px");

    // ----------------------- Add a playgame button that toggles game_mode -----------------------
    settings << UI::Text("game_mode") << "Game Mode: " << 
      UI::Live( [this](){ return (game_mode)? "On" : "Off"; } ) << "<br>";
    settings.AddButton([this](){
      toggleGame();
      auto but = settings.Button("play");
      if (game_mode) but.SetLabel("End Game");
      else but.SetLabel("Play Game");
    }, "Play Game", "play");
    settings << "<br>";
    setButtonStyle("play");
    settings.Button("play").OnMouseOver([this](){ auto but = settings.Button("play"); but.SetCSS("background-color", "grey"); but.SetCSS("cursor", "pointer"); });
    settings.Button("play").OnMouseOut([this](){ auto but = settings.Button("play"); but.SetCSS("background-color", "#D3D3D3"); });
    
    initializeWorld();
    // ----------------------- Input field for modifying the vertical transmission rate -----------------------
    settings << "<b>See what happens at different vertical transmission rates!<br>Please type in a vertical transmisson rate between 0 and 1, then click Reset: </b><br>";
    settings.AddTextArea([this](const std::string & in){
      bool isValidInput = true;
      for (char c : in){
        if (c == 46) continue; // "." is part of a double, skip
        else if (c < 48 || c > 57){ isValidInput = false; break; } // check for valid input string (must be a double <= 1)
      }
      if (in.empty()) { 
        em_vert_trans.SetCSS("opacity", "0");  // set empty_vert so nothing's printed
        empty_vert = true; settings.Text("vert_trans_txt").Redraw();
      } 
      //TO DO: make stod(in) be called only once
      else if (isValidInput && stod(in) <= 1) {
        em_vert_trans.SetCSS("opacity", "0");
        vert_transmit = stod(in); 
        settings.Text("vert_trans_txt").Redraw(); 
        empty_vert = false;
        world.SetVertTrans(vert_transmit); // enable vertical transmission to be updated in the middle of a run
      }
      else { em_vert_trans.SetCSS("opacity", "1"); } // turn on error message
    }, "update_vert_transmit");
    settings << em_vert_trans; 
    settings << "<br>";
    settings << UI::Text("vert_trans_txt") << "Vertical Transmission Rate = " << 
      UI::Live( [this](){ return empty_vert ? "" : std::to_string(vert_transmit); } );


    // ----------------------- Input field for changing the grid setting -----------------------
    settings << "<br>";
    settings << "<b>See how global versus local reproduction changes evolution! <br>Please type in 0 for global reproduction  or 1 for local reproduction, then click Reset: </b><br>";
    settings.AddTextArea([this](const std::string & in){
      bool isValidInput = (in.size() == 1 && (in == "0" || in == "1")); // input must be either "0" or "1"
      if (in.empty()) { 
        em_grid.SetCSS("opacity", "0"); 
        empty_grid = true; // set empty_grid so nothing's printed
        settings.Text("grid_txt").Redraw(); 
      } 
      else if (isValidInput) {
        em_grid.SetCSS("opacity", "0"); 
        grid = stoi(in); 
        settings.Text("grid_txt").Redraw(); 
        empty_grid = false;
      }
      else { em_grid.SetCSS("opacity", "1"); } // turn on error message
    }, "update_grid");
    settings << em_grid;
    settings << "<br>";
    settings << UI::Text("grid_txt") << "Reproduction location = " << 
      UI::Live( [this](){ return grid; } );


    // ----------------------- Error message settings -----------------------
    em_vert_trans << "Invalid Input!";
    em_vert_trans.SetCSS("color", "red");
    em_vert_trans.SetCSS("opacity", "0");

    em_grid << "Invalid Input!";
    em_grid.SetCSS("color", "red");
    em_grid.SetCSS("opacity", "0");

    // Add explanation for organism color:
    settings << "<br><br><img style=\"max-width:175px;\" src=\"diagram.png\"> <br>" <<
      "<img style=\"max-width:600px;\" src = \"gradient.png\"/> <br>";
      

    // ----------------------- Add a button that allows for pause and start toggle -----------------------
    settings << "<br>";
    settings.AddButton([this](){
      // animate up to the number of updates
      ToggleActive();
      auto but = settings.Button("toggle"); 
      if (GetActive()) but.SetLabel("Pause");
      else but.SetLabel("Start");
    }, "Start", "toggle");
    setButtonStyle("toggle");
    settings.Button("toggle").OnMouseOver([this](){ auto but = settings.Button("toggle"); but.SetCSS("background-color", "grey"); but.SetCSS("cursor", "pointer"); });
    settings.Button("toggle").OnMouseOut([this](){ auto but = settings.Button("toggle"); but.SetCSS("background-color", "#D3D3D3"); });

    // ----------------------- Add a reset button to reset the animation/world -----------------------
    /* Note: Must first run world.Reset(), because Inject checks for valid position.
      If a position is occupied, new org is deleted and your world isn't reset.
      Also, canvas must be redrawn to let users see that it is reset */
    settings.AddButton([this](){
      world.Reset();
      settings.Text("update").Redraw();
      initializeWorld();
      p = world.getPop();
    
      if (GetActive()) { // If animation is running, stop animation and adjust button label
        ToggleActive();   
      }
      auto but = settings.Button("toggle"); 
      but.SetLabel("Start"); 

      // redraw petri dish
      auto mycanvas = animation.Canvas("can");
      drawPetriDish(mycanvas);
    }, "Reset", "reset");
    setButtonStyle("reset");
    settings.Button("reset").OnMouseOver([this](){ auto but = settings.Button("reset"); but.SetCSS("background-color", "grey"); but.SetCSS("cursor", "pointer"); });
    settings.Button("reset").OnMouseOut([this](){ auto but = settings.Button("reset"); but.SetCSS("background-color", "#D3D3D3"); });

    // ----------------------- Keep track of number of updates -----------------------
    settings << "<br>";
    settings << UI::Text("update") << "Update = " << UI::Live( [this](){ return world.GetUpdate(); } ) << "  ";
    settings << UI::Text("mut") << "Mutualistic = " << UI::Live( [this](){ return num_mutualistic; } ) << "  ";
    settings << UI::Text("par") << " Parasitic = " << UI::Live( [this](){ return num_parasitic; } );
    settings << "<br>";

    // Add a canvas for petri dish and draw the initial petri dish
    auto mycanvas = animation.AddCanvas(can_size, can_size, "can");
    targets.push_back(mycanvas);
    drawPetriDish(mycanvas);
    animation << "<br>";

    settings << "If you'd like to learn more, please see the publication <a href=\"https://www.mitpressjournals.org/doi/abs/10.1162/artl_a_00273\">Spatial Structure Can Decrease Symbiotic Cooperation</a>.";

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

  void setButtonStyle(string but_id){
    auto but = settings.Button(but_id);
    but.SetCSS("background-color", "#D3D3D3");
    but.SetCSS("border-radius", "4px");
    but.SetCSS("margin-left", "5px");    
  }

  // now draw a virtual petri dish with coordinate offset from the left frame
  void drawPetriDish(UI::Canvas & can){
        int i = 0;
        num_mutualistic = 0;
        num_parasitic = 0;
        //bool temp_passed = true; 
        for (int x = 0; x < side_x; x++){ 
            for (int y = 0; y < side_y; y++){
                emp::vector<Symbiont>& syms = p[i]->GetSymbionts(); // retrieve all syms for this host (assume only 1 sym for each host)
                // color setting for host and symbiont
                std::string color_host = matchColor(p[i]->GetIntVal()); 
                std::string color_sym = matchColor(syms[0].GetIntVal());
                // while drawing, test whether every organism is mutualistic
                //if (p[i]->GetIntVal() <= 0 || syms[0].GetIntVal() <= 0) temp_passed = false;
                if (p[i]->GetIntVal() <= 0) num_parasitic++; 
                else num_mutualistic++;
                if (syms[0].GetIntVal() <= 0) num_parasitic++;
                else num_mutualistic++;
                // Draw host rect and symbiont dot
                can.Rect(offset + x * RECT_WIDTH, offset + y * RECT_WIDTH, RECT_WIDTH, RECT_WIDTH, color_host, "black");
                int radius = RECT_WIDTH / 4;
                can.Circle(offset + x * RECT_WIDTH + RECT_WIDTH/2, offset + y * RECT_WIDTH + RECT_WIDTH/2, radius, color_sym, "black");
                i++;
            }
        }
        //passed = temp_passed; // update passed
  }

  // match the interaction value to colors, assuming that -1.0 <= intVal <= 1.0. 
  // The antogonistic have light colors, and the cooperative have dark, brownish colors.
  std::string matchColor(double intVal){
    if ((-1.0 <= intVal) && (intVal < -0.9)) return "#EFFDF0";
    else if ((-0.9 <= intVal) && (intVal < -0.8)) return "#D4FFDD";
    else if ((-0.8 <= intVal) && (intVal < -0.7)) return "#BBFFDB";
    else if ((-0.7 <= intVal) && (intVal < -0.6)) return "#B2FCE3";
    else if ((-0.6 <= intVal) && (intVal < -0.5)) return "#96FFF7";
    else if ((-0.5 <= intVal) && (intVal < -0.4)) return "#86E9FE";
    else if ((-0.4 <= intVal) && (intVal < -0.3)) return "#6FC4FE";
    else if ((-0.3 <= intVal) && (intVal < -0.2)) return "#5E8EFF";
    else if ((-0.2 <= intVal) && (intVal < -0.1)) return "#4755FF";
    else if ((-0.1 <= intVal) && (intVal < 0.0)) return "#5731FD";
    else if ((0.0 <= intVal) && (intVal < 0.1)) return "#7B1DFF";
    else if ((0.1 <= intVal) && (intVal < 0.2)) return "#AB08FF";
    else if ((0.2 <= intVal) && (intVal < 0.3)) return "#E401E7";
    else if ((0.3 <= intVal) && (intVal < 0.4)) return "#D506AD";
    else if ((0.4 <= intVal) && (intVal < 0.5)) return "#CD0778";
    else if ((0.5 <= intVal) && (intVal < 0.6)) return "#B50142";
    else if ((0.6 <= intVal) && (intVal < 0.7)) return "#A7000F";
    else if ((0.7 <= intVal) && (intVal < 0.8)) return "#891901";
    else if ((0.8 <= intVal) && (intVal < 0.9)) return "#7D3002";
    else return "#673F03";
  }

  void toggleGame(){
    game_mode = !game_mode;
    settings.Text("game_mode").Redraw();
    settings.SetCSS("background-color", bg_colors[game_mode]);
    if (game_mode) { 
      auto str = challenges[challenge_ind].c_str();
      modifyChallenge(str, challenge_ind, challenge_number);
      showChallenge();
    } else challenge_ind = 0; // resetting the game resets the challenge
  }

  void DoFrame() { 
    passed = checkPassed();
    if (game_mode && passed) { // actions that will be taken ONLY when game passes
      ToggleActive(); showSuccess();  // game succeeded. No need to continue the current simulation
      if (++challenge_ind < challenge_number) { // continue challenges if there are any left
        auto str = challenges[challenge_ind].c_str();
        modifyChallenge(str, challenge_ind, challenge_number);
      } else {
        modifyChallenge("", challenge_ind, challenge_number);
      }
    }

    if (world.GetUpdate() == numupdates && GetActive()) {
        ToggleActive();
        if (!passed) showFailure();
    } else {
      auto mycanvas = animation.Canvas("can"); // get canvas by id
      mycanvas.Clear();

      // Update world and draw the new petri dish
      world.Update();
      p = world.getPop();
      drawPetriDish(mycanvas);
      settings.Text("update").Redraw();
      settings.Text("mut").Redraw();
      settings.Text("par").Redraw();
    }
  }

  // Checks if a particular challenge is passed
  bool checkPassed(){
    if (challenge_ind == 0) return (num_parasitic == 0);
    else if (challenge_ind == 1) return (num_mutualistic == 0);
    else return false;
  }
};

#endif
