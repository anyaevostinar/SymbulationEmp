#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"

TEST_CASE( "Host-Symbiont interactions", "[default]") {
  SymConfigBase config;
  config.SYM_LIMIT(3);

  GIVEN( "an empty somewhat generous host without resource type and with 17 points" ) {
    emp::Random random;
    SymWorld world(random, &config);
    double host_interaction_val = 0.5;
    double host_points = 17;
    double host_resource = 100;
    Host host(&random, &world, &config, host_interaction_val, {}, {}, host_points);

    REQUIRE( host.GetIntVal() == .5 );
    REQUIRE( host.GetSymbionts().size() == 0 );
    REQUIRE( host.GetReproSymbionts().size() == 0 );
    REQUIRE( host.GetPoints() == host_points );

    WHEN( "resources are distributed" ) {
      host.DistribResources(host_resource);

      THEN( "the host receives all resources" ) {
        double host_points_theor = host_resource - (host_resource * host_interaction_val) + host_points;
        REQUIRE( host.GetPoints() == Approx(host_points_theor) );
      }
    }

    WHEN( "a somewhat generous symbiont with no resource type and with 203 points is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config,  .6, 203);
      host.AddSymbiont(symbiont);
      host.DistribResources(100);

      THEN( "the host gains that symbiont" ) {
        REQUIRE( host.GetSymbionts().size() == 1 );
        REQUIRE( host.GetSymbionts()[0]->GetIntVal() == .6 );
      }
      THEN( "the host receives more resources than without the symbiont" ) {
        REQUIRE( host.GetPoints() > 117 );
        REQUIRE( host.GetPoints() == Approx(217) );
      }
      THEN( "the symbiont receives resources without any bonus") {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() > 203 );
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(223) );
      }
      THEN( "conservation of resources is not in place" ) {
        REQUIRE( (host.GetPoints()-17)+(host.GetSymbionts()[0]->GetPoints()-203) > 100 );
      }
      THEN( "bonus is applied to returned resources" ) {
        REQUIRE( (host.GetPoints()-17)+(host.GetSymbionts()[0]->GetPoints()-203) - 100 == Approx(100*.5*.6*(5-1)) );
      }
    }

    WHEN( "a somewhat hostile symbiont with no resource type and with 101 points is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config, -.65, 101);
      host.AddSymbiont(symbiont);
      host.DistribResources(113);


      THEN( "the host gains that symbiont" ) {
        REQUIRE( host.GetSymbionts().size() == 1 );
        REQUIRE( host.GetSymbionts()[0]->GetIntVal() == -.65 );
      }
      THEN( "the host receives less than a quarter of the resources" ) {
        REQUIRE( host.GetPoints() < 17+25 );
        REQUIRE( host.GetPoints() == Approx(36.775) );
      }
      THEN( "the symbiont receives resources" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() > 101 );
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(194.225) );
      }
      THEN( "the conservation of resources is in place" ) {
        REQUIRE( host.GetPoints() + host.GetSymbionts()[0]->GetPoints() == Approx(17 + 101 + 113) );
      }
    }

    WHEN( "more symbionts are added than the limit" ) {
      emp::Ptr<Symbiont> symbiont1;
      symbiont1.New(&random, &world, &config, .12, 101);
      host.AddSymbiont(symbiont1);
      emp::Ptr<Symbiont> symbiont2;
      symbiont2.New(&random, &world, &config, .12, 102);
      host.AddSymbiont(symbiont2);
      emp::Ptr<Symbiont> symbiont3;
      symbiont3.New(&random, &world, &config, .12, 103);
      host.AddSymbiont(symbiont3);
      emp::Ptr<Symbiont> symbiont4;
      symbiont4.New(&random, &world, &config, .12, 104);
      host.AddSymbiont(symbiont4);
      emp::Ptr<Symbiont> symbiont5;
      symbiont5.New(&random, &world, &config, .12, 105);
      host.AddSymbiont(symbiont5);
      emp::Ptr<Symbiont> symbiont6;
      symbiont6.New(&random, &world, &config, .12, 106);
      host.AddSymbiont(symbiont6);
      emp::Ptr<Symbiont> symbiont7;
      symbiont7.New(&random, &world, &config, .12, 107);
      host.AddSymbiont(symbiont7);
      emp::Ptr<Symbiont> symbiont8;
      symbiont8.New(&random, &world, &config, .12, 108);
      host.AddSymbiont(symbiont8);


      THEN( "the host gains as many symbionts as the limit" ) {
        REQUIRE( host.GetSymbionts().size() == 3 );
      }

      THEN( "the first added are kept" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == 101 );
        REQUIRE( host.GetSymbionts()[1]->GetPoints() == 102 );
        REQUIRE( host.GetSymbionts()[2]->GetPoints() == 103 );
      }
    }

    WHEN( "a repro symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config, -.7, 37);
      host.AddReproSym(symbiont);
      host.DistribResources(13);

      THEN( "the host gains that repro symbiont" ) {
        REQUIRE( host.GetReproSymbionts().size() == 1 );
        REQUIRE( host.GetReproSymbionts()[0]->GetIntVal() == -.7 );
        REQUIRE( host.GetReproSymbionts()[0]->GetPoints() == 37 );
      }
      THEN( "the host receives all resources" ) {
        REQUIRE( host.GetPoints() == Approx(23.5) );
      }
    }
  }



  GIVEN( "an empty slightly defensive host" ) {
    emp::Random random(10);
    SymWorld world(random, &config);
    Host host(&random, &world, &config, -.2);

    REQUIRE( host.GetIntVal() == -.2 );
    REQUIRE( host.GetSymbionts().size() == 0 );
    REQUIRE( host.GetReproSymbionts().size() == 0 );
    REQUIRE( host.GetPoints() == 0 );

    WHEN( "resources are distributed" ) {
      host.DistribResources(1);

      THEN( "the host does not receive all resources" ) {
        REQUIRE( host.GetPoints() < 1 );
        REQUIRE( host.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a somewhat generous symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config, .6);
      host.AddSymbiont(symbiont);
      host.DistribResources(1);

      THEN( "the host receives some resources" ) {
        REQUIRE( host.GetPoints() == Approx(.8) );
      }
      THEN( "the symbiont does not receive any resources" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(0) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() + host.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a very slightly hostile symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config, -.1);
      host.AddSymbiont(symbiont);
      host.DistribResources(1);

      THEN( "the host receives some resources" ) {
        REQUIRE( host.GetPoints() == Approx(.8) );
      }
      THEN( "the symbiont does not recieve any resources" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(0) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() + host.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a somewhat hostile symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config, -.5);
      host.AddSymbiont(symbiont);
      host.DistribResources(1);

      THEN( "the host receives fewer resources" ) {
        REQUIRE( host.GetPoints() == Approx(.8*.7) );
      }
      THEN( "the symbiont receives resources" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(.8*.3) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() + host.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a fully hostile symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> symbiont;
      symbiont.New(&random, &world, &config, -1);
      host.AddSymbiont(symbiont);
      host.DistribResources(1);

      THEN( "the host receives very few resources" ) {
        REQUIRE( host.GetPoints() == Approx(.8*.2) );
      }
      THEN( "the symbiont receives some resources" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(.8*.8) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() + host.GetPoints() == Approx(.8) );
      }
    }



    WHEN( "many symbionts are added" ) {
      emp::Ptr<Symbiont> symbiont1;
      symbiont1.New(&random, &world, &config, 1);
      host.AddSymbiont(symbiont1);
      emp::Ptr<Symbiont> symbiont2;
      symbiont2.New(&random, &world, &config, -.15);
      host.AddSymbiont(symbiont2);
      emp::Ptr<Symbiont> symbiont3;
      symbiont3.New(&random, &world, &config, -.73);
      host.AddSymbiont(symbiont3);
      emp::Ptr<Symbiont> symbiont4;
      symbiont4.New(&random, &world, &config, .6);
      host.AddSymbiont(symbiont4);
      emp::Ptr<Symbiont> symbiont5;
      symbiont5.New(&random, &world, &config, -1);
      host.AddSymbiont(symbiont5);
      host.DistribResources(1);

      THEN( "the host receives the right amount of resources" ) {
        REQUIRE( host.GetPoints() == Approx(.8*(1+1+.47)/3) );
      }
      THEN( "the nice symbiont receives nothing" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() == Approx(0) );
      }
      THEN( "the not-too-hostile symbiont receives nothing" ) {
        REQUIRE( host.GetSymbionts()[1]->GetPoints() == Approx(0) );
      }
      THEN( "the hostile symbiont receives a bit" ) {
        REQUIRE( host.GetSymbionts()[2]->GetPoints() == Approx(.8*.53/3) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( host.GetSymbionts()[0]->GetPoints() + host.GetSymbionts()[1]->GetPoints() + host.GetSymbionts()[2]->GetPoints() + host.GetPoints() == Approx(.8) );
      }
    }
  }
}


