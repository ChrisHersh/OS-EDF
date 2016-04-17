#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <limits.h>

/*
* Lab 5: Thread Scheduling
* Programmer: Chris Hersh
* Course: CMPE 320
* Section: 2 (11-12:50pm)
* Instructor: S. Lee
*/

#define MAX_THREADS 100

//Declare long list of global variables

int threads_running = 1;

int time_count = 1;
int num_threads = 1;
int current_thread = -1;

sem_t timer;
sem_t ready[MAX_THREADS];

int burst_times[MAX_THREADS];
int period_times[MAX_THREADS];

int next_period[MAX_THREADS];
int finished_current_period[MAX_THREADS];

int base_sleep = 1;

int time_limit;

int cpu_idling = 0;

//Check how many threads are not done
int number_of_running_threads()
{
    int count = 0;
    int i = 0;
    for(; i < 10; i++)
    {
        if(!done_threads[i])
            count++;
    }
    return count;
}

//Get the next earliest deadline
int get_next_thread()
{
    int next_deadline = INT_MAX;
    int next_deadline_thread = INT_MAX;  
    int i;
    for(i = 0; i < num_threads; i++)
    {
        if(next_period < next_deadline && finished_current_period[i] == 0)
        {
            next_deadline = next_period;
            next_deadline_thread = i;
        }
    }
    
    return next_deadline_thread;
}

//Check to see if a thread has entered the next period 
//If it has then set it so it can run again
void check_threads_for_new_period()
{
    int next_deadline = INT_MAX;
    int next_deadline_thread = INT_MAX;  
    int i;
    for(i = 0; i < num_threads; i++)
    {
        if(next_period == time_count && finished_current_period[i] == 1)
        {
            next_deadline = next_period;
            next_deadline_thread = i;
        }
        //Shouldn't hit, means we missed a deadline
        else if(next_period == time_count && finished_current_period[i] == 0)
        {
            printf("WE MISSED A DEADLINE");
        }
    }
}

void cpu_idle()
{
    if(cpu_idling == 0)
    {
        cpu_idling = 1;
        printf("CPU is now idling\n");
    }
    
}

void exit_cpu_idle()
{
    if(cpu_idling == 1)
    {
        if(get_next_thread() != -1)
        {
            current_thread == get_next_thread();
        }
        else
        {
            cpu_idle();
        }
    }
}

//Main timer function
//Pretty much deals with anything time related
void *timer_thread(void *param)
{
    //Give the threads some time to startup before the timing starts
    nanosleep((const struct timespec[]) {{0, 5000000L}}, NULL);
    sem_post(&timer);
    
    while(threads_running)
    {
        int i;
        
       
        //Used to help prevent race conditions, mainly with printing
        nanosleep((const struct timespec[]) {{0, 5000000L}}, NULL);

        printf("%d\n", time_count);
        time_count ++;

        //Update the threads available to be run
        check_threads_for_new_period();

        //update remaining burst times
        if(current_thread != -1)
        {
            burst_times[current_thread]--;
            //Check if the worker thread should be finished
            if(burst_times[current_thread] <= 0)
            {
            next_period[current_thread] += period_times[current_thread];
            finished_current_period[current_thread] = 1; 
            
            sem_post(&timer);
                
                printf("\tThread %d finishes it's job. It will be terminated\n", current_thread);
            }
        }
        if(time_count == time_limit)
        {
            threads_running = 0;
            break;
        }
    }

    return NULL;
}

//Main scheduler thread
//Deals with the scheduling functionality that is not timing related
void *sched(void *param)
{
    while(threads_running)
    {
        sem_wait(&timer);
        current_thread += 1;
        int next_thread = -1;
        int i = current_thread;
        int x = 0;

        

        //Find next thread that needs to be run
/*        for(; i < (num_threads + current_thread) && x < num_threads; i++, x++)
        {
            if(i >= num_threads)
                i = i % num_threads;
            if(burst_times[i] > 0)
            {
                next_thread = i;
                break;
            }
        } */
        
        next_thread = get_next_thread();

/*        //Make sure threads still need to be run
        if(next_thread == -1 || number_of_running_threads() == 0)
        {
            printf("\tAll threads have done their work. The scheduler thread exits.\n");
            threads_running = 0;
            break;
        } */

        //Allow thread to run
        sem_post(&ready[next_thread]);
    }    
    int i;
    for(i = 0; i < 10; i++)
        sem_post(&ready[i]);

    return NULL;
}

//Main work thread, doesn't do much
void *work(void *param)
{
    int number = *((int *)param);
    while(threads_running && !finished_current_period[number])
    {
        sem_wait(&ready[number]);
        if(finished_current_period[number])
            break;
        current_thread = number;
        printf("\tThread %d is now being executed\n", number);
    }    

    return NULL;
}

//Main function
int main(int argc, char** argv)
{
    
    //Take input with some error checking
    if(argc < 3)
    {
        printf("Correct usage is <command> <number of threads> <time quantum>\n");
        exit(1);
    }

    num_threads = atoi(argv[1]);
    time_quantum = atoi(argv[2]);
    
    //Poor lonely index
    int i;
    
    //Init burst times and done status
    for(i = 0; i < 10; i++)
    {
        burst_times[i] = 0;
        finished_current_period[i] = 0;
    }

    //Set threads that are not being used to be done
    for(i = num_threads; i < 10; i++)
    {
        finished_current_period[i] = 1;
    }

    //Take input for the burst times
    for(i = 0; i < num_threads; i++)
    {
        printf("Burst time for Thread %d: ", i);
        scanf("%d", &burst_times[i]);
    }

    //Init each ready semaphore
    for(i = 0; i < num_threads; i++)
    {
        sem_init(&ready[i], 0, 0);
    }
    //Init timer semaphore
    sem_init(&timer, 0, 0);
    
    //Declare pthread types
    pthread_t timer_tid, sch_tid, work_tid[num_threads];
    pthread_attr_t timer_attr, sch_attr, work_attr;

    //Set up the attribute for the workers
    int policy = SCHED_RR;
    pthread_attr_init(&work_attr);
    pthread_attr_setscope(&work_attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setschedpolicy(&work_attr, policy);
    
    //Create each worker
    int nums[] = {0,1,2,3,4,5,6,7,8,9};    
    for(i = 0; i < num_threads; i++)
    {        
        pthread_create(&work_tid[i], &work_attr, work, (void *)&nums[i]);
    }

    //Create timer and scheduler
    pthread_attr_init(&timer_attr);
    pthread_create(&timer_tid, &timer_attr, timer_thread, NULL);

    pthread_attr_init(&sch_attr);
    pthread_create(&sch_tid, &sch_attr, sched, NULL);
    
    //Wait for each thread to finish
    for(i = 0; i < num_threads; i++)
    {
        pthread_join(work_tid[i], NULL);
    }

    pthread_join(timer_tid, NULL);
    pthread_join(sch_tid, NULL);

}
