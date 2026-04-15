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
* To get CPU State now requires going through `GetHardware()`
* If doing program building: `auto& prog_builder = world.GetProgramBuilder();`
* Consult existing functional tests for how to do other things:
    * In unit tests:
        * `SGPWorld.test.cc`
        * `SGPHost.test.cc`
    * In functional tests:
        * `SGPHardware_Tasks.test.cc`
        * `SGPHost_Reproduce.test.cc`

As you run into other differences, add them here!