import os.path
import gzip
from itertools import izip

folder = 'no_Evol/'

treatment_postfixes = [0, 1, 5, 10, 20, 40, 70, 100]
partners = ["Host", "Sym"]
reps = range(10,21)
#reps = range(1001, 1021)
final_update = 3
header = "uid treatment rep update host_count sym_count sym_val host_val\n"

outputFileName = "munged_moi_sweep.dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_postfixes:
    for r in reps:
        host_fname = folder +"HostValsSM"+str(t)+"_Seed" + str(r) + ".data"
        sym_fname = folder +"SymValsSM" + str(t) + "_Seed" + str(r) + ".data"
        uid = str(t) + "_" + str(r)
        host_file = open(host_fname, 'r')
        sym_file = open(sym_fname, 'r')
        with open(host_fname) as host_file, open(sym_fname) as sym_file:
            for host_line, sym_line in izip(host_file, sym_file):
                if (host_line[0] != "u"):
                    splitline = host_line.split(',')
                    symline = sym_line.split(',')
#                    if int(splitline[0]) == final_update:
                    outstring1 = "{} {} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], splitline[2], symline[2], symline[1], splitline[1])
                    outFile.write(outstring1)
outFile.close()
