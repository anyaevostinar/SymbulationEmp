#include "../../default_mode/SymWorld.h"
#include "../../default_mode/Symbiont.h"
#include "../../default_mode/Host.h"
#include "../../default_mode/WorldSetup.cc"

TEST_CASE( "Host Phylogeny", "[default]" ){
  emp::Random random(17);
  SymConfigBase config;
  config.MUTATION_SIZE(0.09);
  config.MUTATION_RATE(1);
  config.PHYLOGENY(1);
  config.NUM_PHYLO_BINS(20);
  int int_val = 0;
  SymWorld world(random, &config);
  int world_size = 4;
  world.Resize(world_size);

  emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
  emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>> host_sys = world.GetHostSys();

  //ORGANISMS ADDED TO SYSTEMATICS
  WHEN("an organism is added to the world"){
    WHEN("the cell it's added to is occupied"){
      size_t pos = 0;
      int_val = -1;

      emp::Ptr<Organism> occupying_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(occupying_host, pos);
      size_t expected_occupying_taxon_info = 0;
      size_t taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();

      REQUIRE(world.GetNumOrgs() == 1);
      REQUIRE(host_sys->GetNumActive() == 1);
      REQUIRE(expected_occupying_taxon_info == taxon_info);

      world.AddOrgAt(host, pos);
      taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      size_t expected_taxon_info = 10;

      THEN("the occupying organism is removed from the systematic"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(host_sys->GetNumActive() == 1);
        REQUIRE(expected_taxon_info == taxon_info);
      }
    }

    WHEN("the cell it's added to is empty"){
      size_t pos = 0;

      REQUIRE(world.GetNumOrgs() == 0);
      REQUIRE(host_sys->GetNumActive() == 0);

      world.AddOrgAt(host, pos);
      size_t taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
      size_t expected_taxon_info = 10;

      THEN("the organism is tracked by the systematic"){
        REQUIRE(world.GetNumOrgs() == 1);
        REQUIRE(host_sys->GetNumActive() == 1);
        REQUIRE(expected_taxon_info == taxon_info);
      }
    }

    WHEN("there are 20 taxonomic bins"){

      size_t count = 7;
      size_t pos = 0;

      double int_vals[7] = {-1, -0.98, -0.9, -0.8, 0.65, 0.9, 1};
      int expected_taxon_infos[7] = {0, 0, 1, 2, 16, 19, 19};

      THEN("it will be placed in the correct bin"){
        for(size_t i = 0; i < count; i++){
          world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), pos);
          int taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
          REQUIRE(taxon_info == expected_taxon_infos[i]);
        }
      }
      host.Delete();
    }

    WHEN("there are 2 taxonomic bins"){
      config.NUM_PHYLO_BINS(2);

      size_t count = 3;
      size_t pos = 0;

      double int_vals[3] = {1, 0, -0.001};
      int expected_taxon_infos[3] = {1, 1, 0};

      THEN("it will be placed in the correct bin"){
        for(size_t i = 0; i < count; i++){
          world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), pos);
          int taxon_info = host_sys->GetTaxonAt(pos)->GetInfo();
          REQUIRE(taxon_info == expected_taxon_infos[i]);
        }
      }
      host.Delete();
    }
  }

  //ORGANISMS AND RELATIONSHIPS TRACKED
  WHEN("Several generations pass"){
    THEN("The phylogenetic relationships are tracked and accurate"){
      world_size = 10;
      world.Resize(world_size);
      int num_descendants = 4;
      //add the first host
      world.AddOrgAt(host, 0);

      //populate the world with descendents with various interaction values
      //Can't use num_descendants for the following array sizes because some
      //compilers don't allow it
      double int_vals[4] = {0.1, -0.05, -0.2, 0.14};
      size_t parents[4] = {0, 1, 1, 3};
      for(int i = 0; i < num_descendants; i++){
        world.AddOrgAt(emp::NewPtr<Host>(&random, &world, &config, int_vals[i]), (i+1), parents[i]);
      }

      char lineages[][30] = {"Lineage:\n10\n",
                             "Lineage:\n11\n10\n",
                             "Lineage:\n9\n11\n10\n",
                             "Lineage:\n8\n11\n10\n",
                             "Lineage:\n11\n8\n11\n10\n",
                           };


      for(int i = 0; i < (num_descendants+1); i++){
        std::stringstream result;
        host_sys->PrintLineage(host_sys->GetTaxonAt(i), result);
        REQUIRE(result.str() == lineages[i]);
      }
    }
  }
}

