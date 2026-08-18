import os.path
import gzip

folder = '..'

treatment_folders = ["Anti_Seed", "NoAnti_Seed"]
reps = range(11,16)
header = "uid treatment rep update sym_nand_count\n"
task_names = {1:"NAND", 2:"NOT", 3:"OR_NOT", 4:"AND", 5:"OR", 6:"AND_NOT", 
    7:"NOR", 8:"XOR", 9:"EQU"}

outputFileName = "munged_nand_count.dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_folders:
    for r in reps:
        fname = folder + "/"+t + str(r) + "/Tasks_data.csv"
        uid = t + "_" + str(r)
        curFile = open(fname, 'r')
        for line in curFile:
            if (line[0] != "u"):
                splitline = line.strip().split(',')
                
                outstring1 = "{} {} {} {} {}\n".format(uid, t, r, splitline[0], splitline[10])
                outFile.write(outstring1)
        curFile.close()
outFile.close()
