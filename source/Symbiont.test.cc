#include "Host.h"
#include "Symbiont.h"

TEST_CASE("Symbiont Constructor") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    
    double int_val = -2;
    REQUIRE_THROWS(new Symbiont(random, world, int_val) ); 

    int_val = -1;
    Symbiont * s = new Symbiont(random, world, int_val);

    int_val = 1;
    Symbiont * s2 = new Symbiont(random, world, int_val);

    int_val = 2;
    REQUIRE_THROWS(new Symbiont(random, world, int_val) ); 
}

TEST_CASE("SetIntVal, GetIntVal") {
    
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    
    double int_val = -1;
    Symbiont * s = new Symbiont(random, world, int_val);
    
    int_val = -2;
    REQUIRE_THROWS( s->SetIntVal(int_val) ); 

    int_val = -1;
    Symbiont * s2 = new Symbiont(random, world, int_val);

    int_val = 1;
    s2->SetIntVal(int_val);
    double orig_int_val = 1;
    
    REQUIRE(s2->GetIntVal() == orig_int_val);

    int_val = -1;
    Symbiont * s3 = new Symbiont(random, world, int_val);

    int_val = 2;
    REQUIRE_THROWS( s3->SetIntVal(int_val) ) ; 

    int_val = 0;
    Symbiont * s4 = new Symbiont(random, world, int_val);

    int_val = -1;
    s4->SetIntVal(int_val);
    orig_int_val = -1;

    REQUIRE( s4->GetIntVal() == orig_int_val) ;

}

TEST_CASE("SetPoints, GetPoints") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;
    
    double int_val = -1;
    Symbiont * s = new Symbiont(random, world, int_val);

    int points = 1;
    s->SetPoints(points);
    double orig_points = 1;

    REQUIRE( s->GetPoints() == orig_points);

    int add_points = -1;
    s->AddPoints(add_points);
    orig_points = 0;

    REQUIRE( s->GetPoints() == orig_points);

    add_points = 150;
    s->AddPoints(add_points);
    orig_points = 150;

    REQUIRE(s->GetPoints() == orig_points);
}

TEST_CASE("mutate") {

    emp::Ptr<emp::Random> random = new emp::Random(37);
    SymWorld w(*random);
    SymWorld * world = &w;

    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        Symbiont * s = new Symbiont(random, world, int_val);
        
        s->mutate();

        double int_val_post_mutation = -0.0007962661;
        THEN("Mutation occurs and interaction value changes") {
            REQUIRE(s->GetIntVal() == Approx(int_val_post_mutation));
        }
    }


    WHEN("Mutation rate is zero") {
        double int_val = 1;
        int orig_int_val = 1;
        double points = 0.0;
        double h_res = 100.0;
        bool h_trans = true;
        double mutation_rate = 0;
        Symbiont * s = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        s->mutate();


        THEN("Mutation does not occur and interaction value does not change") {
            REQUIRE(s->GetIntVal() == orig_int_val);
        }

    }
}

TEST_CASE("reproduce") {

    emp::Ptr<emp::Random> random = new emp::Random(3);
    SymWorld w(*random);
    SymWorld * world = &w;

    
    WHEN("Mutation rate is zero")  {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double points = 0.0;
        double h_res = 100.0;
        bool h_trans = true;
        double mutation_rate = 0;
        Symbiont * s = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        emp::Ptr<Organism> sym_baby = s->reproduce();


        THEN("Offspring's interaction value equals parent's interaction value") {
            int sym_baby_int_val = 0;
            REQUIRE( sym_baby->GetIntVal() == sym_baby_int_val);
            REQUIRE( sym_baby->GetIntVal() == parent_orig_int_val);
            REQUIRE( s->GetIntVal() == parent_orig_int_val);
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }
    }


    WHEN("Mutation rate is not zero") {
        double int_val = 0;
        double parent_orig_int_val = 0;
        double points = 0.0;
        double h_res = 100.0;
        bool h_trans = true;
        double mutation_rate = 0.01;
        Symbiont * s2 = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        emp::Ptr<Organism> sym_baby = s2->reproduce();


        THEN("Offspring's interaction value does not equal parent's interaction value") {
            double sym_baby_int_val = -0.002698581;
            double parent_int_val = -0.0083202466;
            REQUIRE( sym_baby->GetIntVal() != parent_orig_int_val);
            REQUIRE( sym_baby->GetIntVal() == Approx(sym_baby_int_val));
            REQUIRE( s2->GetIntVal() == Approx(parent_int_val));
        }

        THEN("Offspring's points are zero") {
            int sym_baby_points = 0;
            REQUIRE( sym_baby->GetPoints() == sym_baby_points);

        }

    }
 
}

TEST_CASE("process") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymWorld w(*random);
    SymWorld * world = &w;


    WHEN("Horizontal transmission is true and points is greater than sym_h_res") {
        double int_val = 1;
        double parent_orig_int_val = 1;
        double points = 0.0;
        double h_res = 140.0;
        bool h_trans = true;
        double mutation_rate = 0;
        Symbiont * s = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        int add_points = 200;
        s->AddPoints(add_points);
        
        int location = 10;
        s->process(location);


        THEN("Points changes and is set to 0") {
            int points_post_reproduction = 0;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }


    WHEN("Horizontal transmission is true and points is less than sym_h_res") {
        double int_val = 1;
        double parent_orig_int_val = 1;
        double points = 0.0;
        double h_res = 200.0;
        bool h_trans = true;
        double mutation_rate = 0;
        Symbiont * s = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        int add_points = 50;
        s->AddPoints(add_points);
        
        int location = 10;
        s->process(location);
        
        
        THEN("Points does not change") {
            int points_post_reproduction = 50;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }


    WHEN("Horizontal transmission is false and points and points is greater then sym_h_res") {
        double int_val = 1;
        double parent_orig_int_val = 1;
        double points = 100.0;
        double h_res = 80.0;
        bool h_trans = false;
        double mutation_rate = 0;
        Symbiont * s = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        int location = 10;
        s->process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 100;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }

    WHEN("Horizontal transmission is false and points and points is less then sym_h_res") {
        double int_val = 1;
        double parent_orig_int_val = 1;
        double points = 40.0;
        double h_res = 80.0;
        bool h_trans = false;
        double mutation_rate = 0;
        Symbiont * s = new Symbiont(random, world, int_val, points, h_res, h_trans, mutation_rate);
        
        int location = 10;
        s->process(location);


        THEN("Points does not change") {
            int points_post_reproduction = 40;
            REQUIRE(s->GetPoints() == points_post_reproduction);
        }
    }

}