/* clocktest.c - check for clock jitter on SMP machines */
/*
 * # cc -lrt -o myclocktest myclocktest.c
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <signal.h>

#define __USE_GNU 1
#include <sched.h>

#define NSEC_PER_SEC    1000000000
#define MAX_JITTER      (double)0.2
#define ITERATIONS      10000
#define MAX_CPU		24
#define NSEC(ts) (ts.tv_sec*NSEC_PER_SEC + ts.tv_nsec)

#ifdef OLD_SCHED_SETAFFINITY
#define setaffinity(mask) sched_setaffinity(0,&mask)
#else
#define setaffinity(mask) sched_setaffinity(0,sizeof(mask),&mask)
#endif
#define SIGRT SIGRTMIN

int get_clocks(unsigned int cpu);

struct timespec *t;
pthread_cond_t *cond;
pthread_mutex_t *mutex;
int t_id, cond_id, mutex_id;
pid_t *pid;

int get_clocks(unsigned int cpu) {
    cpu_set_t cpumask;
    unsigned int affinity;
    int status = 0;
    struct sched_param param;
    int policy;

    /* Bind this process to the CPU */
    CPU_ZERO(&cpumask); CPU_SET(cpu,&cpumask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpumask) < 0) {
        perror ("sched_setaffinity"); 
        return 1; 
    }
    sched_yield();

    /* Set the scheduler policy to RR (2) and priority to max */
    pid[cpu] = getpid();
#ifdef DEBUG
    policy = sched_getscheduler(pid[cpu]);
    sched_getparam(pid[cpu], &param);
    printf("cpu %d pid %d Current policy %d Current priority %d\n", 
             cpu, pid[cpu], policy, param.sched_priority);
#endif 
    param.sched_priority = sched_get_priority_max(SCHED_RR);
    if(sched_setscheduler(pid[cpu], SCHED_RR, &param) == -1) {
        perror("sched_setscheduler");
        exit(1); 
    }
#ifdef DEBUG
    policy = sched_getscheduler(pid[cpu]);
    sched_getparam(pid[cpu], &param);
    printf("cpu %d pid %d New policy %d New priority %d\n", 
             cpu, pid[cpu], policy, param.sched_priority);
