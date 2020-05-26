#Simplified versino of create_graphs.py

seeds = range(10, 21)
#start_mois = [1, 5, 10, 15]
#slrs = [0.3, 1, 2.7, 3]
verts = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]

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

for a in seeds:
    for b in verts:
        #            command_str = './symbulation -SEED '+str(a)+' -START_MOI '+str(b)+' -SYM_LYSIS_RES '+str(c)+' -FILE_NAME SLR'+str(c)+'_SM'+str(b)+'_Seed'+str(a)
        command_str = './symbulation -SEED '+str(a)+' -VERTICAL_TRANSMISSION '+str(b)+' -FILE_NAME _VT'+str(b)+'_Seed'+str(a)
        print(command_str)
        silent_cmd(command_str)
