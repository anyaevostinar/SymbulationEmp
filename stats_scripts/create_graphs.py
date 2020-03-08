from time import time, sleep
from math import sqrt
t0 = time()

#############################
###   Instructions Begin  ###
#############################
'''

HOW TO USE

This is a tool for automatically running multiple treatments and
treatment sets and generating graphs to summarize their results.

To create a set of graphs, specify parameters in the treatments
section of this file, and then from the command line, run
    "python3.6 create_graphs.py"
while in the folder containing this file (stats_scripts)

"treatments" is an array of treatment sets. Each treatment set is
saved as a single .png file containing separate graphs for each
treatment. You may specify values or lists of values for each
parameter in a treatment set, and the file names should reflect this.

Output will be grouped into files according to the value of
GROUP_NAME, but spaces in group names cause problems right now.

You can edit the trials variable to change the number of trials, and
the population variable to change the population.

The warning message "Removed n rows containing missing values 
(geom_point)." indicates that some trials resulted in all hosts dying.

LIMITATIONS

This program will probably not work on python 2. It should probably
work on any version of python 3, but I have only tested it on 3.6 and
3.7.

Population cannot be variable between treatments or treatment sets,
but you can change the population variable between executions of this
python program.

If two treatments in a treatment set are identical, one will overwrite
the other.


COMPONENTS

In addition to the underlying Symbulation system which generates the
data, which is ultimately graphed, this system is comprised of
  1. This file
  2. The R program cmd_munge_graph_save_hostsurvival_MOI.r
  3. The folder at "/moi_hostsurvival_data_graphs", refered to
     as work_folder in this python program
  4. Various intermediary data folders and files in the work folder
  5. A folder called "run_all_graphs" in the work folder which will
     contain all generated graphs.
Components 3, 4, and 5 will be automatically created by this python
program.

'''
#############################
###   Instructions  End   ###
#############################


#############################
###   Treatments   Begin  ###
#############################

trials = 10
population = 10000

popsq = int(sqrt(population))**2
#Abbreviations are for: HOST_REPRO_RES, SYM_LYSIS_RES, BURST_SIZE, BURST_TIME, SYM_LIMIT, POPULATION
parameter_names = 'GROUP_NAME, HRR, SLR, BS, BT, SL, POP'.split(', ')
treatments = [
    
    ['SYM_LYSIS_RES', 40, [.04*(5.6/.04)**(i/4) for i in range(5)], 999999999, 1, 999999999, popsq],

]

#############################
###   Treatments   End    ###
#############################


#Command line interface
import subprocess

def cmd(command):
    '''This wait causes all executions to run in sieries.
    For parralelization, remove .wait() and instead delay the
    R script calls unitl all neccesary data is created.'''
    return subprocess.Popen(command, shell=True).wait()

def silent_cmd(command):
    '''This wait causes all executions to run in sieries.
    For parralelization, remove .wait() and instead delay the
    R script calls unitl all neccesary data is created.'''
    return subprocess.Popen(command, shell=True, stdout=subprocess.PIPE).wait()


#Work folders
name = "run_"
work_folder = 'moi_hostsurvival_data_graphs'
run_folder = work_folder+'/'+name+str(int(time()))
all_graphs_folder = work_folder+'/'+name+'all_graphs'
data_folder = run_folder + '/data'
graphs_folder = run_folder + '/graphs'

def mkdir(dir):
    cmd('mkdir {}'.format(dir))
for dir in [work_folder, run_folder, all_graphs_folder, data_folder, graphs_folder]:
    mkdir(dir);

