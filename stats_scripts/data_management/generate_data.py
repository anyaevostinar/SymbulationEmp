from time import time
from math import sqrt
t0 = time()

#############################
###   Instructions Begin  ###
#############################
'''

HOW TO USE

This is a tool for automatically running symbulation on multiple
treatments and/or trials and storing and collating their results.

To use this file, specify treatments in the treatments.py file
and then, from the command line, run
    "python3.6 generate_data.py"
while in the folder containing this file (data_management) 

Raw data (HostVals and SymVals) will be stored in the raw_data folder.
Collated data (A data table compatible with R) will be stored in
collated_data.data.

Collated data can be imported to R with
    "data = read.table('data_management/collated_data.data',header=TRUE)"
run from a working directory containing data_management.


INFO

"treatments" is a list of treatments. Each treatment corresponds to a
single execution of symbulation with the contents of SymSettings.cfg
indicated by the treatment.

To run multiple trials at the same treatment, include multiple
treatments differing only by their trial number.

By default, this will not recompute treatments that have already been
calculated. You may override this functionality with the -r flag:
    "python3.6 generate_data.py -r"
to force re-computation. To rerun collate_data.R but not symbulation,
use reprocess_data.py, which will execute much faster.

The loading bar looks like this:
_______________
XXX---XXX
where each _ in the top line represents a single treatment, each X in
the bottom line represents a treatment run, and each - represents a
treatment skipped.


LIMITATIONS

This program will probably not work on python 2. It should probably
work on any version of python 3, but I have only tested it on 3.8.

This system will probably not work on operating systems other than
MacOS. It should probably work on most recent versions of MacOS, but
I have only tested it on Catalina (10.15.3).

Population will be rounded down to the nearest square number.

Identification of identical treatments is not perfect. This may
recompute what should be identical treatments, or fail to generate
treatments that should be novel. At the root of this issue is floating
point error. Try to avoid parameters similar to 0.5600000000000005.


COMPONENTS

This system is comprised of In addition to the underlying Symbulation
system which generates raw data, and the contents of the
data_management folder:

 generate_data.py   this file

    treatments.py   specifies the treatments to run

   collate_data.R   gathers data from a single treatment from raw_data
                    and appends it to collated_data.data

generate_datum.sh   an intermediary bash script which generates data
                    and appends it to collated_data by calling 
                    symbulation and collate_data.R in series. This may
                    be a useful intermediary when introducing
                    parallelization.
 
         raw_data   contains raw data output by symbulation and will
                    be automatically created

reprocess_data.py   for reconstruction collated_data.data using
                    collate_data.R and the contents of raw_data

          version   version indicator to allow integrated preservation
                    of out of date data, and comparison of
                    symbulation behavior among various versions. This
                    may be useful in testing for consistency.

'''

#############################
###   Instructions  End   ###
#############################

#Load treatments
parameter_names = 'HRR, SLR, BS, BT, SL, SYN, POP, UPS, T'.split(', ')
#Abbreviations for HOST_REPRO_RES, SYM_LYSIS_RES, BURST_SIZE, BURST_TIME, SYM_LIMIT, SYNERGY, POPULATION, UPDATES, and TRIALS
from treatments import treatments

#Treatment validation
warned_randomized_seed = False
for treatment in treatments:
    #Validate treatment format
    if not isinstance(treatment, list) \
            or not len(treatment) == len(parameter_names):
        print("I had to skip treatment {} because I didn't understand its format. I was expecting a list of the form {}.".format(treatment, parameter_names))
    
    #Round POP down to nearest square
    POP_index = parameter_names.index('POP')
    treatment[POP_index] = int(sqrt(treatment[POP_index]))**2

    #Warn of randomized seeds
    T_index = parameter_names.index('T')
    if treatment[T_index] < 0 and not warned_randomized_seed:
        print("Warning: seeds will be randomly selected for trials with negative trial numbers.")
        warned_randomized_seed = True

duplicate_treatment_count = len(treatments) - len(set(map(tuple, treatments)))
if duplicate_treatment_count:
    print('Warning: I found {} duplicate treatments. Make sure to vary the trial number.'.format(duplicate_treatment_count))



#Command line interface
from sys import argv
import subprocess

#Command line argument
recompute = '-r' in argv[1:]

#File paths
version = 'version'
collated_data = 'collated_data.data'
raw_data = 'raw_data/'
symbulation_to_parent = 'stats_scripts/data_management/'
generate_datum = './generate_datum.sh'
sym_settings = '../../SymSettings.cfg'

subprocess.run(['mkdir', '-p', raw_data])
subprocess.run(['touch', raw_data])
subprocess.run(['touch', collated_data])



#Load old data
if recompute:
    computed = {}
    print('Recomputing')
