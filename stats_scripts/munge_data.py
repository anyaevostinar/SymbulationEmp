import os.path
import gzip

folder = '../'

treatment_postfixes = ['0.700000']
partners = ["Host", "Sym"]
reps = [10,11, 12, 13]
#reps = range(1001, 1021)
final_update = 100
header = "uid treatment rep update donate partner\n"

outputFileName = "munged_basic.dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_postfixes:
    for r in reps:
        for p in partners:
            fname = folder +p+"Vals" + str(r) + "_" + t + ".data"
            uid = t + "_" + str(r)
            curFile = open(fname, 'r')
            for line in curFile:
                if (line[0] != "u"):
                    splitline = line.split(',')
                    if int(splitline[0]) == final_update:
                        outstring1 = "{} {} {} {} {} {}\n".format(uid, t, r, splitline[0], splitline[1], p)
                        outFile.write(outstring1)
            curFile.close()
outFile.close()