group_folders_made = set()
for treatment_set in treatments:

    treatments_names = None #Validate treatement set and determine number of treatments in it
    for parameter in treatment_set[1:]:
        if isinstance(parameter, list):
            if treatments_names is None or len(parameter) == len(treatments_names):
                treatments_names = list(parameter)
            else:
                print('I had to skip treatment set '+str(treatments)+' because multiple variable parameters were set, but they didn\'t all have the same number of values')
                treatments_names = None
                break
    else:
        if treatments_names is None:
            treatments_names = [0]
    if treatments_names is None:
        continue
    
    group_name = treatment_set[0]#Make data and graphs group folders if neccessary
    if group_name not in group_folders_made:
        group_folders_made.add(group_name)
        mkdir(data_folder+'/'+group_name)
        mkdir(graphs_folder+'/'+group_name)

    output_file = graphs_folder+'/'+group_name+'/'+'HRR{}_SLR{}_BS{}_BT{}_SL{}_POP{}_T{}.png'.format(*[str(p)[:30] for p in treatment_set[1:]]+[trials]).replace(' ', '')
    r_args = [output_file, str(population), str(trials)]
    for treatment_index, treatment_name in enumerate(treatments_names):#Loop through treatments
        treatment = [p[treatment_index] if isinstance(p, list) else p for p in treatment_set]
        
        for index, name in enumerate(parameter_names): #Set config
            locals()[name] = treatment[index]
        SIDE = int(sqrt(POP))
        SEED = 0#(HRR,SLR,BS,BT,SL,SIDE).__hash__()
        
        data_file_root_name = '_HRR{}_SLR{}_BS{}_BT{}_SL{}_POP{}_T{}'.format(HRR, SLR, BS, BT, SL, POP, trials)
        print('Simbulating: {}. Trial:'.format(data_file_root_name[1:]), end='', flush=True)

        for trial in range(trials):#IDENTICAL TREATMENTS
            FILE_NAME = data_file_root_name+'_R{}'.format(trial)
            with open('../SymSettings.cfg', 'r') as SymSettings:
                data = SymSettings.readlines()
                data[3] = "set SEED " + str(SEED+trial) + "                  # What value should the random seed be?\n"
                data[9] = "set GRID_X " + str(SIDE) + "                 # Width of the world, just multiplied by the height to get total size\n"
                data[10] = "set GRID_Y " + str(SIDE) + "                 # Height of world, just multiplied by width to get total size\n"
                data[12] = "set SYM_LIMIT " + str(SL) + "              # Number of symbiont allowed to infect a single host\n"
                data[17] = "set HOST_REPRO_RES " + str(HRR) + "      # How many resources required for host reproduction\n"
                data[18] = "set SYM_LYSIS_RES " + str(SLR) + "         # How many resources required for symbiont to create offspring for lysis each update\n"
                data[15] = "set BURST_SIZE " + str(BS) + "            # If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update\n"
                data[16] = "set BURST_TIME " + str(BT) + "            # If lysis enabled, this is how many updates will pass before lysis occurs\n"
                data[21] = "set FILE_PATH " + 'stats_scripts'+'/'+data_folder+'/'+group_name+'/' + "      # Output file location\n"
                data[22] = "set FILE_NAME " + FILE_NAME + "      # Root output file name\n"
            
            with open('../SymSettings.cfg', 'w') as SymSettings:
                SymSettings.writelines(data)
                SymSettings.flush()

            print(' {}'.format(trial),end='',flush=True);
            silent_cmd('cd ..; ./symbulation') #Run Symbulation

            r_args.append(str(treatment_name))
            r_args.append(data_folder+'/'+group_name+'/'+'HostVals'+data_file_root_name+'_R')#This is everything up to the trial number
            r_args.append(data_folder+'/'+group_name+'/'+'SymVals'+data_file_root_name+'_R')#Still missing trial_number.data at the end (e.g. 0.data)
        print('.')
    #import os
    #print('Generating graph with R script', end='. ')
    #cmd = '/Library/Frameworks/R.framework/Versions/3.6/Resources/Rscript --vanilla ./'+work_folder+'/cmd_munge_graph_save_hostsurvival_MOI.R ' + ' '.join(r_args)
    #out = os.system(cmd)
    #print('Error code: {}'.format(out))
    command = '/Library/Frameworks/R.framework/Versions/3.6/Resources/Rscript --vanilla ./' \
        + 'cmd_munge_graph_save_hostsurvival_MOI.R ' + ' '.join(r_args)
    out = cmd(command)
    if out:
        print(command)
    print('Graph stored in: {}'.format(output_file))

print('Copying graphs to {}'.format(all_graphs_folder))
for folder_name in group_folders_made:
    command = 'rsync -av {}/{} {}'.format(graphs_folder,folder_name,all_graphs_folder)
    try:
        cmd(command)
    except FileNotFoundError:
        print('I couldn\'t find a file while running command "{}", so I skiped copying that set of graphs to the all graphs folder.'.format(command))

print('Run time: {} minutes'.format((time()-t0)/60))
