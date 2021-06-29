#a script to run several replicates of several treatments locally

directory = "Lysogeny_Test_Data/"
#seeds = range(10, 21)
seeds = [10]
#start_mois = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30, 40, 60, 80, 100]
#start_mois = [1]
#slrs = [15]
#verts = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
#sym_ints = [-1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0]
prophage_loss_rt = [0.0, 0.1, 0.5, 0.9, 1]

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

cmd("mkdir "+directory)
print("Copying SymSettings.cfg to "+directory)
cmd("cp SymSettings.cfg "+directory)

for a in seeds:
    for b in prophage_loss_rt:
        command_str = './symbulation -SEED '+str(a)+' -PROPHAGE_LOSS_RATE '+str(b)+' -FILE_PATH '+directory+' -FILE_NAME PLR'+str(b)+'_Seed'+str(a)
        print(command_str)
        cmd(command_str)
