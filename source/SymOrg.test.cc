#include "SymOrg.h"
#include <set>

TEST_CASE( "Host-Symbiont interactions") {

  GIVEN( "an empty somewhat generous host without resource type and with 17 points" ) {
    Host h(.5, {}, {}, std::set<int>(), 17);

    REQUIRE( h.GetIntVal() == .5 );
    REQUIRE( h.GetSymbionts().size() == 0 );
    REQUIRE( h.GetReproSymbionts().size() == 0 );
    REQUIRE( h.GetResTypes().size() == 0 );
    REQUIRE( h.GetPoints() == 17 );
  
    WHEN( "resources are distributed" ) {
      h.DistribResources(100, 5);

      THEN( "the host receives all resources" ) {
        REQUIRE( h.GetPoints() == Approx(117) );
      }
    }

    WHEN( "a somewhat generous symbiont with no resource type and with 203 points is added and resources are distributed" ) {
      Symbiont s(.6, 203);
      h.AddSymbionts(s, 3);
      h.DistribResources(100, 5);

      THEN( "the host gains that symbiont" ) {
        REQUIRE( h.GetSymbionts().size() == 1 );
        REQUIRE( h.GetSymbionts()[0].GetIntVal() == .6 );
        // REQUIRE( h.GetSymbionts()[0] == s );
      }
      THEN( "the host receives more resources than without the symbiont" ) {
        REQUIRE( h.GetPoints() > 117 );
        REQUIRE( h.GetPoints() == Approx(217) );
      }
      THEN( "the symbiont receives resources without any bonus") {
        REQUIRE( h.GetSymbionts()[0].GetPoints() > 203 );
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(223) );
      }
      THEN( "conservation of resources is not in place" ) {
        REQUIRE( (h.GetPoints()-17)+(h.GetSymbionts()[0].GetPoints()-203) > 100 );
      }
      THEN( "bonus is applied to returned resources" ) {
        REQUIRE( (h.GetPoints()-17)+(h.GetSymbionts()[0].GetPoints()-203) - 100 == Approx(100*.5*.6*(5-1)) );
      }
    }

    WHEN( "synergy is higher" ) {
      Symbiont s(.6, 203);
      h.AddSymbionts(s, 3);
      h.DistribResources(100, 5.5);

      THEN( "bonus larger is applied to returned resources" ) {
        REQUIRE( (h.GetPoints()-17)+(h.GetSymbionts()[0].GetPoints()-203) - 100 == Approx(100*.5*.6*(5.5-1)) );
      }
    }

    WHEN( "a somewhat hostile symbiont with no resource type and with 101 points is added and resources are distributed" ) {
      Symbiont s(-.65, 101);
      h.AddSymbionts(s, 3);
      h.DistribResources(113, 5);


      THEN( "the host gains that symbiont" ) {
        REQUIRE( h.GetSymbionts().size() == 1 );
        REQUIRE( h.GetSymbionts()[0].GetIntVal() == -.65 );
        // REQUIRE( h.GetSymbionts()[0] == s );
      }
      THEN( "the host receives less than a quarter of the resources" ) {
        REQUIRE( h.GetPoints() < 17+25 );
        REQUIRE( h.GetPoints() == Approx(36.775) );
      }
      THEN( "the symbiont receives resources" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() > 101 );
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(194.225) );
      }
      THEN( "the conservation of resources is in place" ) {
        REQUIRE( h.GetPoints() + h.GetSymbionts()[0].GetPoints() == Approx(17 + 101 + 113) );
      }
    }

    WHEN( "more symbionts are added than the limit" ) {
      Symbiont s1(.12, 101);
      h.AddSymbionts(s1, 3);
      Symbiont s2(.12, 102);
      h.AddSymbionts(s2, 3);
      Symbiont s3(.12, 103);
      h.AddSymbionts(s3, 3);
      Symbiont s4(.12, 104);
      h.AddSymbionts(s4, 3);
      Symbiont s5(.12, 105);
      h.AddSymbionts(s5, 3);
      Symbiont s6(.12, 106);
      h.AddSymbionts(s6, 3);
      Symbiont s7(.12, 107);
      h.AddSymbionts(s7, 3);
      Symbiont s8(.12, 108);
      h.AddSymbionts(s8, 3);
      

      THEN( "the host gains as many symbionts as the limit" ) {
        REQUIRE( h.GetSymbionts().size() == 3 );
      }

      THEN( "the first added are kept" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() == 101 );
        REQUIRE( h.GetSymbionts()[1].GetPoints() == 102 );
        REQUIRE( h.GetSymbionts()[2].GetPoints() == 103 );
      }
    }

    WHEN( "a repro symbiont is added and resources are distributed" ) {
      Symbiont s(-.7, 37);
      h.AddReproSym(s);
      h.DistribResources(13, 5);

      THEN( "the host gains that repro symbiont" ) {
        REQUIRE( h.GetReproSymbionts().size() == 1 );
        REQUIRE( h.GetReproSymbionts()[0].GetIntVal() == -.7 );
        REQUIRE( h.GetReproSymbionts()[0].GetPoints() == 37 );
        // REQUIRE( h.GetSymbionts()[0] == s );
      }
      THEN( "the host receives all resources" ) {
        REQUIRE( h.GetPoints() == Approx(30) );
      }
    }
  }

  

  GIVEN( "an empty slightly defensive host" ) {
    Host h(-.2);

    REQUIRE( h.GetIntVal() == -.2 );
    REQUIRE( h.GetSymbionts().size() == 0 );
    REQUIRE( h.GetReproSymbionts().size() == 0 );
    REQUIRE( h.GetResTypes().size() == 0 );
    REQUIRE( h.GetPoints() == 0 );
  
    WHEN( "resources are distributed" ) {
      h.DistribResources(1, 5);

      THEN( "the host does not receive all resources" ) {
        REQUIRE( h.GetPoints() < 1 );
        REQUIRE( h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a somewhat generous symbiont is added and resources are distributed" ) {
      Symbiont s(.6);
      h.AddSymbionts(s, 3);
      h.DistribResources(1, 5);

      THEN( "the host receives some resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8) );
      }
      THEN( "the symbiont does not receive any resources" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(0) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a very slightly hostile symbiont is added and resources are distributed" ) {
      Symbiont s(-.1);
      h.AddSymbionts(s, 3);
      h.DistribResources(1, 5);

      THEN( "the host receives some resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8) );
      }
      THEN( "the symbiont does not recieve any resources" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(0) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a somewhat hostile symbiont is added and resources are distributed" ) {
      Symbiont s(-.5);
      h.AddSymbionts(s, 3);
      h.DistribResources(1, 5);

      THEN( "the host receives fewer resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8*.7) );
      }
      THEN( "the symbiont receives resources" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(.8*.3) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }

    WHEN( "a fully hostile symbiont is added and resources are distributed" ) {
      Symbiont s(-1);
      h.AddSymbionts(s, 3);
      h.DistribResources(1, 5);

      THEN( "the host receives very few resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8*.2) );
      }
      THEN( "the symbiont receives some resources" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(.8*.8) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }



    WHEN( "many symbionts are added" ) {
      Symbiont s1(1);
      h.AddSymbionts(s1, 3);
      Symbiont s2(-.15);
      h.AddSymbionts(s2, 3);
      Symbiont s3(-.73);
      h.AddSymbionts(s3, 3);
      Symbiont s4(.6);
      h.AddSymbionts(s4, 3);
      Symbiont s5(-1);
      h.AddSymbionts(s5, 3);
      h.DistribResources(1, 5);

      THEN( "the host receives the right amount of resources" ) {
        REQUIRE( h.GetPoints() == Approx(.8*(1+1+.47)/3) );
      }
      THEN( "the nice symbiont receives nothing" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() == Approx(0) );
      }
      THEN( "the not-too-hostile symbiont receives nothing" ) {
        REQUIRE( h.GetSymbionts()[1].GetPoints() == Approx(0) );
      }
      THEN( "the hostile symbiont receives a bit" ) {
        REQUIRE( h.GetSymbionts()[2].GetPoints() == Approx(.8*.53/3) );
      }
      THEN( "with the exception of resources spent on defense, resources are conserved" ) {
        REQUIRE( h.GetSymbionts()[0].GetPoints() + h.GetSymbionts()[1].GetPoints() + h.GetSymbionts()[2].GetPoints() + h.GetPoints() == Approx(.8) );
      }
    }
  }
}