TEST_CASE("Partner to-from tag evolution tracking", "[default]") {
  // this test expects 32-bit tags
  emp::Random random(19);
  SymConfigBase config;
  config.TAG_MATCHING(1);
  config.TAG_MUTATION_SIZE(0.3);

  SymWorld world(random, &config);

  double int_val = 0;
  
  emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
  emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

  emp::BitSet<TAG_LENGTH> host_tag = emp::BitSet<TAG_LENGTH>("00000000000000001111111111111111");
  host->SetTag(host_tag);
  emp::BitSet<TAG_LENGTH> sym_tag = emp::BitSet<TAG_LENGTH>("00000000111111110000000011111111");
  symbiont->SetTag(sym_tag);
  
  WHEN("A symbiont reproduces") {
    host->AddSymbiont(symbiont);
    
    REQUIRE(symbiont->GetFromPartnerCount() == 0);
    REQUIRE(symbiont->GetTowardsPartnerCount() == 0);
    
    unsigned int towards_gen_1 = 4;
    unsigned int from_gen_1 = 6;
    emp::Ptr<Organism> sym_baby = symbiont->Reproduce();

    // 00000000000000001111111111111111  // host
    // 00000000111111110000000011111111  // sym parent
    // 10010000011101110000101000010111  // sym baby

    // 10010000000000000000000011101000 from (6)
    // 00000000100010000000101000000000 towards (4)

    THEN("Its offspring's tag evolution towards and from its host partner is tracked") {
      REQUIRE(sym_baby->GetFromPartnerCount() == from_gen_1);
      REQUIRE(sym_baby->GetTowardsPartnerCount() == towards_gen_1);
    }

    emp::Ptr<Host> host_2 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::BitSet<TAG_LENGTH> host_2_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
    host_2->SetTag(host_2_tag);

    unsigned int towards_gen_2 = 2;
    unsigned int from_gen_2 = 6;
    host_2->AddSymbiont(sym_baby);
    emp::Ptr<Organism> sym_grandbaby = sym_baby->Reproduce();

    // 00000000000000000000000000000000  // host_2 (sym_baby's host)
    // 10010000011101110000101000010111  // sym baby
    // 10010000111011111000101011110011  // sym grandbaby

    // 00000000100010001000000011100000 from (6)
    // 00000000000100000000000000000100 towards (2)

    THEN("Tag evolution towards and from host partners is tracked cumulatively over generations") {
      // do another generation to make sure inheritance of counts works properly.
      REQUIRE(sym_grandbaby->GetFromPartnerCount() == from_gen_1 + from_gen_2);
      REQUIRE(sym_grandbaby->GetTowardsPartnerCount() == towards_gen_1 + towards_gen_2);
    }

    host_2.Delete();
    sym_grandbaby.Delete();
  }
  
  WHEN("A host reproduces") {
    WHEN("The host parent has a symbiont partner") {
      random.ResetSeed(11);
      host->AddSymbiont(symbiont);
      REQUIRE(host->GetFromPartnerCount() == 0);
      REQUIRE(host->GetTowardsPartnerCount() == 0);

      unsigned int towards_gen_1 = 8;
      unsigned int from_gen_1 = 7;
      emp::Ptr<Organism> host_baby = host->Reproduce();

      // 00000000111111110000000011111111  // symbiont
      // 00000000000000001111111111111111  // host 
      // 11100001100100111000111010001111  // host baby

      // 11100001000000000000000001110000 from (7)
      // 00000000100100110111000100000000 towards (8)

      THEN("Its offspring's tag evolution towards and from its symbiont partner is tracked") {
        REQUIRE(host_baby->GetFromPartnerCount() == from_gen_1);
        REQUIRE(host_baby->GetTowardsPartnerCount() == towards_gen_1);
      }

      emp::Ptr<Symbiont> symbiont_2 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::BitSet<TAG_LENGTH> symbiont_2_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
      symbiont_2->SetTag(symbiont_2_tag);
      host_baby->AddSymbiont(symbiont_2);

      unsigned int towards_gen_2 = 7;
      unsigned int from_gen_2 = 9;
      emp::Ptr<Organism> host_grandbaby = host_baby->Reproduce();

      // 00000000000000000000000000000000  // symbiont_2 (host_baby's symbiont)
      // 11100001100100111000111010001111  // host baby
      // 00111101010111110001100111001110  // host grandbaby

      // 00011100010011000001000101000000 from (9)
      // 11000000100000001000011000000001 towards (7)

      THEN("Tag evolution towards and from symbiont partners is tracked cumulatively over generations") {
        // do another generation to make sure inheritance of counts works properly.
        REQUIRE(host_grandbaby->GetFromPartnerCount() == from_gen_1 + from_gen_2);
        REQUIRE(host_grandbaby->GetTowardsPartnerCount() == towards_gen_1 + towards_gen_2);
      }

      host_baby.Delete();
      host_grandbaby.Delete();
    }
    WHEN("The host parent has no symbiont partner") {
      random.ResetSeed(12);
      REQUIRE(host->GetFromPartnerCount() == 0);
      REQUIRE(host->GetTowardsPartnerCount() == 0);

      unsigned int towards_gen_1 = 0;
      unsigned int from_gen_1 = 0;
      emp::Ptr<Organism> host_baby = host->Reproduce();

      THEN("Tag evolution counts are inherited without edits") {
        REQUIRE(host_baby->GetFromPartnerCount() == from_gen_1);
        REQUIRE(host_baby->GetTowardsPartnerCount() == towards_gen_1);

      }

      host_baby.Delete();
    }
  }
  
  if (!host->HasSym()) symbiont.Delete();
  host.Delete();
}