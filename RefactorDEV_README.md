(delete once Anya is done with joint refactoring)

# Timeline plan
[x] Change renamed config settings (ORGANISM_TYPE) (also cleaned up some other unused config settings)
[x] Get Host task credit and Task Match Check tests not seg faulting
[x] Make mini guide on main changes for tests so others could help with test porting
[x] Move Host reproduction, output buffer processing, and process back into Host class and make sure current tests keep passing
[ ] Add citations and how to cite sections to README (on main), cite empirical JOSS paper, SignalGP arxiv, phylotrackpy on arxiv
[ ] Compare ecto relevant code (i.e. default mode) between main and this refactor to see if something changed, when was the last time the ecto integration test didn't seg fault on Mac? Prior to aux bump?
[ ] Decide whether to try to fix ecto integration test or remove the graveyard fix and merge into main since it apparently works on github??
[ ] Make new version bundle on main for complex features
[ ] Port more Symbiont tests
[ ] Move Symbiont code back into class
[ ] Check here https://github.com/anyaevostinar/SymbulationEmp/commit/9ea1d53c8bf70c612d1454fac0510ddaf0c70e9d for AEV TODO and Refactor note for what else I already had decided would be good to do



# Journal
4/15/26
* Some really confusing errors were happening and I realized that I had lost CURE and other main code that had both touched default-mode, so I started again by branching from main, merging in complex-syms-clean, bundling up all of sgp-mode into "old-sgp-mode" and then merging in Alex's sgp-mode-refactor to make sure that the Cure code came in along with various improvements to default-mode that had been made across the branches. The current version is pure Alex refactor with just the tests that look like they had been updated and a couple that I had written as I was doing my initial attempt at refactoring. Several tests are seg faulting for unclear reasons. I'm somewhat certain that hosts are able to reproduce and get points for doing a task and mutate correctly. Next steps will be figuring out why checking that hosts get credit for doing a task and Task Match Check are seg faulting, then going back to moving code back into Host and Symbiont along with porting all of the tests to work with the new style. I think I will make a guide for the main changes that need to be made to the tests so that others could possibly help with the porting and we can be more confident that things are currently working correctly.
4/3/26
* Task test working, needed to add a call to AssignNewEnvIO in the test, which seems weird since it seems like that should be handled by world.Setup correctly
* Moved output buffer processing back into a host method
4/2/26
* Adds tasks and test for it, currently failing
4/1/26
* Added to test to make sure orgs are getting older and it is failing
* Fixed by adding org to pos 0 instead of pos 1, a bit nervous about why pos 1 wasn't working
* Got Host process all back in
* Mutator called from Host's Mutate now, works when directly called, but I can't see where HostDoMutation was ever called previously so I'm nervous - Alex confirmed it was only called from SGPHost::Mutate, so we're good
3/31/26
* Building with scheduler code, need to test still
3/20/26
* SGPHost can be created in SGPWorld, SGPWorld test with baseline host works, scheduler is copied over but not tested

# Notes
These are function flows that are complicated to follow:

## Reproduction
* Repro Inst -> state.markReproAttempt
* Host Process checks Repro attempt -> calls AttemptReproduction
* AttemptReproduction -> marks ReproInProgress, adds to repro queue
* World processes repro queue after all organisms process
* Repro queue process calls Reproduction and handles HostDoBirth

## InputBuffer setup
* Input Buffer starts at size 0
* CPUState SetInputs is called by SGPWorld Assign New Env IO
    * This doesn't seem to be happening during the tests without specifically calling it, which seems not ideal
    * I think I fixed this, line 1074 of WorldSetup was commented out still...


# Old Timeline plan (deprecated 4/15, see Journal entry)
[x] SGPWorld with basic SGPHost
[x] Get scheduler working
[x] Get host reproduction working (with no point requirement)
[x] Get mutator working
[x] Get tasks working (and hosts completing them and getting points)
[-] Get all host tests working with new setup
    * Decided better to get refactor complete with some tests so that others can then jump in again, so will be doing minimal test porting, just enough to convince myself things are functional
[ ] Get symbionts existing (no interaction)
[ ] Move processsymbiont functionality back into symbionts
[ ] Get Health interaction working
[ ] Get Stress interaction working
[ ] Get Nutrient interaction working
[ ] Get data tracking working 
[ ] Port Multi infection code from Elias fork
[ ] Finish implementation of temporally changing environment
[ ] Make sure spatial structure is working?
[ ] Go back through for other refactoring todos