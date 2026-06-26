#include "../../default_mode/SymWorld.h"
#include "../../default_mode/WorldSetup.cc"
#include "../../default_mode/DataNodes.h"

#include "../test_utils.h"

TEST_CASE("Tag matching", "[default]") {
  int trans_res = 10;
  int starting_res = 15;
  double tag_distance_mean = 0.25;
  double int_val = 0;

  emp::Random random(17);
  SymConfigBase config;
  config.WORLD_WIDTH(2);
  config.WORLD_HEIGHT(2);
  config.SPATIAL_STRUCT_MODE("well-mixed");
  config.FREE_HT_FAILURE(1);
  config.SYM_LIMIT(1);
  config.SYM_HORIZ_TRANS_RES(trans_res);
  config.SYM_VERT_TRANS_RES(trans_res);
  config.TAG_MATCHING(1);
  config.TAG_PERMISSIVENESS(tag_distance_mean);
  config.TAG_MUTATION_SIZE(0.0);
  config.VERTICAL_TRANSMISSION(1.0); // REVIEW NOTE: Is there a reason to not set this to 1?

  WHEN("Hamming metric is used") {
    config.TAG_METRIC("hamming");
    SymWorld world(random, &config);
    world.Setup();

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
        world.Clear(); // Reset world occupants
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(similar_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, similar_tag) <= tag_distance_mean);

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

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
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == true);
          REQUIRE(target_host->GetSymbionts().at(0) == obstructive_symbiont);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
      WHEN("Their tags are too dissimilar") {
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
    }
  }

  WHEN("Streak metric is used") {
    config.TAG_METRIC("streak");
    SymWorld world(random, &config);
    world.Setup();
    emp::BitSet<TAG_LENGTH> symbiont_tag = emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000");
    emp::BitSet<TAG_LENGTH> similar_tag = emp::BitSet<TAG_LENGTH>("00100101011000000000000000000000");
    emp::BitSet<TAG_LENGTH> distant_tag = emp::BitSet<TAG_LENGTH>("00111111111111111111110000000000");

    WHEN("Tag distances are calculated") {
      THEN("Tag has tag distance 0 with itself") {
        double expected_difference = 0;
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, emp::BitSet<TAG_LENGTH>("00000000000000000000000000000000"));
        REQUIRE(std::abs(expected_difference - calculated_difference) < 0.000001);
      }
      THEN("Close tag distance is calculated correctly") {
        int mismatch = 2;
        int match = 21;
        const double pow_match = static_cast<double>(TAG_LENGTH - match + 1) / std::pow(2, match);
        const double pow_mismatch = static_cast<double>(TAG_LENGTH - mismatch + 1) / std::pow(2, mismatch);

        double expected_difference = 1 - (pow_mismatch / (pow_match + pow_mismatch));
        double calculated_difference = (*world.GetTagMetric())(symbiont_tag, similar_tag);

        REQUIRE(expected_difference == calculated_difference);
        REQUIRE(tag_distance_mean > calculated_difference + 0.05);
      }
      THEN("Distant tag distance is calculated correctly") {
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
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

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
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

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
        world.Clear(); // Reset world occupants
        world.AddOrgAt(source_host, source_pos);
        source_host->AddSymbiont(symbiont);
        symbiont->AddPoints(starting_res);

        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(world.IsOccupied(source_pos));

        world.AddOrgAt(target_host, target_pos);
        REQUIRE(world.GetNumOrgs() == 2);

        target_host->SetTag(distant_tag);
        REQUIRE((*world.GetTagMetric())(symbiont_tag, distant_tag) > tag_distance_mean);

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == false);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
    }
  }

  WHEN("Hash metric is used") {
    config.TAG_METRIC("hash");
    SymWorld world(random, &config);
    world.Setup();
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
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

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
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

        THEN("The symbiont fails") {
          REQUIRE(target_host->HasSym() == true);
          REQUIRE(target_host->GetSymbionts().at(0) == obstructive_symbiont);
        }
        THEN("The parent symbiont does not spend points on reproduction") {
          REQUIRE(symbiont->GetPoints() == starting_res);
        }
      }
      WHEN("Their tags are too dissimilar") {
        world.Clear(); // Reset world occupants
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

        symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));

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

TEST_CASE("Evolvable tag permissiveness", "[default]") {
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
  config.TAG_METRIC("hamming");
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

      WHEN("The host's permissiveness is too low") {
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
          symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));
          REQUIRE(!target_host->HasSym());
          REQUIRE(symbiont->GetPoints() == starting_res);
          // since free HT failure is off, we know that if the point check here fails, it's because of
          // a sym_limit block, not a tag block!
        }
      }

      WHEN("The host's permissiveness is sufficiently high") {
        target_host->SetTagPermissiveness(1);
        THEN("The symbiont offspring can horizontally transmit") {
          symbiont->IndependentReproduction(emp::WorldPosition(1, source_pos));
          REQUIRE(target_host->HasSym());
          REQUIRE(symbiont->GetPoints() < starting_res);
        }
      }
    }
  }
}

