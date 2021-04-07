#!/usr/bin/bash
for file in include lib; do
if [[ ! -d ${file} ]]; then
    mkdir ${file}
fi
done

# ------------------------------
# working on src_io files
# ------------------------------

# line the header files
cd include
while read line; do
    if [ ! -L ${line}.h ]; then
        ln -s ../StRoot/${line}/${line}.h .
    fi
done < ../stroot_list
cd ..

# make a script that will compile all the the files
# it looks like:
#    root -l <<EOF
#    gSystem->Load("${ROOUNFOLD}/libRooUnfold.so");
#    .L src_io/ioOptMap.cc+
#    .L src_io/ioXsec_pAu2015.cc+
#    etc...
#    EOF
cd StRoot
temp_script=root_L_script.sh
echo 'root -l <<EOF' > ${temp_script}
# echo 'gSystem->Load("${ROOUNFOLD}/libRooUnfold.so");' >> ${temp_script}
while read line; do
    echo ".L ${line}/${line}.cxx+" >> ${temp_script}
done < ../stroot_list
echo 'EOF' >> ${temp_script}
chmod u+x ${temp_script}
./${temp_script}
rm ${temp_script}
cd ..

cd lib
while read line; do
    if [ ! -L lib${line}.so ]; then
        ln -s ../StRoot/${line}/${line}_cxx.so lib${line}.so
    fi
    if [ ! -L ${line}_cxx_ACLiC_dict_rdict.pcm ]; then
        ln -s ../StRoot/${line}/${line}_cxx_ACLiC_dict_rdict.pcm
    fi
done < ../stroot_list
cd ..
