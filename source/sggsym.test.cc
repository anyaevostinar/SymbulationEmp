#include "Sgghost.h"
#include "sggsym.h"

TEST_CASE("Symbiont Constructor") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    
    double donation = 1;

    double int_val = 0.5;
    SggSymbiont * s = new SggSymbiont(random, world, &config, int_val,donation);
    CHECK(s->Getdonation() == donation);

    donation = 2;
    SggSymbiont * s2 = new SggSymbiont(random, world, &config, int_val,donation);
    CHECK(s2->Getdonation() == 2);

}
TEST_CASE("mutate") {

    emp::Ptr<emp::Random> random = new emp::Random(37);
    SymConfigBase config;
    SymWorld w(*random);
    SymWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double donation =0.01;
        config.MUTATION_SIZE(0.002);
        SggSymbiont * s = new SggSymbiont(random, world, &config, int_val,donation);

        s->mutate();
       
        double int_val_post_mutation = 0.0092037339; 
        THEN("Mutation occurs and interaction value changes") {
            REQUIRE(s->Getdonation() == Approx(int_val_post_mutation));
        }
    }
}   

TEST_CASE("Symbiont ProcessPool"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random);
    config.SYNERGY(5);
    double host_int_val = 1;
    double sym_int_val = 0;
    double donation = 0.1;

    SggSymbiont * s = new SggSymbiont(random, &w, &config, sym_int_val,donation);
    sggHost * h = new sggHost(random, &w, &config, host_int_val);
    h->AddSymbiont(s);

    double piece = 40;
        // double host_donation = 20; //sym_piece * host_int_val;  
    double sym_portion = 0; //host_donation - (host_donation * sym_int_val);
    h->DistribResources(40);
    
    CHECK(s->GetPoints() == 40);
    CHECK(h->GetPoints() == 0);



    
    
}


