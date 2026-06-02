#ifndef TAXONDATA_H
#define TAXONDATA_H

namespace datastruct {

  struct TaxonDataBase {
      using has_fitness_t = std::false_type;
      using has_mutations_t = std::false_type;
      using has_phen_t = std::false_type;
      using taxon_info_t = double;

      emp::DataNode<double, emp::data::Current, emp::data::Range> int_val;

      void RecordIntVal(double _iv) {
        int_val.Add(_iv);
      }

      double GetIntVal() const {
        return int_val.GetMean();
      }
  };

  struct HostTaxonData : TaxonDataBase {
        std::unordered_map<unsigned long long int, int> associated_syms;
        void ClearInteractions() {associated_syms.clear();}
        void AddInteraction(emp::Ptr<emp::Taxon<taxon_info_t, TaxonDataBase>> sym) {
          if (emp::Has(associated_syms, sym->GetID())){
            associated_syms[sym->GetID()]++;
          } else {
            associated_syms[sym->GetID()] = 1;
          }
        }
  };

  struct SymbiontTaxonData : TaxonDataBase {
    size_t lineage_host_switch_count;

    void DetermineHostSwitch(emp::Ptr<emp::Taxon<taxon_info_t, datastruct::TaxonDataBase>> host, emp::Ptr<emp::Taxon<taxon_info_t, datastruct::TaxonDataBase>>  host_of_parent) {
      // check is one host is descended from the other
      bool switched_hosts = CheckIfInLineage(host, host_of_parent);

      // if not, check it the other way
      if (switched_hosts) switched_hosts = CheckIfInLineage(host_of_parent, host);

      // if neither host is descended from the other, increment the host switch count
      if (switched_hosts) lineage_host_switch_count++;
    }

    /**
    * Input: The pointer to two host taxa (a symbiont's host and its parent's host)
    *
    * Output: A boolean representing whether either host is in the other's ancestry
    *
    * Purpose: To determine whether a host switch occurred.
    */
    bool CheckIfInLineage(emp::Ptr<emp::Taxon<taxon_info_t, datastruct::TaxonDataBase>> host, emp::Ptr<emp::Taxon<taxon_info_t, datastruct::TaxonDataBase>> possible_ancestor) {
      emp::Ptr<emp::Taxon<taxon_info_t, datastruct::TaxonDataBase>> cur = host;
      while (cur != nullptr && cur->GetID() != possible_ancestor->GetID() && cur->GetOriginationTime() >= possible_ancestor->GetOriginationTime()) {
        cur = cur->GetParent();
      }
      // true if no parent found or parent found is not the possible ancestor
      return (cur == nullptr || !(cur->GetID() == possible_ancestor->GetID()));
    }

    void SetHostSwitch(size_t _in) {
      lineage_host_switch_count = _in; 
    }

    size_t GetHostSwitch() const {
      return lineage_host_switch_count;
    }
  };

}

#endif