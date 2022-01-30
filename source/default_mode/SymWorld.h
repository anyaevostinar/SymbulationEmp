#ifndef SYM_WORLD_H
#define SYM_WORLD_H

#include "../../../Empirical/include/emp/Evolve/World.hpp"
#include "../../../Empirical/include/emp/data/DataFile.hpp"
#include "../../../Empirical/include/emp/math/random_utils.hpp"
#include "../../../Empirical/include/emp/math/Random.hpp"
#include "../Organism.h"
#include <set>
#include <math.h>

// #include <typeinfo>
// string s = typeid(p).name()

class SymWorld : public emp::World<Organism>{
protected:
  /**
    *
    * Purpose: Represents the vertical transmission rate. This can be set with SetVertTrans()
    *
  */
  double vertTrans = 0;

  /**
    *
    * Purpose: Represents the total resources in the world. This can be set with SetTotalRes()
    *
  */
  int total_res = -1;

  /**
    *
    * Purpose: Represents if resources are limited or not. This can be set with SetLimitedRes()
    *
  */
  bool limited_res = false;

  /**
    *
    * Purpose: Represents if free living symbionts are allowed. This can be set with SetFreeLivingSyms()
    *
  */
  bool do_free_living_syms = false;

  /**
    *
    * Purpose: Represents how many resources each host gets per update. This can be set with SetResPerUpdate()
    *
  */
  double resources_per_host_per_update = 0;

  /**
    *
    * Purpose: Represents if free living symbionts are permitted to move around the world.
    *
  */
  bool move_free_syms = false;

  pop_t sym_pop; //free living sym pop

  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostintval; // New() reallocates this pointer
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_symintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsymintval;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_syminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_freesyminfectchance;
  emp::Ptr<emp::DataMonitor<double, emp::data::Histogram>> data_node_hostedsyminfectchance;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_symcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_freesymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_hostedsymcount;
  emp::Ptr<emp::DataMonitor<int>> data_node_cfu;
  emp::Ptr<emp::DataMonitor<int>> data_node_uninf_hosts;


public:
  /**
   * Input: The world's random seed
   *
   * Output: None
   *
   * Purpose: To construct an instance of SymWorld
   */
  SymWorld(emp::Random & _random) : emp::World<Organism>(_random) {
    fun_print_org = [](Organism & org, std::ostream & os) {
      //os << PrintHost(&org);
      os << "This doesn't work currently";
    };
  }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To destruct the data nodes belonging to SymWorld to conserve memory.
   */
  ~SymWorld() {
    if (data_node_hostintval) data_node_hostintval.Delete();
    if (data_node_symintval) data_node_symintval.Delete();
    if (data_node_freesymintval) data_node_freesymintval.Delete();
    if (data_node_hostedsymintval) data_node_hostedsymintval.Delete();
    if (data_node_syminfectchance) data_node_syminfectchance.Delete();
    if (data_node_freesyminfectchance) data_node_freesyminfectchance.Delete();
    if (data_node_hostedsyminfectchance) data_node_hostedsyminfectchance.Delete();
    if (data_node_hostcount) data_node_hostcount.Delete();
    if (data_node_symcount) data_node_symcount.Delete();
    if (data_node_freesymcount) data_node_freesymcount.Delete();
    if (data_node_hostedsymcount) data_node_hostedsymcount.Delete();
    if (data_node_cfu) data_node_cfu.Delete();
    if (data_node_uninf_hosts) data_node_uninf_hosts.Delete();
  }


  /**
   * Input: The double representing the vertical transmission rate
   *
   * Output: None
   *
   * Purpose: To set the vertical transmission rate
   */
  void SetVertTrans(double vt) {vertTrans = vt;}


  /**
   * Input: The double representing the number of resources each host gets in each update.
   *
   * Output: None
   *
   * Purpose: To set the resources that each host gets per update.
   */
  void SetResPerUpdate(double val) {resources_per_host_per_update = val;}


  /**
   * Input: To boolean representing if resources are limited or not.
   *
   * Output: None
   *
   * Purpose: To allow for resources to be limited or unlimited.
   */
  void SetLimitedRes(bool val) {limited_res = val;}


