#!/bin/bash

#If you want to see what the script is doing, uncomment the following line
#set -x

#DO NOT CHANGE THIS: START
module load cuda/11.6.1
module list &> _out
module_found=`cat _out | grep -i cuda | wc -l | awk '{ print $1 }'`
#Cleanup
rm -f _out
if [ "$module_found" != 1 ]
then
    echo "CUDA module not found. Are you sure you are on a GPU node?"
    echo "Use the following command to allocate a GPU node"
    echo "salloc -N <number_nodes> -A PAS2171 -p gpudebug --gpus-per-node=1 --time=<time_in_minutes>"
    exit
fi

nvidia-smi &> /dev/null
if [ "$?" != "0" ]
then
    echo "No GPUs found. Are you sure you are on a GPU node?"
    echo "Use the following command to allocate a GPU node"
    echo "salloc -N <number_nodes> -A PAS2171 -p gpudebug --gpus-per-node=1 --time=<time_in_minutes>"
    exit
fi
#DO NOT CHANGE THIS: END

subdir="cse5441-cuda-lab"
#Cleanup previous directory for submission
rm -rf $subdir
#Create a directory for submission
mkdir -p $subdir

for program in matrix_mul  matrix_mul_shared_dynamic
do
    #Copy program to submission directory
    cp $program.cu $subdir/
    #Build program
    echo "Build $program" | tee -a $subdir/$program-terminal-output
    #Compile the code with only mutex
    nvcc $program.cu &> $subdir/$program-compilation-output
    
    #Check for successful completion of the program
    if [ "$?" != 0 ]
    then
        echo "Error: There are compilation errors with $program.cu" | tee -a $subdir/$program-terminal-output
        cat $subdir/$program-compilation-output | tee -a $subdir/$program-terminal-output
        echo "Error: Please fix the errors and retry." | tee -a $subdir/$program-terminal-output
        exit
    else
        echo "Success: $program.cu compiled fine" | tee -a $subdir/$program-terminal-output
    fi
    echo "=====================================" | tee -a $subdir/$program-terminal-output

    for block_size in 4 8 16 32 #Add the block sizes you want to test with here
    do
        outfile="$subdir/$program-$block_size-block-output"
        (time ./a.out $block_size) &> $outfile
        if [ "$?" != 0 ]
        then
            echo "Error: There are runtime errors with $program.cu" | tee -a $subdir/$program-terminal-output
            exit
        fi
        #Find the amount of time the program took
        duration=`grep "real" $outfile | awk '{ print $2 }'`
        compute=`grep "Compute took" $outfile | awk '{ print $4 }'`
        echo "Success: $program.cu with block size $block_size ran fine in $duration compute took $compute seconds" | tee -a $subdir/$program-terminal-output
    done
done

#Command to submit the assignment
#/fs/ess/PAS2171/CSE-5441-SP22/submit