TEST_CASE( "Symbiont Phylogeny", "[default]" ){
  emp::Random random(17);
  SymConfigBase config;
  config.MUTATION_SIZE(0.09);
  config.MUTATION_RATE(1);
  config.FREE_LIVING_SYMS(1);
  config.PHYLOGENY(1);
  config.NUM_PHYLO_BINS(20);
  int int_val = 0;
  SymWorld world(random, &config);
  int world_size = 20;
  world.Resize(world_size);

  emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>> sym_sys = world.GetSymSys();

  WHEN("symbionts are added to the world"){
    THEN("they get added to the correct taxonomic bins"){
      REQUIRE(sym_sys->GetNumActive() == 0);
      size_t count = 8;
      //Can't use count for the following array sizes because some
      //compilers don't allow it
      double int_vals[8] = {-1, -0.9, -0.82, 0, 0.5, 0.65, 0.9, 1};
      int taxon_infos[8] = {0, 1, 1, 10, 15, 16, 19, 19};

      emp::Ptr<Organism> syms[count];
      emp::Ptr<Organism> sym;

      for(size_t i = 0; i < count; i++){
        sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_vals[i]);
        world.InjectSymbiont(sym);
        REQUIRE(sym->GetTaxon()->GetInfo() == taxon_infos[i]);
      }
    }
    world.CleanupGraveyard();
  }

  WHEN("symbionts are deleted"){
    THEN("they are no longer tracked by the sym systematic"){
      world_size = 1;
      world.Resize(world_size);
      REQUIRE(sym_sys->GetNumActive() == 0);
      emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.InjectSymbiont(symbiont);
      REQUIRE(sym_sys->GetNumActive() == 1);
      world.DoSymDeath(0);
      REQUIRE(sym_sys->GetNumActive() == 0);
    }

    THEN("hosted and free symbionts are deleted without a segmentation fault") {
      world_size = 4;
      world.Resize(world_size);

      // add a free living sym to the world
      emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      world.InjectSymbiont(symbiont);

      // add a host to the world
      emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.InjectHost(host);

      // add a hosted sym to the host
      emp::Ptr<Organism> hosted_sym = symbiont->Reproduce();
      host->AddSymbiont(hosted_sym);

      // check that free living organisms have properly been added to the world
      REQUIRE(world.GetNumOrgs() == 2);
    }
  }

  WHEN("generations pass"){
    config.MUTATION_SIZE(1);
    config.MUTATION_RATE(1);
    config.PHYLOGENY(1);
    size_t num_syms = 4;

    emp::Ptr<Organism> syms[num_syms];
    syms[0] = emp::NewPtr<Symbiont>(&random, &world, &config, 0);
    world.AddSymToSystematic(syms[0]);

    for(size_t i = 1; i < num_syms; i++){
      syms[i] = syms[i-1]->Reproduce();
    }

    THEN("Their lineages are tracked"){
      char lineages[][30] = {"Lineage:\n10\n",
                             "Lineage:\n16\n10\n",
                             "Lineage:\n19\n16\n10\n",
                             "Lineage:\n16\n19\n16\n10\n",
                           };

      for(size_t i = 0; i < num_syms; i++){
        std::stringstream result;
        sym_sys->PrintLineage(syms[i]->GetTaxon().Cast<taxon_t::sym_taxon_t>(), result);
        REQUIRE(result.str() == lineages[i]);
      }
      syms[0].Delete();
      syms[1].Delete();
    }

    THEN("Their birth and destruction dates are tracked"){
      //all curr syms should have orig times of 0
      for(size_t i = 0; i < num_syms; i++){
        REQUIRE(syms[i]->GetTaxon()->GetOriginationTime() == 0);
      }
      world.Update();

      //after update, times should now be 1
      emp::Ptr<taxon_t::base_taxon_t> dest_tax = syms[0]->GetTaxon();
      syms[0].Delete();
      REQUIRE(dest_tax->GetDestructionTime() == 1);

      //another update, times 2
      world.Update();
      dest_tax = syms[1]->GetTaxon();
      syms[1].Delete();
      REQUIRE(dest_tax->GetDestructionTime() == 2);
    }

    syms[2].Delete();
    syms[3].Delete();
  }
}

TEST_CASE("Interaction Tracking Phylogeny", "[default]") {
  emp::Random random(17);
  SymConfigBase config;
  config.PHYLOGENY(1);
  config.NUM_PHYLO_BINS(20);
  config.TRACK_PHYLOGENY_INTERACTIONS(1);
  int int_val = -1;
  SymWorld world(random, &config);
  size_t grid_side = 4;
  config.WORLD_WIDTH(grid_side);
  config.WORLD_HEIGHT(grid_side);

  emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::HostTaxonData>> host_sys = world.GetHostSys();
  emp::Ptr<emp::Systematics<Organism, taxon_t::info_t, datastruct::SymbiontTaxonData>> sym_sys = world.GetSymSys();


  WHEN("A symbiont is injected into a host (at the beginning of runs)") {
    emp::Ptr<Organism> symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::Ptr<Organism> host = emp::NewPtr<Host>(&random, &world, &config, int_val);

    world.InjectHost(host);
    world.Resize(grid_side, grid_side);
    world.InjectSymbiont(symbiont);
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(host->HasSym());

    size_t expected_host_taxon_info = 0;
    size_t taxon_info = host->GetTaxon()->GetInfo();

    // Check normal phylogeny function
    REQUIRE(world.GetNumOrgs() == 1);
    REQUIRE(host_sys->GetNumActive() == 1);
    REQUIRE(sym_sys->GetNumActive() == 1);
    REQUIRE(expected_host_taxon_info == taxon_info);
    REQUIRE(host->GetSymbionts().size() == 1);

    THEN("Symbiont-host interaction is tracked") {
      // Check that host and symbiont are not marked as interacting
      datastruct::HostTaxonData* data = static_cast<datastruct::HostTaxonData*>(&host->GetTaxon()->GetData());
      REQUIRE(emp::Has(data->associated_syms, symbiont->GetTaxon()->GetID()));
      REQUIRE(data->associated_syms[symbiont->GetTaxon()->GetID()] == 1);
    }
  }


  WHEN("A symbiont is born into a host (symdobirth or dobirth--HT or VT)") {
    config.VERTICAL_TRANSMISSION(1);
    config.SYM_VERT_TRANS_RES(0);
    world.Resize(grid_side, grid_side);

    size_t pos = 2;

    // set up parents
    emp::Ptr<Organism> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::Ptr<Organism> parent_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddSymToSystematic(parent_symbiont);
    world.AddOrgAt(parent_host, pos);

    // set up children
    emp::Ptr<Organism> child_host = parent_host->Reproduce();
    parent_symbiont->VerticalTransmission(child_host);
    emp::Ptr<Organism> child_symbiont = child_host->GetSymbionts()[0];

    // call DoBirth
    emp::WorldPosition child_pos = world.DoBirth(child_host, pos);

    THEN("Symbiont-host interaction is tracked") {
      // Check that host and symbiont are marked as interacting
      datastruct::HostTaxonData* data = static_cast<datastruct::HostTaxonData*>(&child_host->GetTaxon()->GetData());
      REQUIRE(emp::Has(data->associated_syms, child_symbiont->GetTaxon()->GetID()));
      REQUIRE(data->associated_syms[child_symbiont->GetTaxon()->GetID()] == 1);
    }


    // do another generation
    emp::Ptr<Organism> grandchild_host = child_host->Reproduce();
    child_symbiont->VerticalTransmission(grandchild_host);
    emp::Ptr<Organism> grandchild_symbiont = grandchild_host->GetSymbionts()[0];

    world.DoBirth(grandchild_host, child_pos);

    THEN("Symbiont-host interactions are counted") {
      // Check that host and symbiont are marked as interacting
      datastruct::HostTaxonData* data = static_cast<datastruct::HostTaxonData*>(&grandchild_host->GetTaxon()->GetData());
      REQUIRE(emp::Has(data->associated_syms, grandchild_symbiont->GetTaxon()->GetID()));
      REQUIRE(data->associated_syms[grandchild_symbiont->GetTaxon()->GetID()] == 2);
    }
    parent_symbiont.Delete();
  }
}

