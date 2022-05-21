#include "../../default_mode/Host.h"
#include "../../default_mode/Symbiont.h"

TEST_CASE("Symbiont SetHost, GetHost", "[default]") {

    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Organism> host = emp::NewPtr<Host>(random, &world, &config);
    emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, &world, &config, int_val);

    symbiont->SetHost(host);

    REQUIRE(symbiont->GetHost() == host);

    symbiont.Delete();
    host.Delete();
}

TEST_CASE("Host SetSymbionts", "[default]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = 1;
    config.SYM_LIMIT(2);

    emp::Ptr<Organism> host = emp::NewPtr<Host>(random, &world, &config);
    emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
    emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, int_val);

    emp::vector<emp::Ptr<Organism>> syms;
    syms.push_back(sym1);
    syms.push_back(sym2);

    host->SetSymbionts(syms);

    REQUIRE(host->GetSymbionts().size() == syms.size());

    for(size_t i = 0; i < syms.size(); i++){
        emp::vector<emp::Ptr<Organism>> host_syms = host->GetSymbionts();
        emp::Ptr<Organism> curSym = host_syms[i];
        emp::Ptr<Organism> curHost = curSym->GetHost();
        REQUIRE(curSym == (emp::Ptr<Organism>) syms[i]);
        REQUIRE(curHost == host);
    }

    bool has_sym = true;
    REQUIRE(host->HasSym() == has_sym);

    host.Delete();
}

TEST_CASE("Host SymLimit", "[default]") {
    emp::Ptr<emp::Random> random;
    random.New(-1);
    SymConfigBase config;
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config);

    emp::Ptr<Symbiont> sym1;
    sym1.New(random, &world, &config, int_val);
    emp::Ptr<Symbiont> sym2;
    sym2.New(random, &world, &config, int_val);

    emp::vector<emp::Ptr<Organism>> syms;

    syms.push_back(sym1);
    host->AddSymbiont(sym1);
    REQUIRE(host->GetSymbionts() == syms);

    WHEN("A symbiont is added once the SymLimit has been reached ") {
        host->AddSymbiont(sym2);

        THEN("The symbiont is not added to the host's syms") {
            REQUIRE(host->GetSymbionts() == syms); // sym2 is not added, sym limit was reached

        }

    }
    random.Delete();
    host.Delete();
}

TEST_CASE("Host AddSymbiont", "[default]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    config.SYM_LIMIT(6);
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config);

    emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
    emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, int_val);

    emp::vector<emp::Ptr<Organism>> syms;

    syms.push_back(sym1);
    host->AddSymbiont(sym1);

    REQUIRE(host->GetSymbionts() == syms);
    bool has_sym = true;
    REQUIRE(host->HasSym() == has_sym);

    host->AddSymbiont(sym2);
    syms.push_back(sym2);
    REQUIRE(host->GetSymbionts() == syms);

    host.Delete();
}

TEST_CASE("Host AddReproSym, ClearReproSym, GetReproSymbionts", "[default]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    config.SYM_LIMIT(6);
    SymWorld world(*random, &config);
    double int_val = 1;

    emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config);

    emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, int_val);
    emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, int_val);

    emp::vector<emp::Ptr<Organism>> repro_syms;

    repro_syms.push_back(sym1);
    host->AddReproSym(sym1);

    REQUIRE(host->GetReproSymbionts() == repro_syms);

    repro_syms.push_back(sym2);
    host->AddReproSym(sym2);

    REQUIRE(host->GetReproSymbionts() == repro_syms);

    host->ClearReproSyms();
    repro_syms.clear();
    REQUIRE(host->GetReproSymbionts() == repro_syms);

    host.Delete();
    sym1.Delete();
    sym2.Delete();
}

