#include "Bacterium.h"
#include "Host.h"

TEST_CASE("Bacterium constructor, host_incorporation_val"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymConfigBase config;
    SymWorld * world = &w;
    double int_val = -1;

    config.HOST_INC_VAL(-1);
    Bacterium * b = new Bacterium(random, world, &config, int_val);
    REQUIRE(b->GetIncVal() >= 0.0);
    REQUIRE(b->GetIncVal() <= 1.0);

    config.HOST_INC_VAL(0.8);
    Bacterium * b2 = new Bacterium(random, world, &config, int_val);
    double expected_inc_val = 0.8;
    REQUIRE(b2->GetIncVal()==expected_inc_val);

    delete b;
    delete b2;
}

TEST_CASE("Bacterium SetIncVal, GetIncVal"){
    emp::Ptr<emp::Random> random = new emp::Random(4);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);

    double host_incorporation_val = 0.2;
    b->SetIncVal(host_incorporation_val);
    double expected_inc_val = 0.2;
    REQUIRE(b->GetIncVal()==expected_inc_val);

}

TEST_CASE("Bacterium mutate"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = 0;
    double orig_host_inc_val = 0.5;
    config.HOST_INC_VAL(orig_host_inc_val);
    

    WHEN("Mutation rate is not zero and host_inc_val mutations are enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_HOST_INC_VAL(1);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Then mutation occurs and the bacterium's host_inc_val mutates"){
            REQUIRE(b->GetIncVal() != orig_host_inc_val);
            REQUIRE(b->GetIncVal() >= 0.0);
            REQUIRE(b->GetIncVal() <= 1.0);
        }
        
    delete b;
    }

    WHEN("Mutation rate is not zero and host_inc_val mutations are not enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_HOST_INC_VAL(0);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();
        
        THEN("Then mutations occur but do not occur in the host_inc_val"){
            REQUIRE(b->GetIncVal() ==  orig_host_inc_val);
        }
        
    delete b;
    }

    WHEN("Mutation rate is 0 and host_inc_val mutations are enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_HOST_INC_VAL(1);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Mutations do not occur"){
            REQUIRE(b->GetIncVal() ==  orig_host_inc_val);
        }

    delete b;
    }

    WHEN("Mutation rate is 0 and host_inc_val mutations are not enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_HOST_INC_VAL(0);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();

        THEN("Mutations do not occur"){
            REQUIRE(b->GetIncVal() ==  orig_host_inc_val);
        }

    delete b;
    }  
}