TEST_CASE("Tag-based Phylogeny", "[default]") {
  emp::Random random(5);
  SymConfigBase config;
  config.MUTATION_SIZE(0.1);
  config.MUTATION_RATE(1);
  config.PHYLOGENY(1);
  config.TAG_MATCHING(1);
  config.PHYLOGENY_TAXON_TYPE("tag");
  config.VERTICAL_TRANSMISSION(0);
  config.STARTING_TAGS_ONE_PROB(0);

  int int_val = 0;
  emp::WorldPosition fake_pos = emp::WorldPosition(0, 0);
  using taxon_info_t = double;
  using s_taxon_t = emp::Taxon<taxon_info_t, datastruct::SymbiontTaxonData>;
  using h_taxon_t = emp::Taxon<taxon_info_t, datastruct::HostTaxonData>;

  SymWorld world(random, &config);

  emp::HammingMetric<TAG_LENGTH> tag_metric = emp::HammingMetric<TAG_LENGTH>();

  emp::Ptr<emp::Systematics<Organism, taxon_info_t, datastruct::SymbiontTaxonData>> sym_sys = world.GetSymSys();
  emp::Ptr<emp::Systematics<Organism, taxon_info_t, datastruct::HostTaxonData>> host_sys = world.GetHostSys();
  std::unordered_set< emp::Ptr<s_taxon_t>, emp::Ptr<s_taxon_t>::hash_t>* sym_active = sym_sys->GetActivePtr();
  std::unordered_set< emp::Ptr<h_taxon_t>, emp::Ptr<h_taxon_t>::hash_t>* host_active = host_sys->GetActivePtr();

  WHEN("A host and symbiont are added to the world") {
    emp::Ptr<Organism> parent_host = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::Ptr<Organism> parent_symbiont = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::BitSet<TAG_LENGTH> zeros_tag = emp::BitSet<TAG_LENGTH>(random, config.STARTING_TAGS_ONE_PROB());
    parent_host->SetTag(zeros_tag);
    parent_symbiont->SetTag(zeros_tag);

    world.InjectHost(parent_host);
    world.InjectSymbiont(parent_symbiont);
    REQUIRE(parent_host->HasSym());

    // sym int val is only added to taxon data node on Process (to avoid weighing by repro #)
    // so update here to ensure it gets tracked
    world.Update();

    THEN("The host is added to the systematic"){
      REQUIRE(host_active->size() == 1);
      REQUIRE((*host_active->begin())->GetID() == parent_host->GetTaxon()->GetID());
      REQUIRE(parent_host->GetTaxon()->GetData().GetIntVal() == int_val);
    }
    THEN("The symbiont is added to the systematic") {
      REQUIRE(sym_active->size() == 1);
      REQUIRE(sym_active->contains(parent_symbiont->GetTaxon().Cast<emp::Taxon<taxon_info_t, datastruct::SymbiontTaxonData>>()));
      REQUIRE(parent_symbiont->GetTaxon()->GetData().GetIntVal() == int_val);
    }

    WHEN("The symbiont reproduces") {
      WHEN("The child mutates its tag") {
        config.TAG_MUTATION_SIZE(0.5);
        emp::Ptr<Organism> child_symbiont = parent_symbiont->Reproduce();
        REQUIRE(tag_metric.calculate(child_symbiont->GetTag(), parent_symbiont->GetTag()) > 0);
        child_symbiont->Process(fake_pos);

        THEN("The child is placed into a different taxon than its parent") {
          REQUIRE(child_symbiont->GetTaxon()->GetID() != parent_symbiont->GetTaxon()->GetID());
        }
        THEN("The child's taxon is the child of its parent's taxon") {
          REQUIRE(parent_symbiont->GetTaxon()->GetOffspring().contains(child_symbiont->GetTaxon()));
        }
        THEN("Their taxa track their interaction values") {
          REQUIRE(parent_symbiont->GetTaxon()->GetData().GetIntVal() == parent_symbiont->GetIntVal());
          REQUIRE(child_symbiont->GetTaxon()->GetData().GetIntVal() == child_symbiont->GetIntVal());
        }
        child_symbiont.Delete();
      }
      WHEN("The child does not mutate its tag") {
        config.TAG_MUTATION_SIZE(0);
        emp::Ptr<Organism> child_symbiont = parent_symbiont->Reproduce();
        REQUIRE(child_symbiont->GetIntVal() != parent_symbiont->GetIntVal());
        REQUIRE(tag_metric.calculate(child_symbiont->GetTag(), parent_symbiont->GetTag()) == 0);

        WHEN("The child does not call Process"){
          THEN("The child is placed into the same taxon as its parent") {
            REQUIRE(child_symbiont->GetTaxon()->GetID() == parent_symbiont->GetTaxon()->GetID());
          }
          THEN("Their taxon does not incorporate the child's interaction value") {
            REQUIRE(child_symbiont->GetTaxon()->GetData().GetIntVal() == parent_symbiont->GetIntVal());
          }
        }
        WHEN("The child calls Process") {
          child_symbiont->Process(fake_pos);

          THEN("The child is placed into the same taxon as its parent") {
            REQUIRE(child_symbiont->GetTaxon()->GetID() == parent_symbiont->GetTaxon()->GetID());
          }
          THEN("Their taxon incorporates the child's interaction value") {
            REQUIRE(child_symbiont->GetTaxon()->GetData().GetIntVal() == ((child_symbiont->GetIntVal() + parent_symbiont->GetIntVal()) / 2.0));
          }
          THEN("The taxon's mean interaction value is weighed by the process calls of each member") {
            int child_count = 1;
            int parent_count = 1;
            int mean_int_val = -2;
            int expected_mean_int_val = 2;

            child_symbiont->Process(fake_pos);
            child_count++;
            mean_int_val = child_symbiont->GetTaxon()->GetData().GetIntVal();
            expected_mean_int_val = (child_symbiont->GetIntVal() * child_count + parent_symbiont->GetIntVal() * parent_count) / (child_count + parent_count);
            REQUIRE(mean_int_val == expected_mean_int_val);

            child_symbiont->Process(fake_pos);
            child_count++;
            mean_int_val = child_symbiont->GetTaxon()->GetData().GetIntVal();
            expected_mean_int_val = (child_symbiont->GetIntVal() * child_count + parent_symbiont->GetIntVal() * parent_count) / (child_count + parent_count);
            REQUIRE(mean_int_val == expected_mean_int_val);

            world.Update();
            parent_count++;
            mean_int_val = child_symbiont->GetTaxon()->GetData().GetIntVal();
            expected_mean_int_val = (child_symbiont->GetIntVal() * child_count + parent_symbiont->GetIntVal() * parent_count) / (child_count + parent_count);
            REQUIRE(mean_int_val == expected_mean_int_val);
          }
        }

        child_symbiont.Delete();
      }
    }

    WHEN("The host reproduces") {
      size_t child_pos = 1;
      size_t parent_pos = 0;

      WHEN("The child mutates its tag") {
        config.TAG_MUTATION_SIZE(0.5);
        emp::Ptr<Organism> child_host = parent_host->Reproduce();
        world.AddOrgAt(child_host, child_pos, parent_pos);
        REQUIRE(world.GetNumOrgs() == 2);
        REQUIRE(tag_metric.calculate(child_host->GetTag(), parent_host->GetTag()) > 0);

        child_host->Process(fake_pos);

        THEN("The host is placed into a different taxon than its parent") {
          REQUIRE(child_host->GetTaxon()->GetID() != parent_host->GetTaxon()->GetID());
        }
        THEN("The child's taxon is the child of the host parent's taxon") {
          REQUIRE(parent_host->GetTaxon()->GetOffspring().contains(child_host->GetTaxon()));
        }
        THEN("The child's new taxon tracks its interaction value") {
          REQUIRE(child_host->GetTaxon()->GetData().GetIntVal() == child_host->GetIntVal());
        }
      }

      WHEN("The child does not mutate its tag") {
        config.TAG_MUTATION_SIZE(0);
        emp::Ptr<Organism> child_host = parent_host->Reproduce();
        world.AddOrgAt(child_host, child_pos, parent_pos);
        REQUIRE(world.GetNumOrgs() == 2);
        REQUIRE(tag_metric.calculate(child_host->GetTag(), parent_host->GetTag()) == 0);
        REQUIRE(parent_host->GetIntVal() != child_host->GetIntVal());

        WHEN("The child does not call Process") {
          THEN("The child is placed into the same taxon as its parent") {
            REQUIRE(child_host->GetTaxon()->GetID() == parent_host->GetTaxon()->GetID());
          }
          THEN("Their taxon does not incorporate the child's interaction value") {
            REQUIRE(child_host->GetTaxon()->GetData().GetIntVal() == parent_host->GetIntVal());
          }
        }
        WHEN("The child calls Process") {
          child_host->Process(fake_pos);

          THEN("The child is placed into the same taxon as its parent") {
            REQUIRE(child_host->GetTaxon()->GetID() == parent_host->GetTaxon()->GetID());
          }
          THEN("Their taxon incorporates the child's interaction value") {
            REQUIRE(child_host->GetTaxon()->GetData().GetIntVal() == ((child_host->GetIntVal() + parent_host->GetIntVal()) / 2.0));
          }
          THEN("The taxon's mean interaction value is weighed by the process calls of each member") {
            int child_count = 1;
            int parent_count = 1;
            int mean_int_val = -2;
            int expected_mean_int_val = 2;

            child_host->Process(fake_pos);
            child_count++;
            mean_int_val = child_host->GetTaxon()->GetData().GetIntVal();
            expected_mean_int_val = (child_host->GetIntVal() * child_count + parent_host->GetIntVal() * parent_count) / (child_count + parent_count);
            REQUIRE(mean_int_val == expected_mean_int_val);

            child_host->Process(fake_pos);
            child_count++;
            mean_int_val = child_host->GetTaxon()->GetData().GetIntVal();
            expected_mean_int_val = (child_host->GetIntVal() * child_count + parent_host->GetIntVal() * parent_count) / (child_count + parent_count);
            REQUIRE(mean_int_val == expected_mean_int_val);

            world.Update();
            parent_count++;
            mean_int_val = child_host->GetTaxon()->GetData().GetIntVal();
            expected_mean_int_val = (child_host->GetIntVal() * child_count + parent_host->GetIntVal() * parent_count) / (child_count + parent_count);
            REQUIRE(mean_int_val == expected_mean_int_val);
          }
        }
      }
    }
  }
}

