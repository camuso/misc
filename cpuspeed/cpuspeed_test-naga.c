#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

#define MAX_CPUS 256
#define BUFFSIZE 1024

int cpufreq_read(int cpu, char *filename){
	char sys_filename[BUFFSIZE];
	char buffer[BUFFSIZE];
	int fd;
	int value;

	sprintf(sys_filename,"/sys/devices/system/cpu/cpu%d/cpufreq/%s",
			 cpu, filename);

	fd = open(sys_filename, O_RDONLY);
	if (fd < 0){
		perror ("open");
		fprintf (stderr, "unable to open %s\n", sys_filename);
		exit (1);
	}

	read(fd, buffer, BUFFSIZE);
	close(fd);
	value = atoi(buffer);

	return value;
}


void worker_child(int cpu)
{
	int cur_freq;
	int min_freq;
	int max_freq;
	int last_freq;
	cpu_set_t mask;
	int i;
	double x;
	/*
	 * bind this thread to the specified cpu 
	 */
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	sched_setaffinity(0, CPU_SETSIZE, &mask);

	min_freq = cpufreq_read(cpu, "cpuinfo_min_freq");
	max_freq = cpufreq_read(cpu, "cpuinfo_max_freq");
	last_freq = 0;

	while (1){
		cur_freq = cpufreq_read(cpu, "scaling_cur_freq");
		if (cur_freq != last_freq)
			printf ("CPU%d: %d\n",cpu, cur_freq);
		last_freq = cur_freq;

		if (cur_freq == min_freq){
			for (i=0; i<100; i++)
				x = sqrt(x);
		} else {
			sleep(1);
		}
	}

	exit(0);

}


main(){
	cpu_set_t mask;
	int i;

	if (sched_getaffinity(0, CPU_SETSIZE, &mask) < 0){
		perror ("sched_getaffinity");
		exit(1);
	}

	for (i=0; i<1; i++)
		if (CPU_ISSET(i, &mask)){
			printf ("CPU%d\n",i);
			if (fork() == 0)
				worker_child(i);
		}


	wait(NULL);
	exit (0);
}





