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
//char hostname[MPI_MAX_PROCESSOR_NAME];
int data[1500];
int buffer[MAX_BUF_SIZE] = {0};
int location = 0;

int buffer_t[MAX_BUF_SIZE];
pthread_mutex_t mutex;
pthread_cond_t full_cond, empty_cond;
int front;
int rear;
int itemCount;

void buffer_init(void)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&full_cond, NULL);
    pthread_cond_init(&empty_cond, NULL);
    front =0;
    rear = 0;
    itemCount = 0;
}

void buffer_insert(int producerno, int number)
{
    pthread_mutex_lock(&mutex);
    while(itemCount == MAX_BUF_SIZE){
        pthread_cond_wait(&full_cond, &mutex);
    }
    if(itemCount == 0){
        buffer_t[rear] = number;
        pthread_cond_signal(&empty_cond);
    }else {
        buffer_t[rear] = number;
    }
    rear=(rear+1)%MAX_BUF_SIZE;
    printf("producer %d inserting item %d at %d\n",producerno, number,rear);
    itemCount++;
    pthread_mutex_unlock(&mutex);
    fflush(stdout);
}

int buffer_extract(int consumerno)
{
    pthread_mutex_lock(&mutex);
    int item;
    while(itemCount == 0){
        pthread_cond_wait(&empty_cond, &mutex);
    }
    if(itemCount == MAX_BUF_SIZE){
        item = buffer_t[front];
        pthread_cond_signal(&full_cond);
    }else{
        item = buffer_t[front];
    }
    front=(front+1)%MAX_BUF_SIZE;
    printf("consumer %d: extracting item %d from %d\n",((int)consumerno),item, front);
    itemCount--;
    pthread_mutex_unlock(&mutex);
    fflush(stdout);
    return item;                   /* FIX ME */
}

void buffer_clean(void)
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&full_cond);
    pthread_cond_destroy(&empty_cond);
    pthread_exit (NULL);
    free(buffer_t);
    printf("buffer_clean called\n"); /* FIX ME */
}

void *consumer_thread(void *raw_consumerno)
{
    int consumerno = (intptr_t)raw_consumerno; /* dirty trick to pass in an integer */

    printf("  consumer %d: starting\n", consumerno);
    while (1)
    {
        int number = buffer_extract(consumerno);

        if (number < 0)
            break;


        usleep(100000 * number);  /* "interpret" the command */
        fflush(stdout);
    }

    printf("  consumer %d: exiting\n", consumerno);
    return(NULL);
}

/**************************************************************************\
 * producer.  main calls the producer as an ordinary procedure rather     *
 * than creating a new thread.  In other words the original "main" thread *
 * becomes the "producer" thread.                                         *
\**************************************************************************/

#define MAXLINELEN 128

void producer(int nconsumers, int offset, int chunksize)
{
    int number;
    int producerno = 1;
    int j;
    for(j=offset; j<(offset + chunksize); j++){
        number = data[j];
        buffer_insert(producerno, number);
    }
    if(j>=(offset + chunksize)){
        for (int k = 0; k < nconsumers; k++) {
            buffer_insert(-1, -1);
        }
    printf("producer %d: exiting\n", producerno);
    }   
}

