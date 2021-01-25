#!/opt/star/sl73_gcc485/bin/python

import subprocess, os
from sys import argv
from glob import glob

# default input:
inlist='/gpfs/mnt/gpfs01/star/pwg/dstewart/outroot/AN/hadd_dummy'
outpath=''
maxFilesPerProcess=50
simSubmission='false'

if len(argv)>1:
    inlist = argv[1]
if len(argv)>2:
    outpath = argv[2]
if len(argv)>3:
    maxFilesPerProcess = argv[3]
if len(argv)>4:
    simSubmission = argv[4]


print('Running with arguments:\n inlist:%s\n outpath:%s\n maxFilesPerProcess:%i\n simSubmission:%s'
        %(inlist,outpath,maxFilesPerProcess,simSubmission))

# if inlist doesn't end with *.list, assume it is a path and make a default hadd.list if possibles
if not inlist[-5:] == '.list':
    print('inlist:%s not named like "*.list".\n -> Attempting to make hadd.list'%inlist)
    if not os.path.isdir(inlist):
        print('fatal: directory inlist:"%s" not found'%inlist)
        exit(2)
    files = glob('%s/*.root'%inlist)
    if not files:
        print('fatal: "*.root" files not found in inlist:%s'%inlist)
        exit(2)
    else:
        print('Writing %i files from inlist:%s to hadd.list'%(len(files),inlist))
        with open('hadd.list','w') as f_out:
            for file in files:
                f_out.write('%s\n'%file)
        



# required: existence of inlist and rootfiles in that path

print('end')
