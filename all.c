#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <linux/input.h>

//declaring condition broadcast variables
pthread_cond_t sync_start, sync_start_line2, sync_start_line4;
//declaring mutexes which will be used to sync broadcast
pthread_mutex_t sync_start_mutex;
//declaring variables to handle env varibale for LOG's path and MOUSE's input file
char *logpath, *LOG_OUT, *MOUSE_PATH;

//Function responsible for executing periodic tasks
int periodic_task(int limit1, int limit2, int prio){
    
	int i;
	int j=0;
	struct timespec START, STOP, ELAPSED;
	FILE *fwrite;
	fwrite = (fopen(LOG_OUT, "a"));
	time_t start = time(NULL);
	fprintf(fwrite, "I am prio %d thread and I started at: %s", prio, ctime(&start));
	clock_gettime(CLOCK_REALTIME, &START);
	for(i=limit1; i<=limit2; i++)
	{
		j=j+1;
	}
	clock_gettime(CLOCK_REALTIME, &STOP);
	start = time(NULL);
	ELAPSED.tv_nsec = STOP.tv_nsec - START.tv_nsec; //calculating elapsed time in nanoseconds
	fprintf(fwrite, "I am prio %d. I made J = %d and ended at: %s. I took %ld nanosecs. \n \n", prio, j, ctime(&start), ELAPSED.tv_nsec);
	fclose(fwrite);
	return 0;
}

//Function responsible for executing aperiodic tasks
int aperiodic_task(int limit1, int limit2){

	int i;
	int j=0;
	struct timespec START, STOP, ELAPSED;
	FILE *fwrite;
	fwrite = (fopen(LOG_OUT, "a"));
	time_t start = time(NULL);
	fprintf(fwrite, "I am an Aperiodic Task and I started at: %s", ctime(&start));
	clock_gettime(CLOCK_REALTIME, &START);
	for(i=limit1; i<=limit2; i++)
	{
		j=j+1;
	}
	clock_gettime(CLOCK_REALTIME, &STOP);
	start = time(NULL);
	ELAPSED.tv_nsec = STOP.tv_nsec - START.tv_nsec;
	fprintf(fwrite, "I am an Aperiodic Task and I ended at: %s. I took %ld nanosecs. \n \n", ctime(&start), ELAPSED.tv_nsec);
	fclose(fwrite);
	return 0;

}

//thread used to detect mouse's event
void *mouse_detection(void *arg){

    int fd, bytes;
    unsigned char data[3];	
    int left,right;
    int flag_left = 0;
    int flag_right = 0;

    fd = open(MOUSE_PATH, O_RDWR);
    if(fd == -1)
    {
        printf("%s could not be opened. ERROR! \n", MOUSE_PATH);
    }

    while(1)
    {    
        bytes = read(fd, data, sizeof(data));
        if(bytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;

		if(left==1){
			flag_left = 1;		
		}	
		if(flag_left == 1 && left == 0){
			aperiodic_task(500, 500); //aperiodic function call on left released
			flag_left = 0;
		}
		if(right==2){
			flag_right = 1;		//aperiodic function call on right released
		}	
		if(flag_right == 1 && right == 0){
			aperiodic_task(500, 600);
			flag_right = 0;
		}	
        }   
    }
    close(fd);
    pthread_exit(NULL);

}

//thread with highest priority
void *line3_thread(void *arg){
	
	pthread_mutex_lock(&sync_start_mutex);
	pthread_cond_wait(&sync_start, &sync_start_mutex); //conditional wait on broadcast from main()
	pthread_cond_broadcast(&sync_start_line2); //broadcast for medium priority thread
	pthread_mutex_unlock(&sync_start_mutex);
	while(1){
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); // disabling pthread_cancel()
	periodic_task(100,300,10); //call to periodic function
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //enabling pthread_cancel()
	usleep(200*1000);
	}
	pthread_exit(NULL);

}

void *line2_thread(void *arg){

	pthread_mutex_lock(&sync_start_mutex);
	pthread_cond_wait(&sync_start_line2, &sync_start_mutex); //conditional wait on broadcast from line3_thread (highest priority)
	pthread_cond_broadcast(&sync_start_line4); //broadcast for least priority thread
	pthread_mutex_unlock(&sync_start_mutex);
	while(1){
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); // disabling pthread_cancel()
	periodic_task(200,400,20); //call to periodic function
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  //enabling pthread_cancel()
	usleep(500*1000);
	}
	pthread_cond_destroy(&sync_start_line2); //destroying conditional variable
	pthread_exit(NULL);

}


