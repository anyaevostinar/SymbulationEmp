// This file contains all operations related to the web interface
#ifndef SGP_SYM_ANIMATE_H
#define SGP_SYM_ANIMATE_H
#include "emp/base/Ptr.hpp"
#include "emp/bits/Bits.hpp"

#include "../../sgp_mode/SGPWorld.h"
#include "../../ConfigSetup.h"
#include "../../default_mode/DataNodes.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"

#include "../../sgp_mode/hardware/SGPHardwareSpec.h"
#include "../../sgp_mode/SGPConfigSetup.h"
#include "../../sgp_mode/SGPWorld.h"

// Empirical doesn't support more than one translation unit, so any CC files are
// included last. It still fixes include issues, but doesn't improve build time.
#include "../../default_mode/WorldSetup.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../../Empirical/include/emp/config/ArgManager.hpp"

#include <fstream>
#include <memory>
#include <string>

#include "../../default_mode/WorldSetup.cc"
#include "../../sgp_mode/SGPWorld.cc"
#include "../../sgp_mode/SGPWorldSetup.cc"
#include "../../sgp_mode/SGPWorldData.cc"
#include "../../sgp_mode/SGPW_InteractionMechanismSetup.cc"
#include "../../sgp_mode/SGPW_TaskProfileSetup.cc"

#include "../../../Empirical/include/emp/config/ArgManager.hpp"


#include "emp/web/Document.hpp"
#include "emp/web/Canvas.hpp"
#include "emp/web/web.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"



#include <iostream>

#include <emscripten.h>
#include <stdio.h>

namespace UI = emp::web;
sgpmode::SymConfigSGP config; // load the default configuration


void onLoad(const char* file) {
    
  }

void onError(const char* file) {
    // Wow file not loaded
}

class SGPSymAnimate : public UI::Animate {
private:

  UI::Document animation;
  UI::Document settings;
  UI::Document explanation;
  UI::Document learnmore;
  UI::Document buttons;
  UI::Canvas mycanvas;

  const int RECT_WIDTH = 10;

  emp::Random random{config.SEED()};
  sgpmode::SGPWorld world{random, &config};


  emp::vector<emp::Ptr<Organism>> p;


  int num_mutualistic = 0;
  int num_parasitic = 0;


public:

