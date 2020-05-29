#Simplified versino of create_graphs.py

directory = "no_Evol/"
seeds = range(10, 21)
start_mois = [0, 1, 5, 10, 20, 40, 70, 100]
#slrs = [0.3, 1, 2.7, 3]
#verts = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]


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

print("Copying SymSettings.cfg to "+directory)
silent_cmd("cp SymSettings.cfg "+directory)

for a in seeds:
    for b in start_mois:
        command_str = './symbulation -SEED '+str(a)+' -START_MOI '+str(b)+' -FILE_PATH '+directory+' -FILE_NAME SM'+str(b)+'_Seed'+str(a)
        #command_str = './symbulation -SEED '+str(a)+' -VERTICAL_TRANSMISSION '+str(b)+' -FILE_NAME _VT'+str(b)+'_Seed'+str(a)

        print(command_str)
        silent_cmd(command_str)
