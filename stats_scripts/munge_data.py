import os.path
import gzip

folder = '../'

treatment_postfixes = ['000000_0.000000', '000000_0.100000', '000000_0.300000', '000000_0.400000', '000000_0.500000']
partners = ["Host", "Sym"]
reps = range(43, 54)
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
