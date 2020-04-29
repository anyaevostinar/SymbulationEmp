#Parameter names are: HRR, SLR, BS, BT, SL, SYN, POP, UPS, and T
#Abbreviations for HOST_REPRO_RES, SYM_LYSIS_RES, BURST_SIZE, BURST_TIME, SYM_LIMIT, SYNERGY, POPULATION, UPDATES, and TRIALS
#The last value in a treatment set is the number of trials to perform
#See generate_data.py for instructions
#This file specifies the trearments to run

slrs = [round(.04*(5.6/.04)**(i/4), 14) for i in range(5)]
bts = [1, 2, 3, 5, 10, 30, 100]
trials = list(range(10))

treatments = [
    [30, slr, 999999999, 1, 9999999990, 5, 1600, updates, trial] \
         for slr in slrs[1:] for updates in [101,201,401,801] for trial in trials
]+[
    [30*bt, int(slr*bt) if int(round(slr*bt,14)) == round(slr*bt,14) else round(slr*bt,14), 999999999, bt, 999999999, 5, 1600, 400*bt+1, trial] \
         for slr in slrs[1:] for bt in bts for trial in trials
]+[
]

treatments = sorted(treatments, key=lambda x:x[-1])