TEST_CASE("Host DistribResources", "[default]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld world(*random, &config);
    config.SYM_LIMIT(6);
    config.SYNERGY(5);

    WHEN("Host interaction value >= 0 and  Symbiont interaction value >= 0") {

        double host_int_val = 0.5;
        double sym_int_val = 1;

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);


        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
        emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {sym1, sym2, sym3};
        host->SetSymbionts(syms);

        double resources = 120;
        host->DistribResources(resources);


        int num_syms = 3;
        double sym_piece = resources / num_syms; // how much resource each sym gets
        double host_donation = sym_piece * host_int_val;
        double host_portion = sym_piece - host_donation;
        double sym_return = (host_donation * sym_int_val) * config.SYNERGY();
        double sym_portion = host_donation - (host_donation * sym_int_val);
        host_portion += sym_return;

        double host_points = num_syms * host_portion; // * by num_syms bc points are added during each iteration through host's syms
        double sym_points = sym_portion;


        THEN("Host and Symbionts points increase") {

            for( emp::Ptr<Organism> symbiont : syms) {
                REQUIRE(symbiont->GetPoints() == sym_points);
            }
            REQUIRE(host->GetPoints() == host_points);
        }
        host.Delete();
    }


    WHEN("Host interaction value <= 0 and Symbiont interaction value < 0") {

        WHEN("Host interaction value < Symbionts' interaction value") {
            double host_int_val = -0.5;
            double sym_int_val = -0.1;
            double host_orig_points = 0;
            double sym_orig_points = 0;

            emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);


            emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
            emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
            emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
            emp::vector<emp::Ptr<Organism>> syms = {sym1, sym2, sym3};
            host->SetSymbionts(syms);

            double resources = 120;
            host->DistribResources(resources);

            int num_syms = 3;
            double sym_piece = resources / num_syms; // how much resource each sym gets
            double host_defense = -1 * (host_int_val * sym_piece);
            double remaining_resources = sym_piece - host_defense;
            double host_points = remaining_resources * num_syms; // * by num_syms bc points are added during each iteration through host's syms

            THEN("Symbiont points do not change (gets nothing from host), Host points increase") {
                for( emp::Ptr<Organism> symbiont : syms) {
                    REQUIRE(symbiont->GetPoints() == sym_orig_points);
                }
                REQUIRE(host->GetPoints() == host_points);
                REQUIRE(host->GetPoints() > host_orig_points);
            }
            host.Delete();
        }

        WHEN("Host interaction value > Symbionts' interaction value") {
            double host_int_val = -0.2;
            double sym_int_val = -0.6;
            double host_orig_points = 0;
            double sym_orig_points = 0;

            emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);


            emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
            emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
            emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
            emp::vector<emp::Ptr<Organism>> syms = {sym1, sym2, sym3};
            host->SetSymbionts(syms);

            double resources = 120;
            host->DistribResources(resources);

            int num_syms = 3;
            double sym_piece = resources / num_syms; // how much resource each sym gets
            double host_defense = -1 * (host_int_val * sym_piece);
            double remaining_resources = sym_piece - host_defense;
            double sym_steals = (host_int_val - sym_int_val) * remaining_resources;
            double sym_points = sym_steals;
            double host_points = (remaining_resources - sym_steals) * num_syms; // * by num_syms bc points are added during each iteration through host's syms

            THEN("Symbionts points and Host points increase") {
                for( emp::Ptr<Organism> symbiont : syms) {
                    REQUIRE(symbiont->GetPoints() == sym_points);
                    REQUIRE(symbiont->GetPoints() > sym_orig_points);
                }
                REQUIRE(host->GetPoints() == host_points);
                REQUIRE(host->GetPoints() > host_orig_points);
            }
            host.Delete();
        }
    }
    WHEN("Host interaction value > 0 and Symbiont interaction value < 0, single symbiont") {
        double host_int_val = 0.1;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);
        emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val, sym_orig_points);
        host->AddSymbiont(symbiont);

        int resources = 100;
        host->DistribResources(resources);

        // int host_donation = 10; //host_int_val * resources
        int host_portion = 90;  //remaining amount
        int sym_steals = 9; //host_portion * sym_int_val * -1; new code value should be 18
        int sym_portion = 19; //sym_steals + host_donation; new code value should be 28
        host_portion = host_portion - sym_steals; //remove stolen resources from host's portion

        THEN("Symbionts points and Host points increase the correct amounts") {
            REQUIRE(symbiont->GetPoints() == sym_orig_points+sym_portion);
            REQUIRE(host->GetPoints() == host_orig_points+host_portion);
        }
        host.Delete();
    }
    WHEN("Host interaction value > 0 and Symbiont interaction value < 0, multiple symbionts") {
        double host_int_val = 0.1;
        double sym_int_val = -0.1;
        double host_orig_points = 0;
        double sym_orig_points = 0;

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);


        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val, sym_orig_points);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val, sym_orig_points);
        emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val, sym_orig_points);
        emp::vector<emp::Ptr<Organism>> syms = {sym1, sym2, sym3};
        host->SetSymbionts(syms);

        double resources = 120;
        host->DistribResources(resources);


        int num_syms = 3;
        // double sym_piece = 40; //resources / num_syms

        // int host_donation = 4; //host_int_val * sym_piece
        double host_portion = 36;  //remaining amount
        double sym_steals = 3.6; //host_portion * sym_int_val * -1
        double sym_portion = 7.6; //sym_steals + host_donation
        host_portion = host_portion - sym_steals; //remove stolen resources from host's portion

        double host_final_portion = host_portion * num_syms;


       THEN("Symbionts points and Host points increase") {
           for( emp::Ptr<Organism> symbiont : syms) {


               REQUIRE(symbiont->GetPoints() == sym_portion);
               REQUIRE(symbiont->GetPoints() > sym_orig_points);
            }

            REQUIRE(host->GetPoints() == host_final_portion);
            REQUIRE(host->GetPoints() > host_orig_points);
        }
        host.Delete();
    }


    WHEN("Host interaction value < 0 and Symbiont interaction value >= 0") {
        double host_int_val = -0.1;
        double sym_int_val = 0.8;
        double host_orig_points = 0;
        double symbiont_orig_points = 0;

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, &world, &config, host_int_val);


        emp::Ptr<Symbiont> sym1 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
        emp::Ptr<Symbiont> sym2 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
        emp::Ptr<Symbiont> sym3 = emp::NewPtr<Symbiont>(random, &world, &config, sym_int_val);
        emp::vector<emp::Ptr<Organism>> syms = {sym1, sym2, sym3};
        host->SetSymbionts(syms);

        double resources = 120;
        host->DistribResources(resources);

        int num_syms = 3;
        double sym_piece = resources / num_syms;
        double host_defense = -1 * (host_int_val * sym_piece);
        double host_portion = sym_piece - host_defense;
        double sym_portion = 0;

        double sym_points = sym_portion;
        double host_points = host_portion * num_syms; // * by num_syms bc points are added during each iteration through host's syms

        THEN("Symbiont points do not change (gets nothing from host), Host points increase") {
            for( emp::Ptr<Organism> symbiont : syms) {
                REQUIRE(symbiont->GetPoints() == sym_points);
                REQUIRE(symbiont->GetPoints() == symbiont_orig_points);
            }
            REQUIRE(host->GetPoints() == host_points);
            REQUIRE(host->GetPoints() > host_orig_points);

        }
        host.Delete();
    }
}

