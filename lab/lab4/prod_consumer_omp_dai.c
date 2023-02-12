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
#include <omp.h>

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

/* DO NOT change MAX_BUF_SIZE */
#define MAX_BUF_SIZE    10

/* The simplest way to implement this would be with a stack as below. But you
 * are free to choose any other data structure you prefer. */
int buffer[MAX_BUF_SIZE] = {0};
int location = 0;

int empty = 1;
int full = 0;
int count = 0;
int nextin = 0;
int nextout = 0;
int nconsumers = 1;
int nconsumers_tmp;

void insert_data(int producerno, int number)
{

    /* Wait until consumers consumed something from the buffer and there is space */

    /* Put data in the buffer */

    /* This function or producer should have print of the following form. It is
     * used for data validation by run_script.sh */

    buffer[nextin] = number;
    int location = nextin;
    nextin = (nextin + 1) % MAX_BUF_SIZE;

    count++;
    if (count == MAX_BUF_SIZE)
        full = 1;
    if (count == 1) // buffer was empty
        empty = 0;

    printf("producer %d inserting %d at %d\n", producerno, number, location);

}

int extract_data(int consumerno)
{
    int done = 0;
    int value = -1;

    /* Wait until producers have put something in the buffer */

    /* This function or consumer should have print of the following form. It is
     * used for data validation by run_script.sh */

    value = buffer[nextout];
    location = nextout;
    nextout = (nextout + 1) % MAX_BUF_SIZE;
    count--;
    if (count == 0) // buffer is empty
        empty = 1;
    if (count == (MAX_BUF_SIZE-1))
        // buffer was full
        full = 0;

    printf("consumer %d extracting %d from %d\n", consumerno, value, location);

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

void consumer(int nproducers, int n_sumers)
{   

    /* Do not move this declaration */
    int number;
    int consumerno = n_sumers;
    printf("consumer %d: starting\n", consumerno);

    while (1)
    {
        #pragma omp critical (CriticalSection2)
        {
            while (1) {
                if (empty !=1){
                    number = extract_data(consumerno);
                    break;
                }
            }
        }

        if (number<0){break;}
        // usleep(1000 * number);  /* "interpret" command for development */
        usleep(100000 * number);  /* "interpret" command for submission */
        fflush(stdout);
    }

    printf("consumer %d: exiting\n", consumerno);
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

int fetch_data(char* buffer_data){
    int number;
    if (fgets(buffer_data, MAXLINELEN, stdin) != NULL){
        number = atoi(buffer_data);
    }
    else{
        if (nconsumers > 0){
            nconsumers = nconsumers - 1;
            number = -1;
        }
        else{ number = -2;}
    }
    return number;
}

void producer(int nproducers, int nconsumers)
{
    /* Thread number */
    printf("producers %d: starting\n", nproducers);
    int producerno = nproducers;
    char buffer_data[MAXLINELEN];
    int number;
    int flag = 1;
    while (flag) {
        #pragma omp critical (CriticalSection1)
        {
            number = fetch_data(buffer_data);
            if (number > -2){
                while (1) {
                    if (full != 1){
                        insert_data(producerno, number);
                        break;
                    }
                }
            }
            else{flag = 0;}
        }
    }

    printf("producer: read EOF, sending %d '-1' numbers\n", nconsumers);
    printf("producer %d: exiting\n", producerno);
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
    int tid = -1;
    int nproducers = 1;

    if (argc != 3) {
        fprintf(stderr, "Error: This program takes one inputs.\n");
        fprintf(stderr, "e.g. ./a.out nproducers nconsumers < <input_file>\n");
        exit (1);
    } else {
        nproducers = atoi(argv[1]);
        nconsumers = atoi(argv[2]);
        nconsumers_tmp = nconsumers;
        if (nproducers <= 0 || nconsumers <= 0) {
            fprintf(stderr, "Error: nproducers & nconsumers should be >= 1\n");
            exit (1);
        }
    }

    printf("main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);
    int num_of_threads = nconsumers + nproducers;

    printf("producer: read EOF, sending %d '-1' numbers\n", nconsumers);
    #pragma omp parallel private(tid) num_threads(num_of_threads)
    {
        tid=omp_get_thread_num();
        if(tid < nconsumers_tmp) {consumer(tid, tid);}
        else {producer(tid, nconsumers);}
    }


    return(0);
}
