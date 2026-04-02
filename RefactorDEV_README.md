(delete once Anya is done with joint refactoring)

# Timeline plan
[x] SGPWorld with basic SGPHost
[x] Get scheduler working
[x] Get host reproduction working (with no point requirement)
[x] Get mutator working
[ ] Get tasks working (and hosts completing them and reproducing)
[ ] Move code from World's ProcessHost back into Host (make sure tests relevant to just hosts are now passing)
[ ] Get data tracking working 
[ ] Check that is everything outside of symbionts and special interactions
[ ] Get symbionts existing (no interaction)
[ ] Move processsymbiont functionality back into symbionts
[ ] Get Health interaction working
[ ] Get Stress interaction working
[ ] Get Nutrient interaction working
[ ] Port Multi infection code from Elias fork
[ ] Finish implementation of temporally changing environment
[ ] Make sure spatial structure is working?
[ ] Go back through for other refactoring todos



# Journal
4/1/26
* Added to test to make sure orgs are getting older and it is failing
* Fixed by adding org to pos 0 instead of pos 1, a bit nervous about why pos 1 wasn't working
* Got Host process all back in
* Mutator called from Host's Mutate now, works when directly called, but I can't see where HostDoMutation was ever called previously so I'm nervous - Alex confirmed it was only called from SGPHost::Mutate, so we're good
3/31/26
* Building with scheduler code, need to test still
3/20/26
* SGPHost can be created in SGPWorld, SGPWorld test with baseline host works, scheduler is copied over but not tested