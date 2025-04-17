#ifndef DATA_H
#define DATA_H

#include "../../Empirical/include/emp/io/File.hpp"

#include "SymWorld.h"

/**
* Input: None.
*
* Output: None.
*
* Purpose: To create and set up the data files (excluding for phylogeny) that contain data for the experiment.
*/
void SymWorld::CreateDataFiles(){
  int TIMING_REPEAT = my_config->DATA_INT();
  std::string file_ending = "_SEED"+std::to_string(my_config->SEED())+".data";

  SetupHostIntValFile(my_config->FILE_PATH()+"HostVals"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  SetupSymIntValFile(my_config->FILE_PATH()+"SymVals"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  SetUpTransmissionFile(my_config->FILE_PATH()+"TransmissionRates"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  SetupSymDiversityFile(my_config->FILE_PATH()+"SymDiversity"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  SetUpReproHistFile(my_config->FILE_PATH() + "ReproHist" + my_config->FILE_NAME() + file_ending).SetTimingRepeat(TIMING_REPEAT);
  if(my_config->FREE_LIVING_SYMS() == 1){
    SetUpFreeLivingSymFile(my_config->FILE_PATH()+"FreeLivingSyms_"+my_config->FILE_NAME()+file_ending).SetTimingRepeat(TIMING_REPEAT);
  }
  if (my_config->TAG_MATCHING()) {
    SetUpTagDistFile(my_config->FILE_PATH() + "TagDist" + my_config->FILE_NAME() + file_ending).SetTimingRepeat(TIMING_REPEAT);
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
  file.AddMax(node, "max_intval", "Maximum symbiont interaction value");
  file.AddMin(node, "min_intval", "Minimum symbiont interaction value");
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
  file.AddMax(node, "max_intval", "Maximum host interaction value");
  file.AddMin(node, "min_intval", "Minimum host interaction value");
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
 * Input: The address of the string representing the file to be
 * created's name
 *
 * Output: The address of the DataFile that has been created.
 *
 * Purpose: To set up the file that will be used to track mean
 * information about the number of reproductions in the world.[
 * If tag matching is on, this file also tracks tag similarity / 
 * dissimilarity to parents / partners. 
 */
emp::DataFile& SymWorld::SetUpReproHistFile(const std::string& filename) {
  auto& file = SetupFile(filename);
  auto& host_repro_count_node = GetHostReproCountDataNode();
  auto& sym_repro_count_node = GetSymReproCountDataNode();

  file.AddVar(update, "update", "Update");
  file.AddMean(host_repro_count_node, "host_mean_repro_count", "Average host lineage reproduction count");
  file.AddMean(sym_repro_count_node, "sym_mean_repro_count", "Average symbiont lineage reproduction count");

  if (my_config->TAG_MATCHING()) {

    auto& host_towards_partner_rate = GetHostTowardsPartnerRateDataNode();
    auto& host_from_partner_rate = GetHostFromPartnerRateDataNode();
    auto& sym_towards_partner_rate = GetSymTowardsPartnerRateDataNode();
    auto& sym_from_partner_rate = GetSymFromPartnerRateDataNode();

    file.AddMean(host_towards_partner_rate, "host_towards_partner_rate", "Average host lineage flips towards partner count divided by that lineage's repro count");
    file.AddMean(host_from_partner_rate, "host_from_partner_rate", "Average host lineage flips from partner count divided by that lineage's repro count");

    file.AddMean(sym_towards_partner_rate, "sym_towards_partner_rate", "Average symbiont lineage flips towards partner count divided by that lineage's repro count");
    file.AddMean(sym_from_partner_rate, "sym_from_partner_rate", "Average symbiont lineage flips from partner count divided by that lineage's repro count");
  }
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

  if (my_config->TRACK_PHYLOGENY_INTERACTIONS()) {
    emp::File interaction_file;
    // interaction_file << "host, symbiont, host_interaction, sym_interaction, count";
    interaction_file << "host, symbiont, count";

    for (emp::Ptr<emp::Taxon<taxon_info_t, datastruct::HostTaxonData>> t : host_sys->GetActive()) {
      for (auto interaction : t->GetData().associated_syms) {
        // It feels like there should be a better way to do this, but all the
        // obvious solutions involved converting all these values to the same
        // numerical type, which doesn't end well (since they're a mix of large
        // integers and small floating points)
        interaction_file << emp::to_string(t->GetID()) + "," +
          emp::to_string(interaction.first) + "," +
          // emp::to_string(t->GetInfo()) + "," + 
          // emp::to_string(interaction.first->GetInfo()) + "," + 
          emp::to_string(interaction.second);
      }
    }

    for (emp::Ptr<emp::Taxon<taxon_info_t, datastruct::HostTaxonData>> t : host_sys->GetAncestors()) {
      for (auto interaction : t->GetData().associated_syms) {
        // It feels like there should be a better way to do this, but all the
        // obvious solutions involved converting all these values to the same
        // numerical type, which doesn't end well (since they're a mix of large
        // integers and small floating points)
        interaction_file << emp::to_string(t->GetID()) + "," +
          emp::to_string(interaction.first) + "," +
          // emp::to_string(t->GetInfo()) + "," + 
          // emp::to_string(interaction.first->GetInfo()) + "," + 
          emp::to_string(interaction.second);
      }
    }

    for (emp::Ptr<emp::Taxon<taxon_info_t, datastruct::HostTaxonData>> t : host_sys->GetOutside()) {
      for (auto interaction : t->GetData().associated_syms) {
        // It feels like there should be a better way to do this, but all the
        // obvious solutions involved converting all these values to the same
        // numerical type, which doesn't end well (since they're a mix of large
        // integers and small floating points)
        interaction_file << emp::to_string(t->GetID()) + "," +
          emp::to_string(interaction.first) + "," +
          // emp::to_string(t->GetInfo()) + "," + 
          // emp::to_string(interaction.first->GetInfo()) + "," + 
          emp::to_string(interaction.second);
      }
    }

    interaction_file.Write("InteractionSnapshot_" + filename);
  }
  if (my_config->WRITE_CURRENT_INTERACTION_COUNTS()) {
    emp::File cur_interaction_file;
    cur_interaction_file << "host,symbiont,count";
    std::unordered_map<unsigned long long int, std::unordered_map<unsigned long long int, int>> current_interactions;

    for (size_t i = 0; i < GetSize(); i++) {
      if (IsOccupied(i)) {
        unsigned long long int host_taxon = pop[i]->GetTaxon()->GetID();
        for (auto sym : pop[i]->GetSymbionts()) {
          unsigned long long int sym_taxon = sym->GetTaxon()->GetID();
          if (!current_interactions.contains(host_taxon)) {
            std::unordered_map<unsigned long long int, int> syms_map = { std::pair(sym_taxon, 1) };
            current_interactions.emplace(host_taxon, syms_map);
          }
          else if (!current_interactions.at(host_taxon).contains(sym_taxon)) {
            current_interactions.at(host_taxon).emplace(sym_taxon, 1);
          }
          else {
            current_interactions.at(host_taxon).at(sym_taxon)++;
          }
        }
      }
    }

    for (auto host_pair : current_interactions) {
      for (auto sym_pair : host_pair.second) {
        cur_interaction_file << emp::to_string(host_pair.first) + "," + emp::to_string(sym_pair.first) + "," + emp::to_string(sym_pair.second);
      }
    }

    cur_interaction_file.Write("CurrentInteractionsSnapshot_" + filename);
  }

}

/**
 * Input: None.
 *
 * Output: None.
 *
 * Purpose: Helper function that makes map of all the symbiont taxa associated with each host taxon,
 * (including counts of how common each interaction is)
 */
void SymWorld::MapPhylogenyInteractions() {
  for (emp::Ptr<emp::Taxon<taxon_info_t, datastruct::HostTaxonData>> t : host_sys->GetActive()) {
    t->GetData().ClearInteractions();
  }

  for (size_t pos = 0; pos < pop.size(); pos++) {
    if (!IsOccupied(pos)) {
      continue;
    }
    datastruct::HostTaxonData & host_data = host_sys->GetTaxonAt(pos)->GetData();
    for (emp::Ptr<Organism> sym : pop[pos]->GetSymbionts()) {
      host_data.AddInteraction(sym->GetTaxon());
    }

  }

}

/**
 * Input: The address of the string representing the suffixes for the files to be created.
 *
 * Output: None.
 *
 * Purpose: To setup and write to the files that track the counts of attempted
 * transmissions.
 */

emp::DataFile & SymWorld::SetUpTransmissionFile(const std::string & filename){
  auto & file = SetupFile(filename);
  auto & node1 = GetHorizontalTransmissionAttemptCount();
  auto & node2 = GetHorizontalTransmissionSuccessCount();
  auto & node3 = GetVerticalTransmissionAttemptCount();
  auto & node4 = GetVerticalTransmissionSuccessCount(); 
  auto & node5 = GetHorizontalTransmissionTagFailCount();
  auto & node6 = GetHorizontalTransmissionSizeFailCount();

  file.AddVar(update, "update", "Update");
  //horizontal transmission
  file.AddHistBin(node1, 0, "horiz_attempt_-1_-0.6", "Count for histogram bin for horizontal attempts with int val -1 to <-0.6");
  file.AddHistBin(node1, 1, "horiz_attempt_-0.6_-0.2", "Count for histogram bin for horizontal attempts with int val -0.6 to <-0.2");
  file.AddHistBin(node1, 2, "horiz_attempt_-0.2_0.2", "Count for histogram bin for horizontal attempts with int val -0.2 to <0.2");
  file.AddHistBin(node1, 3, "horiz_attempt_0.2_0.6", "Count for histogram bin for horizontal attempts with int val 0.2 to <0.6");
  file.AddHistBin(node1, 4, "horiz_attempt_0.6_1", "Count for histogram bin for horizontal attempts with int val 0.6 to 1", true);

  file.AddHistBin(node2, 0, "horiz_success_-1_-0.6", "Count for histogram bin for horizontal successes with int val -1 to <-0.6");
  file.AddHistBin(node2, 1, "horiz_success_-0.6_-0.2", "Count for histogram bin for horizontal successes with int val -0.6 to <-0.2");
  file.AddHistBin(node2, 2, "horiz_success_-0.2_0.2", "Count for histogram bin for horizontal successes with int val -0.2 to <0.2");
  file.AddHistBin(node2, 3, "horiz_success_0.2_0.6", "Count for histogram bin for horizontal successes with int val 0.2 to <0.6");
  file.AddHistBin(node2, 4, "horiz_success_0.6_1", "Count for histogram bin for horizontal successes with int val 0.6 to 1", true);

  //vertical transmission
  file.AddHistBin(node3, 0, "vert_attempt_-1_-0.6", "Count for histogram bin for vertical attempts with int val -1 to <-0.6");
  file.AddHistBin(node3, 1, "vert_attempt_-0.6_-0.2", "Count for histogram bin for vertical attempts with int val -0.6 to <-0.2");
  file.AddHistBin(node3, 2, "vert_attempt_-0.2_0.2", "Count for histogram bin for vertical attempts with int val -0.2 to <0.2");
  file.AddHistBin(node3, 3, "vert_attempt_0.2_0.6", "Count for histogram bin for vertical attempts with int val 0.2 to <0.6");
  file.AddHistBin(node3, 4, "vert_attempt_0.6_1", "Count for histogram bin for vertical attempts with int val 0.6 to 1", true);

  file.AddHistBin(node4, 0, "vert_success_-1_-0.6", "Count for histogram bin for vertical successes with int val -1 to <-0.6");
  file.AddHistBin(node4, 1, "vert_success_-0.6_-0.2", "Count for histogram bin for vertical successes with int val -0.6 to <-0.2");
  file.AddHistBin(node4, 2, "vert_success_-0.2_0.2", "Count for histogram bin for vertical successes with int val -0.2 to <0.2");
  file.AddHistBin(node4, 3, "vert_success_0.2_0.6", "Count for histogram bin for vertical successes with int val 0.2 to <0.6");
  file.AddHistBin(node4, 4, "vert_success_0.6_1", "Count for histogram bin for vertical successes with int val 0.6 to 1", true);

  // horiz failure 
  file.AddHistBin(node5, 0, "horiz_tagfail_-1_-0.6", "Count for histogram bin for horizontal tag failure with int val -1 to <-0.6");
  file.AddHistBin(node5, 1, "horiz_tagfail_-0.6_-0.2", "Count for histogram bin for horizontal tag failure with int val -0.6 to <-0.2");
  file.AddHistBin(node5, 2, "horiz_tagfail_-0.2_0.2", "Count for histogram bin for horizontal tag failure with int val -0.2 to <0.2");
  file.AddHistBin(node5, 3, "horiz_tagfail_0.2_0.6", "Count for histogram bin for horizontal tag failure with int val 0.2 to <0.6");
  file.AddHistBin(node5, 4, "horiz_tagfail_0.6_1", "Count for histogram bin for horizontal tag failure with int val 0.6 to 1", true);

  file.AddHistBin(node6, 0, "horiz_sizefail_-1_-0.6", "Count for histogram bin for horizontal size failure with int val -1 to <-0.6");
  file.AddHistBin(node6, 1, "horiz_sizefail_-0.6_-0.2", "Count for histogram bin for horizontal size failure with int val -0.6 to <-0.2");
  file.AddHistBin(node6, 2, "horiz_sizefail_-0.2_0.2", "Count for histogram bin for horizontal size failure with int val -0.2 to <0.2");
  file.AddHistBin(node6, 3, "horiz_sizefail_0.2_0.6", "Count for histogram bin for horizontal size failure with int val 0.2 to <0.6");
  file.AddHistBin(node6, 4, "horiz_sizefail_0.6_1", "Count for histogram bin for horizontal size failure with int val 0.6 to 1", true);

  file.PrintHeaderKeys();

  return file;
}

/**
 * Input: The address of the string representing the suffixes for the files to be created.
 *
 * Output: None.
 *
 * Purpose: To write the tag distances of host-symbionts over time.
 */
emp::DataFile& SymWorld::SetUpTagDistFile(const std::string& filename) {
  auto& file = SetupFile(filename);
  auto& tag_dist_node = GetTagDistanceDataNode();

  auto& host_tag_shannon = GetHostTagShannonDiversity();
  auto& symbiont_tag_richness = GetSymbiontTagRichness();
  auto& symbiont_tag_shannon = GetSymbiontTagShannonDiversity();
  auto& host_tag_richness = GetHostTagRichness();

  file.AddVar(update, "update", "Update");
  file.AddMean(tag_dist_node, "mean_tag_distance", "The mean tag distance between symbionts and their hosts");
  file.AddTotal(host_tag_richness, "host_tag_richness", "The host tag richness");
  file.AddTotal(host_tag_shannon, "host_tag_shannon", "The host tag shannon diversity");
  file.AddTotal(symbiont_tag_richness, "symbiont_tag_richness", "The symbiont tag richness");
  file.AddTotal(symbiont_tag_shannon, "symbiont_tag_shannon", "The symbiont tag shannon diversity");
  file.AddHistBin(tag_dist_node, 0, "tag_0.1", "Count for tag distance histogram bin 0 to <0.1");
  file.AddHistBin(tag_dist_node, 1, "tag_0.2", "Count for tag distance histogram bin 0.1 to <0.2");
  file.AddHistBin(tag_dist_node, 2, "tag_0.3", "Count for tag distance histogram bin 0.2 to <0.3");
  file.AddHistBin(tag_dist_node, 3, "tag_0.4", "Count for tag distance histogram bin 0.3 to <0.4");
  file.AddHistBin(tag_dist_node, 4, "tag_0.5", "Count for tag distance histogram bin 0.4 to <0.5");
  file.AddHistBin(tag_dist_node, 5, "tag_0.6", "Count for tag distance histogram bin 0.5 to <0.6");
  file.AddHistBin(tag_dist_node, 6, "tag_0.7", "Count for tag distance histogram bin 0.6 to <0.7");
  file.AddHistBin(tag_dist_node, 7, "tag_0.8", "Count for tag distance histogram bin 0.7 to <0.8");
  file.AddHistBin(tag_dist_node, 8, "tag_0.9", "Count for tag distance histogram bin 0.8 to <0.9");
  file.AddHistBin(tag_dist_node, 9, "tag_1.0", "Count for tag distance histogram bin 0.9 to 1.0");
  
  file.PrintHeaderKeys();

  return file;
}

/**
 * Input: The address of the string representing the suffixes for the files to be created.
 *
 * Output: None.
 *
 * Purpose: To write the tags of hosts and symbionts to a data file after an experiment is 
 * concluded
 */
void SymWorld::WriteOrgDumpFile(const std::string& filename) {
  std::ofstream out_file(filename);
  out_file << "host_int,sym_int,host_repro_count,host_towards_partner_count,host_from_partner_count," << 
    "sym_repro_count,sym_towards_partner_count,sym_from_partner_count";
  if (my_config->TAG_MATCHING()) out_file << ",host_tag,sym_tag,tag_distance";
  out_file << "\n";

  for (size_t i = 0; i < size(); i++) {
    if (IsOccupied(i)) {
      if (pop[i]->HasSym()) {
        emp::vector<emp::Ptr<Organism>> symbionts = pop[i]->GetSymbionts();
        for (size_t j = 0; j < symbionts.size(); j++) {
          out_file << pop[i]->GetIntVal() << "," << symbionts[j]->GetIntVal() << "," << pop[i]->GetReproCount() << 
            "," << pop[i]->GetTowardsPartnerCount() << "," << pop[i]->GetFromPartnerCount() << 
            "," << symbionts[j]->GetReproCount() << "," << symbionts[j]->GetTowardsPartnerCount() << 
            "," << symbionts[j]->GetFromPartnerCount();
          if (my_config->TAG_MATCHING()) {
            out_file << "," << pop[i]->GetTag().ToBinaryString() << "," << symbionts[j]->GetTag().ToBinaryString() << 
              "," << hamming_metric->calculate(pop[i]->GetTag(), symbionts[j]->GetTag());
          }
        }
      }
      else {
        out_file << pop[i]->GetIntVal() << ",," << pop[i]->GetReproCount() << "," << 
          pop[i]->GetTowardsPartnerCount() << "," << pop[i]->GetFromPartnerCount() << ",,,";
        if (my_config->TAG_MATCHING()) {
          out_file << "," << pop[i]->GetTag().ToBinaryString() << ",,";
        }
      }
      out_file << "\n";
    }
  }
  out_file.close();
}

void SymWorld::WriteTagMatrixFile(const std::string& filename) {
  std::ofstream out_file(filename);
  
  // write the ID of every sym
  out_file << ',';
  int id = 0;
  for (size_t i = 0; i < size(); i++) {
    if (IsOccupied(i)) {
      if (pop[i]->HasSym()) {
        emp::vector<emp::Ptr<Organism>> symbionts = pop[i]->GetSymbionts();
        for (size_t j = 0; j < symbionts.size(); j++) {
          out_file << std::to_string(id) << ",";
          if(j>0) id++; // needed for multi-infection
        }
      }
      id++;
    }
    
  }
  out_file << "\n";

  // for every host, calculate the tag distance to every sym
  for (size_t k = 0; k < size(); k++) {
    if (IsOccupied(k)) {
      out_file << k << ',';

      // calculate tag distance to every sym
      for (size_t i = 0; i < size(); i++) {
        if (IsOccupied(i) && pop[i]->HasSym()) {
          emp::vector<emp::Ptr<Organism>> symbionts = pop[i]->GetSymbionts();
          for (size_t j = 0; j < symbionts.size(); j++) {
            out_file << hamming_metric->calculate(pop[k]->GetTag(), symbionts[j]->GetTag()) << ",";
          }
        }
      }
      out_file << "\n";
    }
  }
  out_file.close();
}

  emp::DataFile & SymWorld::SetupSymDiversityFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetWithinHostVarianceDataNode();
    auto & node1 = GetWithinHostMeanDataNode();
    node.SetupBins(-0.05, 1.5, 21); //Necessary because range exclusive
    node1.SetupBins(-1.0, 1.1, 21); //Necessary because range exclusive
    file.AddVar(update, "update", "Update");
    file.AddHistBin(node, 0, "Variance_Hist_0", "Count for histogram bin 0");
    file.AddHistBin(node, 1, "Variance_Hist_1", "Count for histogram bin 1");
    file.AddHistBin(node, 2, "Variance_Hist_2", "Count for histogram bin 2");
    file.AddHistBin(node, 3, "Variance_Hist_3", "Count for histogram bin 3");
    file.AddHistBin(node, 4, "Variance_Hist_4", "Count for histogram bin 4");
    file.AddHistBin(node, 5, "Variance_Hist_5", "Count for histogram bin 5");
    file.AddHistBin(node, 6, "Variance_Hist_6", "Count for histogram bin 6");
    file.AddHistBin(node, 7, "Variance_Hist_7", "Count for histogram bin 7");
    file.AddHistBin(node, 8, "Variance_Hist_8", "Count for histogram bin 8");
    file.AddHistBin(node, 9, "Variance_Hist_9", "Count for histogram bin 9");
    file.AddHistBin(node, 10, "Variance_Hist_10", "Count for histogram bin 10");
    file.AddHistBin(node, 11, "Variance_Hist_11", "Count for histogram bin 11");
    file.AddHistBin(node, 12, "Variance_Hist_12", "Count for histogram bin 12");
    file.AddHistBin(node, 13, "Variance_Hist_13", "Count for histogram bin 13");
    file.AddHistBin(node, 14, "Variance_Hist_14", "Count for histogram bin 14");
    file.AddHistBin(node, 15, "Variance_Hist_15", "Count for histogram bin 15");
    file.AddHistBin(node, 16, "Variance_Hist_16", "Count for histogram bin 16");
    file.AddHistBin(node, 17, "Variance_Hist_17", "Count for histogram bin 17");
    file.AddHistBin(node, 18, "Variance_Hist_18", "Count for histogram bin 18");
    file.AddHistBin(node, 19, "Variance_Hist_19", "Count for histogram bin 19");
    file.AddHistBin(node, 20, "Variance_Hist_20", "Count for histogram bin 20");

    file.AddHistBin(node1, 0, "Mean_Hist_-1", "Count for histogram bin -1 to <-0.9");
    file.AddHistBin(node1, 1, "Mean_Hist_-0.9", "Count for histogram bin -0.9 to <-0.8");
    file.AddHistBin(node1, 2, "Mean_Hist_-0.8", "Count for histogram bin -0.8 to <-0.7");
    file.AddHistBin(node1, 3, "Mean_Hist_-0.7", "Count for histogram bin -0.7 to <-0.6");
    file.AddHistBin(node1, 4, "Mean_Hist_-0.6", "Count for histogram bin -0.6 to <-0.5");
    file.AddHistBin(node1, 5, "Mean_Hist_-0.5", "Count for histogram bin -0.5 to <-0.4");
    file.AddHistBin(node1, 6, "Mean_Hist_-0.4", "Count for histogram bin -0.4 to <-0.3");
    file.AddHistBin(node1, 7, "Mean_Hist_-0.3", "Count for histogram bin -0.3 to <-0.2");
    file.AddHistBin(node1, 8, "Mean_Hist_-0.2", "Count for histogram bin -0.2 to <-0.1");
    file.AddHistBin(node1, 9, "Mean_Hist_-0.1", "Count for histogram bin -0.1 to <0.0");
    file.AddHistBin(node1, 10, "Mean_Hist_0.0", "Count for histogram bin 0.0 to <0.1");
    file.AddHistBin(node1, 11, "Mean_Hist_0.1", "Count for histogram bin 0.1 to <0.2");
    file.AddHistBin(node1, 12, "Mean_Hist_0.2", "Count for histogram bin 0.2 to <0.3");
    file.AddHistBin(node1, 13, "Mean_Hist_0.3", "Count for histogram bin 0.3 to <0.4");
    file.AddHistBin(node1, 14, "Mean_Hist_0.4", "Count for histogram bin 0.4 to <0.5");
    file.AddHistBin(node1, 15, "Mean_Hist_0.5", "Count for histogram bin 0.5 to <0.6");
    file.AddHistBin(node1, 16, "Mean_Hist_0.6", "Count for histogram bin 0.6 to <0.7");
    file.AddHistBin(node1, 17, "Mean_Hist_0.7", "Count for histogram bin 0.7 to <0.8");
    file.AddHistBin(node1, 18, "Mean_Hist_0.8", "Count for histogram bin 0.8 to <0.9");



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
      for (size_t i = 0; i< pop.size(); i++){
        if (IsOccupied(i)){
          data_node_hostcount->AddDatum(1);
        }
      }
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
      for (size_t i = 0; i< pop.size(); i++){
        if (IsOccupied(i)){
          data_node_hostedsymcount->AddDatum(pop[i]->GetSymbionts().size());
        }
      }
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
      for (size_t i = 0; i< pop.size(); i++){
        if (sym_pop[i]){
          data_node_freesymcount->AddDatum(1);
        }
      }
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
      for (size_t i = 0; i< pop.size(); i++){
        if (IsOccupied(i)){
          data_node_hostintval->AddDatum(pop[i]->GetIntVal());
        }
      }
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
 * Output: The DataMonitor<double,emp::data::Histogram>& that has the information representing
 * how many attempts were made to horizontally transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of attempted horizontal transmissions.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetHorizontalTransmissionAttemptCount() {
  if (!data_node_attempts_horiztrans) {
    data_node_attempts_horiztrans.New();
    data_node_attempts_horiztrans->SetupBins(-1.0, 1.1, 6);
  }
  
  return *data_node_attempts_horiztrans;
}

/**
 * Input: None
 *
 * Output: The DataMonitor<double,emp::data::Histogram>& that has the information representing
 * how many horizontal transmissions failed ONLY due to tag mismatch.
 *
 * Purpose: To retrieve the data nodes that is tracking how many horizontal transmissions 
 * failed ONLY due to tag mismatch.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetHorizontalTransmissionTagFailCount() {
  if (!data_node_tagfail_horiztrans) {
    data_node_tagfail_horiztrans.New();
    data_node_tagfail_horiztrans->SetupBins(-1.0, 1.1, 6);
  }

  return *data_node_tagfail_horiztrans;
}

/**
 * Input: None
 *
 * Output: The DataMonitor<double,emp::data::Histogram>& that has the information representing
 * how many horizontal transmissions failed ONLY due to insufficient space in the host.
 *
 * Purpose: To retrieve the data nodes that is tracking how many horizontal transmissions failed 
 * ONLY due to insufficient space in the host
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetHorizontalTransmissionSizeFailCount() {
  if (!data_node_sizefail_horiztrans) {
    data_node_sizefail_horiztrans.New();
    data_node_sizefail_horiztrans->SetupBins(-1.0, 1.1, 6);
  }

  return *data_node_sizefail_horiztrans;
}



/**
 * Input: None
 *
 * Output: The DataMonitor<double,emp::data::Histogram>& that has the information representing
 * how many successful attempts were made to horizontally transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of successful horizontal transmissions.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetHorizontalTransmissionSuccessCount() {
  if (!data_node_successes_horiztrans) {
    data_node_successes_horiztrans.New();
    data_node_successes_horiztrans->SetupBins(-1.0, 1.1, 6);
  }
  
  return *data_node_successes_horiztrans;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double,emp::data::Histogram>& that has the information representing
 * how many attempts were made to vertically transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of attempted vertical transmissions.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetVerticalTransmissionAttemptCount() {
  if (!data_node_attempts_verttrans) {
    data_node_attempts_verttrans.New();
    data_node_attempts_verttrans->SetupBins(-1.0, 1.1, 6);
  }
  return *data_node_attempts_verttrans;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double,emp::data::Histogram>& that has the information representing
 * how many successful attempts were made to vertically transmit.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * number of successful vertical transmissions.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetVerticalTransmissionSuccessCount() {
  if (!data_node_successes_verttrans) {
    data_node_successes_verttrans.New();
    data_node_successes_verttrans->SetupBins(-1.0, 1.1, 6);
  }
  
  return *data_node_successes_verttrans;
}


/**
 * Input: None
 *
 * Output: The DataMonitor<double>& that has the information representing
 * the average tag distance between hosts and their symbionts.
 *
 * Purpose: To retrieve the data nodes that is tracking the
 * average tag distance between hosts and their symbionts.
 */
emp::DataMonitor<double, emp::data::Histogram>& SymWorld::GetTagDistanceDataNode() {
  if (!data_node_tag_dist) {
    data_node_tag_dist.New();
    OnUpdate([this](size_t) {
      data_node_tag_dist->Reset();
      for (size_t i = 0; i < pop.size(); i++) {
        if (IsOccupied(i)) {
          if (pop[i]->HasSym()) {
            emp::vector<emp::Ptr<Organism>> symbionts = pop[i]->GetSymbionts();
            for (size_t j = 0; j < symbionts.size(); j++) {
              double distance = hamming_metric->calculate(pop[i]->GetTag(), symbionts[j]->GetTag());
              data_node_tag_dist->AddDatum(distance);
            }
          }
        } //endif
      } //end for
    }); //end OnUpdate
  } //end if
  data_node_tag_dist->SetupBins(0, 1.1, 11);
  return *data_node_tag_dist;
}

  emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetWithinHostVarianceDataNode() {
    if (!data_node_within_host_variance) {
      data_node_within_host_variance.New();
      OnUpdate([this](size_t){
        data_node_within_host_variance->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i) && pop[i]->IsHost() && pop[i]->HasSym()) {
	          emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
	          size_t sym_size = syms.size();
            if (sym_size > 1) { // Can't take the variance of 1 thing
              emp::vector<double> int_vals(sym_size);
              for(size_t j=0; j< sym_size; j++){
                int_vals[j] = syms[j]->GetIntVal();
              }//close for
              data_node_within_host_variance->AddDatum(emp::Variance(int_vals));
  	        } else {
              data_node_within_host_variance->AddDatum(0);
            }

          } //close if
	      }//close for
      });
    }
    return *data_node_within_host_variance;
  }

  emp::DataMonitor<double,emp::data::Histogram>& SymWorld::GetWithinHostMeanDataNode() {
    if (!data_node_within_host_mean) {
      data_node_within_host_mean.New();
      OnUpdate([this](size_t){
        data_node_within_host_mean->Reset();
        for (size_t i = 0; i< pop.size(); i++) {
          if (IsOccupied(i) && pop[i]->IsHost() && pop[i]->HasSym()) {
	          emp::vector<emp::Ptr<Organism>>& syms = pop[i]->GetSymbionts();
	          size_t sym_size = syms.size();
	          emp::vector<double> int_vals(sym_size);
            for(size_t j=0; j< sym_size; j++){
	            int_vals[j] = syms[j]->GetIntVal();
	          }//close for
            data_node_within_host_mean->AddDatum(emp::Mean(int_vals));
	        }//close if
	      }//close for
      });
    }
    return *data_node_within_host_mean;
  }


  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the average number of reproductions each host lineage has accumulated.
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * average number of reproductions each host lineage has accumulated.
   */
  emp::DataMonitor<size_t>& SymWorld::GetHostReproCountDataNode() {
    if (!data_node_host_repro_count) {
      data_node_host_repro_count.New();
      OnUpdate([this](size_t) {
        data_node_host_repro_count->Reset();
        data_node_sym_repro_count->Reset();
        for (size_t i = 0; i < pop.size(); i++) {
          if (IsOccupied(i) && pop[i]->IsHost()) {
            data_node_host_repro_count->AddDatum(pop[i]->GetReproCount());
            for (emp::Ptr<Organism> sym : pop[i]->GetSymbionts()) {
              data_node_sym_repro_count->AddDatum(sym->GetReproCount());
            }
          }
        }
      });
    }
    return *data_node_host_repro_count;
  }


  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the average number of reproductions each symbiont lineage has accumulated.
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * average number of reproductions each symbiont lineage has accumulated.
   */
  emp::DataMonitor<size_t>& SymWorld::GetSymReproCountDataNode() {
    if (!data_node_sym_repro_count) {
      data_node_sym_repro_count.New();
    }
    return *data_node_sym_repro_count;
  }


  /**
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   * the average number of flips towards a partner each host lineage has accumulated.
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * average number of flips towards a partner each host lineage has accumulated.
   */
  emp::DataMonitor<double>& SymWorld::GetHostTowardsPartnerRateDataNode() {
    if (!data_node_host_towards_partner_rate) {
      data_node_host_towards_partner_rate.New();
      OnUpdate([this](size_t) {
        data_node_host_towards_partner_rate->Reset();
        data_node_host_from_partner_rate->Reset();
        data_node_sym_towards_partner_rate->Reset();
        data_node_sym_from_partner_rate->Reset();

        for (size_t i = 0; i < pop.size(); i++) {
          if (IsOccupied(i) && pop[i]->IsHost()) {
            data_node_host_towards_partner_rate->AddDatum((double)pop[i]->GetTowardsPartnerCount() / (double)pop[i]->GetReproCount());
            data_node_host_from_partner_rate->AddDatum((double)pop[i]->GetFromPartnerCount() / (double)pop[i]->GetReproCount());
            for (emp::Ptr<Organism> sym : pop[i]->GetSymbionts()) {
              data_node_sym_towards_partner_rate->AddDatum((double)sym->GetTowardsPartnerCount() / (double)sym->GetReproCount());
              data_node_sym_from_partner_rate->AddDatum((double)sym->GetFromPartnerCount() / (double)sym->GetReproCount());
            }
          }
        }
        });
    }
    return *data_node_host_towards_partner_rate;
  }


  /**
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   * the average number of flips from a partner each host lineage has accumulated.
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * average number of flips from a partner each host lineage has accumulated.
   */
  emp::DataMonitor<double>& SymWorld::GetHostFromPartnerRateDataNode() {
    if (!data_node_host_from_partner_rate) {
      data_node_host_from_partner_rate.New();
    }
    return *data_node_host_from_partner_rate;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   * the average number of flips towards a partner each symbiont lineage has accumulated.
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * average number of flips towards a partner each symbiont lineage has accumulated.
   */
  emp::DataMonitor<double>& SymWorld::GetSymTowardsPartnerRateDataNode() {
    if (!data_node_sym_towards_partner_rate) {
      data_node_sym_towards_partner_rate.New();
    }
    return *data_node_sym_towards_partner_rate;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<double>& that has the information representing
   * the average number of flips from a partner each symbiont lineage has accumulated.
   *
   * Purpose: To retrieve the data nodes that is tracking the
   * average number of flips from a partner each symbiont lineage has accumulated.
   */
  emp::DataMonitor<double>& SymWorld::GetSymFromPartnerRateDataNode() {
    if (!data_node_sym_from_partner_rate) {
      data_node_sym_from_partner_rate.New();
    }
    return *data_node_sym_from_partner_rate;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the host tag richness this update.
   *
   * Purpose: To retrieve the data node that is tracking the
   * host tag richness this update.
   */
  emp::DataMonitor<int>& SymWorld::GetHostTagRichness() {
    if (!data_node_host_tag_richness) {
      data_node_host_tag_richness.New();
      OnUpdate([this](size_t) {
        emp::vector<emp::BitSet<32>> host_tags;
        emp::vector<emp::BitSet<32>> symbiont_tags;
        data_node_host_tag_richness->Reset();
        data_node_host_tag_shannon->Reset();
        data_node_symbiont_tag_richness->Reset();
        data_node_symbiont_tag_shannon->Reset();

        for (size_t i = 0; i < pop.size(); i++) {
          if (IsOccupied(i) && pop[i]->IsHost()) {
            host_tags.push_back(pop[i]->GetTag());
            for (emp::Ptr<Organism> sym : pop[i]->GetSymbionts()) {
              symbiont_tags.push_back(sym->GetTag());
            }
          }
        }

        data_node_host_tag_richness->AddDatum(emp::UniqueCount(host_tags));
        data_node_host_tag_shannon->AddDatum(emp::ShannonEntropy(host_tags));
        data_node_symbiont_tag_richness->AddDatum(emp::UniqueCount(symbiont_tags));
        data_node_symbiont_tag_shannon->AddDatum(emp::ShannonEntropy(symbiont_tags));
        });
    }
    return *data_node_host_tag_richness;


    if (!data_node_host_tag_richness) {
      data_node_host_tag_richness.New();
    }
    return *data_node_host_tag_richness;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the host tag shannon this update.
   *
   * Purpose: To retrieve the data node that is tracking the
   * host tag shannon this update.
   */
  emp::DataMonitor<double>& SymWorld::GetHostTagShannonDiversity() {
    if (!data_node_host_tag_shannon) {
      data_node_host_tag_shannon.New();
    }
    return *data_node_host_tag_shannon;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the host tag shannon this update.
   *
   * Purpose: To retrieve the data node that is tracking the
   * symbiont tag richness this update.
   */
  emp::DataMonitor<int>& SymWorld::GetSymbiontTagRichness() {
    if (!data_node_symbiont_tag_richness) {
      data_node_symbiont_tag_richness.New();
    }
    return *data_node_symbiont_tag_richness;
  }

  /**
   * Input: None
   *
   * Output: The DataMonitor<int>& that has the information representing
   * the symbiont tag shannon this update.
   *
   * Purpose: To retrieve the data node that is tracking the
   * host symbiont shannon this update.
   */
  emp::DataMonitor<double>& SymWorld::GetSymbiontTagShannonDiversity() {
    if (!data_node_symbiont_tag_shannon) {
      data_node_symbiont_tag_shannon.New();
    }
    return *data_node_symbiont_tag_shannon;
  }
  
#endif
