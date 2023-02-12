#!/bin/bash

#If you want to see what the script is doing, uncomment the following line
#set -x

#Create a directory for output
mkdir -p output

for program in prod_consumer_mutex prod_consumer_condvar
do
    echo "Build $program"
    #Compile the code with only mutex
    gcc -pthread $program.c &> _comp_output
    
    #Check for successful completion of the program
    if [ "$?" != 0 ]
    then
        echo "Error: There are compilation errors with $program.c"
        cat _comp_output
        rm -f _comp_output
        echo "Error: Please fix the errors and retry."
        exit
    else
        echo "Success: $program.c compiled fine"
        rm -f _comp_output
    fi
    echo "====================================="
    
    num_shortlist=`wc -l shortlist | awk '{ print $1 }'`
    num_longlist=`wc -l longlist | awk '{ print $1 }'`

    #Run the program with shortlist and longlist
    for input in shortlist longlist
    do
        echo "Number of entries in $input = `wc -l $input | awk '{ print $1 }'`"
        echo "Running $program with $input"
        echo "====================================="
        #Run the program with different number of consumers
        for consumers in 2 4 6 8
        do
            #These are the number of entries we expect
            if [ "$input" == "shortlist" ]
            then
                num_output_entries=`echo "$num_shortlist+$consumers" | bc -l`
            else
                num_output_entries=`echo "$num_longlist+$consumers" | bc -l`
            fi
            #Run the program
            (time ./a.out $consumers < $input) &> output/$program-$input-output-$consumers.txt
            #The number of entries the producer inserted
            num_prod_entries=`grep "producer inserting" output/$program-$input-output-$consumers.txt | wc -l`
            #The number of entries the consumer extracted
            num_cons_entries=`grep "consumer" output/$program-$input-output-$consumers.txt | grep "extract" | wc -l`
            #Check for the correct number of entries
            if [ "$num_output_entries" != "$num_prod_entries" ]
            then
                echo "Error: $program.c did not generate correct number of entries on the producer side for $input"
                echo "Expected: $num_output_entries, Actual: $num_prod_entries"
                exit
            fi
            if [ "$num_output_entries" != "$num_cons_entries" ]
            then
                echo "Error: $program.c did not generate correct number of entries on the consumer side for $input"
                echo "Expected: $num_output_entries, Actual: $num_cons_entries"
                exit
            fi
            #Find the amount of time the program took
            duration=`grep "real" output/$program-$input-output-$consumers.txt | awk '{ print $2 }'`
            echo "Success: $program.c ran fine with $consumers consumers for $input in $duration"
        done
        echo "====================================="
    done
done
