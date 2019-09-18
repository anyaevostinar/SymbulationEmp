import os.path
import gzip
from itertools import izip

folder = '../'

treatment_postfixes = ['0.700000']
partners = ["Host", "Sym"]
reps = range(10,19)
#reps = range(1001, 1021)
final_update = 100
header = "uid treatment rep update host_count sym_count\n"

outputFileName = "munged_moi.dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_postfixes:
    for r in reps:
        host_fname = folder +"HostVals" + str(r) + "_" + t + ".data"
        sym_fname = folder +"SymVals" + str(r) + "_" + t + ".data"
        uid = t + "_" + str(r)
        host_file = open(host_fname, 'r')
        sym_file = open(sym_fname, 'r')
        with open(host_fname) as host_file, open(sym_fname) as sym_file:
            for host_line, sym_line in izip(host_file, sym_file):
                if (host_line[0] != "u"):
                    splitline = host_line.split(',')
                    symline = sym_line.split(',')
                    if int(splitline[0]) == final_update:
                        outstring1 = "{} {} {} {} {} {}\n".format(uid, t, r, splitline[0], splitline[2], symline[2])
                        outFile.write(outstring1)
outFile.close()