  /**
   * Input: The boolean representing if symbionts are allowed to be free living.
   *
   * Output: None
   *
   * Purpose: To allow for free-living symbionts
   */
  void SetFreeLivingSyms(bool flp) {do_free_living_syms = flp; }


  /**
   * Input: The bool representing if free living symbionts are
   * permitted to move around in the world.
   *
   * Output: None
   *
   * Purpose: To set the value representing if FLS are
   * permitted to move around.
   */
  void SetMoveFreeSyms(bool mfs) {move_free_syms = mfs;}


  /**
   * Input: The int representing the total number of resources for the world.
   *
   * Output: None
   *
   * Purpose: To set the total number of resources in the world. If limited resources
   * is off, then the total resource value is of no consequence.
   */
  void SetTotalRes(int val) {
    if(val<0){
      SetLimitedRes(false);
    } else {
      SetLimitedRes(true);
      total_res = val;
    }
  }


  /**
   * Input: None
   *
   * Output: The pop_t value that represents the world's population.
   *
   * Purpose: To get the world's population of organisms.
   */
  emp::World<Organism>::pop_t GetPop() {return pop;}


  /**
   * Input: None
   *
   * Output: The pop_t value that represent the world's symbiont
   * population.
   *
   * Purpose: To get the world's symbiont population.
   */
  emp::World<Organism>::pop_t GetSymPop() {return sym_pop;}


  /**
   * Input: None
   *
   * Output: The boolean representing if vertical transmission will occur
   *
   * Purpose: To determine if vertical transmission will occur
   */
  bool WillTransmit() {
    bool result = GetRandom().GetDouble(0.0, 1.0) < vertTrans;
    return result;
  }


  /**
   * Input: The amount of resourcces an organism wants from the world.
   *
   * Output: If there are unlimited resources or the total resources are greater than those requested,
   * returns the amount of desired resources.
   * If total_res is less than the desired resources, but greater than 0,
   * then total_res will be returned. If none of these are true, then 0 will be returned.
   *
   * Purpose: To determine how many resources to distribute to each organism.
   */
  int PullResources(int desired_resources) {
    if(!limited_res) {
      return desired_resources;
    } else {
      if (total_res>=desired_resources) {
        total_res = total_res - desired_resources;
        return desired_resources;
      } else if (total_res>0) {
        int resources_to_return = total_res;
        total_res = 0;
        return resources_to_return;
      } else {
        return 0;
      }
    }
  }


  /**
   * Input: The size_t representing the world's new width;
   * the size_t representing the world's new height.
   *
   * Output: None
   *
   * Purpose: To overwrite the Empirical resize so that sym_pop is also resized
   */
  void Resize(size_t new_width, size_t new_height) {
    size_t new_size = new_width * new_height;
    pop.resize(new_size);
    sym_pop.resize(new_size);
    pop_sizes.resize(2);
    pop_sizes[0] = new_width; pop_sizes[1] = new_height;
  }


  /**
   * Input: The size_t representing the new size of the world
   *
   * Output: None
   *
   * Purpose: To override the Empirical Resize function with
   * a single-arg method that can be used for AddOrgAt vector
   * expansions
   */
  void Resize(size_t new_size){
    pop.resize(new_size);
    sym_pop.resize(new_size);
    pop_sizes.resize(2);
  }


  /**
   * Input: The pointer to the new organism;
   * the world position of the location to add
   * the new organism.
   *
   * Output: None
   *
   * Purpose: To overwrite the empirical AddOrgAt function to permit syms to
   * be added into sym_pop
   */
  void AddOrgAt(emp::Ptr<Organism> new_org, emp::WorldPosition pos, emp::WorldPosition p_pos=emp::WorldPosition()) {
    emp_assert(new_org);         // The new organism must exist.
    emp_assert(pos.IsValid());   // Position must be legal.

    //if the pos it out of bounds, expand the worlds so that they can fit it.
    if(pos.GetIndex() >= sym_pop.size() || pos.GetIndex() >= pop.size()){
      Resize(pos.GetIndex() + 1);
    }

    if(new_org->IsHost()){ //if the org is a host, use the empirical addorgat function
      emp::World<Organism>::AddOrgAt(new_org, pos,p_pos);
    } else { //if it is not a host, then add it to the sym population
      size_t pos_index = pos.GetIndex();

      //if it is adding a sym to the pop, add to the num_org count
      //otherwise, delete the sym currently occupying the spot
      if(!sym_pop[pos_index]) ++num_orgs;
      else sym_pop[pos_index].Delete();

      //set the pointer to NULL
      sym_pop[pos_index] = nullptr;
      sym_pop[pos_index] = new_org;
    }
  }


