This is a document for development notes, i.e. a readme but for the development process, which should be removed once this is pulled into main (fingers crossed someday!).


## Current issues

### Hacky-ness
Much of the new parasite/mutualist implementation was hacked in at the expense of the previous sgp implementation. I think we want to stick with this, so we need to just go delete the old things:
    * In [GenomeLibrary.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/GenomeLibrary.h) I commented out instructions: `PrivateIO`, `Reuptake`, `Infect`, `Steal` to have the syms behave as just health parasites/health mutualists
    * The penalty for `PrivateIO` is turned off currently in `Instructions.h`
    * In [Tasks.h](https://github.com/anyaevostinar/SymbulationEmp/blob/complex-syms-clean/source/sgp_mode/Tasks.h), commented out the code limiting orgs to only one task, since we kind of want them to do multiple, probably more weird stuff in here, since this is where a lot of tinkering happened

### Don't lose the ecto code
Kai Johnson implemented support for ectosymbionts with complex genomes in the branch `complex-genomes`, which isn't currently compatible with this branch, but shouldn't be forgotten since it's good stuff.

## Tests to write
* Figure out why integration test in HealthHost.test.cc gets inconsistent results, maybe the sgp random seed needs to be set?
  * sgpl::tlrand.Get().ResetSeed(3); perhaps?
* Health host with parasite loses cycle 50% of time
* Health host with mutualist gains cycle 50% of time
  * in signalgp-lite/include/sgpl/hardware/Cpu.hpp there is GetCore(0)
  * in signalgp-lite/include/sgpl/hardware/Core.hpp there is GetProgramCounter()
  * that hopefully will show whether organism has advanced program counter
* Also test with health host with NOT, give it just barely enough CPUs to finish, check whether it manages to complete NOT
* Trickier for mutualist, check just before enough CPUs and it should manage to finish