TEST_CASE("Individual-level phylogenies", "[default]") {
  emp::Random random(17);
  SymConfigBase config;
  int repro_points = 100;
  config.SYM_HORIZ_TRANS_RES(repro_points);

  config.MUTATION_RATE(0); // no phenotypic difference

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE("individual");

  SymWorld world(random, &config);
  int int_val = 0;
  int world_size = 10;
  world.Resize(world_size);

  WHEN("Free living symbionts reproduce and die") {
    config.FREE_LIVING_SYMS(1);

    emp::WorldPosition symbiont_1_pos = emp::WorldPosition(0, 0);
    emp::Ptr<Organism> symbiont_1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    world.AddSymToSystematic(symbiont_1);
    world.AddOrgAt(symbiont_1, symbiont_1_pos);
    world.Update(); // update 1

    emp::Ptr<Organism> symbiont_2 = symbiont_1->Reproduce(); // symbionts are added to systematic on Reproduce()
    emp::WorldPosition symbiont_2_pos = world.SymDoBirth(symbiont_2, symbiont_1_pos);

    REQUIRE(world.GetNumOrgs() == 2);
    REQUIRE(symbiont_2_pos.GetPopID() != symbiont_1_pos.GetPopID());

    emp::Ptr< taxon_t::base_taxon_t> symbiont_1_taxon = symbiont_1->GetTaxon();
    emp::Ptr< taxon_t::base_taxon_t> symbiont_2_taxon = symbiont_2->GetTaxon();

    THEN("Symbiont offspring are placed into a new taxon") {
      REQUIRE(symbiont_2->GetIntVal() == symbiont_1->GetIntVal());
      REQUIRE(symbiont_1_taxon->GetID() != symbiont_2_taxon->GetID());
    }

    THEN("Symbiont taxon origination times are tracked") {
      REQUIRE(symbiont_1_taxon->GetOriginationTime() == 0);
      REQUIRE(symbiont_2_taxon->GetOriginationTime() == 1);
      REQUIRE(symbiont_1_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
      REQUIRE(symbiont_2_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
    }

    THEN("Symbiont taxon destruction times are tracked") {
      world.Update(); // update 2
      // birth and overwriting death happen during update 2
      emp::Ptr<Organism> symbiont_3 = symbiont_2->Reproduce(); // symbionts are added to systematic on Reproduce()
      world.AddOrgAt(symbiont_3, symbiont_1_pos, symbiont_2_pos);
      world.CleanupGraveyard(); // call Delete() for symbiont 1, which was overwritten in the preceding AddOrgAt call
                                // destruction time is calculated based on the destructor call for the last org in the taxon

      emp::Ptr< taxon_t::base_taxon_t> symbiont_3_taxon = symbiont_3->GetTaxon();

      world.Update(); // update 3
      world.Update(); // update 4
      world.Update(); // update 5

      symbiont_2->SetDead(); // symbiont dies during update 6
      world.Update(); // update 6

      REQUIRE(world.GetNumOrgs() == 1);
      REQUIRE(symbiont_3_taxon->GetOriginationTime() == 2);
      REQUIRE(symbiont_1_taxon->GetDestructionTime() == 2);
      REQUIRE(symbiont_2_taxon->GetDestructionTime() == 6);
      REQUIRE(symbiont_3_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
    }
  }

  WHEN("Hosted symbionts reproduce and die") {
    config.FREE_LIVING_SYMS(0);
    config.VERTICAL_TRANSMISSION(1);

    // test:
    // birth through VT and HT
    // death through host death, symbiont death, and ousting

    emp::Ptr<Organism> host_1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::WorldPosition host_1_pos = emp::WorldPosition(0, 0);
    world.AddOrgAt(host_1, host_1_pos);

    emp::Ptr<Organism> symbiont_1 = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    world.AddSymToSystematic(symbiont_1);
    emp::Ptr< taxon_t::base_taxon_t> symbiont_1_taxon = symbiont_1->GetTaxon();
    host_1->AddSymbiont(symbiont_1);

    THEN("Origination times are tracked") {
      REQUIRE(symbiont_1_taxon->GetOriginationTime() == 0);
      REQUIRE(symbiont_1_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
    }

    world.Update(); // update 1
    emp::Ptr<Organism> host_2 = host_1->Reproduce();
    emp::WorldPosition host_2_pos = world.DoBirth(host_2, host_1_pos);
    config.MUTATION_RATE(1);
    symbiont_1->VerticalTransmission(host_2);
    config.MUTATION_RATE(0);
    emp::Ptr<Organism> symbiont_2 = host_2->GetSymbionts().at(0);
    emp::Ptr< taxon_t::base_taxon_t> symbiont_2_taxon = symbiont_2->GetTaxon();

    WHEN("A symbiont is vertically transmitted") {
      THEN("It is placed into a new taxon") {
        REQUIRE(symbiont_2_taxon->GetData().GetIntVal() != symbiont_1_taxon->GetData().GetIntVal());
        REQUIRE(symbiont_2_taxon->GetInfo() != symbiont_1_taxon->GetInfo());
        REQUIRE(symbiont_2_taxon->GetOriginationTime() == 1);
      }
      THEN("Taxon origination/destruction times are tracked") {
        REQUIRE(symbiont_2_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
        REQUIRE(symbiont_1_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
      }
    }

    random.ResetSeed(10);
    world.Update(); // update 2
    emp::Ptr<Organism> host_3 = host_2->Reproduce();
    world.DoBirth(host_3, host_2_pos);
    symbiont_1->SetPoints(repro_points + 10);
    symbiont_1->IndependentReproduction(emp::WorldPosition(1, host_1_pos.GetIndex()));
    emp::Ptr<Organism> symbiont_3 = host_3->GetSymbionts().at(0);
    emp::Ptr< taxon_t::base_taxon_t> symbiont_3_taxon = symbiont_3->GetTaxon();

    WHEN("A symbiont is horizontally transmitted") {
      REQUIRE(world.GetNumOrgs() == 3);
      REQUIRE(host_3->HasSym());
      THEN("It is placed into a new taxon") {
        REQUIRE(symbiont_3_taxon->GetInfo() != symbiont_1_taxon->GetInfo());
        REQUIRE(symbiont_3_taxon->GetInfo() != symbiont_2_taxon->GetInfo());

      }
      THEN("Taxon origination/destruction times are tracked") {
        REQUIRE(symbiont_2_taxon->GetOriginationTime() == 1);
        REQUIRE(symbiont_3_taxon->GetOriginationTime() == 2);

        REQUIRE(symbiont_1_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
        REQUIRE(symbiont_2_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
        REQUIRE(symbiont_3_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
      }
    }

    world.Update(); // update 3
    symbiont_1->SetDead(); // dies "during" update 4
    world.Update(); // update 4
    WHEN("A symbiont dies within an undying host") {
      REQUIRE(world.GetNumOrgs() == 3);
      REQUIRE(!host_1->HasSym());
      THEN("The symbiont's taxon is marked as destroyed") {
        REQUIRE(symbiont_1_taxon->GetOriginationTime() == 0);
        REQUIRE(symbiont_1_taxon->GetDestructionTime() == 4);
      }
    }

    config.OUSTING(1);
    emp::Ptr<Organism> symbiont_4 = symbiont_3->Reproduce(); // reproduce happens "during" update 4
    host_3->AddSymbiont(symbiont_4); // cleanup of graveyard happens after update increment in Update(), so dest. time is 5
    emp::Ptr< taxon_t::base_taxon_t> symbiont_4_taxon = symbiont_4->GetTaxon();
    world.Update(); // update 5
    WHEN("A symbiont is ousted") {
      REQUIRE(host_3->GetSymbionts().size() == 1);
      REQUIRE(host_3->GetSymbionts().at(0) == symbiont_4);
      THEN("The symbiont's taxon is marked as destroyed") {
        REQUIRE(symbiont_3_taxon->GetOriginationTime() == 2);
        REQUIRE(symbiont_3_taxon->GetDestructionTime() == 5);

        REQUIRE(symbiont_4_taxon->GetOriginationTime() == 4);
        REQUIRE(symbiont_4_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
      }
    }

    world.Update(); // update 6
    emp::Ptr<Organism> host_4 = host_2->Reproduce();
    world.DoBirth(host_4, host_2_pos);
    symbiont_2->VerticalTransmission(host_4); // vertical transmission happens "during" update 6
    emp::Ptr<Organism> symbiont_5 = host_4->GetSymbionts().at(0);
    emp::Ptr< taxon_t::base_taxon_t> symbiont_5_taxon = symbiont_5->GetTaxon();

    host_2->SetDead(); // dies "during" update 7
    world.Update(); // update 7

    WHEN("A host with symbionts dies") {
      REQUIRE(world.GetNumOrgs() == 3);
      THEN("Its symbionts' taxa are marked destroyed") {
        REQUIRE(symbiont_2_taxon->GetOriginationTime() == 1);
        REQUIRE(symbiont_2_taxon->GetDestructionTime() == 7);
        REQUIRE(symbiont_5_taxon->GetOriginationTime() == 6);
        REQUIRE(symbiont_5_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
      }

    }

    REQUIRE(world.GetSymSys()->GetNumTaxa() == 5);
    REQUIRE(world.GetSymSys()->GetActive().size() == 2);
    REQUIRE(world.GetSymSys()->GetAncestors().size() == 3);

    // current lineages: sym 1 -> sym 2 -> sym 5
    // sym 1 -> sym 3 -> sym 4
    symbiont_5->SetDead();
    world.Update(); // update 8
    WHEN("A symbiont tip dies") {
      THEN("The phylogeny is pruned") {
        // sym 2 and sym 5 taxa get removed
        REQUIRE(world.GetSymSys()->GetNumTaxa() == 3);
        REQUIRE(world.GetSymSys()->GetActive().size() == 1);
        REQUIRE(world.GetSymSys()->GetAncestors().size() == 2);

        REQUIRE((*world.GetSymSys()->GetActive().begin())->GetID() == symbiont_4_taxon->GetID());
        REQUIRE((*world.GetSymSys()->GetAncestors().begin())->GetID() == symbiont_3_taxon->GetID());
        REQUIRE((*(++world.GetSymSys()->GetAncestors().begin()))->GetID() == symbiont_1_taxon->GetID());
      }
    }
  }

  WHEN("Hosts reproduce and die") {
    emp::WorldPosition host_1_pos = emp::WorldPosition(0, 0);
    emp::Ptr<Organism> host_1 = emp::NewPtr<Host>(&random, &world, &config, int_val);
    world.AddOrgAt(host_1, host_1_pos);
    world.Update(); // update 1

    emp::Ptr<Organism> host_2 = host_1->Reproduce(  );
    emp::WorldPosition host_2_pos = world.DoBirth(host_2, host_1_pos);

    emp::Ptr< taxon_t::base_taxon_t> host_1_taxon = host_1->GetTaxon();
    emp::Ptr< taxon_t::base_taxon_t> host_2_taxon = host_2->GetTaxon();

    THEN("Host offspring are placed into a new taxon") {
      REQUIRE(host_2->GetIntVal() == host_1->GetIntVal());
      REQUIRE(host_1_taxon->GetID() != host_2_taxon->GetID());
    }
    THEN("Host taxon origination times are tracked") {
      REQUIRE(host_1_taxon->GetOriginationTime() == 0);
      REQUIRE(host_2_taxon->GetOriginationTime() == 1);
      REQUIRE(host_1_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
      REQUIRE(host_2_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
    }

    world.Update(); // update 2
    // birth and overwriting death happen during update 2
    emp::Ptr<Organism> host_3 = host_2->Reproduce();
    world.AddOrgAt(host_3, host_1_pos, host_2_pos);
    emp::Ptr< taxon_t::base_taxon_t> host_3_taxon = host_3->GetTaxon();

    world.Update(); // update 3
    world.Update(); // update 4
    world.Update(); // update 5

    host_2->SetDead(); // host dies during update 6
    world.Update(); // update 6

    THEN("Host taxon destruction times are tracked") {
      REQUIRE(world.GetNumOrgs() == 1);
      REQUIRE(host_3_taxon->GetOriginationTime() == 2);
      REQUIRE(host_1_taxon->GetDestructionTime() == 2);
      REQUIRE(host_2_taxon->GetDestructionTime() == 6);
      REQUIRE(host_3_taxon->GetDestructionTime() == std::numeric_limits<double>::infinity());
    }



    REQUIRE(world.GetHostSys()->GetNumTaxa() == 3);
    REQUIRE(world.GetHostSys()->GetActive().size() == 1);
    REQUIRE(world.GetHostSys()->GetAncestors().size() == 2);
    // current lineage: host 1 -> host 2 -> host 3
    host_3->SetDead();
    world.Update();

    WHEN("Host tips die") {
      THEN("Pruning occurs") {
        REQUIRE(world.GetHostSys()->GetNumTaxa() == 0);
        REQUIRE(world.GetHostSys()->GetActive().size() == 0);
        REQUIRE(world.GetHostSys()->GetAncestors().size() == 0);
      }
    }
  }
}

TEST_CASE("Host switch counter", "[default]") {
  emp::Random random(17);
  SymConfigBase config;
  int repro_points = 100;
  config.SYM_HORIZ_TRANS_RES(repro_points);

  config.MUTATION_RATE(0); // no phenotypic difference
  config.VERTICAL_TRANSMISSION(0);

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE("individual");

  SymWorld world(random, &config);
  int int_val = 0;
  int world_size = 4;
  world.Resize(world_size);

  WHEN("A symbiont vertically transmits") {
    config.VERTICAL_TRANSMISSION(1);

    emp::Ptr<Organism> host_parent = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    world.AddSymToSystematic(symbiont_parent);
    host_parent->AddSymbiont(symbiont_parent);

    emp::Ptr<Organism> host_offspring = host_parent->Reproduce();
    symbiont_parent->VerticalTransmission(host_offspring);
    REQUIRE(host_offspring->HasSym());
    emp::Ptr<Organism> symbiont_offspring = host_offspring->GetSymbionts().at(0);

    emp::Ptr< taxon_t::sym_taxon_t> symbiont_parent_taxon = symbiont_parent->GetTaxon().Cast<taxon_t::sym_taxon_t>();
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon().Cast<taxon_t::sym_taxon_t>();
    THEN("Its host switch counter does not increment") {
      REQUIRE(symbiont_parent_taxon->GetData().GetHostSwitch() == 0);
      REQUIRE(symbiont_offspring_taxon->GetData().GetHostSwitch() == 0);
      REQUIRE(symbiont_parent_taxon->GetID() != symbiont_offspring_taxon->GetID());
    }

    host_parent.Delete();
    host_offspring.Delete();
  }

  WHEN("A symbiont horizontally transmits into a host who is descended from the symbiont parent's partner") {
    size_t host_switch_count = 43;
    emp::WorldPosition host_of_parent_pos = emp::WorldPosition(0, 0);

    emp::Ptr<Organism> host_of_parent = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::Ptr<Organism> host_of_offspring = host_of_parent->Reproduce();

    world.AddSymToSystematic(symbiont_parent);
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_parent_taxon = symbiont_parent->GetTaxon().Cast<taxon_t::sym_taxon_t>();
    symbiont_parent_taxon->GetData().SetHostSwitch(host_switch_count);

    symbiont_parent->SetPoints(repro_points + 10);
    host_of_parent->AddSymbiont(symbiont_parent);

    world.AddOrgAt(host_of_parent, host_of_parent_pos);
    world.AddOrgAt(host_of_offspring, 1, host_of_parent_pos);
    REQUIRE(host_of_parent->GetTaxon()->GetID() == host_of_offspring->GetTaxon()->GetParent()->GetID());

    symbiont_parent->IndependentReproduction(emp::WorldPosition(1, host_of_parent_pos.GetIndex()));
    REQUIRE(host_of_offspring->HasSym());

    emp::Ptr<Organism> symbiont_offspring = host_of_offspring->GetSymbionts().at(0);
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon().Cast<taxon_t::sym_taxon_t>();

    THEN("Its host switch counter does not") {
      REQUIRE(symbiont_parent_taxon->GetData().GetHostSwitch() == host_switch_count);
      REQUIRE(symbiont_offspring_taxon->GetData().GetHostSwitch() == host_switch_count);
      REQUIRE(symbiont_parent_taxon->GetID() != symbiont_offspring_taxon->GetID());
    }
  }

  WHEN("A symbiont horizontally transmits into a host who is an ancestor of the symbiont parent's partner") {
    size_t host_switch_count = 19;
    emp::WorldPosition host_of_offspring_pos = emp::WorldPosition(2, 0);
    emp::WorldPosition host_of_parent_pos = emp::WorldPosition(0, 0);

    emp::Ptr<Organism> host_of_offspring = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::Ptr<Organism> host_of_parent = host_of_offspring->Reproduce();

    world.AddSymToSystematic(symbiont_parent);
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_parent_taxon = symbiont_parent->GetTaxon().Cast<taxon_t::sym_taxon_t>();
    symbiont_parent_taxon->GetData().SetHostSwitch(host_switch_count);

    symbiont_parent->SetPoints(repro_points + 10);
    host_of_parent->AddSymbiont(symbiont_parent);

    world.AddOrgAt(host_of_offspring, host_of_offspring_pos);
    world.AddOrgAt(host_of_parent, host_of_parent_pos, host_of_offspring_pos);
    REQUIRE(host_of_parent->GetTaxon()->GetParent()->GetID() == host_of_offspring->GetTaxon()->GetID());

    symbiont_parent->IndependentReproduction(emp::WorldPosition(1, host_of_parent_pos.GetIndex()));
    REQUIRE(host_of_offspring->HasSym());

    emp::Ptr<Organism> symbiont_offspring = host_of_offspring->GetSymbionts().at(0);
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon().Cast<taxon_t::sym_taxon_t>();

    THEN("Its host switch counter does not increment") {
      REQUIRE(symbiont_parent_taxon->GetData().GetHostSwitch() == host_switch_count);
      REQUIRE(symbiont_offspring_taxon->GetData().GetHostSwitch() == host_switch_count);
      REQUIRE(symbiont_parent_taxon->GetID() != symbiont_offspring_taxon->GetID());
    }
  }

  WHEN("A symbiont horizontally transmits into a host who is an unrelated to the symbiont parent's partner") {
    size_t host_switch_count = 8;
    emp::WorldPosition symbiont_parent_pos = emp::WorldPosition(1, 0);

    emp::Ptr<Organism> host_of_parent = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::Ptr<Organism> symbiont_parent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    emp::Ptr<Organism> host_of_offspring = emp::NewPtr<Host>(&random, &world, &config, int_val);

    world.AddSymToSystematic(symbiont_parent);
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_parent_taxon = symbiont_parent->GetTaxon().Cast<taxon_t::sym_taxon_t>();
    symbiont_parent_taxon->GetData().SetHostSwitch(host_switch_count);

    symbiont_parent->SetPoints(repro_points + 10);
    host_of_parent->AddSymbiont(symbiont_parent);

    world.AddOrgAt(host_of_parent, symbiont_parent_pos.GetPopID());
    world.AddOrgAt(host_of_offspring, 1);

    symbiont_parent->IndependentReproduction(symbiont_parent_pos);
    REQUIRE(host_of_offspring->HasSym());

    emp::Ptr<Organism> symbiont_offspring = host_of_offspring->GetSymbionts().at(0);
    emp::Ptr< taxon_t::sym_taxon_t> symbiont_offspring_taxon = symbiont_offspring->GetTaxon().Cast<taxon_t::sym_taxon_t>();
    THEN("Its host switch increments") {
      REQUIRE(symbiont_parent_taxon->GetData().GetHostSwitch() == host_switch_count);
      REQUIRE(symbiont_offspring_taxon->GetData().GetHostSwitch() == host_switch_count + 1);
      REQUIRE(symbiont_parent_taxon->GetID() != symbiont_offspring_taxon->GetID());
    }
  }
}

TEST_CASE("Unpruned phylogenies", "[default]") {

  emp::Random random(17);
  SymConfigBase config;
  int repro_points = 100;
  config.SYM_HORIZ_TRANS_RES(repro_points);

  config.MUTATION_RATE(0); // no phenotypic difference

  config.PHYLOGENY(1);
  config.PHYLOGENY_TAXON_TYPE("individual");
  config.STORE_EXTINCT(1);

  SymWorld world(random, &config);
  int int_val = 0;
  int world_size = 5;
  world.Resize(world_size);

  // set up the lineages
  emp::Ptr<Organism> host_grandparent = emp::NewPtr<Host>(&random, &world, &config, int_val);
  emp::Ptr<Organism> host_parent = host_grandparent->Reproduce();
  emp::Ptr<Organism> host = host_parent->Reproduce();

  emp::WorldPosition grandparent_pos = emp::WorldPosition(0, 0);
  world.AddOrgAt(host_grandparent, grandparent_pos);
  emp::WorldPosition parent_pos = world.DoBirth(host_parent, grandparent_pos);
  emp::WorldPosition pos = world.DoBirth(host, parent_pos);

  REQUIRE(world.GetNumOrgs() == 3);

  emp::Ptr<Organism> sym_grandparent = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
  host_grandparent->AddSymbiont(sym_grandparent);
  world.AddSymToSystematic(sym_grandparent);
  REQUIRE(host_grandparent->HasSym());

  emp::Ptr<Organism> sym_parent = sym_grandparent->Reproduce();
  host_parent->AddSymbiont(sym_parent);
  REQUIRE(host_parent->HasSym());
  REQUIRE(host_parent->GetSymbionts().at(0) == sym_parent);

  emp::Ptr<Organism> sym = sym_parent->Reproduce();
  host->AddSymbiont(sym);
  REQUIRE(host->HasSym());

  // all organisms should be in their own taxon; all taxa should be active
  REQUIRE(world.GetSymSys()->GetNumTaxa() == 3);
  REQUIRE(world.GetHostSys()->GetNumTaxa() == 3);
  REQUIRE(world.GetSymSys()->GetNumActive() == 3);
  REQUIRE(world.GetHostSys()->GetNumActive() == 3);

  // collect taxon pointers before we kill organisms
  emp::Ptr<taxon_t::base_taxon_t> sym_grandparent_taxon = sym_grandparent->GetTaxon();
  emp::Ptr<taxon_t::base_taxon_t> sym_parent_taxon = sym_parent->GetTaxon();
  emp::Ptr<taxon_t::base_taxon_t> sym_taxon = sym->GetTaxon();

  emp::Ptr<taxon_t::base_taxon_t> host_grandparent_taxon = host_grandparent->GetTaxon();
  emp::Ptr<taxon_t::base_taxon_t> host_parent_taxon = host_parent->GetTaxon();
  emp::Ptr<taxon_t::base_taxon_t> host_taxon = host->GetTaxon();

  // leave only the tip alive for now
  world.DoDeath(grandparent_pos);
  world.DoDeath(parent_pos);
  REQUIRE(world.GetNumOrgs() == 1);

  WHEN("A symbiont lineage goes extinct") {
    WHEN("The tip survived for at least one update") {
      world.Update();

      REQUIRE(world.GetSymSys()->GetNumActive() == 1);
      REQUIRE(world.GetSymSys()->GetNumAncestors() == 2);
      REQUIRE(world.GetSymSys()->GetNumOutside() == 0);

      world.DoDeath(pos);
      world.Update();

      THEN("The tip and its ancestors is stored in the set of outside taxa") {
        REQUIRE(world.GetSymSys()->GetNumOutside() == 3);
        REQUIRE(world.GetSymSys()->outside_taxa.contains(sym_grandparent_taxon.Cast<taxon_t::sym_taxon_t>()));
        REQUIRE(world.GetSymSys()->outside_taxa.contains(sym_parent_taxon.Cast<taxon_t::sym_taxon_t>()));
        REQUIRE(world.GetSymSys()->outside_taxa.contains(sym_taxon.Cast<taxon_t::sym_taxon_t>()));
      }
    }
    WHEN("The tip survived for 0 updates") {
      REQUIRE(world.GetSymSys()->GetNumActive() == 1);
      REQUIRE(world.GetSymSys()->GetNumAncestors() == 2);
      REQUIRE(world.GetSymSys()->GetNumOutside() == 0);

      world.DoDeath(pos);
      world.Update();

      THEN("The tip is not stored in the set of outside taxa; the tip's ancestors are stored in the set of outside taxa"){
        REQUIRE(world.GetSymSys()->GetNumOutside() == 2);
        REQUIRE(world.GetSymSys()->outside_taxa.contains(sym_grandparent_taxon.Cast<taxon_t::sym_taxon_t>()));
        REQUIRE(world.GetSymSys()->outside_taxa.contains(sym_parent_taxon.Cast<taxon_t::sym_taxon_t>()));
      }
    }
  }

  WHEN("A host lineage goes extinct") {
    WHEN("The tip survived for at least one update") {
      world.Update();

      REQUIRE(world.GetHostSys()->GetNumActive() == 1);
      REQUIRE(world.GetHostSys()->GetNumAncestors() == 2);
      REQUIRE(world.GetHostSys()->GetNumOutside() == 0);

      world.DoDeath(pos);
      world.Update();

      THEN("The tip and its ancestors is stored in the set of outside taxa") {
        REQUIRE(world.GetHostSys()->GetNumOutside() == 3);
        REQUIRE(world.GetHostSys()->outside_taxa.contains(host_grandparent_taxon.Cast<taxon_t::host_taxon_t>()));
        REQUIRE(world.GetHostSys()->outside_taxa.contains(host_parent_taxon.Cast<taxon_t::host_taxon_t>()));
        REQUIRE(world.GetHostSys()->outside_taxa.contains(host_taxon.Cast<taxon_t::host_taxon_t>()));
      }
    }

    WHEN("The tip survived for 0 updates") {
      // hosts are waiting to be deleted during Update()
      REQUIRE(world.GetHostSys()->GetNumActive() == 3);
      REQUIRE(world.GetHostSys()->GetNumOutside() == 0);

      world.DoDeath(pos);
      world.Update();

      THEN("The tip is not stored in the set of outside taxa; the tip's ancestors are stored in the set of outside taxa") {
        REQUIRE(world.GetHostSys()->GetNumOutside() == 2);
        REQUIRE(world.GetHostSys()->outside_taxa.contains(host_grandparent_taxon.Cast<taxon_t::host_taxon_t>()));
        REQUIRE(world.GetHostSys()->outside_taxa.contains(host_parent_taxon.Cast<taxon_t::host_taxon_t>()));
      }
    }
  }
}
