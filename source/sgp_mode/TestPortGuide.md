# Starting point
* Add all the using types that are relevant (you can do this outside of a `TEST_CASE` at the top of the file so that it applies to all the tests):
```cpp
using world_t = sgpmode::SGPWorld;
using cpu_state_t = sgpmode::CPUState<world_t>;
using hw_spec_t = sgpmode::SGPHardwareSpec<sgpmode::Library, cpu_state_t, world_t>;
using hardware_t = sgpmode::SGPHardware<hw_spec_t>;
using program_t = typename world_t::sgp_prog_t;
using sgp_host_t = sgpmode::SGPHost<hw_spec_t>;
using sgp_sym_t = sgpmode::SGPSymbiont<hw_spec_t>;
```
* `SymConfigSGP` -> `sgpmode::SymConfigSGP`
* Add `config.TASK_ENV_CFG_PATH("source/test/sgp_mode_test/hardware-test-env.json");`
* World doesn't need tasks anymore: `world_t world(random, &config);`
* Change all the types to the using above
* To get CPU State now requires going through `GetHardware().GetCPUState()`
* If doing program building: `auto& prog_builder = world.GetProgramBuilder();` and `builder.CreateNotProgram(100)`
    * Is there a way to make custom programs now? It'd be handy for the more complicated tests
* Make sure to call `world.Setup()` since that initializes a lot of arrays
* If you run into seg faults, you might need to have `world.AssignNewEnvIO(my_host->GetHardware().GetCPUState());`, it should be handled any time `AddSymbiont` or `AddOrgAt` is called, but maybe you are doing something that doesn't call those
* Consult existing functional tests for how to do other things:
    * In unit tests:
        * `SGPWorld.test.cc`
        * `SGPHost.test.cc`
    * In functional tests:
        * `SGPHardware_Tasks.test.cc`
        * `SGPHost_Reproduce.test.cc`

As you run into other differences, add them here!

# Tests to Port
* There are tests in `sgp_mode_test/need_updating` that are from Alex's fork and weren't all functional, they hopefully need minimal updating, but also might be completely outdated, not sure
    * As they are evaluated, just move them up to their appropriate folder and delete them from that `need_updating` folder
* All the tests from `complex-syms-clean` are in `test/old_sgp_mode_test`. I've pulled small pieces of a few of them, but it would be great to port all those that are still relevant and make note of what is no longer relevant (to make sure it's replacement is tested)
    * As they are ported, delete them from `old_sgp_mode_test` so we can keep track
