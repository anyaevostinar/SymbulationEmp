#!/bin/bash

# taken from https://github.com/devosoft/Empirical/blob/master/doc/headtail.sh

stdbuf -i0 -o0 -e0 awk -v offset=${MAX_LINES:-20} \
          '{
               if (NR <= offset) print;
               else {
                   a[NR] = $0;
                   delete a[NR-offset];
                   printf "." > "/dev/stderr"
                   }
           }
           END {
             print "" > "/dev/stderr";
             for(i=NR-offset+1 > offset ? NR-offset+1: offset+1 ;i<=NR;i++)
             { print a[i]}
           }'
