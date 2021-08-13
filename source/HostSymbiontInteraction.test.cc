#include "Host.h"
#include "Symbiont.h"
#include <set>

TEST_CASE( "Host-Symbiont interactions") {
  SymConfigBase config;
  config.SYM_LIMIT(3);

  GIVEN( "an empty somewhat generous host without resource type and with 17 points" ) {
    emp::Random random;
    SymWorld w(random);
    double host_interaction_val = 0.5;
    double host_points = 17;
    double host_resource = 100;
    Host h(&random, &w, &config, host_interaction_val, {}, {}, std::set<int>(), host_points);

    REQUIRE( h.GetIntVal() == .5 );
    REQUIRE( h.GetSymbionts().size() == 0 );
    REQUIRE( h.GetReproSymbionts().size() == 0 );
    REQUIRE( h.GetResTypes().size() == 0 );
    REQUIRE( h.GetPoints() == host_points );

    WHEN( "resources are distributed" ) {
      h.DistribResources(host_resource);

      THEN( "the host receives all resources" ) {
        double host_points_theor = host_resource - (host_resource * host_interaction_val) + host_points;
        REQUIRE( h.GetPoints() == Approx(host_points_theor) );
      }
    }

    WHEN( "a somewhat generous symbiont with no resource type and with 203 points is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config,  .6, 203);
      h.AddSymbiont(s);
      h.DistribResources(100);

      THEN( "the host gains that symbiont" ) {
        REQUIRE( h.GetSymbionts().size() == 1 );
        REQUIRE( h.GetSymbionts()[0]->GetIntVal() == .6 );
        // REQUIRE( h.GetSymbionts()[0] == s );
      }
      THEN( "the host receives more resources than without the symbiont" ) {
        REQUIRE( h.GetPoints() > 117 );
        REQUIRE( h.GetPoints() == Approx(217) );
      }
      THEN( "the symbiont receives resources without any bonus") {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() > 203 );
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(223) );
      }
      THEN( "conservation of resources is not in place" ) {
        REQUIRE( (h.GetPoints()-17)+(h.GetSymbionts()[0]->GetPoints()-203) > 100 );
      }
      THEN( "bonus is applied to returned resources" ) {
        REQUIRE( (h.GetPoints()-17)+(h.GetSymbionts()[0]->GetPoints()-203) - 100 == Approx(100*.5*.6*(5-1)) );
      }
    }

    WHEN( "a somewhat hostile symbiont with no resource type and with 101 points is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config, -.65, 101);
      h.AddSymbiont(s);
      h.DistribResources(113);


      THEN( "the host gains that symbiont" ) {
        REQUIRE( h.GetSymbionts().size() == 1 );
        REQUIRE( h.GetSymbionts()[0]->GetIntVal() == -.65 );
        // REQUIRE( h.GetSymbionts()[0] == s );
      }
      THEN( "the host receives less than a quarter of the resources" ) {
        REQUIRE( h.GetPoints() < 17+25 );
        REQUIRE( h.GetPoints() == Approx(36.775) );
      }
      THEN( "the symbiont receives resources" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() > 101 );
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(194.225) );
      }
      THEN( "the conservation of resources is in place" ) {
        REQUIRE( h.GetPoints() + h.GetSymbionts()[0]->GetPoints() == Approx(17 + 101 + 113) );
      }
    }

    WHEN( "more symbionts are added than the limit" ) {
      emp::Ptr<Symbiont> s1;
      s1.New(&random, &w, &config, .12, 101);
      h.AddSymbiont(s1);
      emp::Ptr<Symbiont> s2;
      s2.New(&random, &w, &config, .12, 102);
      h.AddSymbiont(s2);
      emp::Ptr<Symbiont> s3;
      s3.New(&random, &w, &config, .12, 103);
      h.AddSymbiont(s3);
      emp::Ptr<Symbiont> s4;
      s4.New(&random, &w, &config, .12, 104);
      h.AddSymbiont(s4);
      emp::Ptr<Symbiont> s5;
      s5.New(&random, &w, &config, .12, 105);
      h.AddSymbiont(s5);
      emp::Ptr<Symbiont> s6;
      s6.New(&random, &w, &config, .12, 106);
      h.AddSymbiont(s6);
      emp::Ptr<Symbiont> s7;
      s7.New(&random, &w, &config, .12, 107);
      h.AddSymbiont(s7);
      emp::Ptr<Symbiont> s8;
      s8.New(&random, &w, &config, .12, 108);
      h.AddSymbiont(s8);


      THEN( "the host gains as many symbionts as the limit" ) {
        REQUIRE( h.GetSymbionts().size() == 3 );
      }

      THEN( "the first added are kept" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == 101 );
        REQUIRE( h.GetSymbionts()[1]->GetPoints() == 102 );
        REQUIRE( h.GetSymbionts()[2]->GetPoints() == 103 );
      }
    }

    WHEN( "a repro symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config, -.7, 37);
      h.AddReproSym(s);
      h.DistribResources(13);

      THEN( "the host gains that repro symbiont" ) {
        REQUIRE( h.GetReproSymbionts().size() == 1 );
        REQUIRE( h.GetReproSymbionts()[0]->GetIntVal() == -.7 );
        REQUIRE( h.GetReproSymbionts()[0]->GetPoints() == 37 );
        // REQUIRE( h.GetSymbionts()[0] == s );
      }
      THEN( "the host receives all resources" ) {
        REQUIRE( h.GetPoints() == Approx(23.5) );
      }
    }
  }



  GIVEN( "an empty slightly defensive host" ) {
    emp::Random random(10);
    SymWorld w(random);
    Host h(&random, &w, &config, -.2);

    REQUIRE( h.GetIntVal() == -.2 );
    REQUIRE( h.GetSymbionts().size() == 0 );
    REQUIRE( h.GetReproSymbionts().size() == 0 );
    REQUIRE( h.GetResTypes().size() == 0 );
    REQUIRE( h.GetPoints() == 0 );

    WHEN( "resources are distributed" ) {
      h.DistribResources(1);

      THEN( "the host does not receive all resources" ) {
        REQUIRE( h.GetPoints() < 1 );
        REQUIRE( h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a somewhat generous symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config, .6);
      h.AddSymbiont(s);
      h.DistribResources(1);

      THEN( "the host receives some resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8) );
      }
      THEN( "the symbiont does not receive any resources" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(0) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a very slightly hostile symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config, -.1);
      h.AddSymbiont(s);
      h.DistribResources(1);

      THEN( "the host receives some resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8) );
      }
      THEN( "the symbiont does not recieve any resources" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(0) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a somewhat hostile symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config, -.5);
      h.AddSymbiont(s);
      h.DistribResources(1);

      THEN( "the host receives fewer resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8*.7) );
      }
      THEN( "the symbiont receives resources" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(.8*.3) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a fully hostile symbiont is added and resources are distributed" ) {
      emp::Ptr<Symbiont> s;
      s.New(&random, &w, &config, -1);
      h.AddSymbiont(s);
      h.DistribResources(1);

      THEN( "the host receives very few resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8*.2) );
      }
      THEN( "the symbiont receives some resources" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(.8*.8) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }



    WHEN( "many symbionts are added" ) {
      emp::Ptr<Symbiont> s1;
      s1.New(&random, &w, &config, 1);
      h.AddSymbiont(s1);
      emp::Ptr<Symbiont> s2;
      s2.New(&random, &w, &config, -.15);
      h.AddSymbiont(s2);
      emp::Ptr<Symbiont> s3;
      s3.New(&random, &w, &config, -.73);
      h.AddSymbiont(s3);
      emp::Ptr<Symbiont> s4;
      s4.New(&random, &w, &config, .6);
      h.AddSymbiont(s4);
      emp::Ptr<Symbiont> s5;
      s5.New(&random, &w, &config, -1);
      h.AddSymbiont(s5);
      h.DistribResources(1);

      THEN( "the host receives the right amount of resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8*(1+1+.47)/3) );
      }
      THEN( "the nice symbiont receives nothing" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() == Approx(0) );
      }
      THEN( "the not-too-hostile symbiont receives nothing" ) {
        REQUIRE( h.GetSymbionts()[1]->GetPoints() == Approx(0) );
      }
      THEN( "the hostile symbiont receives a bit" ) {
        REQUIRE( h.GetSymbionts()[2]->GetPoints() == Approx(.8*.53/3) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0]->GetPoints() + h.GetSymbionts()[1]->GetPoints() + h.GetSymbionts()[2]->GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }
  }
}

TEST_CASE("Ectosymbiosis"){
  emp::Random random(17);
  SymConfigBase config;
  SymWorld w(random);
  w.Resize(1,1);

  WHEN("Ectosymbiosis is off"){
    config.ECTOSYMBIOSIS(0);
    config.SYM_INFECTION_CHANCE(0.0);
    w.SetMoveFreeSyms(0);
    w.SetFreeLivingSyms(1);
    double int_val = 0.5;

    emp::Ptr<Host> host = new Host(&random, &w, &config, int_val);
    emp::Ptr<Organism> sym = new Symbiont(&random, &w, &config, int_val);
    w.AddOrgAt(sym,0);
    w.AddOrgAt(host,0);
    REQUIRE(sym->GetPoints() == 0);
    REQUIRE(host->GetPoints() == 0);

    double res = 10;
    double sym_res = 0;
    double host_res = res * int_val;

    res = host->HandleEctosymbiosis(res, 0);
    host->DistribResources(res);

    THEN("Parallel organisms don't distribute resources together"){
      REQUIRE(sym->GetPoints() == sym_res);
      REQUIRE(host->GetPoints() == host_res);
    }
  }
  WHEN("Ectosymbiosis is on"){
    config.ECTOSYMBIOSIS(1);
    config.SYM_INFECTION_CHANCE(0.0);
    w.SetMoveFreeSyms(0);
    w.SetFreeLivingSyms(1);

    WHEN("Parallel organisms are mutualistic"){
      double int_val = 0.5;

      config.ECTOSYMBIOSIS(1);
      emp::Ptr<Host> host = new Host(&random, &w, &config, int_val);
      emp::Ptr<Organism> sym = new Symbiont(&random, &w, &config, int_val);
      w.AddOrgAt(sym,0);
      w.AddOrgAt(host,0);

      REQUIRE(sym->GetPoints() == 0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;
      int synergy = 5;

      double host_res = (res * int_val) + (res * int_val * int_val * synergy); //host portion + sym donation
      double sym_res = res * int_val * int_val;

      res = host->HandleEctosymbiosis(res, 0);
      host->DistribResources(res);

      THEN("Parallel organism interaction benefits both"){
        REQUIRE(sym->GetPoints() == sym_res);
        REQUIRE(host->GetPoints() == host_res);
      }
    }
    WHEN("A hosted sym confers immunity to ectosymbiosis"){
      double int_val = 0.5;

      config.ECTOSYMBIOSIS(1);
      config.ECTOSYMBIOTIC_IMMUNITY(1);

      emp::Ptr<Host> host = new Host(&random, &w, &config, int_val);
      emp::Ptr<Organism> parallel_sym = new Symbiont(&random, &w, &config, int_val);
      emp::Ptr<Organism> hosted_sym = new Symbiont(&random, &w, &config, int_val);
      w.AddOrgAt(parallel_sym,0);
      w.AddOrgAt(host,0);
      host->AddSymbiont(hosted_sym);

      REQUIRE(parallel_sym->GetPoints() == 0);
      REQUIRE(hosted_sym->GetPoints() == 0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;
      int synergy = 5;

      double host_res = (res * int_val) + (res * int_val * int_val * synergy); //host portion + sym donation
      double hosted_sym_res = res * int_val * int_val;

      res = host->HandleEctosymbiosis(res, 0);
      host->DistribResources(res);

      THEN("The parallel symbiont does not recieve resources"){
        REQUIRE(hosted_sym->GetPoints() == hosted_sym_res);
        REQUIRE(parallel_sym->GetPoints() == 0);
        REQUIRE(host->GetPoints() == host_res);
      }
    }
    WHEN("There is no parallel sym"){
      double int_val = 0.5;

      config.ECTOSYMBIOSIS(1);

      emp::Ptr<Host> host = new Host(&random, &w, &config, int_val);
      w.AddOrgAt(host,0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;
      double host_res = res * int_val;

      res = host->HandleEctosymbiosis(res, 0);
      host->DistribResources(res);

      THEN("The host does not get a sym modifier on its resources"){
        REQUIRE(host->GetPoints() == host_res);
      }
    }
  }
}