// NOTE: This test is a little bit frustrating to maintain if we change calls to
//        random number generator. Tweaking it to be more maintainable, but I think
//        still test the intended code.
TEST_CASE("SetupSymbionts with tag matching on", "[default]") {
  using sym_world_t = test_utils::TestingWorldWrapper<SymWorld>;
  GIVEN("a world") {
    emp::Random random(18);
    size_t world_size = 1;
    SymConfigBase config;
    test_utils::SetWellMixed(config, world_size);
    sym_world_t world(random, &config);

    WHEN("SetupSymbionts is called") {
      config.TAG_MATCHING(1);
      config.TAG_PERMISSIVENESS(0.0);
      world.SetTagMetric(emp::NewPtr<emp::HammingMetric<TAG_LENGTH>>());
      world.SetupSpatialStructure();
      // Setup hosts with one prob = 0
      config.HOST_STARTING_TAGS_ONE_PROB(0.5);
      world.SetupHosts(&world_size);
      world.SetupSymbionts(&world_size);

      THEN("A symbiont should be added to the world with a tag that matches their host") {
        Organism& host = world.GetOrg(0);
        REQUIRE(host.GetSymbionts().size() == 1);
        Organism& sym = host.GetSymbiont(0);
        const auto& host_tag = host.GetTag();
        const auto& sym_tag = sym.GetTag();
        REQUIRE(world.CalcTagMetric(sym_tag, host_tag) == 0);
      }
    }
  }
}

TEST_CASE("SetupHosts with tag matching on", "[default]") {
  using sym_world_t = test_utils::TestingWorldWrapper<SymWorld>;
  GIVEN("a world") {
    emp::Random random(17);
    SymConfigBase config;
    size_t num_to_add = 5;
    test_utils::SetWellMixed(config, num_to_add);
    sym_world_t world(random, &config);
    world.SetupSpatialStructure();

    WHEN("Random starting tags are on") {
      config.TAG_MATCHING(1);
      world.SetTagMetric(emp::NewPtr<emp::HammingMetric<TAG_LENGTH>>());
      config.HOST_STARTING_TAGS_ONE_PROB(0.1);
      // we expect 3.2ish 1s per tag

      world.SetupHosts(&num_to_add);

      size_t num_added = world.GetNumOrgs();
      REQUIRE(num_added == num_to_add);

      THEN("Tags are randomly initialized per the starting ones probability") {
        int total_ones = 0;
        for(size_t i = 0; i < num_added; i++) {
          int ones = world.GetOrg(i).GetTag().CountOnes();
          total_ones += ones;
          REQUIRE(ones >= 0);
          REQUIRE(ones <= 5);
        }
        REQUIRE(total_ones > 12);
        REQUIRE(total_ones < 18);
      }
    }
  }
}

TEST_CASE("Host Mutate tag", "[default]") {
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
  SymConfigBase config;
  SymWorld world(*random, &config);
  double int_val = -0.31;

  //TAG MUTATION SIZE
  WHEN("Tag matching is on") {
    THEN("Tags mutate according to tag mutation rate") {
      emp::HammingMetric<TAG_LENGTH> metric = emp::HammingMetric<TAG_LENGTH>();
      config.TAG_MATCHING(1);
      config.TAG_MUTATION_SIZE(0.1);
      emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);
      emp::BitSet<TAG_LENGTH> bit_set = emp::BitSet<TAG_LENGTH>();
      host->SetTag(bit_set);

      REQUIRE(metric.calculate(host->GetTag(), bit_set) == 0);
      host->Mutate();
      REQUIRE(metric.calculate(host->GetTag(), bit_set) > 0);
      REQUIRE(metric.calculate(host->GetTag(), bit_set) <= 1);

      host.Delete();
      config.TAG_MATCHING(0); // don't try to delete a non-existent
      // SymWorld metric
    }
  }
  random.Delete();
}

