#include "../sgp_mode/Instructions.h"
#include "../sgp_mode/Tasks.h"
#include "../sgp_mode/CPUState.h"
#include "../sgp_mode/SGPHost.h"

TEST_CASE("Square Task", "[sgp]") {
emp::Ptr<emp::Random> random = new emp::Random(-1);
SymConfigBase config;
SGPWorld world(*random, &config, DefaultTasks);

emp::Ptr<SGPHost> host = emp::NewPtr<SGPHost>(random, &world, &config, host_int_val);
}