#a script to run several replicates of several treatments locally
import sys

directory = "PartPop/"
#start_mois = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30, 40, 60, 80, 100]
start_mois = [1]
slrs = [15]
#verts = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
#sym_ints = [-1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0]

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

start_range = 10
end_range = 21

if(len(sys.argv) > 1):
    start_range = int(sys.argv[1])
    end_range = int(sys.argv[2])

seeds = range(start_range, end_range)

cmd("mkdir "+directory)
print("Copying SymSettings.cfg and executable to "+directory)
cmd("cp SymSettings.cfg "+directory)
cmd("cp symbulation "+directory)
cmd("cd "+directory)
print("Using seeds", start_range, "up to", end_range)

for a in seeds:
    for b in start_mois:
        for c in slrs:
            command_str = './symbulation -SEED '+str(a)+' -START_MOI '+str(b)+' -FILE_PATH '+directory+' -FILE_NAME SM'+str(b)+'_Seed'+str(a)+'_SLR'+str(c)+' -SYM_LYSIS_RES '+str(c)
            # command_str = './symbulation -SEED '+str(a)+' -START_MOI '+str(b)+' -FILE_PATH '+directory+' -FILE_NAME SM'+str(b)+'_Seed'+str(a)+'_VT'+str(c)+' -VERTICAL_TRANSMISSION '+str(c)
            # command_str = './symbulation -SEED '+str(a)+' -START_MOI '+str(b)+' -FILE_PATH '+directory+' -FILE_NAME SM'+str(b)+'_Seed'+str(a)+'_SINT'+str(c)+' -SYM_INT '+str(c)
            # command_str = './symbulation -SEED '+str(a)+' -VERTICAL_TRANSMISSION '+str(b)+' -FILE_NAME _VT'+str(b)+'_Seed'+str(a) + " -FILE_PATH "+directory
            settings_filename = directory+"Output_SM"+str(b)+"_Seed"+str(a)+"_SLR"+str(c)+".data"
            print(command_str)
            cmd(command_str+" > "+settings_filename)
