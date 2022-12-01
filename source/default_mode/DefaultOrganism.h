#ifndef DEFAULT_ORG_H
#define DEFAULT_ORG_H

#include "../Organism.h"

class DefaultOrganism : public virtual Organism {
protected:
  /**
   * Purpose: Represents the interaction value between the host and symbiont.
   * A negative interaction value represent antagonism, while a positive
   * one represents mutualism. Zero is a neutral value.
   */
  double interaction_val = 0;

public:
  DefaultOrganism(double interaction_val) : interaction_val(interaction_val) {
    if (interaction_val == -2) {
      interaction_val = random->GetDouble(-1, 1);
    }
    if (interaction_val > 1 || interaction_val < -1) {
      // Exception for invalid interaction value
      throw "Invalid interaction value. Must be between -1 and 1";
    };
  }

  /**
   * Input: None
   *
   * Output: The double representing the organism's interaction value
   *
   * Purpose: To get a organism's interaction value.
   */
  double GetIntVal() const { return interaction_val; }

  int GetPhyloBin() const override {
    size_t num_phylo_bins = my_config->NUM_PHYLO_BINS();
    // classify orgs into bins base on interaction values,
    // inclusive of lower bound, exclusive of upper
    float size_of_bin = 2.0 / num_phylo_bins;
    double int_val = GetIntVal();
    float prog = (int_val + 1);
    prog = (prog / size_of_bin) + (0.0000000000001);
    size_t bin = (size_t)prog;
    if (bin >= num_phylo_bins)
      bin = num_phylo_bins - 1;
    return bin;
  }

  /**
   * Input: The double representing the new interaction value of an organism
   *
   * Output: None
   *
   * Purpose: To set an organism's interaction value
   */
  void SetIntVal(double _in) {
    if (_in > 1 || _in < -1) {
      // Exception for invalid interaction value
      throw "Invalid interaction value. Must be between -1 and 1";
    } else {
      interaction_val = _in;
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: To mutate an organism's interaction value. The mutation value is
   * chosen from a normal distribution centered on 0 with the mutation size as
   * the standard deviation.
   */
  void Mutate() override {
    double local_rate = my_config->MUTATION_RATE();
    double local_size = my_config->MUTATION_SIZE();

    if (random->GetDouble(0.0, 1.0) <= local_rate) {
      interaction_val += random->GetRandNormal(0.0, local_size);
      if (interaction_val < -1)
        interaction_val = -1;
      else if (interaction_val > 1)
        interaction_val = 1;
    }
  }
};

#endif