TEST_CASE("Vertical Transmission of Symbiont", "[default]") {
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    SymConfigBase config;
    SymWorld w(*random, &config);
    SymWorld * world = &w;



    WHEN("When vertical transmission is enabled and the sym has enough resources to transmit"){
        config.VERTICAL_TRANSMISSION(1);
        double points_required = 50;
        double points_recieved = points_required;
        config.SYM_VERT_TRANS_RES(points_required);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, host_int_val);
        emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, world, &config, sym_int_val);
        symbiont->AddPoints(points_recieved);

        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, world, &config, host->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size() + 1;
        symbiont->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }

        THEN("Symbiont parent has lost the points needed for reproduction and is down to 0") {
            REQUIRE(symbiont->GetPoints() == 0);
        }
        symbiont.Delete();
        host.Delete();
        host_baby.Delete();
    }
    WHEN("When vertical transmission is disabled"){
        config.VERTICAL_TRANSMISSION(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, host_int_val);
        emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, world, &config, sym_int_val);

        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, world, &config, host->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
        symbiont->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are not injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }
        symbiont.Delete();
        host.Delete();
        host_baby.Delete();
    }
    WHEN("When the sym does not have enough resources to transmit"){
        config.VERTICAL_TRANSMISSION(1);
        double int_val = 0;
        double points_required = 50;
        double points_recieved = points_required - 1;
        config.SYM_VERT_TRANS_RES(points_required);

        emp::Ptr<Host> host = emp::NewPtr<Host>(random, world, &config, int_val);
        emp::Ptr<Symbiont> symbiont = emp::NewPtr<Symbiont>(random, world, &config, int_val);

        symbiont->AddPoints(points_recieved);

        emp::Ptr<Host> host_baby = emp::NewPtr<Host>(random, world, &config, host->GetIntVal());
        long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
        symbiont->VerticalTransmission(host_baby);

        THEN("Symbiont offspring are not injected into host offspring") {
            REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
        }

        symbiont.Delete();
        host.Delete();
        host_baby.Delete();
    }
}

