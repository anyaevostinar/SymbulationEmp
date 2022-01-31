#include "../../efficient_mode/EfficientHost.h"
#include <typeinfo>

TEST_CASE("EfficientHost makeNew", "[efficient]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    EfficientWorld w(*random);
    SymConfigBase config;

    double parent_int_val = 0.2;
    double parent_efficiency = 0.5;
    Organism * h1 = new EfficientHost(random, &w, &config, parent_int_val);
    h1->SetEfficiency(parent_efficiency);
    Organism * h2 = h1->makeNew();

    THEN("The new host has properties of the original host, and has 0 points and 0 age"){
        REQUIRE(h2->GetIntVal() == h1->GetIntVal());
        REQUIRE(h2->GetEfficiency() == h1->GetEfficiency());
        REQUIRE(h2->GetPoints() == 0);
        REQUIRE(h2->GetAge() == 0);
        //TODO: figure out type information of new object
        //std::cout << typeid(h2).name() << std::endl;
    }
}
