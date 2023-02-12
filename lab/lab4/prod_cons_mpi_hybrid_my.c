/* 
 * This problem has you solve the classic "bounded buffer" problem with
 * multiple producers and multiple consumers:
 *
 *  ------------                         ------------
 *  | producer |-\                    /->| consumer |
 *  ------------ |                    |  ------------
 *               |                    |
 *  ------------ |                    |  ------------
 *  | producer | ----> bounded buffer -->| consumer |
 *  ------------ |                    |  ------------
 *               |                    |
 *  ------------ |                    |  ------------
 *  | producer |-/                    \->| consumer |
 *  ------------                         ------------
 *
 *  The program below includes everything but the implementation of the
 *  bounded buffer itself.  main() should do the following
 *
 *  1. starts N producers as per the first argument (default 1)
 *  2. starts N consumers as per the second argument (default 1)
 *
 *  The producer reads positive integers from standard input and passes those
 *  into the buffer.  The consumers read those integers and "perform a
 *  command" based on them (all they really do is sleep for some period...)
 *
 *  on EOF of stdin, the first producer passes N copies of -1 into the buffer.
 *  The consumers interpret -1 as a signal to exit.
 */

#include <stdio.h>
#include <stdlib.h>             /* atoi() */
#include <unistd.h>             /* usleep() */
#include <assert.h>             /* assert() */
#include <signal.h>             /* signal() */
#include <alloca.h>             /* alloca() */
#include <omp.h>                /* For OpenMP */
#include <mpi.h>                /* For MPI */
#include <signal.h>             /* signal() */
#include <alloca.h>             /* alloca() */
#include <pthread.h>

/**************************************************************************\
 *                                                                        *
 * Bounded buffer.  This is the only part you need to modify.  Your       *
 * buffer should have space for up to 10 integers in it at a time.        *
 *                                                                        *
 * Add any data structures you need (globals are fine) and fill in        *
 * implementations for these two procedures:                              *
 *                                                                        *
 * void insert_data(int producerno, int number)                           *
 *                                                                        *
 *      insert_data() inserts a number into the next available slot in    *
 *      the buffer.  If no slots are available, the thread should wait    *
 *      for an empty slot to become available.                            *
 *      Note: multiple producer may call insert_data() simulaneously.     *
 *                                                                        *
 * int extract_data(int consumerno)                                       *
 *                                                                        *
 *      extract_data() removes and returns the number in the next         *
 *      available slot.  If no number is available, the thread should     *
 *      wait for a number to become available.                            *
 *      Note: multiple consumers may call extract_data() simulaneously.   *
 *                                                                        *
\**************************************************************************/

/* DO NOT change MAX_BUF_SIZE or MAX_NUM_PROCS */
#define MAX_BUF_SIZE    10
#define MAX_NUM_PROCS   5
int num_procs = -1, myid = -1;
char hostname[MPI_MAX_PROCESSOR_NAME];
int data[1500];
int buffer[MAX_BUF_SIZE]={0};
int in = 0;
int out = 0;
int itemCount = 0;


void insert_data(int producerno, int number)
{
    /* Wait until consumers have consume something in the buffer */
    while(itemCount == MAX_BUF_SIZE) {;}
    #pragma omp critical
    {
        buffer[in] = number;
        in = (in+1)%MAX_BUF_SIZE;
        itemCount++;
    
        /* This print must be present in this function. Do not remove this print. Used for data validation */
    fprintf(stderr, "Process: %d on host %s producer %d inserting %d at %d\n", myid, hostname, producerno, number, in);
    }
}

int extract_data(int consumerno)
{
    int value = -1;

    /* Wait until producers have put something in the buffer */
    while(itemCount == 0) {;}
    #pragma omp critical
    {
        value = buffer[out];
        out = (out+1)%MAX_BUF_SIZE;
        itemCount--;
   
         /* This print must be present in this function. Do not remove this print. Used for data validation */
	    fprintf(stderr, "Process: %d on host %s consumer %d extracting %d from %d\n", myid, hostname, consumerno, value, out);

    }

    return value;
}

/**************************************************************************\
 *                                                                        *
 * The consumer. Each consumer reads and "interprets"                     *
 * numbers from the bounded buffer.                                       *
 *                                                                        *
 * The interpretation is as follows:                                      *
 *                                                                        *
 * o  positive integer N: sleep for N * 100ms                             *
 * o  negative integer:  exit                                             *
 *                                                                        *
\**************************************************************************/

void consumer(int nproducers, int nconsumers)
{
    /* Do not move this declaration */
    int number = -1;
    int consumerno = -1;

    #pragma omp parallel private(number,consumerno)
    {
        #pragma omp for
        for(int j =0;j<nconsumers;j++){
            consumerno = j;
            fprintf(stderr, "consumer %d: starting\n", consumerno);
            while (1)
            {
                number = extract_data(consumerno);
    
                if (number < 0)
                    break;
    
                //usleep(100 * number);  /* "interpret" command for development */
                usleep(10 * number);  /* "interpret" command for submission */
                fflush(stdout);
            }
        }
    }
    fprintf(stderr, "consumer %d: exiting\n", consumerno);

    return;
}

/**************************************************************************\
 *                                                                        *
 * Each producer reads numbers from stdin, and inserts them into the      *
 * bounded buffer.  On EOF from stdin, it finished up by inserting a -1   *
 * for every consumer so that all the consumers exit cleanly              *
 *                                                                        *
\**************************************************************************/