else:
    with open(version) as f:
        version = f.read().strip()
    with open(collated_data) as f:
        computed = {tuple(s[:15] for s in line.split()[:len(parameter_names)+1]) for line in f.readlines()}



#Loading bar
print('_'*len(treatments))



#Timing
simmulations_run = 0
def print_report(status):
    total_time = (time()-t0)/60
    ast = ', average time: {} minutes.'.format(total_time/simmulations_run) if simmulations_run else '.'
    print('\n{} Run time: {} minutes. {} simulations completed{}\n'.format(status, total_time, simmulations_run, ast))



#Data generation
try:
    for treatment in treatments:
        
        #Skip already computed treatments
        if tuple(map(lambda x: str(x)[:15], ['"'+version+'"']+treatment)) in computed:
            print('-', end='', flush=True)#Loading bar
            continue
        #print(treatment)
        
        #Select file name
        file_name = '_'.join(a+'='+str(b) for a,b in [['VERSION',version]]+list(zip(parameter_names, treatment))+[['TIME',time()]])

        #Load treatment values into local scope
        for name, value in zip(parameter_names, treatment): #Set config
            locals()[name] = value
        
        #Calculate configuration values not directly in treatments
        SIDE = int(sqrt(POP))
        SEED = T+1#tuple(treatment).__hash__()
            
        #Set configuration file
        with open(sym_settings, 'r') as f:
            data = f.readlines()

        # ### DEFAULT ###
        # # Default settings group

        data[3] = 'set SEED '+str(SEED)+'                  # What value should the random seed be? If seed <= 0, then it is randomly re-chosen.\n'
        # set MUTATION_RATE 0.002      # Standard deviation of the distribution to mutate by
        data[5] = 'set SYNERGY '+str(SYN)+'                # Amount symbiont\'s returned resources should be multiplied by\n'
        # set VERTICAL_TRANSMISSION 0  # Value 0 to 1 of probability of symbiont vertically transmitting when host reproduces
        # set HOST_INT -2               # Interaction value from -1 to 1 that hosts should have initially, -2 for random
        # set SYM_INT -1                # Interaction value from -1 to 1 that symbionts should have initially, -2 for random
        data[9] = 'set GRID_X '+str(SIDE)+'                 # Width of the world, just multiplied by the height to get total size\n'
        data[10] = 'set GRID_Y '+str(SIDE)+'                 # Height of world, just multiplied by width to get total size\n'
        data[11] = 'set UPDATES '+str(UPS)+'                # Number of updates to run before quitting\n'
        data[12] = 'set SYM_LIMIT '+str(SL)+'              # Number of symbionts allowed to infect a single host\n'
        # set LYSIS 1                  # Should lysis occur? 0 for no, 1 for yes
        # set HORIZ_TRANS 0            # Should non-lytic horizontal transmission occur? 0 for no, 1 for yes
        data[15] = 'set BURST_SIZE '+str(BS)+'            # If there is lysis, this is how many symbionts should be produced during lysis. This will be divided by burst_time and that many symbionts will be produced every update\n'
        data[16] = 'set BURST_TIME '+str(BT)+'            # If lysis enabled, this is how many updates will pass before lysis occurs\n'
        data[17] = 'set HOST_REPRO_RES '+str(HRR)+'      # How many resources required for host reproduction\n'
        data[18] = 'set SYM_LYSIS_RES '+str(SLR)+'         # How many resources required for symbiont to create offspring for lysis each update\n'
        # set SYM_HORIZ_TRANS_RES 100  # How many resources required for symbiont non-lytic horizontal transmission
        # set GRID 0                   # Do offspring get placed immediately next to parents on grid, same for symbiont spreading
        data[21] = 'set FILE_PATH '+str(symbulation_to_parent+raw_data)+'      # Output file location, leave blank for current folder\n'
        data[22] = 'set FILE_NAME _'+str(file_name)+'      # Root output file name\n'

        with open(sym_settings, 'w') as f:
            f.writelines(data)
            f.flush()

        #Loading bar
        print('X',end='',flush=True)



        #GENERATE DATUM
        completed_process = subprocess.run([generate_datum, file_name], capture_output = True)
        # This command is blocking. for full parallelization, make it
        # non-blocking, and address at least the following races:
        #    between symbulation reading the configuration file and this rewriting it
        #    among various executions of collate_data.R appending to collated_data.data



        #print(completed_process.stdout.decode())

        #Timing
        simmulations_run += 1

        #Error reporting
        if completed_process.returncode:
            print('datum generation failed.\nreturn code: {}\nargs: {}\nstdout: {}\nstderr: {}'\
                .format(
                    completed_process.returncode, \
                    completed_process.args, \
                    completed_process.stdout.decode(), \
                    completed_process.stderr.decode()))
            break
except:
    print_report('Failed.')
    raise

print_report('Finished!')
