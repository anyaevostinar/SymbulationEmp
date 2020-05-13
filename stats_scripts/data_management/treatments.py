#Parameter names are: HRR, SLR, BS, BT, SL, SYN, POP, UPS, SMOI, and T
#Abbreviations for HOST_REPRO_RES, SYM_LYSIS_RES, BURST_SIZE, BURST_TIME, SYM_LIMIT, SYNERGY, POPULATION, UPDATES, START_MOI, and TRIALS
#The last value in a treatment set is the number of trials to perform
#See generate_data.py for instructions
#This file specifies the trearments to run

slrs = [round(.04*(5.6/.04)**(i/4), 14) for i in range(5)]
bts = [1, 2, 3, 5, 10, 30, 100]
trials = list(range(4))

bt = 30

#Parameter names are: HRR, SLR, BS, BT, SL, SYN, POP, UPS, SMOI, and T
treatments = [
    [30*bt, int(.5*bt), 999999999, bt, 999999999, 5, 10000, 300, smoi, trial] \
         for smoi in [0,1,3,10,30] for trial in range(5)
]

treatments = sorted(treatments, key=lambda x:x[-1])
