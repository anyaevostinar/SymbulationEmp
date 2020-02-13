#############################
###   Treatments   Begin  ###
#############################

#Notes:
#Tested in Python 3.7.6
#
#Treatments is an array of treatment sets.
#Each treatment set is displayed on a signle graph
#
#You may specify values or lists of values for each parameter
#
#Population cannot be variable at this time.
#
#If two treatments in a treatment set are identical, one will overwrite the other.
#
#Spaces in group names cause problems right now.
#
#Abbreviations are for: HOST_REPRO_RES, SYM_LYSIS_RES, BURST_SIZE, BURST_TIME, SYM_LIMIT, POPULATION
population = 10000
parameter_names = 'GROUP_NAME, HRR, SLR, BS, BT, SL, POP'.split(', ')
treatments = [
    ['Burst_Size', 1000, 1, [0,1,3,10,30], 3, 10, population],
    ['Burst_Time', 1000, 1, 3, [0,1,5,10,15], 3, population],
    ['SLR', 1000, [25, 10, 5, 1, 0], 3, 10, 10, population],
    ['Sym_Limit', 1000, 1, 15, 5, [0, 1, 5, 15, 50, 1000], population],
##    ['Burst_Size', 10, 10, [0,5,10,15,20], 3, 10, population],
##    ['Burst_Size', 100, 0, [0,5,10,15,20], 3, 10, population],
##    ['Burst_Size', 100, 15, [0,5,10,15,20], 3, 10, population],
##    ['Burst_Size', 25, 5, [0,5,10,15,20], 3, 1, population],
##    ['Burst_Time', 1000, 1, 3, [0,1,5,10,15], 3, population],
##    ['Burst_Time', 20, 20, 1, [0,1,5,10,15], 3, population],
##    ['Burst_Time', 25, 25, 10, [0,1,5,10,15], 3, population],
##    ['Burst_Time', 25, 25, 5, [0,1,5,10,15], 3, population],
##    ['SLR', 1, [25, 10, 5, 1, 0], 3, 10, 1, population],
##    ['Sym_Limit', 1000, 1, 15, 5, [0, 1, 5, 15, 50, 1000], population],
##    ['group_1', list(range(1000,1003)), 1, 15, 3, 10, population],
##    ['host_resources', [200, 500, 1000, 2000, 4000], 1, 15, 3, 10, population],
##    ['host_resources', [200, 500, 1000, 2000, 4000], 5, 15, 3, 10, population],
##    ['host_resources', [200, 500, 1000, 2000, 4000], 25, 15, 3, 10, population],
##    ['burst_size', 500, 30, list(range(0,30,5)), 3, 10, population],
##    ['burst_size', 1000, 15, list(range(0,30,5)), 3, 10, population],
##    ['burst_time', 500, 30, 15, list(range(0,6,1)), 10, population],
##    ['burst_time', 1000, 15, 15, list(range(0,6,1)), 10, population],
]
name = "run_"

#############################
###   Treatments   End    ###
#############################


from time import time
from math import sqrt
t0 = time()

work_folder = 'stats_scripts/moi_hostsurvival'
run_folder = work_folder+'/'+name+str(int(time()))
data_folder = run_folder + '/data'
graphs_folder = run_folder + '/graphs'

#Command line interface
import subprocess

def srun_cmd(fun, *args):
    subprocess.run([fun, *args])
def run_cmd(fun, *args):
    return eval(str(subprocess.check_output([fun, *args]))[1:])
def prun_cmd(fun, *args):
    print(run_cmd(fun, *args))


srun_cmd('mkdir', run_folder)
srun_cmd('mkdir', data_folder)
srun_cmd('mkdir', graphs_folder)
group_folders_made = set()
for treatment_set in treatments:

    num_treatments_in_set = None #Validate treatement set and determine number of treatments in it
    for parameter in treatment_set[1:]:
        if isinstance(parameter, list):
            if num_treatments_in_set is None or len(parameter) == num_treatments_in_set:
                num_treatments_in_set = len(parameter)
            else:
                print('I had to skip treatment set '+str(treatments)+' because multiple variable parameters were set, but they didn\'t all have the same number of values')
                num_treatments_in_set = None
                break
    else:
        if num_treatments_in_set is None:
            num_treatments_in_set = 1
    if num_treatments_in_set is None:
        continue
    
    group_name = treatment_set[0]#Make data and graphs group folders if neccessary
    if group_name not in group_folders_made:
        group_folders_made.add(group_name)
        srun_cmd('mkdir', data_folder+'/'+group_name)
        srun_cmd('mkdir', graphs_folder+'/'+group_name)

    output_file = graphs_folder+'/'+group_name+'/'+'HRR{}_SLR{}_BS{}_BT{}_SL{}_POP{}.png'.format(*treatment_set[1:]).replace(' ', '')
    r_args = [output_file, str(population), '0', '1']
    for treatment_index in range(num_treatments_in_set):#Loop through treatments
        treatment = [p[treatment_index] if isinstance(p, list) else p for p in treatment_set]
        
        for index, name in enumerate(parameter_names): #Set config
            locals()[name] = treatment[index]
        
        SIDE = int(sqrt(POP))
        FILE_NAME = '_HRR{}_SLR{}_BS{}_BT{}_SL{}_POP{}_'.format(HRR, SLR, BS, BT, SL, POP)
        SEED = HRR+SLR+BS+BT+SL+SIDE
        with open('SymSettings.cfg', 'r') as SymSettings:
            data = SymSettings.readlines()
            data[3] = "set SEED " + str(SEED) + "                  # What value should the random seed be?\n"
            data[9] = "set GRID_X " + str(SIDE) + "                 # Width of the world, just multiplied by the height to get total size\n"
            data[10] = "set GRID_Y " + str(SIDE) + "                 # Height of world, just multiplied by width to get total size\n"
            data[12] = "set SYM_LIMIT " + str(SL) + "              # Number of symbiont allowed to infect a single host\n"
            data[17] = "set HOST_REPRO_RES " + str(HRR) + "      # How many resources required for host reproduction\n"
            data[18] = "set SYM_LYSIS_RES " + str(SLR) + "         # How many resources required for symbiont to create offspring for lysis each update\n"
            data[15] = "set BURST_SIZE " + str(BS) + "            # If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update\n"
            data[16] = "set BURST_TIME " + str(BT) + "            # If lysis enabled, this is how many updates will pass before lysis occurs\n"
            data[22] = "set FILE_PATH " + data_folder+'/'+group_name+'/' + "      # Output file location\n"
            data[23] = "set FILE_NAME " + FILE_NAME + "      # Root output file name\n"
        
        with open('SymSettings.cfg', 'w') as SymSettings:
            SymSettings.writelines(data)

        print('Running simmulation: ' + FILE_NAME[1:-1])
        srun_cmd('./symbulation') #Run symbulation

        r_args.append(data_folder+'/'+group_name+'/'+'HostVals'+FILE_NAME+'0.data')
        r_args.append(data_folder+'/'+group_name+'/'+'SymVals'+FILE_NAME+'0.data')
    import os
    print('Generating graph with R script', end='')
    cmd = '/Library/Frameworks/R.framework/Versions/3.6/Resources/Rscript --vanilla ./'+work_folder+'/cmd_munge_graph_save_hostsurvival_MOI.R ' + ' '.join(r_args)
    out = os.system(cmd)
    print('Error code: {}'.format(out))
    if out:
        print(cmd)
    print('Graph stored in: {}'.format(output_file))

print('Run time: {} minutes'.format((time()-t0)/60))
