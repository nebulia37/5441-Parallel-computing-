#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

pthread_barrier_t   barrier; // the barrier synchronization object

void * thread1 (void *not_used)
{
    int rc = 0;
    time_t  now;
    char    buf [27];

    time (&now);
    fprintf(stderr, "thread1 starting at %s", ctime_r (&now, buf));

    // do the computation
    // let's just do a sleep here...
    sleep (5);
    time (&now);
    fprintf(stderr, "thread1 entering barrier at %s", ctime_r (&now, buf));
    rc = pthread_barrier_wait (&barrier);
    if (rc ==  PTHREAD_BARRIER_SERIAL_THREAD) {
        fprintf(stderr, "thread1 elected to do serial portion\n");
    }
    // after this point, all three threads have completed.
    time (&now);
    fprintf(stderr, "barrier in thread1() done at %s", ctime_r (&now, buf));

    pthread_exit(not_used);
}

void * thread2 (void *not_used)
{
    int rc = 0;
    time_t  now;
    char    buf [27];

    time (&now);
    fprintf(stderr, "thread2 starting at %s", ctime_r (&now, buf));

    // do the computation
    // let's just do a sleep here...
    sleep (10);
    time (&now);
    fprintf(stderr, "thread2 entering barrier at %s", ctime_r (&now, buf));
    rc = pthread_barrier_wait (&barrier);
    if (rc ==  PTHREAD_BARRIER_SERIAL_THREAD) {
        fprintf(stderr, "thread2 elected to do serial portion\n");
    }
    // after this point, all three threads have completed.
    time (&now);
    fprintf(stderr, "barrier in thread2() done at %s", ctime_r (&now, buf));

    pthread_exit(not_used);
}

void main ()
{
    int rc = 0;
    time_t  now;
    char    buf [27];
    void *status;
    pthread_attr_t attr;
    pthread_t threads[2];

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // create a barrier object with a count of 3
    pthread_barrier_init (&barrier, NULL, 3);

    // start up two threads, thread1 and thread2
    pthread_create (&threads[0], &attr, thread1, NULL);
    pthread_create (&threads[1], &attr, thread2, NULL);

    // at this point, thread1 and thread2 are running

    // now wait for completion
    time (&now);
    fprintf(stderr, "main () waiting for barrier at %s", ctime_r (&now, buf));
    rc = pthread_barrier_wait (&barrier);
    if (rc ==  PTHREAD_BARRIER_SERIAL_THREAD) {
        fprintf(stderr, "main () elected to do serial portion\n");
    }

    // after this point, all three threads have completed.
    time (&now);
    fprintf(stderr, "barrier in main () done at %s", ctime_r (&now, buf));

    pthread_join(threads[0], &status);
    pthread_join(threads[1], &status);

    time (&now);
    fprintf(stderr, "Main completed at %s", ctime_r (&now, buf));
}