#ifndef SGPWORLD_H
#define SGPWORLD_H

#include "../default_mode/SymWorld.h"
#include "../default_mode/DataNodes.h"

// Avoid annoying header cycles since Scheduler depends on SGPWorld
void runCpus(SymWorld &);

class SGPWorld : public SymWorld {
public:
    double SymPointsDonated = 0.0;
    double SymPointsEarned = 0.0;

    SGPWorld(emp::Random& r, emp::Ptr<SymConfigBase> _config) : SymWorld(r, _config) {}

    emp::vector<std::pair<emp::Ptr<Organism>, emp::WorldPosition>> toReproduce;

    void Update() {
        // First run all organisms' CPUs, then perform all reproduction scheduled for this update
        runCpus(*this);

        std::unordered_set<uint32_t> replaced;
        for (auto org : toReproduce) {
            if (replaced.count(org.second.GetIndex())) {
                // This organism has been replaced, it's dead
                continue;
            }
            emp::Ptr<Organism> child = org.first->Reproduce();
            if (child->IsHost()) {
                // Host::Reproduce() doesn't take care of vertical transmission, that happens here
                for (auto &sym : org.first->GetSymbionts()) {
                    sym->VerticalTransmission(child);
                }
                emp::WorldPosition new_pos = DoBirth(child, org.second);
                replaced.insert(new_pos.GetIndex());
            } else {
                SymDoBirth(child, org.second);
            }
        }
        toReproduce.clear();

        SymWorld::Update();
    }
};

#endif