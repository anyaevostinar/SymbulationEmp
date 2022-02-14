#ifndef DATA_H
#define DATA_H

#include "SymWorld.h"


/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: The address of the DataFile that has been created.
 *
 * Purpose: To set up the file that will be used to track the average symbiont
 * interaction value, the total number of symbionts, the total number of symbionts
 * in a host, the total number of free syms and set up a histogram of the
 * symbiont's interaction values.
 */
emp::DataFile & SymWorld::SetupSymIntValFile(const std::string & filename) {
  auto & file = SetupFile(filename);
  auto & node = GetSymIntValDataNode();
  auto & node1 = GetSymCountDataNode();

  node.SetupBins(-1.0, 1.1, 21); //Necessary because range exclusive
  file.AddVar(update, "update", "Update");
  file.AddMean(node, "mean_intval", "Average symbiont interaction value");
  file.AddTotal(node1, "count", "Total number of symbionts");

  //interaction val histogram
  file.AddHistBin(node, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
  file.AddHistBin(node, 1, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
  file.AddHistBin(node, 2, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
  file.AddHistBin(node, 3, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
  file.AddHistBin(node, 4, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
  file.AddHistBin(node, 5, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
  file.AddHistBin(node, 6, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
  file.AddHistBin(node, 7, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
  file.AddHistBin(node, 8, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
  file.AddHistBin(node, 9, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
  file.AddHistBin(node, 10, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
  file.AddHistBin(node, 11, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
  file.AddHistBin(node, 12, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
  file.AddHistBin(node, 13, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
  file.AddHistBin(node, 14, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
  file.AddHistBin(node, 15, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
  file.AddHistBin(node, 16, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
  file.AddHistBin(node, 17, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
  file.AddHistBin(node, 18, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
  file.AddHistBin(node, 19, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");

  file.PrintHeaderKeys();

  return file;
}


/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: The address of the DataFile that has been created.
 *
 * Purpose: To set up the file that will be used to track host's
 * interaction values, the total number of hosts, the total
 * number of colony forming units, and the histogram of the
 * host's interaction values
 */
emp::DataFile & SymWorld::SetupHostIntValFile(const std::string & filename) {
  auto & file = SetupFile(filename);
  auto & node = GetHostIntValDataNode();
  auto & node1 = GetHostCountDataNode();
  auto & cfu_node = GetCFUDataNode();
  auto & uninf_hosts_node = GetUninfectedHostsDataNode();
  node.SetupBins(-1.0, 1.1, 21);

  file.AddVar(update, "update", "Update");
  file.AddMean(node, "mean_intval", "Average host interaction value");
  file.AddTotal(node1, "count", "Total number of hosts");
  file.AddTotal(cfu_node, "cfu_count", "Total number of colony forming units"); //colony forming units are hosts that
  //either aren't infected at all or only with lysogenic phage if lysis is enabled
  file.AddTotal(uninf_hosts_node, "uninfected_host_count", "Total number of hosts that are uninfected");
  file.AddHistBin(node, 0, "Hist_-1", "Count for histogram bin -1 to <-0.9");
  file.AddHistBin(node, 1, "Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
  file.AddHistBin(node, 2, "Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
  file.AddHistBin(node, 3, "Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
  file.AddHistBin(node, 4, "Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
  file.AddHistBin(node, 5, "Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
  file.AddHistBin(node, 6, "Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
  file.AddHistBin(node, 7, "Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
  file.AddHistBin(node, 8, "Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
  file.AddHistBin(node, 9, "Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
  file.AddHistBin(node, 10, "Hist_0.0", "Count for histogram bin 0.0 to <0.1");
  file.AddHistBin(node, 11, "Hist_0.1", "Count for histogram bin 0.1 to <0.2");
  file.AddHistBin(node, 12, "Hist_0.2", "Count for histogram bin 0.2 to <0.3");
  file.AddHistBin(node, 13, "Hist_0.3", "Count for histogram bin 0.3 to <0.4");
  file.AddHistBin(node, 14, "Hist_0.4", "Count for histogram bin 0.4 to <0.5");
  file.AddHistBin(node, 15, "Hist_0.5", "Count for histogram bin 0.5 to <0.6");
  file.AddHistBin(node, 16, "Hist_0.6", "Count for histogram bin 0.6 to <0.7");
  file.AddHistBin(node, 17, "Hist_0.7", "Count for histogram bin 0.7 to <0.8");
  file.AddHistBin(node, 18, "Hist_0.8", "Count for histogram bin 0.8 to <0.9");
  file.AddHistBin(node, 19, "Hist_0.9", "Count for histogram bin 0.9 to 1.0");
  file.PrintHeaderKeys();

  return file;
}


/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: The address of the DataFile that has been created.
 *
 * Purpose: To set up the file that will be used to track mean
 * information about the free living symbionts in the world.
 *  This includes: (1) their total count, (2) the counts
 * of the free and hosted symbionts, (3) the interaction
 * values for the free and hosted symbionts, and (4) the
 * infection chances from the total population, free symbionts,
 * and hosted symbionts.
 */
emp::DataFile & SymWorld::SetUpFreeLivingSymFile(const std::string & filename){
  auto & file = SetupFile(filename);
  auto & node1 = GetSymCountDataNode(); //count
  auto & node2 = GetCountFreeSymsDataNode();
  auto & node3 = GetCountHostedSymsDataNode();
  auto & node4 = GetSymIntValDataNode(); //interaction_val
  auto & node5 = GetFreeSymIntValDataNode();
  auto & node6 = GetHostedSymIntValDataNode();
  auto & node7 = GetSymInfectChanceDataNode(); //infect chance
  auto & node8 = GetFreeSymInfectChanceDataNode();
  auto & node9 = GetHostedSymInfectChanceDataNode();

  file.AddVar(update, "update", "Update");

  //count
  file.AddTotal(node1, "count", "Total number of symbionts");
  file.AddTotal(node2, "free_syms", "Total number of free syms");
  file.AddTotal(node3, "hosted_syms", "Total number of syms in a host");


  //interaction val
  file.AddMean(node4, "mean_intval", "Average symbiont interaction value");
  file.AddMean(node5, "mean_freeintval", "Average free symbiont interaction value");
  file.AddMean(node6, "mean_hostedintval", "Average hosted symbiont interaction value");

  //infection chance
  file.AddMean(node7, "mean_infectchance", "Average symbiont infection chance");
  file.AddMean(node8, "mean_freeinfectchance", "Average free symbiont infection chance");
  file.AddMean(node9, "mean_hostedinfectchance", "Average hosted symbiont infection chance");

  file.PrintHeaderKeys();

  return file;
}


/**
 * Input: The address of the string representing the suffixes for the files to be created.
 *
 * Output: None.
 *
 * Purpose: To setup and write to the files that track the symbiont systematic information,
 * the host systematic information, and the basics about the dominant lineage of each species.
 */
void SymWorld::WritePhylogenyFile(const std::string & filename) {
  WriteDominantPhylogenyFiles("Dominant"+filename);
  sym_sys->Snapshot("SymSnapshot_"+filename);
  host_sys->Snapshot("HostSnapshot_"+filename);
}


/**
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: None.
 *
 * Purpose: To setup and write to the file that tracks the lineages of
 * the dominant (most highly populated) taxons for both symbionts and hosts.
 */
void SymWorld::WriteDominantPhylogenyFiles(const std::string & filename){
  std::ofstream phylo_file;
  phylo_file.open(filename);
  phylo_file << "phylogeny_name, dom_tax_info, dom_tax_orgcount, dom_tax_lineage\n";

  emp::vector<emp::Ptr<emp::Taxon<int>>> dom_taxons = {GetDominantSymTaxon(), GetDominantHostTaxon()};
  emp::vector<std::string> names = {"all_syms", "host"};

  if(do_free_living_syms){ //also track hosted & free symbionts
    emp::vector<emp::Ptr<emp::Taxon<int>>> dom_free_hosted = GetDominantFreeHostedSymTaxon();
    dom_taxons.push_back(dom_free_hosted[0]);
    dom_taxons.push_back(dom_free_hosted[1]);
    names.push_back("free_syms");
    names.push_back("hosted_syms");
  }

  for(size_t i = 0; i < dom_taxons.size(); i++){
    phylo_file << names[i];
    if(dom_taxons[i] != nullptr){
      std::stringstream result;
      sym_sys->PrintLineage(dom_taxons[i], result);
      std::string lineage = result.str();
      lineage.erase(0,9); //strip lineage string
      lineage.erase(lineage.end() - 1);
      std::replace( lineage.begin(), lineage.end(), '\n', '<');

      phylo_file << "," << dom_taxons[i]->GetInfo()
        << "," <<  dom_taxons[i]->GetNumOrgs()
        << "," <<   lineage << "\n";
    } else phylo_file << ",-nan,-nan,-nan\n";
  }
  phylo_file.close();
}


/**
 * Input: None
 *
 * Output: The most populated taxon amongst all symbionts
 *
 * Purpose: To determine the dominant symbiont taxon
 */
emp::Ptr<emp::Taxon<int>> SymWorld::GetDominantSymTaxon(){
  emp::Ptr<emp::Taxon<int>> dominant_taxon = nullptr;
  for (size_t i = 0; i < pop.size(); i++){
    if(sym_pop[i]){
      if((dominant_taxon == nullptr) || (sym_pop[i]->GetTaxon()->GetNumOrgs() > dominant_taxon->GetNumOrgs())){
        dominant_taxon = sym_pop[i]->GetTaxon();
      }
    }
    if(IsOccupied(i) && pop[i]->HasSym()){
      for(size_t j = 0; j < pop[i]->GetSymbionts().size(); j++){
        if((dominant_taxon == nullptr) || (pop[i]->GetSymbionts()[j]->GetTaxon()->GetNumOrgs() > dominant_taxon->GetNumOrgs())){
          dominant_taxon = pop[i]->GetSymbionts()[j]->GetTaxon();
        } //end if
      } //end for
    } //end if
  } //end for
  return dominant_taxon;
}


/**
 * Input: None
 *
 * Output: The most populated taxon amongst free symbionts and the most
 * populated taxon amongst hosted symbionts
 *
 * Purpose: To determine the dominant hosted and free symbiont taxons
 */
emp::vector<emp::Ptr<emp::Taxon<int>>> SymWorld::GetDominantFreeHostedSymTaxon(){
  emp::Ptr<emp::Taxon<int>> dominant_free_taxon = nullptr;
  emp::Ptr<emp::Taxon<int>> dominant_hosted_taxon = nullptr;
  for (size_t i = 0; i < pop.size(); i++){
    if(sym_pop[i]){ //free symbionts
      if((dominant_free_taxon == nullptr) || (sym_pop[i]->GetTaxon()->GetNumOrgs() > dominant_free_taxon->GetNumOrgs())){
        dominant_free_taxon = sym_pop[i]->GetTaxon();
      }
    }
    if(IsOccupied(i) && pop[i]->HasSym()){ //hosted symbionts
      for(size_t j = 0; j < pop[i]->GetSymbionts().size(); j++){
        if((dominant_hosted_taxon == nullptr) || (pop[i]->GetSymbionts()[j]->GetTaxon()->GetNumOrgs() > dominant_hosted_taxon->GetNumOrgs())){
          dominant_hosted_taxon = pop[i]->GetSymbionts()[j]->GetTaxon();
        } //end if
      } //end for
    } //end if
  } //end for
  return {dominant_free_taxon, dominant_hosted_taxon};
}


/**
 * Input: None
 *
 * Output: The most populated taxon amongst all hosts
 *
 * Purpose: To determine the dominant host taxon
 */
emp::Ptr<emp::Taxon<int>> SymWorld::GetDominantHostTaxon(){
  emp::Ptr<emp::Taxon<int>> dominant_taxon = nullptr;
  for (size_t i = 0; i < pop.size(); i++){
    if(IsOccupied(i)){
      if((dominant_taxon == nullptr) || (host_sys->GetTaxonAt(i)->GetNumOrgs() > dominant_taxon->GetNumOrgs())){
        dominant_taxon = host_sys->GetTaxonAt(i);
      }
    }
  }
  return dominant_taxon;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * the host count.
 *
 * Purpose: To collect data on the host count to be saved to the
 * data file that is tracking host count
 */
emp::DataMonitor<int>& SymWorld::GetHostCountDataNode() {
  if(!data_node_hostcount) {
    data_node_hostcount.New();
    OnUpdate([this](size_t){
      data_node_hostcount -> Reset();
      for (size_t i = 0; i< pop.size(); i++)
        if(IsOccupied(i))
          data_node_hostcount->AddDatum(1);
    });
  }
  return *data_node_hostcount;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * the symbiont count.
 *
 * Purpose: To collect data on the symbiont count to be saved to the
 * data file that is tracking symbiont count
 */
emp::DataMonitor<int>& SymWorld::GetSymCountDataNode() {
  if(!data_node_symcount) {
    data_node_symcount.New();
    OnUpdate([this](size_t){
      data_node_symcount -> Reset();
      for (size_t i = 0; i < pop.size(); i++){
        if(IsOccupied(i)){
          data_node_symcount->AddDatum((pop[i]->GetSymbionts()).size());
        }
        if(sym_pop[i]){
          data_node_symcount->AddDatum(1);
        }
      }
    });
  }
  return *data_node_symcount;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double>& that has the information representing
 * the count of the hosted symbionts.
 *
 * Purpose: To collect data on the count of the hosted symbionts to be saved to the
 * data file that is tracking the count of the hosted symbionts.
 */
emp::DataMonitor<int>& SymWorld::GetCountHostedSymsDataNode(){
  if (!data_node_hostedsymcount) {
    data_node_hostedsymcount.New();
    OnUpdate([this](size_t){
      data_node_hostedsymcount->Reset();
      for (size_t i = 0; i< pop.size(); i++)
        if (IsOccupied(i))
          data_node_hostedsymcount->AddDatum(pop[i]->GetSymbionts().size());
    });
  }
  return *data_node_hostedsymcount;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double>& that has the information representing
 * the count of the free symbionts.
 *
 * Purpose: To collect data on the count of the free symbionts to be saved to the
 * data file that is tracking the count of the free symbionts.
 */
emp::DataMonitor<int>& SymWorld::GetCountFreeSymsDataNode(){
  if (!data_node_freesymcount) {
    data_node_freesymcount.New();
    OnUpdate([this](size_t){
      data_node_freesymcount->Reset();
      for (size_t i = 0; i< pop.size(); i++)
        if (sym_pop[i])
          data_node_freesymcount->AddDatum(1);
    });
  }
  return *data_node_freesymcount;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * the count of the uninfected hosts
 *
 * Purpose: To collect data on the count of the uninfected hosts to be saved to the
 * data file that is tracking the count of the uninfected hosts.
 */
emp::DataMonitor<int>& SymWorld::GetUninfectedHostsDataNode() {
  //keep track of host organisms that are uninfected
  if(!data_node_uninf_hosts) {
    data_node_uninf_hosts.New();
    OnUpdate([this](size_t){
  data_node_uninf_hosts -> Reset();

  for (size_t i = 0; i < pop.size(); i++) {
    if(IsOccupied(i)) {
      if((pop[i]->GetSymbionts()).empty()) {
        data_node_uninf_hosts->AddDatum(1);
      }
    } //endif
  } //end for
}); //end OnUpdate
  } //end if
  return *data_node_uninf_hosts;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * the number of colony forming units.
 *
 * Purpose: To collect data on the CFU count to be saved to the
 * data file that is tracking CFU
 */
emp::DataMonitor<int>& SymWorld::GetCFUDataNode() {
  //keep track of host organisms that are uninfected or infected with only lysogenic phage
  if(!data_node_cfu) {
    data_node_cfu.New();
    OnUpdate([this](size_t){
      data_node_cfu -> Reset();

      for (size_t i = 0; i < pop.size(); i++) {
        if(IsOccupied(i)) {
          //uninfected hosts
          if((pop[i]->GetSymbionts()).empty()) {
            data_node_cfu->AddDatum(1);
          }

          //infected hosts, check if all symbionts are lysogenic
          if(pop[i]->HasSym()) {
            emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
            bool all_lysogenic = true;
            for(long unsigned int j = 0; j < syms.size(); j++){
              if(syms[j]->IsPhage()){
                if(syms[j]->GetLysogeny() == false){
                  all_lysogenic = false;
                }
              }
            }
            if(all_lysogenic){
              data_node_cfu->AddDatum(1);
            }
          }
        } //endif
      } //end for
  }); //end OnUpdate
} //end if
  return *data_node_cfu;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
 * the host interaction value.
 *
 * Purpose: To collect data on the host interaction value to be saved to the
 * data file that is tracking host interaction value.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetHostIntValDataNode() {
  if (!data_node_hostintval) {
    data_node_hostintval.New();
    OnUpdate([this](size_t){
      data_node_hostintval->Reset();
      for (size_t i = 0; i< pop.size(); i++)
        if (IsOccupied(i))
          data_node_hostintval->AddDatum(pop[i]->GetIntVal());
    });
  }
  return *data_node_hostintval;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
 * the symbiont interaction value.
 *
 * Purpose: To collect data on the symbiont interaction value to be saved to the
 * data file that is tracking symbionts interaction value.
 */
emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetSymIntValDataNode() {
  if (!data_node_symintval) {
    data_node_symintval.New();
    OnUpdate([this](size_t){
      data_node_symintval->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (IsOccupied(i)) {
          emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
          size_t sym_size = syms.size();
          for(size_t j=0; j< sym_size; j++){
            data_node_symintval->AddDatum(syms[j]->GetIntVal());
          }//close for
        }
        if (sym_pop[i]) {
          data_node_symintval->AddDatum(sym_pop[i]->GetIntVal());
        } //close if
      }//close for
    });
  }
  return *data_node_symintval;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double>& that has the information representing
 *  the free symbiont's interaction value.
 *
 * Purpose: To collect data on the interaction value of the free symbionts to be saved to the
 * data file that is tracking the interaction value of the free symbionts.
 */
emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetFreeSymIntValDataNode() {
  if (!data_node_freesymintval) {
    data_node_freesymintval.New();
    OnUpdate([this](size_t){
      data_node_freesymintval->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (sym_pop[i]) {
          data_node_freesymintval->AddDatum(sym_pop[i]->GetIntVal());
        } //close if
      }//close for
    });
  }
  return *data_node_freesymintval;
}


/**
 * Input:None
 *
 * Output:
 *
 * Purpose: To access the data node that is tracking
 * the hosted symbiont interaction value
 */
emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetHostedSymIntValDataNode() {
  if (!data_node_hostedsymintval) {
    data_node_hostedsymintval.New();
    OnUpdate([this](size_t){
      data_node_hostedsymintval->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (IsOccupied(i)) {
          emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
          size_t sym_size = syms.size();
          for(size_t j=0; j< sym_size; j++){
            data_node_hostedsymintval->AddDatum(syms[j]->GetIntVal());
          }//close for
        }//close if
      }//close for
    });
  }
  return *data_node_hostedsymintval;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
 * the infection chance for each symbionts.
 *
 * Purpose: To access the data node that is tracking the
 * symbiont infection chance
 */
emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetSymInfectChanceDataNode() {
  if (!data_node_syminfectchance) {
    data_node_syminfectchance.New();
    OnUpdate([this](size_t){
      data_node_syminfectchance->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (IsOccupied(i)) {
          emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
          size_t sym_size = syms.size();
          for(size_t j=0; j< sym_size; j++){
            data_node_syminfectchance->AddDatum(syms[j]->GetInfectionChance());
          }//close for
        }
        if (sym_pop[i]) {
          data_node_syminfectchance->AddDatum(sym_pop[i]->GetInfectionChance());
        } //close if
      }//close for
    });
  }
  return *data_node_syminfectchance;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
 * the free symbionts' chance of infection
 *
 *
 * Purpose: To access the data node that is tracking the
 * infection chance within the free symbionts.
 */
emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetFreeSymInfectChanceDataNode() {
  if (!data_node_freesyminfectchance) {
    data_node_freesyminfectchance.New();
    OnUpdate([this](size_t){
      data_node_freesyminfectchance->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (sym_pop[i]) {
          data_node_freesyminfectchance->AddDatum(sym_pop[i]->GetInfectionChance());
        } //close if
      }//close for
    });
  }
  return *data_node_freesyminfectchance;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double, emp::data::Histogram>& that has the information representing
 * the infection chance for the hosted symbionts
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * infection chance within the hosted symbionts.
 */
emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetHostedSymInfectChanceDataNode() {
  if (!data_node_hostedsyminfectchance) {
    data_node_hostedsyminfectchance.New();
    OnUpdate([this](size_t){
      data_node_hostedsyminfectchance->Reset();
      for (size_t i = 0; i< pop.size(); i++) {
        if (IsOccupied(i)) {
          emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
          size_t sym_size = syms.size();
          for(size_t j=0; j< sym_size; j++){
            data_node_hostedsyminfectchance->AddDatum(syms[j]->GetInfectionChance());
          }//close for
        }
      }//close for
    });
  }
  return *data_node_hostedsyminfectchance;
}
#endif