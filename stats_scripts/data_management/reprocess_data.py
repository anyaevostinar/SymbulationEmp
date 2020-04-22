from time import time
t0 = time()

'''
This python3 script reprocesses all data referenced in collated_data.data
by rerunning the R script collate_data.R, while re-using raw_data computed
by symbulation. Old collated data is stashed as stash_collated_data.data.

This reprocessing should be significantly faster than generate_data.py

This is suitable for refreshing results after changed R analysis, but not
after changed Symbulation source code or configurations other than those
specified in generate_data.py. In either of the latter cases, the proper
course of action is to increment the version, and re generate any needed
data with generate_data.py.
'''

parameter_names = 'HRR, SLR, BS, BT, SL, POP, T'.split(', ')

#Command line interface
import subprocess

#File paths
version = 'version'
collated_data = 'collated_data.data'
raw_data = 'raw_data/'
symbulation_to_parent = 'stats_scripts/data_management/'
generate_datum = './generate_datum.sh'
sym_settings = '../../SymSettings.cfg'
stash = 'stash_'

subprocess.run(['mkdir', '-p', raw_data])
subprocess.run(['touch', raw_data])



#Load file name list
try:
    with open(collated_data) as f:
        lines = f.readlines()
        index = lines[0].split().index('"file_name"')
        file_names = [line.split()[index].strip('"') for line in lines[1:]]
except:
    print('There is no data to reprocess. Perhaps it has been stashed?')
    raise


#Stash old collated data
subprocess.run(['mv', collated_data, stash+collated_data])


#Loading bar
print('_'*len(file_names))




#Reprocessing
for file_name in file_names:

    #Loading bar
    print('X',end='',flush=True)



    #REPREOCESS DATUM (this command is blocking. for full paralelization, make it non-blocking)
    completed_process = subprocess.run(['/Library/Frameworks/R.framework/Versions/3.6/Resources/Rscript', '--vanilla', './collate_data.R', file_name], capture_output = True)



    #Error reporting
    if completed_process.returncode:
        print('datum generation failed.\nreturn code: {}\nargs: {}\nstdout: {}\nstderr: {}'\
            .format(
                completed_process.returncode, \
                completed_process.args, \
                completed_process.stdout.decode(), \
                completed_process.stderr.decode()))
        break


print('\nRun time: {} minutes'.format((time()-t0)/60))