#endif 

    while (1) {     
        pthread_mutex_lock(&mutex[cpu]);
        pthread_cond_wait(&cond[cpu], &mutex[cpu]);

        if (clock_gettime(CLOCK_REALTIME, &t[cpu]) < 0) {
                perror("clock_gettime");
                return 1;
        }
        pthread_mutex_unlock(&mutex[cpu]);
   }
}
int test_clock_jitter() {
    unsigned long nsec;
    unsigned slow_cpu, fast_cpu;
    cpu_set_t cpumask;
    double jitter;
    unsigned cpu, num_cpus, iter;
    int failures = 0;
    double largest_jitter = 0.0;
    pthread_t thread[MAX_CPU];
    int status;
    
    num_cpus = sysconf(_SC_NPROCESSORS_CONF);
    if (num_cpus == 1) {
        printf("Single CPU detected. No clock jitter testing necessary.\n");
        return 0;
    }
    /* Create an array for storring pid of child processes */
    pid = (pid_t *)malloc(num_cpus * sizeof(pid_t));
    /* Create the shared memory variables */ 
    if ((t_id = shmget(0x34, num_cpus * sizeof(struct timespec), 0777 | IPC_CREAT)) == -1)
        { perror("shmget[t]"); return 1; }
    if ( (unsigned)(t = (struct timespec *)shmat(t_id, NULL, 0)) == -1)  { perror("shmat[t]"); return 1; }
 
    if ((cond_id = shmget(0x35, num_cpus * sizeof(pthread_cond_t), 0777 | IPC_CREAT)) == -1)
        { perror("shmget[cond]"); return 1; }
    if ( (unsigned)(cond = (pthread_cond_t *)shmat(cond_id, NULL, 0)) == -1)  { perror("shmat[cond]"); return 1; }

    if ((mutex_id = shmget(0x36, num_cpus * sizeof(pthread_mutex_t), 0777 | IPC_CREAT)) == -1)
        { perror("shmget[mutex]"); return 1; }
    if ( (unsigned)(mutex = (pthread_mutex_t *)shmat(mutex_id, NULL, 0)) == -1)  { perror("shmat[mutex]"); return 1; }

    /* initialize the condition and mutex veriables */
    for (cpu=0; cpu < num_cpus; cpu++) {
    	pthread_mutex_init(&mutex[cpu], NULL);
    	pthread_cond_init(&cond[cpu], NULL);
    }
   
    /* Fork a process for each cpu will be tied to it and 
       retrieve time frequently. */
    for (cpu=0; cpu < num_cpus; cpu++) {
        if ((pid[cpu] = fork()) == 0) {
            get_clocks(cpu);
        }
    }
    
    /* Give 1 sec for child processes to initialize */
#ifdef DEBUG
    printf("Waiting for spawning processes...\n");
#endif
    sleep(1); 

    /* Signal the condition variable so that all processes can retrieve 
       the first snapshot of cpu time */ 
    for (cpu=0; cpu < num_cpus; cpu++) 
          pthread_cond_broadcast(&cond[cpu]); 
   
    /* Wait for sometime to let all processes take the first snapshot */
#ifdef DEBUG
    printf("Waiting for the first snapshot of cpu time...\n");
#endif
    usleep(1);
    printf ("Testing for clock jitter on %u cpus\n", num_cpus);

    for (iter=0; iter<ITERATIONS; iter++) {
#ifdef DEBUG
        printf("\nITERATION [%d]\n", iter);
#endif
        for (cpu=0; cpu < num_cpus; cpu++) 
            pthread_mutex_lock(&mutex[cpu]);
        slow_cpu = fast_cpu = 0; 
        for (cpu=0; cpu < num_cpus; cpu++) {
            nsec = NSEC(t[cpu]);
#ifdef DEBUG
            printf("cpu %d time( %d sec  %d nsec)\n", cpu, t[cpu].tv_sec, t[cpu].tv_nsec); 
#endif
            if (nsec < NSEC(t[slow_cpu])) { slow_cpu = cpu; }
            if (nsec > NSEC(t[fast_cpu])) { fast_cpu = cpu; }
        }
        jitter = ((double)(NSEC(t[fast_cpu]) - NSEC(t[slow_cpu])) 
                  / (double)NSEC_PER_SEC);
    	if (jitter > MAX_JITTER || jitter < -MAX_JITTER) {
	    printf ("ERROR, jitter = %f, cpus = %u,%u\n", jitter, slow_cpu, fast_cpu);
            failures++;
    	}
        else {
#ifdef DEBUG
	    printf ("PASSED, jitter = %f, cpus = %u,%u\n", jitter, slow_cpu, fast_cpu);
#endif
        }
        if (jitter > largest_jitter)
            largest_jitter = jitter;
        for (cpu=0; cpu < num_cpus; cpu++) 
            pthread_mutex_unlock(&mutex[cpu]);
        for (cpu=0; cpu < num_cpus; cpu++) 
            pthread_cond_broadcast(&cond[cpu]); 
        /* Wait some time to avoid this process from competing for
           mutex again immediately */
        usleep(1);   
    }

    if (failures == 0)
        printf ("PASSED, largest jitter seen was %lf\n",largest_jitter);
    else
        printf ("FAILED, %u iterations failed\n",failures);
    for(cpu=0; cpu<num_cpus; cpu++) {
        kill(pid[cpu], SIGTERM);
        waitpid(pid[cpu], &status, 0);
    }
    return (failures > 0);
}

void handle_interrupt(int signum) {
       shmctl(t_id, IPC_RMID, NULL);
       shmctl(cond_id, IPC_RMID, NULL);
       shmctl(mutex_id, IPC_RMID, NULL);
       exit(0);
}

int main()
{
        int status;

        signal(SIGINT, handle_interrupt);
	int failures = test_clock_jitter();
        shmctl(t_id, IPC_RMID, NULL);
        shmctl(cond_id, IPC_RMID, NULL);
        shmctl(mutex_id, IPC_RMID, NULL);
	return failures > 0;
}