TEST_CASE("Host Mutate tag permissiveness mutation rate", "[default]") {
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
  SymConfigBase config;
  double int_val = 1;

  config.MUTATION_RATE(1);
  config.TAG_MATCHING(1);
  config.HOST_TAG_PERMISSIVENESS_EVOLVES(1);
  config.HOST_TAG_PERMISSIVENESS_MUTATION_SIZE(0.6);
  config.TAG_PERMISSIVENESS(0.125);
  config.HOST_MUTATION_RATE(-1);
  config.MUTATION_RATE(1);
  config.HOST_MUTATION_RATE(0.25);

  SymWorld world(*random, &config);

  emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);

  WHEN("Tag permissiveness mutation rate is positive and high") {
    config.HOST_TAG_PERMISSIVENESS_MUTATION_RATE(0.6);

    THEN("Tag permissiveness mutates often") {
      int mutation_count = 0;
      for (int i = 0; i < 10; i++) {
        emp::Ptr<Organism> host_baby = host->Reproduce();
        mutation_count += (host_baby->GetTagPermissiveness() != host->GetTagPermissiveness());
        host_baby.Delete();
      }
      REQUIRE(mutation_count > 5);
      REQUIRE(mutation_count < 8);
    }
  }

  WHEN("Tag permissiveness mutation rate is positive and low") {
    config.HOST_TAG_PERMISSIVENESS_MUTATION_RATE(0.1);

    THEN("Tag permissiveness mutates rarely") {
      int mutation_count = 0;
      for (int i = 0; i < 10; i++) {
        emp::Ptr<Organism> host_baby = host->Reproduce();
        mutation_count += (host_baby->GetTagPermissiveness() != host->GetTagPermissiveness());
        host_baby.Delete();
      }
      REQUIRE(mutation_count > 0);
      REQUIRE(mutation_count < 3);
    }
  }

  WHEN("Tag permissiveness mutation rate is -1") {
    config.HOST_TAG_PERMISSIVENESS_MUTATION_RATE(-1);

    WHEN("Host mutation rate is -1") {
      config.HOST_MUTATION_RATE(-1);
      config.MUTATION_RATE(0.3);

      THEN("Tag permissiveness mutation rate inherits from mutation rate") {
        int mutation_count = 0;
        for (int i = 0; i < 10; i++) {
          emp::Ptr<Organism> host_baby = host->Reproduce();
          mutation_count += (host_baby->GetTagPermissiveness() != host->GetTagPermissiveness());
          host_baby.Delete();
        }
        REQUIRE(mutation_count >= 2);
        REQUIRE(mutation_count <= 4);
      }
    }
    WHEN("Host mutation rate is positive") {
      config.HOST_MUTATION_RATE(0.5);
      THEN("Tag permissiveness mutation rate inherits from host mutation rate") {
        int mutation_count = 0;
        for (int i = 0; i < 10; i++) {
          emp::Ptr<Organism> host_baby = host->Reproduce();
          mutation_count += (host_baby->GetTagPermissiveness() != host->GetTagPermissiveness());
          host_baby.Delete();
        }
        REQUIRE(mutation_count >= 4);
        REQUIRE(mutation_count <= 6);
      }
    }
  }
  host.Delete();
  random.Delete();
}

TEST_CASE("Host Mutate tag permissiveness mutation size", "[default]") {
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(3);
  SymConfigBase config;
  double int_val = 1;
  double tag_permissiveness = 0.125;
  double tag_permissiveness_mut_size = 0.1;

  config.MUTATION_RATE(1);
  config.TAG_MATCHING(1);
  config.HOST_TAG_PERMISSIVENESS_EVOLVES(1);
  config.HOST_TAG_PERMISSIVENESS_MUTATION_SIZE(tag_permissiveness_mut_size);
  config.TAG_PERMISSIVENESS(tag_permissiveness);

  SymWorld world(*random, &config);

  emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, int_val);

  WHEN("Host mutate is called") {

    REQUIRE(host->GetTagPermissiveness() == tag_permissiveness);
    host->Mutate();
    THEN("Host tag permissiveness is mutated") {
      REQUIRE(host->GetTagPermissiveness() != tag_permissiveness);
      REQUIRE(host->GetTagPermissiveness() < tag_permissiveness + tag_permissiveness_mut_size);
      REQUIRE(host->GetTagPermissiveness() > tag_permissiveness - tag_permissiveness_mut_size);
    }
  }
  host.Delete();
  random.Delete();
}

TEST_CASE("Symbiont Mutate tag", "[default]") {
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(37);
  SymConfigBase config;
  config.TAG_MATCHING(1);
  emp::Ptr<SymWorld> world = emp::NewPtr<SymWorld>(*random, &config);

  WHEN("Tag matching is on") {
    THEN("Tags mutate according to tag mutation rate") {
      emp::HammingMetric<TAG_LENGTH> metric = emp::HammingMetric<TAG_LENGTH>();
      config.TAG_MATCHING(1);
      config.TAG_MUTATION_SIZE(0.1);
      emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, world, &config, 0);
      emp::BitSet<TAG_LENGTH> bit_set = emp::BitSet<TAG_LENGTH>();
      symbiont->SetTag(bit_set);

      REQUIRE(metric.calculate(symbiont->GetTag(), bit_set) == 0);
      symbiont->Mutate();
      REQUIRE(metric.calculate(symbiont->GetTag(), bit_set) > 0);
      REQUIRE(metric.calculate(symbiont->GetTag(), bit_set) <= 1);

      symbiont.Delete();
    }
  }
  world.Delete();
  random.Delete();
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