int main(int argc, char *argv[])
{

    int  dest, offset, tag1, tag2, tag3, source,chunksize, i, number, nproducers, nconsumers;
    FILE* inFile;
    int tid = -1;
    printf("input counts: %d\n", argc);
    printf("first par: %d\n",atoi(argv[1]));

    pthread_t *consumers;
    nproducers = 1;
    nconsumers = 16;
    int kount;


    if (argc == 3) {
        nproducers = atoi(argv[1]);
        nconsumers = atoi(argv[2]);
        if (nproducers <= 0 || nconsumers <= 0) {
            fprintf(stderr, "Error: nproducers & nconsumers should be >= 1\n");
            exit (1);
        }
    }

    MPI_Status status;
    /***** MPI Initializations - get rank, comm_size and hostname - refer to bugs/examples for necessary code *****/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    printf ("MPI task %d has started...  ", myid);
    tag1 = 1;
    tag2 = 2;
    tag3 = 3;
    char tmp_buffer[128];
        
    /***** Master task only ******/
    if(myid == 0){
        while (fgets(tmp_buffer, 128, stdin) != NULL) {
            number = atoi(tmp_buffer);
            data[i] = number;
            i++;
        }
        chunksize = i/num_procs;
    
        printf("read file into array: array size = %d\n",i);
        printf("num_procs= %d  chunksize= %d \n",num_procs,chunksize);
        
         /* Send each task its portion of the array - master keeps 1st part */
        offset = chunksize;
        for (dest=1; dest<num_procs; dest++) {
            MPI_Send(&offset, 1, MPI_INT, dest, tag1, MPI_COMM_WORLD);
            MPI_Send(&chunksize, 1, MPI_INT, dest, tag2, MPI_COMM_WORLD);
            MPI_Send(&data[offset], chunksize, MPI_INT, dest, tag3, MPI_COMM_WORLD);
            printf("Sent %d elements to task %d offset= %d\n",chunksize,dest,offset);
            offset = offset + chunksize;
        }

        /* Master keep 1st part of the data */
        offset = 0;
        printf("in process %d, offset = %d\n",myid,offset);
        
        /*Master multiple thread for prodcuer and consumer*/
        printf("main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);
   
        buffer_init();
        signal(SIGALRM, SIG_IGN);     /* evil magic for usleep() under solaris */

        consumers = (pthread_t *)alloca(nconsumers * sizeof(pthread_t));
        for (kount = 0; kount < nconsumers; kount++)
        {
            int test = pthread_create(&consumers[kount], /* pthread number */
                NULL,            /* "attributes" (unused) */
                consumer_thread, /* procedure */
                (void *)kount);  /* hack: consumer number */

            assert(test == 0);
        }

        producer(nconsumers, offset, chunksize);
        //        * n. clean up: the producer told all the consumers to shut down (by sending -1 to each).  Now wait for them all to finish.
        for (kount = 0; kount < nconsumers; kount++)
        {
            int test = pthread_join(consumers[kount], NULL);

            assert(test == 0);
        }
        //buffer_clean();
        printf("producer: read EOF, sending %d '-1' numbers\n", nconsumers);
        
    }
    
    if(myid >0){
    /* Receive my portion of array from the master task */
        source = 0;
        MPI_Recv(&offset, 1, MPI_INT, source, tag1, MPI_COMM_WORLD, &status);
        MPI_Recv(&chunksize, 1, MPI_INT, source, tag2, MPI_COMM_WORLD, &status);
        MPI_Recv(&data[offset], chunksize, MPI_INT, source, tag3, MPI_COMM_WORLD, &status);

        printf("in process %d, offset = %d\n",myid,offset);
        printf("in process %d, chunksize = %d\n",myid,chunksize);

        /* Do my part of the work */
        printf("main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);
        printf("main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);
          
        //       * 1. initialization
        buffer_init();
        signal(SIGALRM, SIG_IGN);     /* evil magic for usleep() under solaris */

        // 2. start up N consumer threads
        consumers = (pthread_t *)alloca(nconsumers * sizeof(pthread_t));
        for (kount = 0; kount < nconsumers; kount++)
        {
            int test = pthread_create(&consumers[kount], /* pthread number */
                NULL,            /* "attributes" (unused) */
                consumer_thread, /* procedure */
                (void *)kount);  /* hack: consumer number */

            assert(test == 0);
        }

        // 3. run the producer in this thread.
        producer(nconsumers, offset, chunksize);

        /* clean up: the producer told all the consumers to shut down */
        for (kount = 0; kount < nconsumers; kount++)
        {
            int test = pthread_join(consumers[kount], NULL);

            assert(test == 0);
        }
        //buffer_clean();
        printf("producer: read EOF, sending %d '-1' numbers\n", nconsumers);
    }

    /* Finalize and cleanup */
    MPI_Finalize();
    return(0);
}