TEST_CASE("HandleEctosymbiosis"){
  emp::Random random(17);
  SymConfigBase config;
  SymWorld world(random, &config);
  world.Resize(1,1);
  double int_val = 0.5;\

  config.FREE_LIVING_SYMS(1);
  config.SYM_INFECTION_CHANCE(0.0);

  WHEN("Ectosymbiosis is off"){
    config.ECTOSYMBIOSIS(0);

    emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
    emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
    world.AddOrgAt(sym,0);
    world.AddOrgAt(host,0);
    REQUIRE(sym->GetPoints() == 0);
    REQUIRE(host->GetPoints() == 0);

    double res = 10;
    double sym_res = 0;
    double host_res = res * int_val;

    double leftover_res = host->HandleEctosymbiosis(res, 0);
    host->DistribResources(leftover_res);

    THEN("Parallel organisms don't distribute resources together"){
      REQUIRE(sym->GetPoints() == sym_res);
      REQUIRE(host->GetPoints() == host_res);
      REQUIRE(leftover_res == res);
    }
  }
  WHEN("Ectosymbiosis is on"){
    config.ECTOSYMBIOSIS(1);

    WHEN("There is no endosymbiont, only ectosymbiont"){
      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Organism> sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      world.AddOrgAt(sym,0);
      world.AddOrgAt(host,0);

      REQUIRE(sym->GetPoints() == 0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;
      int synergy = 5;

      double host_res = (res * int_val) + (res * int_val * int_val * synergy); //host portion + sym donation
      double sym_res = res * int_val * int_val;

      double leftover_res = host->HandleEctosymbiosis(res, 0);

      THEN("Ecto symbiont benefits fully"){
        REQUIRE(sym->GetPoints() == sym_res);
        REQUIRE(host->GetPoints() == host_res);
        REQUIRE(leftover_res == 0);
      }
    }

    WHEN("A hosted sym does not confer immunity to ectosymbiosis"){
      int synergy = 1;
      config.ECTOSYMBIOTIC_IMMUNITY(0);
      config.SYNERGY(synergy);

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Organism> parallel_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Organism> hosted_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      world.AddOrgAt(parallel_sym,0);
      world.AddOrgAt(host,0);
      host->AddSymbiont(hosted_sym);

      REQUIRE(parallel_sym->GetPoints() == 0);
      REQUIRE(hosted_sym->GetPoints() == 0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;

      double host_res = (res * int_val) + (res * int_val * int_val * synergy); //host portion + sym donation
      //10 res, sym portion is 5, host with interaction val of .5 keeps 2.5 of each sym piece
      //each sym keeps 1.25 of a piece, and donates 1.25 back, meaning the host gets 7.5 total
      double sym_res = (res/2 * int_val * int_val); //syms

      double leftover_res = host->HandleEctosymbiosis(res, 0);
      host->DistribResources(leftover_res);

      THEN("Both symbionts get the same amount of resources"){
        REQUIRE(hosted_sym->GetPoints() == sym_res);
        REQUIRE(parallel_sym->GetPoints() == sym_res);
        REQUIRE(host->GetPoints() == host_res);
        REQUIRE(leftover_res == (res/2)); //resources are split between two syms
      }
    }

    WHEN("A hosted sym confers immunity to ectosymbiosis"){
      config.ECTOSYMBIOTIC_IMMUNITY(1);

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      emp::Ptr<Organism> parallel_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);
      emp::Ptr<Organism> hosted_sym = emp::NewPtr<Symbiont>(&random, &world, &config, int_val);

      world.AddOrgAt(parallel_sym,0);
      world.AddOrgAt(host,0);
      host->AddSymbiont(hosted_sym);

      REQUIRE(parallel_sym->GetPoints() == 0);
      REQUIRE(hosted_sym->GetPoints() == 0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;
      int synergy = 5;

      double host_res = (res * int_val) + (res * int_val * int_val * synergy); //host portion + sym donation
      double hosted_sym_res = res * int_val * int_val;

      double leftover_res = host->HandleEctosymbiosis(res, 0);
      host->DistribResources(leftover_res);

      THEN("The parallel symbiont does not recieve resources"){
        REQUIRE(hosted_sym->GetPoints() == hosted_sym_res);
        REQUIRE(parallel_sym->GetPoints() == 0);
        REQUIRE(host->GetPoints() == host_res);
        REQUIRE(leftover_res == res); //ectosymbiosis doesn't happen
      }
    }
    WHEN("There is no parallel sym"){
      double int_val = 0.5;

      config.ECTOSYMBIOSIS(1);

      emp::Ptr<Host> host = emp::NewPtr<Host>(&random, &world, &config, int_val);
      world.AddOrgAt(host,0);
      REQUIRE(host->GetPoints() == 0);

      double res = 10;
      double host_res = res * int_val;

      double leftover_res = host->HandleEctosymbiosis(res, 0);
      host->DistribResources(leftover_res);

      THEN("The host does not get a sym modifier on its resources"){
        REQUIRE(host->GetPoints() == host_res);
        REQUIRE(leftover_res == res);
      }
    }
  }
}