#define MAXLINELEN 128

void producer(int nproducers, int nconsumers, int offset, int chunksize)
{
    int number;
    int producerno = 1;
    int j;
    for(j=offset; j<(offset + chunksize); j++){
        number = data[j];
        insert_data(producerno, number);
    }
    if(j>=(offset + chunksize)){
        for (int k = 0; k < nconsumers; k++) {
            insert_data(-1, -1);
        }
        fprintf(stderr,"producer %d: exiting\n", producerno);
    }   
}

/*************************************************************************\
 *                                                                       *
 * main program.  Main calls does necessary initialization.              *
 * Calls the main consumer and producer functions which extracts and     *
 * inserts data in parallel.                                             *
 *                                                                       *
\*************************************************************************/

int main(int argc, char *argv[])
{
    //int tid = -1, len = 0;
    int nproducers = 1;
    int nconsumers = 1;


    int  dest, offset, tag1, tag2, tag3, source,chunksize, i, number;
    FILE* inFile;
    fprintf(stderr, "input counts: %d\n", argc);
    fprintf(stderr, "first par: %d\n",atoi(argv[1]));

    if (argc != 3) {
        fprintf(stderr, "Error: This program takes one input.\n");
        fprintf(stderr, "e.g. ./a.out nproducers nconsumers < <input_file>\n");
        exit (1);
    } else {
        nproducers = atoi(argv[1]);
        nconsumers = atoi(argv[2]);
        if (nproducers <= 0 || nconsumers <= 0) {
            fprintf(stderr, "Error: nproducers & nconsumers should be >= 1\n");
            exit (1);
        }
    }

    MPI_Status status;
    /***** MPI Initializations - get rank, comm_size and hostname - refer to
     * bugs/examples for necessary code *****/
    MPI_Init( &argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    fprintf (stderr, "MPI task %d has started...\n", myid);
    MPI_Barrier(MPI_COMM_WORLD);

    tag1 = 1;
    tag2 = 2;
    tag3 = 3;
    char tmp_buffer[128];
  
    if (num_procs > MAX_NUM_PROCS) {
        fprintf(stderr, "Error: Max num procs should <= 5\n");
        exit (1);
    }

    fprintf(stderr, "main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);
    
    

    if(myid == 0){

        //inFile = fopen(argv[3], "r");
        // read file content into data array
        // printf("start process 0\n");
        //char tmp_buffer[MAXLINELEN];
        i = 0;
        while (fgets(tmp_buffer, MAXLINELEN, stdin) != NULL) {
            number = atoi(tmp_buffer);
            data[i] = number;
            i++;
        }
        fprintf(stderr, "read file into array: array size = %d\n",i);

        chunksize = (i / num_procs);
        
         /* Send each task its portion of the array - master keeps 1st part */
        offset = chunksize;
        for (dest=1; dest<num_procs; dest++) {
            MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
            MPI_Send(&chunksize, 1, MPI_INT, dest, tag2, MPI_COMM_WORLD);
            MPI_Send(&data[offset], chunksize, MPI_INT, dest, tag3, MPI_COMM_WORLD);
            fprintf(stderr, "Sent %d elements to task %d offset= %d\n",chunksize,dest,offset);
            offset = offset + chunksize;
        }

        /* Master keep 1st part of the data */
        offset = 0;
        fprintf(stderr, "in process %d, offset = %d\n",myid,offset);

        /*Master multiple thread for prodcuer and consumer*/
        omp_set_nested(1);
        #pragma omp parallel num_threads(2)
        {
            if (omp_get_thread_num() == 0){
                #pragma omp parallel num_threads(nconsumers) shared(out, itemCount)
                {
                    /* Spawn N Consumer OpenMP Threads */
                    consumer(nproducers, nconsumers);
                }
            }else{
                #pragma omp parallel num_threads(nproducers) shared(in, itemCount,offset, chunksize)
                {
                    /* Spawn N Producer OpenMP Threads */
                    producer(nproducers, nconsumers, offset, chunksize);
                }
            } 
        }
    }
    
    if(myid >0){
    /* Receive my portion of array from the master task */
        source = 0;
        MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
        MPI_Recv(&chunksize, 1, MPI_INT, source, tag2, MPI_COMM_WORLD, &status);
        MPI_Recv(&data[offset], chunksize, MPI_INT, source, tag3, MPI_COMM_WORLD, &status);

        fprintf(stderr, "in process %d, offset = %d\n",myid,offset);
        fprintf(stderr, "in process %d, chunksize = %d\n",myid,chunksize);

    /* Do my part of the work */
        omp_set_nested(1);
        #pragma omp parallel num_threads(2)
        {
            if (omp_get_thread_num() == 1){
                #pragma omp parallel num_threads(nconsumers) shared(out, itemCount)
                {
                    /* Spawn N Consumer OpenMP Threads */
                    consumer(nproducers, nconsumers);
                }
            }else{
                #pragma omp parallel num_threads(nproducers) shared(in, itemCount, offset, chunksize)
                {
                    /* Spawn N Producer OpenMP Threads */
                    producer(nproducers, nconsumers, offset, chunksize);
                }
            }
        }
    }

    /* Finalize and cleanup */
    MPI_Finalize();
    return(0);
}