  /**
   * 
   * The contructor for SymAnimate
   * 
   */
  SGPSymAnimate() : animation("emp_animate"), settings("emp_settings"), explanation("emp_explanation"), learnmore("emp_learnmore"), buttons("emp_buttons") {

    config.WORLD_WIDTH(30);
    config.WORLD_HEIGHT(30);
    config.CYCLES_PER_UPDATE(16);
    config.UPDATES(30000);
    config.HOST_REPRO_RES(128);
    config.START_MOI(1);

    const char *json_data = R"json({
        "shared": {
            "tasks": [
                {"name": "NOT", "value": 1, "reward_mode": "add"},
                {"name": "NAND", "value": 1, "reward_mode": "add"},
                {"name": "OR_NOT", "value": 2, "reward_mode": "add"},
                {"name": "AND", "value": 2, "reward_mode": "add"},
                {"name": "OR", "value": 4, "reward_mode": "add"},
                {"name": "AND_NOT", "value": 4, "reward_mode": "add"},
                {"name": "NOR", "value": 8, "reward_mode": "add"},
                {"name": "XOR", "value": 8, "reward_mode": "add"},
                {"name": "EQU", "value": 16, "reward_mode": "add"}
            ]
        }
    })json";

    FILE *file = fopen("environment.json", "w");
    if (file == NULL) {
        printf("Error opening file inside virtual filesystem!\n");
      
    }

    fprintf(file, "%s", json_data);
    fclose(file);
    

    //Make json itself and add it to file
    //config.TASK_ENV_CFG_PATH("/flat-reward-1-env.json");
   

    animation.SetCSS("flex-grow", "1");
    animation.SetCSS("max-width", "500px");
    settings.SetCSS("flex-grow", "1");
    settings.SetCSS("flex-shrink", "0");
    settings.SetCSS("min-width", "100");
    settings.SetCSS("max-width", "3000px");
    explanation.SetCSS("flex-grow", "1");
    explanation.SetCSS("max-width", "600px");
    learnmore.SetCSS("flex-grow", "1");
    learnmore.SetCSS("max-width", "600px");
    buttons.SetCSS("flex-grow", "1");
    buttons.SetCSS("max-width", "600px");


    //emscripten_async_wget("http://localhost:8000/flat-reward-1-env.json", "/flat-reward-1-env.json", onLoad, onError);

     // apply configuration query params and config files to config
    auto specs = emp::ArgManager::make_builtin_specs(&config);
    emp::ArgManager am(emp::web::GetUrlParams(), specs);
    // cfg.Read("config.cfg");
    am.UseCallbacks();
    if (am.HasUnused()) std::exit(EXIT_FAILURE);

    initializeWorld();

    emp::prefab::ConfigPanel config_panel(config, true);
    //Exclude all the settings that control
    //things that don't show up in the GUI correctly
    config_panel.ExcludeSetting("SYM_LIMIT");
    config_panel.ExcludeSetting("DATA_INT");
    config_panel.ExcludeSetting("POP_SIZE");
    config_panel.ExcludeSetting("FILE_PATH");
    config_panel.ExcludeSetting("FILE_NAME");
    config_panel.ExcludeSetting("COMPETITION_MODE");
    config_panel.ExcludeSetting("SYM_INFECTION_CHANCE");
    config_panel.ExcludeSetting("SYM_INFECTION_FAILURE_RATE");
    config_panel.ExcludeSetting("MUTATION_SIZE");
    config_panel.ExcludeSetting("HOST_MUTATION_SIZE");
    config_panel.ExcludeSetting("HOST_MUTATION_SIZE");
    config_panel.ExcludeSetting("MUTATE_LYSIS_CHANCE");
    config_panel.ExcludeSetting("MUTATE_INDUCTION_CHANCE");
    config_panel.ExcludeSetting("MUTATE_INT_VAL");
    config_panel.ExcludeSetting("MUTATE_INDUCTION_CHANCE");
    config_panel.ExcludeSetting("EFFICIENCY_MUT_RATE");
    config_panel.ExcludeSetting("INT_VAL_MUT_RATE");
    config_panel.ExcludeSetting("HORIZ_MUTATION_SIZE");

    config_panel.ExcludeSetting("DONATION_STEAL_INST");
    config_panel.ExcludeSetting("SYM_DONATE_PROP");
    config_panel.ExcludeSetting("SYM_STEAL_PROP");
    config_panel.ExcludeSetting("INTERACTION_MECHANISM");

    config_panel.ExcludeSetting("PHYLOGENY");
    config_panel.ExcludeSetting("NUM_PHYLO_BINS");

    config_panel.ExcludeGroup("LYSIS");
    config_panel.ExcludeGroup("DTH");
    config_panel.ExcludeGroup("PGG");
    config_panel.ExcludeGroup("ECTOSYMBIOSIS");
    config_panel.ExcludeGroup("TAG_MATCHING");
    config_panel.ExcludeGroup("PHYLOGENY");

    config_panel.SetRange("HOST_INT", "-2", "1");
    config_panel.SetRange("SYM_INT", "-2", "1");
    config_panel.SetRange("SYM_AGE_MAX", "-1", "10000");
    config_panel.SetRange("HOST_AGE_MAX", "-1", "10000");
    config_panel.SetRange("LIMITED_RES_TOTAL", "-1", "10000");
    
    config_panel.SetRange("CYCLES_PER_UPDATE", "1", "16","1");
    config_panel.SetRange("FIND_NEIGHBOR_HOST_ATTEMPTS", "1", "16","1");
    config_panel.SetRange("HOST_MIN_CYCLES_BEFORE_REPRO", "0", "640","1");
    config_panel.SetRange("SYM_MIN_CYCLES_BEFORE_REPRO", "0", "640","1");

    config_panel.SetRange("STRESS_FREQUENCY", "0.0", "5000","1");
    config_panel.SetRange("PARASITE_DEATH_CHANCE", "0.0", "1.0","0.01");
    config_panel.SetRange("MUTUALIST_DEATH_CHANCE", "0.0", "1.0","0.01");
    config_panel.SetRange("BASE_DEATH_CHANCE", "0.0", "1.0","0.01");
    config_panel.SetRange("PARASITE_NUM_OFFSPRING_ON_STRESS_INTERACTION", "0", "16","1");

    config_panel.SetRange("MUTUALIST_CYCLE_GAIN_PROP", "0.0", "1.0","0.01");
    config_panel.SetRange("MUTUALIST_CYCLE_DONATE_MULTIPLIER", "0.0", "10.0","0.1");
    config_panel.SetRange("PARASITE_CYCLE_LOSS_PROP", "0.0", "1.0","0.01");
    config_panel.SetRange("PARASITE_CYCLE_STEAL_MULTIPLIER", "0.0", "10.0","0.1");
    config_panel.SetRange("PARASITE_BASE_CYCLE_PROP", "0.0", "1.0","0.01");
    config_panel.SetRange("HEALTH_INTERACTION_CHANCE", "0.0", "1.0","0.01");

    config_panel.SetRange("NUTRIENT_DONATE_PROP", "0.0", "1.0","0.01");
    config_panel.SetRange("NUTRIENT_STEAL_PROP", "0.0", "1.0","0.01");
    config_panel.SetRange("NUTRIENT_INTERACTION_MULTIPLIER", "0.0", "10.0","0.1");
    config_panel.SetRange("PARASITE_BASE_TASK_VALUE_PROP", "0.0", "1.0","0.01");

    emp::prefab::Card config_panel_ex("INIT_CLOSED");
    config_panel_ex.AddHeaderContent("<h3>Settings</h3>");

    // setup configuration panel
    //config_panel.Setup(config);
    config_panel_ex << config_panel;
    settings << config_panel_ex;


    // Add explanation for organism color:
    explanation << "<br><br><img style=\"max-width:175px;\" src=\"diagram1.png\"> <br>" <<
      "<img style=\"max-width:600px;\" src = \"gradient1.png\"/> <br>";


    // ----------------------- Add a button that allows for pause and start toggle -----------------------
    buttons << "<br>";
    buttons.AddButton([this](){
      // animate up to the number of updates
      ToggleActive();
      auto but = buttons.Button("toggle");
      if (GetActive()) but.SetLabel("Pause");
      else but.SetLabel("Start");
    }, "Start", "toggle");
    setButtonStyle("toggle");
    buttons.Button("toggle").OnMouseOver([this](){ auto but = buttons.Button("toggle"); but.SetCSS("background-color", "grey"); but.SetCSS("cursor", "pointer"); });
    buttons.Button("toggle").OnMouseOut([this](){ auto but = buttons.Button("toggle"); but.SetCSS("background-color", "#D3D3D3"); });

    // ----------------------- Add a reset button to reset the animation/world -----------------------
    /* Note: Must first run world.Reset(), because Inject checks for valid position.
      If a position is occupied, new org is deleted and your world isn't reset.
      Also, canvas must be redrawn to let users see that it is reset */
    buttons.AddButton([this](){
      world.Reset();
      buttons.Text("update").Redraw();
      initializeWorld();
      p = world.GetPop();

      if (GetActive()) { // If animation is running, stop animation and adjust button label
        ToggleActive();
      }
      auto but = buttons.Button("toggle");
      but.SetLabel("Start");

      // redraw petri dish
      mycanvas.SetWidth(RECT_WIDTH*config.WORLD_WIDTH());
      mycanvas.SetHeight(RECT_WIDTH*config.WORLD_HEIGHT());
      drawPetriDish(mycanvas);
      ToggleActive();//turn on quick to update the grid if the size changed
      ToggleActive();//turn off again
    }, "Reset", "reset");
    setButtonStyle("reset");
    buttons.Button("reset").OnMouseOver([this](){ auto but = buttons.Button("reset"); but.SetCSS("background-color", "grey"); but.SetCSS("cursor", "pointer"); });
    buttons.Button("reset").OnMouseOut([this](){ auto but = buttons.Button("reset"); but.SetCSS("background-color", "#D3D3D3"); });

    // ----------------------- Keep track of number of updates -----------------------
    buttons << "<br>";
    buttons << UI::Text("update") << "Update = " << UI::Live( [this](){ return world.GetUpdate(); } ) << "  ";
    buttons << UI::Text("mut") << "Mutualistic = " << UI::Live( [this](){ return num_mutualistic; } ) << "  ";
    buttons << UI::Text("par") << " Parasitic = " << UI::Live( [this](){ return num_parasitic; } );
    buttons << "<br>";

    // Add a canvas for petri dish and draw the initial petri dish
    mycanvas = animation.AddCanvas(RECT_WIDTH*config.WORLD_WIDTH(), RECT_WIDTH*config.WORLD_HEIGHT(), "can");
    targets.push_back(mycanvas);
    drawPetriDish(mycanvas);
    animation << "<br>";

    learnmore << "If you'd like to learn more, please see the publication <a href=\"https://www.mitpressjournals.org/doi/abs/10.1162/artl_a_00273\">Spatial Structure Can Decrease Symbiotic Cooperation</a>.";

  }


  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpose: To initialize the world based upon the config setting given 
   */
  void initializeWorld(){
     // Reset the seed and the random machine of world to ensure consistent result (??)
    random.ResetSeed(config.SEED());
    world.SetRandom(random);

    world.Setup();

    p = world.GetPop();

  }


  /**
   * Input: The string representing the button identification. 
   * 
   * Output: None
   * 
   * Purpose: To add style to the buttons displayed. 
   */
  void setButtonStyle(std::string but_id){
    auto but = buttons.Button(but_id);
    but.SetCSS("background-color", "#D3D3D3");
    but.SetCSS("border-radius", "4px");
    but.SetCSS("margin-left", "5px");
  }


  /**
   * Input: The canvas being used. 
   * 
   * Output: None
   * 
   * Purpose: To draw the petri dish of basteria and phage. 
   */
  // now draw a virtual petri dish with coordinate offset from the left frame
  void drawPetriDish(UI::Canvas & can){
        int i = 0;
        num_mutualistic = 0;
        num_parasitic = 0;
        //bool temp_passed = true;
        for (int x = 0; x < config.WORLD_WIDTH(); x++){
            for (int y = 0; y < config.WORLD_HEIGHT(); y++){
                emp::vector<emp::Ptr<Organism>>& syms = p[i]->GetSymbionts(); // retrieve all syms for this host (assume only 1 sym for each host)
                // color setting for host and symbiont



                sgpmode::SGPWorld::sgp_host_t& host = *static_cast<sgpmode::SGPWorld::sgp_host_t*>(p[i].Raw());
                std::string color_host = "#EFFDF0";
                for(int task_id = 8; task_id >= 0; task_id--){
                  if(host.GetHardware().GetCPUState().GetTaskPerformed(task_id)){
                    
                    color_host = matchColor(task_id);
                    break;
                  }
                }
                
                if(color_host == "#EFFDF0"){
                  for(int task_id = 8; task_id >= 0; task_id--){
                    if(host.GetHardware().GetCPUState().GetParentTaskPerformed(task_id)){
                      
                      color_host = matchColor(task_id);
                      break;
                    }
                  }
                }
                

                
     


                // Draw host rect and symbiont dot
                can.Rect(x * RECT_WIDTH, y * RECT_WIDTH, RECT_WIDTH, RECT_WIDTH, color_host, "black");
                int radius = RECT_WIDTH / 4;
                if(syms.size() == 1) {
                  sgpmode::SGPWorld::sgp_sym_t& sym = *static_cast<sgpmode::SGPWorld::sgp_sym_t*>(syms[0].Raw());
                  std::string color_sym = "#EFFDF0";
                  for(int task_id = 8; task_id >= 0; task_id--){
                    if(sym.GetHardware().GetCPUState().GetTaskPerformed(task_id)){
                      color_sym = matchColor(task_id);
                      break;
                    }
                  }

                  if(color_sym == "#EFFDF0"){
                  for(int task_id = 8; task_id >= 0; task_id--){
                    if(sym.GetHardware().GetCPUState().GetParentTaskPerformed(task_id)){
                      
                      color_sym = matchColor(task_id);
                      break;
                    }
                  }
                }
                  
                  // while drawing, test whether every organism is mutualistic
                  if (syms[0]->GetIntVal() <= 0) num_parasitic++;
                  else num_mutualistic++;
                  can.Circle(x * RECT_WIDTH + RECT_WIDTH/2, y * RECT_WIDTH + RECT_WIDTH/2, radius, color_sym, "black");
                }
                i++;

            }
        }
        //passed = temp_passed; // update passed
  }

  // match the interaction value to colors, assuming that -1.0 <= intVal <= 1.0.
  // The antogonistic have light colors, and the cooperative have dark, brownish colors.


  /**
   * Input: The double representing symbiont or host's interaction value 
   * 
   * Output: The string representing the hex value for the color of the organism. 
   * 
   * Purpose: To determine the color that an organism should be, given its
   * interaction value. 
   */  
  std::string matchColor(int taskComplete){
    if(taskComplete == 0){return "#000000";}
    else if(taskComplete == 1){return "#0051ff";}
    else if(taskComplete == 2){return "#36c6ff";}
    else if(taskComplete == 3){return "#31f7c6";}
    else if(taskComplete == 4){return "#31f738";}
    else if(taskComplete == 5){return "#bef244";}
    else if(taskComplete == 6){return "#ffc400";}
    else if(taskComplete == 7){return "#ff6200";}
    else if(taskComplete == 8){return "#ff0000";}

  }


  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpose: To update the frame displayed of the current 
   * world state. 
   */
  void DoFrame() {

    if (world.GetUpdate() >= config.UPDATES() && GetActive()) {
        ToggleActive();
    } else {
      mycanvas = animation.Canvas("can"); // get canvas by id
      mycanvas.Clear();

      // Update world and draw the new petri dish
      for(int i = 0; i < 10;i++){
      world.Update();
      }
      p = world.GetPop();
      std::cout << "hey hey"<< std::endl;
      drawPetriDish(mycanvas);
      buttons.Text("update").Redraw();
      //buttons.Text("mut").Redraw();
      //buttons.Text("par").Redraw();
    }
  }
};
#endif