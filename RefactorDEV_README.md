(delete once Anya is done with joint refactoring)

# Timeline plan
[x] SGPWorld with basic SGPHost
[ ] Get scheduler working
[ ] Get host reproduction working (with no point requirement)
[ ] Get mutator working
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
3/31/26
* Building with scheduler code, need to test still
3/20/26
* SGPHost can be created in SGPWorld, SGPWorld test with baseline host works, scheduler is copied over but not tested