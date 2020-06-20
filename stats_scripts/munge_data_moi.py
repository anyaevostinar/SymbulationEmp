import os.path
import gzip
from itertools import izip

folder = 'VTMOI/'

treatment_postfixes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30, 40, 60, 80, 100]
vts = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
#treatment_postfixes = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
slrs = [15]
partners = ["Host", "Sym"]
reps = range(10,15)
#reps = range(1001, 1021)
final_update = 3
header = "uid smoi vt rep update host_count sym_count sym_val host_val burst_size uninfected\n"

outputFileName = folder+"munged_data.dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_postfixes:
    for s in vts:
        for r in reps:
            host_fname = folder +"HostValsSM"+str(t)+"_Seed" + str(r)+"_VT"+str(s) + ".data"
 #           host_fname = folder +"HostVals_VT"+str(t)+"_Seed" + str(r)+".data"
            sym_fname = folder +"SymValsSM" + str(t) + "_Seed" + str(r)+"_VT"+str(s) + ".data"
            #sym_fname = folder +"SymVals_VT" + str(t) + "_Seed" + str(r)+ ".data"
            lysis_fname = folder +"LysisSM" + str(t) + "_Seed" + str(r) +"_VT"+str(s) + ".data"
            #lysis_fname = folder +"SymVals_VT" + str(t) + "_Seed" + str(r)+".data"
            uid = str(t) + "_" + str(r)
            host_file = open(host_fname, 'r')
            sym_file = open(sym_fname, 'r')
            lysis_file = open(lysis_fname, 'r')
            with open(host_fname) as host_file, open(sym_fname) as sym_file:
                for host_line, sym_line, lysis_line in izip(host_file, sym_file, lysis_file):
                    if (host_line[0] != "u"):
                        splitline = host_line.split(',')
                        symline = sym_line.split(',')
                        lysisline = lysis_line.split(',')
                    #                    if int(splitline[0]) == final_update:
                        outstring1 = "{} {} {} {} {} {} {} {} {} {} {}\n".format(uid, t, s, r, splitline[0], splitline[2], symline[2], symline[1], splitline[1], lysisline[1].strip(), splitline[3])
                        outFile.write(outstring1)
outFile.close()
