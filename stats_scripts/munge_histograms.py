import os.path
import gzip

folder = 'Example'

#treatment_postfixes = ['000000_0.000000', '000000_0.100000', '000000_0.300000', '000000_0.400000', '000000_0.500000']
treatment_postfixes = ["Test"]
partners = ["Host"]
#reps = range(43, 54)
reps = [1]
#reps = range(1001, 1021)
header = "uid treatment rep update interval count partner\n"

outputFileName = "munged_histogram.dat"

outFile = open(outputFileName, 'w')
outFile.write(header)

for t in treatment_postfixes:
    for r in reps:
        for p in partners:
            #fname = folder +p+"Vals" + str(r) + "_" + t + ".data"
            fname = folder +p+"Vals"  + "_" + t + ".data"
            uid = t + "_" + str(r)
            curFile = open(fname, 'r')
            for line in curFile:
                if (line[0] != "u"):
                    splitline = line.split(',')
                    
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-1_-.9", splitline[4], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.9_-.8", splitline[5], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.8_-.7", splitline[6], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.7_-.6", splitline[7], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.6_-.5", splitline[8], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.5_-.4", splitline[9], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.4_-.3", splitline[10], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.3_-.2", splitline[11], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.2_-.1", splitline[12], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "-.1_0", splitline[13], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], "0_.1", splitline[14], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".1_.2", splitline[15], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".2_.3", splitline[16], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".3_.4", splitline[17], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".4_.5", splitline[18], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".5_.6", splitline[19], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".6_.7", splitline[20], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".7_.8", splitline[21], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".8_.9", splitline[22], p)
                    outFile.write(outstring1)
                    outstring1 = "{} {} {} {} {} {} {}\n".format(uid, t, r, splitline[0], ".9_1", splitline[23], p)
                    outFile.write(outstring1)
            curFile.close()
outFile.close()
