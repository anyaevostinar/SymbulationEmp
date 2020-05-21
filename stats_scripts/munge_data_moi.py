import os.path
import gzip
from itertools import izip

folder = '../'

slr = "2.7"
treatment_postfixes = [1, 5, 10, 15]
partners = ["Host", "Sym"]
reps = range(10,16)
#reps = range(1001, 1021)
final_update = 3
header = "uid treatment rep update host_count sym_count\n"

outputFileName = "munged_moi_"+slr+".dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_postfixes:
    for r in reps:
        host_fname = folder +"HostValsSLR"+slr+"_SM" + str(t) + "_Seed" + str(r) + ".data"
        sym_fname = folder +"SymValsSLR"+slr+"_SM" + str(t) + "_Seed" + str(r) + ".data"
        uid = str(t) + "_" + str(r)
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
