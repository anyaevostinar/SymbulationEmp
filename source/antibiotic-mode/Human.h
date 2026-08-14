/*
Properties:
* SGPWorld - microbiome
* 
*/

#ifndef HUMAN_H
#define HUMAN_H

#include "../sgp_mode/SGPWorld.h"
#include "AntibioticConfigSetup.h"

namespace antibioticmode {

class Human {

protected:

    /**
     * The SGPWorld that holds the microbiome of this human.
     */
    emp::Ptr<sgpmode::SGPWorld> microbiome;

    /**
     * Config for all settings
     */
    emp::Ptr<AntibioticConfig> my_config;

public:
    Human(emp::Random& _random, emp::Ptr<AntibioticConfig> _config) :
    my_config(_config)
    {
        microbiome = emp::NewPtr<sgpmode::SGPWorld>(_random, _config);
        microbiome->Setup();
    }

    emp::Ptr<sgpmode::SGPWorld> GetMicrobiome() { return microbiome; }

};
}
#endif //HUMAN_H    