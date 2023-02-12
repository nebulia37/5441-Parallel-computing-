#!/bin/bash

#If you want to see what the script is doing, uncomment the following line
#set -x

subdir="cse5441-mpi-lab"
#Cleanup previous directory for submission
rm -rf $subdir
#Create a directory for submission
mkdir -p $subdir

#DO NOT CHANGE THIS
num_cores=140
num_procs=5
num_nodes=5

for program in prod_cons_mpi_hybrid
do
    #Cleanup
    rm -f $subdir/$program-terminal-output
    #Copy program to submission directory
    cp $program.c $subdir/
    #Build program
    echo "Build $program" | tee -a $subdir/$program-terminal-output
    #Compile the code with only mutex
    mpicc -pthread $program.c &> $subdir/$program-compilation-output
    
    #Check for successful completion of the program
    if [ "$?" != 0 ]
    then
        echo "Error: There are compilation errors with $program.c" | tee -a $subdir/$program-terminal-output
        cat $subdir/$program-compilation-output | tee -a $subdir/$program-terminal-output
        echo "Error: Please fix the errors and retry." | tee -a $subdir/$program-terminal-output
        exit
    else
        echo "Success: $program.c compiled fine" | tee -a $subdir/$program-terminal-output
    fi
    echo "=====================================" | tee -a $subdir/$program-terminal-output

    num_shortlist=`wc -l shortlist | awk '{ print $1 }'`
    num_longlist=`wc -l longlist | awk '{ print $1 }'`

    #Get an allocation
    #salloc -N $num_nodes -n $num_cores -A PAS2171 --time=30
    #Run the program with shortlist and longlist
    for input in shortlist longlist
    do
        echo "Running $program with $input" | tee -a $subdir/$program-terminal-output
        echo "=====================================" | tee -a $subdir/$program-terminal-output
        #Run the program with different number of producers
        for producers in 1
        do
            #Run the program with different number of consumers
            for consumers in 4 6 8 16 20
            do
                #DO NOT CHANGE THIS
                nthreads=`echo "$producers + $consumers" | bc -l`
                # export MV2_THREADS_PER_PROCESS=$nthreads
                #These are the number of entries we expect
                if [ "$input" == "shortlist" ]
                then
                    num_output_entries=`echo "$num_shortlist+($consumers*$num_procs)" | bc -l`
                else
                    num_output_entries=`echo "$num_longlist+($consumers*$num_procs)" | bc -l`
                fi
                outfile="$subdir/$program-$input-$producers-producers-$consumers-consumers"
                #Run the program
                (time srun -N $num_nodes -n $num_procs -A PAS2171 --time=240 ./a.out $producers $consumers < $input) &> $outfile
                if [ "$?" != 0 ]
                then
                    echo "Error: There are runtime errors with $program.c" | tee -a $subdir/$program-terminal-output
                fi
                #The number of entries the producer inserted
                num_prod_entries=`grep "inserting" $outfile | wc -l`
                #The number of entries the consumer extracted
                num_cons_entries=`grep "extracting" $outfile | wc -l`
                #Check for the correct number of entries
                if [ "$num_output_entries" != "$num_prod_entries" ]
                then
                    echo "Error: $program.c did not generate correct number of entries on the producer side for $input" | tee -a $subdir/$program-terminal-output
                    echo "Expected: $num_output_entries, Actual: $num_prod_entries" | tee -a $subdir/$program-terminal-output
                    exit
                fi
                if [ "$num_output_entries" != "$num_cons_entries" ]
                then
                    echo "Error: $program.c did not generate correct number of entries on the consumer side for $input" | tee -a $subdir/$program-terminal-output
                    echo "Expected: $num_output_entries, Actual: $num_cons_entries" | tee -a $subdir/$program-terminal-output
                    exit
                fi
                #Find the amount of time the program took
                duration=`grep "real" $outfile | awk '{ print $2 }'`
                echo "Success: $program.c ran fine with $producers producers and $consumers consumers for $input in $duration" | tee -a $subdir/$program-terminal-output
            done
        done
    done
done

#Command to submit the assignment
#/fs/ess/PAS2171/CSE-5441-SP22/submit
