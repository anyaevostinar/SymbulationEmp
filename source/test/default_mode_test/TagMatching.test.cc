#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/WorldSetup.cc"

TEST_CASE("Tag matching", "[default]") {
  int trans_res = 10;
  int starting_res = 15;
  double tag_distance_mean = 0.25;
  double int_val = 0;

  emp::Random random(17);
  SymConfigBase config;
  config.GRID_X(2);
  config.GRID_Y(2);
  config.FREE_HT_FAILURE(1);
  config.SYM_LIMIT(1);
  config.SYM_HORIZ_TRANS_RES(trans_res);
  config.SYM_VERT_TRANS_RES(trans_res);
  config.TAG_MATCHING(1);
  config.TAG_PERMISSIVENESS(tag_distance_mean);
  config.TAG_MUTATION_SIZE(0.0);
  
  WHEN("Hamming metric is used") {
    config.TAG_METRIC(0);
    SymWorld world(random, &config);

    emp::BitSet<TAG_LENGTH> symbiont_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
    emp::BitSet<TAG_LENGTH> similar_tag =  emp::BitSet<TAG_LENGTH>("00000000000000010000010000100000");
    emp::BitSet<TAG_LENGTH> distant_tag =  emp::BitSet<TAG_LENGTH>("11110000000000000000000111111111");

    WHEN("Tag distances are calculated") {
      THEN("Tag has tag distance 0 with itself") {
        double expected_difference = 0;
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000"));
        REQUIRE(expected_difference == calculated_difference);
      }
      THEN("Close tag distance is calculated correctly") {
        double mismatch = 3;
        double expected_difference = mismatch / TAG_LENGTH;
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, similar_tag);

        REQUIRE(expected_difference == calculated_difference);
        REQUIRE(tag_distance_mean > calculated_difference + 0.05);
      }
      THEN("Distant tag distance is calculated correctly") {
        double mismatch = 13;
        double expected_difference = mismatch / TAG_LENGTH;
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, distant_tag);

        REQUIRE(expected_difference == calculated_difference);
        REQUIRE(tag_distance_mean < calculated_difference - 0.05);
      }
    }


    WHEN("A symbiont tries to vertically transmit offspring into a host child") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      
      symbiont->SetTag(symbiont_tag);
      symbiont->AddPoints(starting_res);

      WHEN("Their tags are sufficiently close") {
        host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->VerticalTransmission(host);

        THEN("The symbiont succeeds") {
          REQUIRE(host->HasSym() == true);
        }
        THEN("The parent symbiont spends points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res-trans_res);
        }
        THEN("The child symbiont starts with 0 points") {
          REQUIRE(host->GetSymbionts().at(0)->GetPoints() == 0);
        }
      }
      WHEN("Their tags are too dissimilar") {
        host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, distant_tag) > tag_distance_mean);

        symbiont->VerticalTransmission(host);

        THEN("The symbiont fails") {
          REQUIRE(host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }

      host.Delete();
      symbiont.Delete();
    }
    
    WHEN("A symbiont tries to horizontally transmit offspring into a host") {
      // note that these can fail simply because the empty 
      // neighbor host is not selected by getrandomneighbor

      emp::Ptr<Host> source_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Host> target_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      symbiont->SetTag(symbiont_tag);
      
      WHEN("Their tags are sufficiently close and the host has room") {
        size_t source_pos = 0;
        size_t target_pos = 1;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));
        
        THEN("The symbiont succeeds") {
          REQUIRE(target_host->HasSym() == true);
        }
        THEN("The parent symbiont spends points on reproduction"){
          // horiz trans sets points to 0, rather than subtracting
          REQUIRE(symbiont->GetPoints() == 0);
        }
        THEN("The child symbiont starts with 0 points"){
          REQUIRE(target_host->GetSymbionts().at(0)->GetPoints() == 0);
        }
      }
      WHEN("Their tags are sufficiently close and the host does not have room") {
        emp::Ptr<Organism> obstructive_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
        
        size_t source_pos = 1;
        size_t target_pos = 0;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        target_host->AddSymbiont(obstructive_symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == true);
          REQUIRE(target_host->GetSymbionts().at(0) == obstructive_symbiont);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
      WHEN("Their tags are too dissimilar") {
        size_t source_pos = 1;
        size_t target_pos = 0;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, distant_tag) > tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
    }
  }
  
  WHEN("Streak metric is used"){
    config.TAG_METRIC(1);
    SymWorld world(random, &config);

    emp::BitSet<TAG_LENGTH> symbiont_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
    emp::BitSet<TAG_LENGTH> similar_tag = emp::BitSet<TAG_LENGTH>("00100101011000000000000000000000");
    emp::BitSet<TAG_LENGTH> distant_tag = emp::BitSet<TAG_LENGTH>("00111111111111111111110000000000");

    WHEN("Tag distances are calculated") {
      THEN("Tag has tag distance 0 with itself") {
        double expected_difference = 0;
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000"));
        REQUIRE(std::abs(expected_difference - calculated_difference) < 0.000001);
      }
      THEN("Close tag distance is calculated correctly"){
        int mismatch = 2;
        int match = 21;
        const double pow_match = static_cast<double>(TAG_LENGTH - match + 1) / std::pow(2, match);
        const double pow_mismatch = static_cast<double>(TAG_LENGTH - mismatch + 1) / std::pow(2, mismatch);

        double expected_difference = 1 - (pow_mismatch / (pow_match + pow_mismatch));
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, similar_tag);

        REQUIRE(expected_difference == calculated_difference);
        REQUIRE(tag_distance_mean > calculated_difference + 0.05);
      }
      THEN("Distant tag distance is calculated correctly"){
        int mismatch = 20;
        int match = 10;
        const double pow_match = static_cast<double>(TAG_LENGTH - match + 1) / std::pow(2, match);
        const double pow_mismatch = static_cast<double>(TAG_LENGTH - mismatch + 1) / std::pow(2, mismatch);

        double expected_difference = 1 - (pow_mismatch / (pow_match + pow_mismatch));
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, distant_tag);

        REQUIRE(expected_difference == calculated_difference);
        REQUIRE(tag_distance_mean < calculated_difference - 0.05);
      }
    }

    WHEN("A symbiont tries to vertically transmit offspring into a host child") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

      symbiont->SetTag(symbiont_tag);
      symbiont->AddPoints(starting_res);

      WHEN("Their tags are sufficiently close") {
        host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->VerticalTransmission(host);

        THEN("The symbiont succeeds") {
          REQUIRE(host->HasSym() == true);
        }
        THEN("The parent symbiont spends points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res - trans_res);
        }
        THEN("The child symbiont starts with 0 points") {
          REQUIRE(host->GetSymbionts().at(0)->GetPoints() == 0);
        }
      }
      WHEN("Their tags are too dissimilar") {
        host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, distant_tag) > tag_distance_mean);

        symbiont->VerticalTransmission(host);

        THEN("The symbiont fails") {
          REQUIRE(host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }

      host.Delete();
      symbiont.Delete();
    }

    WHEN("A symbiont tries to horizontally transmit offspring into a host") {
      emp::Ptr<Host> source_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Host> target_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      symbiont->SetTag(symbiont_tag);

      WHEN("Their tags are sufficiently close and the host has room") {
        size_t source_pos = 0;
        size_t target_pos = 1;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont succeeds") {
          REQUIRE(target_host->HasSym() == true);
        }
        THEN("The parent symbiont spends points on reproduction") {
          // horiz trans sets points to 0, rather than subtracting
          REQUIRE(symbiont->GetPoints() == 0);
        }
        THEN("The child symbiont starts with 0 points") {
          REQUIRE(target_host->GetSymbionts().at(0)->GetPoints() == 0);
        }
      }
      WHEN("Their tags are sufficiently close and the host does not have room") {
        emp::Ptr<Organism> obstructive_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

        size_t source_pos = 1;
        size_t target_pos = 0;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        target_host->AddSymbiont(obstructive_symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == true);
          REQUIRE(target_host->GetSymbionts().at(0) == obstructive_symbiont);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
      WHEN("Their tags are too dissimilar") {
        size_t source_pos = 1;
        size_t target_pos = 0;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, distant_tag) > tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
    }
  }
  
  WHEN("Hash metric is used"){
    config.TAG_METRIC(2);
    SymWorld world(random, &config);

    emp::BitSet<TAG_LENGTH> symbiont_tag = emp::BitSet<TAG_LENGTH>("10000000011000000000000000000000");
    emp::BitSet<TAG_LENGTH> similar_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
    emp::BitSet<TAG_LENGTH> distant_tag = emp::BitSet<TAG_LENGTH>("00000000011000000010000000000001");

    WHEN("Tag distances are calculated") {
      THEN("All 0s tag does NOT have tag distance 0 with itself") {
        double expected_difference = 0.0774070847;
        double calculated_difference = (*world.GetTagMetric())(emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000"), emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000"));
        REQUIRE(std::abs(expected_difference - calculated_difference) < 0.00001);
      }
      THEN("Close tag distance is calculated correctly") {
        double expected_difference = 0.0704272055;
        double calculated_difference = (*world.GetTagMetric())(similar_tag, symbiont_tag);

        REQUIRE(std::abs(expected_difference - calculated_difference) < 0.00001);
        REQUIRE(tag_distance_mean > calculated_difference + 0.05);
      }
      THEN("Distance tag distance is calculated correctly") {
        double expected_difference = 0.4593601281;
        double calculated_difference = (*world.GetTagMetric())(distant_tag, symbiont_tag);

        REQUIRE(std::abs(expected_difference - calculated_difference) < 0.00001);
        REQUIRE(tag_distance_mean < calculated_difference - 0.05);
      }
    }
    
    WHEN("A symbiont tries to vertically transmit offspring into a host child") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

      symbiont->SetTag(symbiont_tag);
      symbiont->AddPoints(starting_res);

      WHEN("Their tags are sufficiently close") {
        host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(similar_tag, symbiont_tag) <= tag_distance_mean);

        symbiont->VerticalTransmission(host);

        THEN("The symbiont succeeds") {
          REQUIRE(host->HasSym() == true);
        }
        THEN("The parent symbiont spends points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res - trans_res);
        }
        THEN("The child symbiont starts with 0 points") {
          REQUIRE(host->GetSymbionts().at(0)->GetPoints() == 0);
        }
      }
      WHEN("Their tags are too dissimilar") {
        host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(distant_tag, symbiont_tag) > tag_distance_mean);

        symbiont->VerticalTransmission(host);

        THEN("The symbiont fails") {
          REQUIRE(host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }

      host.Delete();
      symbiont.Delete();
    }
    
    WHEN("A symbiont tries to horizontally transmit offspring into a host") {
      emp::Ptr<Host> source_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Host> target_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      symbiont->SetTag(symbiont_tag);

      WHEN("Their tags are sufficiently close and the host has room") {
        size_t source_pos = 0;
        size_t target_pos = 1;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);

        REQUIRE((*world.GetTagMetric())(similar_tag, symbiont_tag) <= tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont succeeds") {
          REQUIRE(target_host->HasSym() == true);
        }
        THEN("The parent symbiont spends points on reproduction") {
          // horiz trans sets points to 0, rather than subtracting
          REQUIRE(symbiont->GetPoints() == 0);
        }
        THEN("The child symbiont starts with 0 points") {
          REQUIRE(target_host->GetSymbionts().at(0)->GetPoints() == 0);
        }
      }
      WHEN("Their tags are sufficiently close and the host does not have room") {
        emp::Ptr<Organism> obstructive_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

        size_t source_pos = 1;
        size_t target_pos = 0;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        target_host->AddSymbiont(obstructive_symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(similar_tag, symbiont_tag) <= tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == true);
          REQUIRE(target_host->GetSymbionts().at(0) == obstructive_symbiont);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
      WHEN("Their tags are too dissimilar") {
        size_t source_pos = 1;
        size_t target_pos = 0;
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(distant_tag, symbiont_tag) > tag_distance_mean);

        symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
    }
  }
}

