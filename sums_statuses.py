#!/opt/star/sl73_gcc485/bin/python

'''
    Current:
    * Read output files in sums/out/* and see if the file ran to completion.
    * See if the corresponding output file is in ./_output/ (otherwise error with no-copy)
    * Write a sums-resub.csh file 

    Possible in the future:
    * If output is in ./output/ then get time to run, num calls, and events

'''

# look through the files in sums/out and get statistics on what failed and what passed and why
from glob import glob
import os, subprocess
from sys import argv


def read_SumsOutFile(file_name):
    '''
    input: file name
    return: dict{'status':
        {
          good,
          failed-no-We-BEGIN,
          failed-no-We-END,
          failed-no-open
          failed-no-copy
        }, 
        'n_events':nevents, 
        'has_time':{True,False}, 
        'time':n_seconds
        'n_events_total':n_events
        'n_events_completed':n_events_completed
        }
    '''
    rval = dict()
    try:
        lines = open(file_name).readlines()
    except:
        rval['status'] = 'failed-no-open'
        return rval

    # Find the time
    i = 0
    while True:
        try:
            if 'We BEGIN' in lines[i]:
                time_str = lines[i].split()[-3]
                day_str = lines[i].split()[-4]
                hr = int(time_str.split(':')[0])
                mm = int(time_str.split(':')[1])
                ss = int(time_str.split(':')[2])
                time_start = hr*3600+mm*60+ss
                break
            else:
                i += 1
        except:
            rval['status'] = 'failed-no-We-BEGIN'
            return rval

    # Get everything else
    if 'We END' in lines[-1]:
        try:
            time_strB = lines[-1].split()[-3]
            day_strB = lines[-1].split()[-4]
            hrB = int(time_strB.split(':')[0])
            mmB = int(time_strB.split(':')[1])
            ssB = int(time_strB.split(':')[2])
            time_end = hrB*3600+mmB*60+ssB
            time = time_end - time_start
            if day_strB != day_str:
                time += 24*3600
            time = '%02i:%02i:%02i'%(time/3600,(time%3600)/60,(time%60))
            rval['time'] = time
            rval['status'] = 'good'
            rval['has_time'] = True
        except:
            rval['status'] = 'error'
            print('failed read: We END in line:')
            print(lines[-1])
    else:
        rval['has_time'] = False
        rval['status'] = 'failed-no-We-END'

    rval['n_events_total'] = 0
    for line in lines:
        if 'Number of Events' == line[:16]:
            rval['n_events_total'] = int(line.split()[-1]) 
            break

    rval['n_events_completed'] = 0
    i = len(lines)-1 
    while i > 0:
        if lines[i][:10] == '- Finished':
            rval['n_events_completed'] = int(lines[i].split()[-2])
            break
        else:
            i -= 1
    return rval

#------------------------------------------
# MAIN: Start of program
#------------------------------------------

n_files = -1 #set to small positive value for testing purposes
if len(argv) > 1:
    try:
        n_files = int(argv[1])
    except:
        exit('Bad command line argument for max number of files.'
             ' (Must be an integer). Was: "%s"'%argv[1])
    
i = 0
results = dict()
sums_ofiles = glob('sums/out/*')
sums_ofiles.sort()

test = 0
for f in sums_ofiles:
    test += 1
    if n_files > 0 and test > n_files:
        break
    name = f.split('/')[-1]
    results[name] = read_SumsOutFile(f)
    if results[name]['status'] == 'good':
        if not os.path.isfile('_output/v1/%s.root'%name):
            results[name]['status']='failed-no-copy'

n_good = 0
n_no_We_BEGIN = 0
n_no_open = 0
n_no_We_END = 0
n_other = 0
n_total = 0
n_no_copy = 0

for K in results:
    n_total += 1
    if   results[K]['status'] == 'failed-no-We-BEGIN':
        n_no_We_BEGIN += 1
    elif results[K]['status'] == 'failed-no-We-END':
        n_no_We_END += 1
    elif results[K]['status'] == 'failed-no-open':
        n_no_We_END += 1
    elif results[K]['status'] == 'failed-no-copy':
        n_no_copy += 1
    elif results[K]['status'] == 'good':
        n_good += 1
    else: 
        n_other += 1

print(' - writing file: sums_statuses.txt')
with open ('sums_statuses.txt','w') as f_out:
    # print out the results
    f_out.write( "--------------------------------------------\n")
    f_out.write( " n_good: %i\n"%n_good )
    f_out.write( " ratio good/total: %4.2f\n"% ((1.)*n_good / n_total))
    f_out.write( "--------------------------------------------\n")
    f_out.write( " n_failed total: %i \n" %( n_no_We_END +n_no_We_BEGIN + n_no_open + n_no_copy))
    f_out.write( " n_no_open:      %i \n" % n_no_open)
    f_out.write( " n_no_We_END:    %i \n" % n_no_We_END)
    f_out.write( " n_no_We_BEGIN:  %i \n" % n_no_We_BEGIN)
    f_out.write( " n_no_copy:      %i \n" % n_no_copy)
    # f_out.write( "--------------------------------------------\n")
    f_out.write( " n_other: %i\n"% n_other)
    # f_out.write( "--------------------------------------------\n")

    def print_status(results, f_out, status_key):
        f_out.write( "--------------------------------------------\n")
        f_out.write( " files with status %s:\n"%status_key.replace('-','_') )
        # f_out.write( "--------------------------------------------\n")
        for K in results:
            D = results[K]
            if D['status'] == status_key:
                f_out.write('%-38s'%K)
                if 'n_events_completed' in D:
                    f_out.write(' NEvCompleted: %6i '%D['n_events_completed'])
                if 'n_events_total' in D:
                    f_out.write(' NEvTotal: %6i '%D['n_events_total'])
                if 'time' in D:
                    f_out.write('time: %s '%D['time'])
                f_out.write("\n")

    # f_out.write( "\n Lists of files:\n")
    f_out.write("\n")
    if (n_no_open):     print_status(results,f_out,'failed-no-open')
    if (n_no_We_BEGIN): print_status(results,f_out,'failed-no-We-BEGIN')
    if (n_no_We_END):   print_status(results,f_out,'failed-no-We-END')
    if (n_no_copy):     print_status(results,f_out,'failed-no-copy')
    if (n_other):       print_status(results,f_out,'other')
    if (n_good):        print_status(results,f_out,'good')

if not n_good == n_total:
    ''' Write a resub script that can be used '''
    
    # Find the .session.xml file
    files = glob('*.session.xml')
    if len(files) == 0:
        print('No *.session.xml files. Will not write a sums_resub.csh file.')
        exit()
    elif len(files) > 1:
        print('WARNING: more than one *.session.xml file!\n')
        print('   Defaulting to most recently generated.')
        files.sort(key=os.path.getmtime)
    session_xml = files[-1]

    # print('Writing a resubmit script in sums_resub.csh file for all files that did completely run.')
    print(' - Not all jobs ran successfully.\n'
          '   -> Writing file: sums_resub.csh.txt')
    with open('sums_resub.csh','w') as f_out:
        f_out.write('#!/usr/bin/csh\n')
        fail_n = []
        for K in results:
            if not results[K]['status'] == 'good':
                fail_n.append(K.split('_')[-1])
        f_out.write('star-submit -r %s %s\n'%(','.join(fail_n),session_xml))
    subprocess.call(['chmod','u+x','sums_resub.csh'])      
