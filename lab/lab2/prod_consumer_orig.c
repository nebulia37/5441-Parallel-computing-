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

void insert_data(int producerno, int number)
{
    /* Wait until consumers consumed something from the buffer and there is space */

    /* Put data in the buffer */

    /* This function or producer should have print of the following form. It is
     * used for data validation by run_script.sh */
    printf("producer %d inserting %d at %d\n", producerno, number, location);
}

int extract_data(int consumerno)
{
    int done = 0;
    int value = -1;

    /* Wait until producers have put something in the buffer */

    /* This function or consumer should have print of the following form. It is
     * used for data validation by run_script.sh */
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

void consumer(int nproducers, int nconsumers)
{
    /* Do not move this declaration */
    int number = -1;
    int consumerno = -1;

    {
        printf("consumer %d: starting\n", consumerno);
        while (1)
        {
            number = extract_data(consumerno);
    
            if (number < 0)
                break;
    
            //usleep(10 * number);  /* "interpret" command for development */
            usleep(100000 * number);  /* "interpret" command for submission */
            fflush(stdout);
        }
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

void producer(int nproducers, int nconsumers)
{
    int i = 0;
    /* Thread number */
    int producerno = -1;
    char buffer[MAXLINELEN];
    int number = -1;

    {
        while (fgets(buffer, MAXLINELEN, stdin) != NULL)
        {
            number = atoi(buffer);
            insert_data(producerno, number);
        }
    }

    /* For simplicity, you can make it so that only one producer inserts the
     * "-1" to all the consumers. However, if you are able to create a logic to
     * distribute this among the producer threads, that is also fine. */
    {
        printf("producer: read EOF, sending %d '-1' numbers\n", nconsumers);

        for (i = 0; i < nconsumers; i++) {
            insert_data(-1, -1);
        }
    }

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
    int nconsumers = 1;

    if (argc != 3) {
        fprintf(stderr, "Error: This program takes one inputs.\n");
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

    printf("main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);

    /* Spawn N Consumer OpenMP Threads */
    consumer(nproducers, nconsumers);
    /* Spawn N Producer OpenMP Threads */
    producer(nproducers, nconsumers);

    return(0);
}