TEST_CASE("Evolvable tag permissiveness", "[default]"){

  int trans_res = 10;
  int starting_res = 15;
  double tag_distance_mean = 0.25;
  double int_val = 0;

  emp::Random random(17);
  SymConfigBase config;
  config.FREE_HT_FAILURE(0);
  config.SYM_LIMIT(1);
  config.SYM_HORIZ_TRANS_RES(trans_res);
  config.SYM_VERT_TRANS_RES(trans_res);

  config.TAG_MATCHING(1);
  config.TAG_MUTATION_SIZE(0.0);
  config.TAG_METRIC(0);
  config.TAG_PERMISSIVENESS(tag_distance_mean);
  config.HOST_TAG_PERMISSIVENESS_EVOLVES(1);
  config.HOST_TAG_PERMISSIVENESS_MUTATION_SIZE(0);

  SymWorld world(random, &config);

  WHEN("Host tag permissiveness values can evolve") {
    emp::BitSet<TAG_LENGTH> symbiont_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
    emp::BitSet<TAG_LENGTH> host_tag = emp::BitSet<TAG_LENGTH>("00000001000000010000010000100000");

    WHEN("A symbiont transmits vertically") {
      config.VERTICAL_TRANSMISSION(1);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      
      symbiont->SetPoints(starting_res);
      symbiont->SetTag(symbiont_tag);
      host->SetTag(host_tag);
      host->AddSymbiont(symbiont);
      
      WHEN("The host's permissiveness is too low"){
        double host_permissiveness = 0;
        host->SetTagPermissiveness(host_permissiveness);
        THEN("The symbiont offspring cannot vertically transmit") {
          emp::Ptr<Organism> host_baby = host->Reproduce();
          symbiont->VerticalTransmission(host_baby);
          REQUIRE(host_baby->GetTagPermissiveness() == host_permissiveness);
          REQUIRE(!host_baby->HasSym());
          REQUIRE(symbiont->GetPoints() == starting_res);
          host_baby.Delete();
        }
      }

      WHEN("The host's permissiveness is sufficiently high") {
        double host_permissiveness = 1;
        host->SetTagPermissiveness(host_permissiveness);
        THEN("The symbiont offspring can vertically transmit") {
          emp::Ptr<Organism> host_baby = host->Reproduce();
          symbiont->VerticalTransmission(host_baby);
          REQUIRE(host_baby->GetTagPermissiveness() == host_permissiveness);
          REQUIRE(host_baby->HasSym());
          REQUIRE(symbiont->GetPoints() < starting_res);
          host_baby.Delete();
        }
      }
      
      host.Delete();
    }
    
    WHEN("A symbiont transmits horizontally") {
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Host> source_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Host> target_host = emp::NewPtr<Host>(&random, &world, &config, int_val);

      source_host->AddSymbiont(symbiont);
      symbiont->AddPoints(starting_res);
      symbiont->SetTag(symbiont_tag);
      target_host->SetTag(host_tag);

      size_t source_pos = 0;
      size_t target_pos = 1;
      world.AddOrgAt(source_host, source_pos);
      world.AddOrgAt(target_host, target_pos);
      REQUIRE(world.GetNumOrgs() == 2);

      WHEN("The host's permissiveness is too low") {
        target_host->SetTagPermissiveness(0);
        THEN("The symbiont offspring cannot horizontally transmit") {
          symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));
          REQUIRE(!target_host->HasSym());
          REQUIRE(symbiont->GetPoints() == starting_res);
          // since free HT failure is off, we know that if the point check here fails, it's because of
          // a sym_limit block, not a tag block!
        }
      }

      WHEN("The host's permissiveness is sufficiently high") {
        target_host->SetTagPermissiveness(1);
        THEN("The symbiont offspring can horizontally transmit") {
          symbiont->HorizontalTransmission(emp::WorldPosition(1, source_pos));
          REQUIRE(target_host->HasSym());
          REQUIRE(symbiont->GetPoints() < starting_res);
        }
      }
    }
  }
}
