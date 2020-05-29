Hi there!

This file is supposed to help you understand what is going on in the data_management folder.

This system exists to help organize and save generated data to avoid repeatedly regenerating data, and to allow quick and powerful manipulation of all data yet gathered within an R script.

_Unfortunately, the repetition detection system is not very robust, and has to be actively maintained when the form of data to be collected changes._ Depending on your particular situation, it may not be worth it to maintain that feature (it is currently not functional (May 13, 2020)). Nevertheless, the key feature of being able to work with all data simultaneously in R is still functional, requires very little maintenance, and I (Lilith Orion Hafner on May 12, 2020) think that for many people, it is worth using and maintaining this system during development because the features outweigh the overhead.

The instructions from generate_data.py are as follows:


#############################
###   Instructions Begin  ###
#############################
'''

HOW TO USE

This is a tool for automatically running symbulation on multiple
treatments and/or trials and storing and collating their results.

To use this file, specify treatments in the treatments.py file
and then, from the command line, run
    "python3.6 generate_data.py"
while in the folder containing this file (data_management) 

Raw data (HostVals and SymVals) will be stored in the raw_data folder.
Collated data (A data table compatible with R) will be stored in
collated_data.data.

Collated data can be imported to R with
    "data = read.table('data_management/collated_data.data',header=TRUE)"
run from a working directory containing data_management.


INFO

"treatments" is a list of treatments. Each treatment corresponds to a
single execution of symbulation with the contents of SymSettings.cfg
indicated by the treatment.

To run multiple trials at the same treatment, include multiple
treatments differing only by their trial number.

By default, this will not recompute treatments that have already been
calculated. You may override this functionality with the -r flag:
    "python3.6 generate_data.py -r"
to force re-computation. To rerun collate_data.R but not symbulation,
use reprocess_data.py, which will execute much faster.

The loading bar looks like this:
_______________
XXX---XXX
where each _ in the top line represents a single treatment, each X in
the bottom line represents a treatment run, and each - represents a
treatment skipped.


LIMITATIONS

This program will probably not work on python 2. It should probably
work on any version of python 3, but I have only tested it on 3.8.

This system will probably not work on operating systems other than
MacOS. It should probably work on most recent versions of MacOS, but
I have only tested it on Catalina (10.15.3).

Population will be rounded down to the nearest square number.

Identification of identical treatments is not perfect. This may
recompute what should be identical treatments, or fail to generate
treatments that should be novel. At the root of this issue is floating
point error. Try to avoid parameters similar to 0.5600000000000005.


COMPONENTS

This system is comprised of In addition to the underlying Symbulation
system which generates raw data, and the contents of the
data_management folder:

 generate_data.py   this file

    treatments.py   specifies the treatments to run

   collate_data.R   gathers data from a single treatment from raw_data
                    and appends it to collated_data.data

generate_datum.sh   an intermediary bash script which generates data
                    and appends it to collated_data by calling 
                    symbulation and collate_data.R in series. This may
                    be a useful intermediary when introducing
                    parallelization.
 
         raw_data   contains raw data output by symbulation and will
                    be automatically created

reprocess_data.py   for reconstruction collated_data.data using
                    collate_data.R and the contents of raw_data

          version   version indicator to allow integrated preservation
                    of out of date data, and comparison of
                    symbulation behavior among various versions. This
                    may be useful in testing for consistency.

'''

#############################
###   Instructions  End   ###
#############################