void *line4_thread(void *arg){
	
	pthread_mutex_lock(&sync_start_mutex);
	pthread_cond_wait(&sync_start_line4, &sync_start_mutex); //conditional wait on broadcast from line4_thread (medium priority)
	pthread_mutex_unlock(&sync_start_mutex);
	while(1){
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); //disabling pthread_cancel()
	periodic_task(500,500,30); //call to periodic function
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //enabling pthread_cancel()
	usleep(750*1000);
	}
	pthread_cond_destroy(&sync_start_line4); //destroying conditional variable
	pthread_exit(NULL);

}

int main(){

	logpath = getenv("LOG_PATH"); //getting LOG_PATH env variable that will be set in env using readme file
	MOUSE_PATH = getenv("MOUSE_INPUT"); //getting MOUSE_INPUT from env which will be set using readme
	LOG_OUT = strcat(logpath, "/log.txt"); //concatinating with log file's name

	pthread_cond_init(&sync_start,NULL);
	pthread_cond_init(&sync_start_line2,NULL);
	pthread_cond_init(&sync_start_line4,NULL);

	//declaring CPU affinity to 0
	cpu_set_t cpuset; 
        CPU_ZERO(&cpuset);
        CPU_SET(0, &cpuset);

	//thread names	
	pthread_t mouse_check;	
	pthread_t line2;
	pthread_t line3;
	pthread_t line4;

	//attributes for threads
	pthread_attr_t attr_for_mouse; 
	pthread_attr_t attr_for_line2;
	pthread_attr_t attr_for_line3;
	pthread_attr_t attr_for_line4;

	//priorities for threads
	int prio_mouse = 199; 
	int prio_line2 = 20;
	int prio_line3 = 10;
	int prio_line4 = 40;

	//scheduled params for threads
	struct sched_param p_mouse; 
	struct sched_param p_line2;
	struct sched_param p_line3;
	struct sched_param p_line4;

	//initialization of attributes
	pthread_attr_init (&attr_for_mouse);  
	pthread_attr_init (&attr_for_line2);
	pthread_attr_init (&attr_for_line3);
	pthread_attr_init (&attr_for_line4);

	//setting priorities in params for scheduling
	p_mouse.sched_priority = prio_mouse; 
	p_line2.sched_priority = prio_line2;
	p_line3.sched_priority = prio_line3;
	p_line4.sched_priority = prio_line4;

	//setting scheduling params for threads:
	pthread_attr_setschedparam (&attr_for_mouse, &p_mouse); 
	pthread_attr_setschedpolicy(&attr_for_mouse, SCHED_FIFO);

	pthread_attr_setschedparam (&attr_for_line2, &p_line2);
	pthread_attr_setschedpolicy(&attr_for_line2, SCHED_FIFO);

	pthread_attr_setschedparam (&attr_for_line3, &p_line3);
	pthread_attr_setschedpolicy(&attr_for_line3, SCHED_FIFO);

	pthread_attr_setschedparam (&attr_for_line4, &p_line4);
	pthread_attr_setschedpolicy(&attr_for_line4, SCHED_FIFO);

	time_t start = time(NULL); //start time for code
	time_t stop = start + 5; //what should be the end time for code

	//creation of threads  with defined attributes
	pthread_create (&mouse_check, &attr_for_mouse, &mouse_detection,NULL); 
	pthread_setaffinity_np(mouse_check,sizeof(cpu_set_t),&cpuset);	

	pthread_create (&line3, &attr_for_line3, &line3_thread, NULL);
	pthread_setaffinity_np(line3,sizeof(cpu_set_t),&cpuset);
	usleep(1000); //1 ms delay between thread creation to ensure that always highest priority thread leads with execution. Even without this prremption of low priority happens when high priority thread comes.

	pthread_create (&line2, &attr_for_line2, &line2_thread, NULL);
	pthread_setaffinity_np(line2,sizeof(cpu_set_t),&cpuset);
	usleep(1000);

	pthread_create (&line4, &attr_for_line4, &line4_thread, NULL);
	pthread_setaffinity_np(line4,sizeof(cpu_set_t),&cpuset);
	usleep(1000);

	pthread_cond_broadcast(&sync_start); //broadcast for thread with highest priority to start execution

	//condition to check if program execution time has been exceeded or not
	enter_if:
	if(stop > start){
	
	start = time(NULL);
	goto enter_if;
	}
	else{
	//cancelling threads. If pthread_SETCANCELSTATE is enabled only then it will be killed.
	pthread_cancel(mouse_check); 
	pthread_cancel(line2);
	pthread_cancel(line3);
	pthread_cancel(line4);
	}

	pthread_mutex_destroy(&sync_start_mutex); //destroying mutex
	pthread_cond_destroy(&sync_start); //destroying broadcast variable for highest priority thread.

	return 0;
}
