#include "Bacterium.h"
#include "Host.h"

TEST_CASE("Bacterium constructor, random starter genome"){
    emp::Ptr<emp::Random> random = new emp::Random(5);
    SymWorld w(*random);
    SymConfigBase config;
    SymWorld * world = &w;
    double int_val = -1;

    config.GENOME_VAL(-1);
    Bacterium * b = new Bacterium(random, world, &config, int_val);
    double expected_genome_value = 0.7096757176;
    REQUIRE(b->GetGenomeValue()==Approx(expected_genome_value));

    config.GENOME_VAL(0.8);
    Bacterium * b2 = new Bacterium(random, world, &config, int_val);
    expected_genome_value = 0.8;
    REQUIRE(b2->GetGenomeValue()==Approx(expected_genome_value));

    delete b;
    delete b2;
}

TEST_CASE("Bacterium SetGenomeValue, GetGenomeValue"){
    emp::Ptr<emp::Random> random = new emp::Random(4);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = -1;
    emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);

    double genome = 0.2;
    b->SetGenomeValue(genome);
    double expected_genome_value = 0.2;
    REQUIRE(b->GetGenomeValue()==expected_genome_value);

}

TEST_CASE("Bacterium mutate"){
    emp::Ptr<emp::Random> random = new emp::Random(12);
    SymWorld w(*random);
    SymWorld * world = &w;
    SymConfigBase config;
    double int_val = 0;
    double expected_genome_post_mutation = 0.0;
    config.GENOME_VAL(0.5);
    

    WHEN("Mutation rate is not zero and genome mutations are enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_GENOME(1);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();
        expected_genome_post_mutation = 0.4983370739;

        THEN("Then mutation occurs and the bacterium's genome mutates"){
            REQUIRE(b->GetGenomeValue() == Approx(expected_genome_post_mutation));
        }
        
    delete b;
    }

    WHEN("Mutation rate is not zero and genome mutations are not enabled"){
        config.MUTATION_SIZE(0.002);
        config.MUTATION_RATE(1);
        config.MUTATE_GENOME(0);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();
        expected_genome_post_mutation = 0.5;

        THEN("Then mutations occur but do not occur in the genome"){
            REQUIRE(b->GetGenomeValue() == Approx(expected_genome_post_mutation));
        }
        
    delete b;
    }

    WHEN("Mutation rate is 0 and genome mutations are enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_GENOME(1);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();
        expected_genome_post_mutation = 0.5;

        THEN("Mutations do not occur"){
            REQUIRE(b->GetGenomeValue() == Approx(expected_genome_post_mutation));
        }

    delete b;
    }

    WHEN("Mutation rate is 0 and genome mutations are not enabled"){
        config.MUTATION_RATE(0.0);
        config.MUTATION_SIZE(0.0);
        config.MUTATE_GENOME(0);
        emp::Ptr<Bacterium> b = new Bacterium(random, world, &config, int_val);
        b->mutate();
        expected_genome_post_mutation = 0.5;

        THEN("Mutations do not occur"){
            REQUIRE(b->GetGenomeValue() == Approx(expected_genome_post_mutation));
        }

    delete b;
    }  
}