  //Overriding World's DoBirth to take a pointer instead of a reference
  //Because it takes a pointer, it doesn't support birthing multiple copies
  /**
   * Input: (1) The pointer to the organism that is being birthed;
   * (2) The size_t location of the parent organism.
   *
   * Output: The WorldPosition of the position of the new organism.
   *
   * Purpose: To introduce new organisms to the world. If the new organism is a host,
   * and the position generated for its location is occupied by a symbiont, then the
   * symbiont will be added to the host's symbionts. If the new organism is a host,
   * and the position generated for its location is unoccupied or occupied by a host,
   * then the new organism will be added regularly.
   */
  emp::WorldPosition DoBirth(emp::Ptr<Organism> new_org, size_t parent_pos) {
    before_repro_sig.Trigger(parent_pos);
    emp::WorldPosition pos; // Position of each offspring placed.

    offspring_ready_sig.Trigger(*new_org, parent_pos);
    pos = fun_find_birth_pos(new_org, parent_pos);
    if (pos.IsValid() && pos.GetIndex() != parent_pos) {
      //Add to the specified position, overwriting what may exist there
      AddOrgAt(new_org, pos, parent_pos);
    }
    else {
      new_org.Delete();
    } // Otherwise delete the organism.
    return pos;
  }


  /**
   * Input: The size_t value representing the location whose neighbors
   * are being searched.
   *
   * Output: If there are no occupied neighboring positions, -1 will be returned.
   * If there are occupied neighboring positions, then the location of one
   * occupied position will be returned.
   *
   * Purpose: To determine the location of a valid occupied neighboring position.
   */
  int GetNeighborHost (size_t i) {
    const emp::vector<size_t> validNeighbors = GetValidNeighborOrgIDs(i);
    if (validNeighbors.empty()) return -1;
    else {
      int randI = GetRandom().GetUInt(0, validNeighbors.size());
      return validNeighbors[randI];
    }
  }


  /**
   * Input: The pointer to an organism that will be injected into a host.
   *
   * Output: None
   *
   * Purpose: To add a symbiont to a host's symbionts.
   */
  void InjectSymbiont(emp::Ptr<Organism> new_sym){
    size_t new_loc;
    if(!do_free_living_syms){
      new_loc = GetRandomOrgID();
      //if the position is acceptable, add the sym to the host in that position
      if(IsOccupied(new_loc)) pop[new_loc]->AddSymbiont(new_sym);
    } else {
      new_loc = GetRandomCellID();
      //if the position is within bounds, add the sym to it
      if(new_loc < sym_pop.size()) AddOrgAt(new_sym, new_loc);
    }
  }


  /**
   * Definitions of data node functions, expanded in DataNodes.h
   */
  emp::DataFile & SetupSymIntValFile(const std::string & filename);
  emp::DataFile & SetupHostIntValFile(const std::string & filename);
  emp::DataFile & SetUpFreeLivingSymFile(const std::string & filename);
  emp::DataMonitor<int>& GetHostCountDataNode();
  emp::DataMonitor<int>& GetSymCountDataNode();
  emp::DataMonitor<int>& GetCountHostedSymsDataNode();
  emp::DataMonitor<int>& GetCountFreeSymsDataNode();
  emp::DataMonitor<int>& GetUninfectedHostsDataNode();
  emp::DataMonitor<int>& GetCFUDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetHostIntValDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetSymIntValDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetFreeSymIntValDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetHostedSymIntValDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetSymInfectChanceDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetFreeSymInfectChanceDataNode();
  emp::DataMonitor<double,emp::data::Histogram>& GetHostedSymInfectChanceDataNode();

  /**
   * Input: The pointer to the symbiont that is moving, the size_t to its
   * location.
   *
   * Output: None
   *
   * Purpose: To move a symbiont into a new world position.
   */
  void MoveIntoNewFreeWorldPos(emp::Ptr<Organism> sym, size_t i){
    emp::WorldPosition newLoc = GetRandomNeighborPos(i);
    if(newLoc.IsValid()){
      sym->SetHost(nullptr);
      AddOrgAt(sym, newLoc, i);
    } else sym.Delete();
  }

