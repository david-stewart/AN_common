# Purpose:
    input: <path>/*.root 
    output: <path>/<new-dir>/(hadd-ed *.root to less files)

# How:
    run `hadd_sums.py <inpath> <outpath> <maxFilesPerProcess=50> <simSubmission>`

# What happens:
    `hadd_sums.py` will check for the appropriate input and output directories.
    -> If they do not exist, it will make them
    -> It will check again for the dir and files, and fail if it cannot find them
    -> It will run `star-submit-template -template /star/u/djs232/AN/common/hadd_sums/sums_template.xml

