This is a document for development notes, i.e. a readme but for the development process, which should be removed once this is pulled into main (fingers crossed someday!).


## Current issues

### Main compatibility (and updating Empirical version)
On 7/18/24, Anya tried to merge in changes from main, but everything broke horribly because signalgp-lite is using an old version of Empirical (and conduit, a submodule of SGP-Lite as well) and the vendorizing within sgp-lite is apparently not enough to allow for me to easily update the version that Symbulation is using. 
* Things to still try: update the makefile borrowing from the [Digital Evolution project](https://anyaevostinar.github.io/classes/361-f22/hw-de) to specify the version of Empirical, since that did work before, I just forgot when I was trying to do the quick update

### Hacky-ness
Much of the disease parasite implementation was hacked in and so needs to be written more modularly to allow for the different kinds of parasitism and mutualism, how to best go about doing this both efficiently and modularly?
* First step: make a list of the hacky functionality here:
    * [DiversityAnalysis.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/DiversityAnalysis.h) and [ModularityAnalysis.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/ModularityAnalysis.h) can probably be scrapped for now
    * [SGPWorldSetup.cc](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/SGPWorldSetup.cc) is injecting a full population of hosts that can do NOT and symbionts that complement; this file also has the checking for infection via task matching code `GetNeighborHost`
    * In [GenomeLibrary.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/GenomeLibrary.h) I commented out instructions: `PrivateIO`, `Reuptake`, `Infect`, `Steal` to have the syms behave as just disease parasites/health mutualists
    * The penalty for `PrivateIO` is turned off currently in `Instructions.h`
    * Big thing: In [SGPHost.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/SGPHost.h) is where the disease parasite functionality is hard coded (and the comment is wrong lol):
        ```cpp
        bool host_cycle = true;
        if (HasSym()) {
        host_cycle = random->P(0.5);
        }

        // Randomly decide whether to run before or after the symbiont
        bool run_before = random->P(1.0);
        if (host_cycle && run_before) {
        cpu.RunCPUStep(pos, my_config->CYCLES_PER_UPDATE());
        }

        if (HasSym() && !host_cycle) { // let each sym do whatever they need to do
        ```
        Really need to figure out how to nicely wrap that into a configurable thing, probably by putting it into the symbiont
    * In [Tasks.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/Tasks.h), commented out the code limiting orgs to only one task, since we kind of want them to do multiple, probably more weird stuff in here, since this is where a lot of tinkering happened

Main things:
* Hosts always lose CPU cycles to symbionts
* Tasks are set in a specific way

Anya's Todo:
* Actually delete things that aren't relevant to this project
    * Including tests for code that doesn't exist anymore
* Get the code back to a point of parasite prelim data
    * [Plot from proposal 1](https://github.com/anyaevostinar/SymbulationEmp/blob/d8a18f28b5ae882ce43db9ea9fcb298789b25bc3/Analysis/12-12-22-LimitedRes/HostTasksLong.png) - from experiment folder [12-12-22-LimitedRes](https://github.com/anyaevostinar/SymbulationEmp/tree/d8a18f28b5ae882ce43db9ea9fcb298789b25bc3/Data/12-12-22-LimitedRes)
    * [Plot from proposal 2](https://github.com/anyaevostinar/SymbulationEmp/blob/d8a18f28b5ae882ce43db9ea9fcb298789b25bc3/Analysis/05-02-23-ParasitesLongSpatial/FinalUpdateHost.png) - from experiment folder [05-02-23-ParasitesLongSpatial](https://github.com/anyaevostinar/SymbulationEmp/tree/d8a18f28b5ae882ce43db9ea9fcb298789b25bc3/Data/05-02-23-ParasitesLongSpatial)
    * [Magical compare URL](https://github.com/anyaevostinar/SymbulationEmp/compare/b9fd9f7a46c39206b4b7208ad7dcaa8830ed7d0d...complex-syms-clean)
    * Settings different than default: -LIMITED_RES_TOTAL 10 -LIMITED_RES_INFLOW 500

### Don't lose the ecto code
Kai Johnson implemented support for ectosymbionts with complex genomes in the branch `complex-genomes`, which isn't currently compatible with this branch, but shouldn't be forgotten since it's good stuff.

## Tests to write
* (with Data interval at 50) When parasites absent, hosts do NOT 7k-8k in Tasks file, when parasites present, hosts drop down below 7k by update 5000
* Something also with the spatial structure results probably?