  /**
   * Input: The pointer to the organism that is being birthed, and the size_t location
   * of the parent symbiont.
   *
   * Output: None
   *
   * Purpose: To birth a new symbiont. If free living symbionts is on, the new symbiont
   * can be put into an unoccupied place in the world. If not, then it will be placed
   * in a host near its parent's location, or deleted if the parent's location has
   * no eligible near-by hosts.
   */
  void SymDoBirth(emp::Ptr<Organism> sym_baby, size_t i) {
    if(!do_free_living_syms){
      int newLoc = GetNeighborHost(i);
      if (newLoc > -1) { //-1 means no living neighbors
        pop[newLoc]->AddSymbiont(sym_baby);
      } else {
        sym_baby.Delete();
      }
    } else {
      MoveIntoNewFreeWorldPos(sym_baby, i);
    }
  }


  /**
   * Input: The size_t location of the symbiont to be moved.
   *
   * Output: None
   *
   * Purpose: To move a symbiont, either into a host, or into a free world position
   */
  void MoveFreeSym(size_t i){
    //the sym can either move into a parallel sym or to some random position
    if(IsOccupied(i) && sym_pop[i]->WantsToInfect()) {
      emp::Ptr<Organism> sym = ExtractSym(i);
      if(sym->InfectionFails()) sym.Delete(); //if the sym tries to infect and fails it dies
      else pop[i]->AddSymbiont(sym);
    }
    else if(move_free_syms) {
      MoveIntoNewFreeWorldPos(ExtractSym(i), i);
    }
  }

  /*
  * Input: The size_t location of the sym to be pointed to.
  *
  * Output: A pointer to the sym.
  *
  * Purpose: To allow access to syms at a specified location in the sym_pop.
  */
  emp::Ptr<Organism> GetSymAt(size_t location){
    if (location >= 0 && location < sym_pop.size()){
      return sym_pop[location];
    } else {
      throw "Attempted to get out of bounds sym.";
    }
  }

  /**
   * Input: The size_t representing the location of the symbiont to be
   * extracted from the world.
   *
   * Output: The pointer to the organism that was removed from the world
   *
   * Purpose: To remove a symbiont from the world
   */
  emp::Ptr<Organism> ExtractSym(size_t i){
    emp::Ptr<Organism> sym;
    if(sym_pop[i]){
      sym = sym_pop[i];
      num_orgs--;
      sym_pop[i] = nullptr;
    }
    return sym;
  }

  /**
   * Input: The size_t representing the location of the symbiont to be
   * deleted from the world.
   *
   * Output: None
   *
   * Purpose: To delete a symbiont from the world.
   */
  void DoSymDeath(size_t i){
    if(sym_pop[i]){
      sym_pop[i].Delete();
      sym_pop[i] = nullptr;
      num_orgs--;
    }
  }


  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To call the process functions for hosts and symbionts.
   */
  void Update() {
    emp::World<Organism>::Update();

    //TODO: put in fancy scheduler at some point
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());

    // divvy up and distribute resources to host and symbiont in each cell
    for (size_t i : schedule) {
      if (IsOccupied(i) == false && !sym_pop[i]){ continue;} // no organism at that cell
      //Would like to shove reproduction into Process, but it gets sticky with Symbiont reproduction
      //Could put repro in Host process and population calls Symbiont process and place offspring as necessary?
      if(IsOccupied(i)){//can't call GetDead on a deleted sym, so
        pop[i]->Process(i);
        if (pop[i]->GetDead()) { //Check if the host died
          DoDeath(i);
        }
      }
      if(sym_pop[i]){ //for sym movement reasons, syms are deleted the update after they are set to dead
        if (sym_pop[i]->GetDead()) DoSymDeath(i); //Might have died since their last time being processed
        else sym_pop[i]->Process(i);
        //if (sym_pop[i]->GetDead()) DoSymDeath(i); //Checking if they died during their process and cleaning up the corpse
        //TODO: fix the reason why the corpse can't be immediately cleaned up
      }
    } // for each cell in schedule
  } // Update()
};// SymWorld class
#endif
