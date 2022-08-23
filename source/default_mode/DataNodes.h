#ifndef DATA_H
#define DATA_H

#include "SymWorld.h"

/**
* Input: None.
*
* Output: None.
*
* Purpose: To create and set up the data files (excluding for phylogeny) that contain data for the experiment.
*/
void SymWorld::CreateDateFiles(){
  int TIMING_REPEAT = my_config->DATA_INT();
  std::string file_ending = "_SEED"+std::to_string(my_config->SEED())+".data";

  SetupHostIntValFile(my_config->FILE_PATH()+"HostVals"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  SetupSymIntValFile(my_config->FILE_PATH()+"SymVals"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  SetUpTransmissionFile(my_config->FILE_PATH()+"TransmissionRates"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);

  if(my_config->FREE_LIVING_SYMS() == 1){
    SetUpFreeLivingSymFile(my_config->FILE_PATH()+"FreeLivingSyms_"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  }
}

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
 * host's interaction values. Prints header keys to the file.
 */
emp::DataFile & SymWorld::SetupHostIntValFile(const std::string & filename) {
  auto & file = SetupFile(filename);
  SetupHostFileColumns(file);
  file.PrintHeaderKeys();
  return file;
}


/**
 * Input: The Empirical DataFile object tracking data nodes.
 *
 * Output: None.
 *
 * Purpose: To define which data nodes should be tracked by this data file. Defines
 * what columns should be called.
 */
void SymWorld::SetupHostFileColumns(emp::DataFile & file){
  auto & node = GetHostIntValDataNode();
  auto & node1 = GetHostCountDataNode();
  auto & uninf_hosts_node = GetUninfectedHostsDataNode();

  file.AddVar(update, "update", "Update");
  file.AddMean(node, "mean_intval", "Average host interaction value");
  file.AddTotal(node1, "count", "Total number of hosts");
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
 * Purpose: To setup and write to the files that track the symbiont systematic information and
 * the host systematic information
 */
void SymWorld::WritePhylogenyFile(const std::string & filename) {
  sym_sys->Snapshot("SymSnapshot_"+filename);
  host_sys->Snapshot("HostSnapshot_"+filename);
}


/**
 * Input: The address of the string representing the suffixes for the files to be created.
 *
 * Output: None.
 *
 * Purpose: To setup and write to the files that track the counts of attempted
 * tranmissions.
 */

emp::DataFile & SymWorld::SetUpTransmissionFile(const std::string & filename){
  auto & file = SetupFile(filename);
  auto & node1 = GetHorizontalTransmissionAttemptCount();
  auto & node2 = GetHorizontalTransmissionSuccessCount();
  auto & node3 = GetVerticalTransmissionAttemptCount();
  auto & node4 = GetVerticalTransmissionSuccessCount();

  file.AddVar(update, "update", "Update");

  //horizontal transmission
  file.AddTotal(node1, "attempts_horiztrans", "Total number of horizontal transmission attempts", true);
  file.AddTotal(node2, "successes_horiztrans", "Total number of horizontal transmission successes", true);

  //vertical transmission
  file.AddTotal(node3, "attempts_verttrans", "Total number of vertical transmission attempts", true);
  file.AddTotal(node4, "successes_verttrans", "Total number of vertical transmission successes", true);

  file.PrintHeaderKeys();

  return file;
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
  data_node_hostintval->SetupBins(-1.0, 1.1, 21);
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
  data_node_symintval->SetupBins(-1.0, 1.1, 21);
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
  data_node_freesymintval->SetupBins(-1.0, 1.1, 21);
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
  data_node_hostedsymintval->SetupBins(-1.0, 1.1, 21);
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
  data_node_syminfectchance->SetupBins(0, 1.1, 11);
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
  data_node_freesyminfectchance->SetupBins(0, 1.1, 11);
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
  data_node_hostedsyminfectchance->SetupBins(0, 1.1, 11);
  return *data_node_hostedsyminfectchance;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * how many attempts were made to horizontally transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of attempted horizontal transmissions.
 */
emp::DataMonitor<int>& SymWorld::GetHorizontalTransmissionAttemptCount() {
  if (!data_node_attempts_horiztrans) {
    data_node_attempts_horiztrans.New();
  }
  return *data_node_attempts_horiztrans;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * how many successful attempts were made to horizontally transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of successful horizontal transmissions.
 */
emp::DataMonitor<int>& SymWorld::GetHorizontalTransmissionSuccessCount() {
  if (!data_node_successes_horiztrans) {
    data_node_successes_horiztrans.New();
  }
  return *data_node_successes_horiztrans;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * how many attempts were made to vertically transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of attempted vertical transmissions.
 */
emp::DataMonitor<int>& SymWorld::GetVerticalTransmissionAttemptCount() {
  if (!data_node_attempts_verttrans) {
    data_node_attempts_verttrans.New();
  }
  return *data_node_attempts_verttrans;
}

/**
 * Input: None
 *
 * Output: The DataMonitor<int>& that has the information representing
 * how many attempts to vertically transmit were successful.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of successful vertical transmissions.
 */
emp::DataMonitor<int>& SymWorld::GetVerticalTransmissionSuccessCount() {
  if (!data_node_successes_verttrans) {
    data_node_successes_verttrans.New();
  }
  return *data_node_successes_verttrans;
}

#endif
