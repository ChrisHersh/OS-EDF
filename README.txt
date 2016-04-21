/*
* Project 2: CPU Scheduling
* Programmer: Chris Hersh
* Course: CMPE 320
* Section: 2 (11-12:50pm)
* Instructor: S. Lee
*/

Algorithm:
	On startup it will ask for the thread information, after that it will init everything that needs it and start the threads. 

	Timer:
		The timer will keep track of the time. For every time unit it will sleep for a short amount of time (5000000 ns, or ~5ms) to prevent race conditions with the other threads, it mainly exists so the output will be in the right order. Then it will update and output the current time, and it will also update the remaining burst time for the current thread. This is handled by the timer as it's a time related function and it was easiest to place in the timer, and it also makes it easy to handle the case when the burst time is less than the time quantum. Since the timer handles updating the burst times, it also handles the case when the burst times reach 0, or when the threads should finish for that period.

	Scheduler:
		The Scheduler is mainly in charge of organizing the worker threads. It will wait for the timer thread to update the timer semaphore, then it figures out which thread should be next for processing based on which threads are still able to run and their next deadlines. When it figures out the next thread it updates it's semaphore, and I'm using multiple semaphores just to handle the queueing better. Once the scheduler is finished it will post each ready semaphore once to free up any worker threads that are waiting on a semaphore still.

	Worker:
		These mainly just wait for a lock, then start waiting for the lock again pretending they are actually doing work.

Description:
	This program will take a number of threads, then ask for the burst time and period for each and the total time running. Then it will run each thread in an earliest deadline first fashion and output the entire process.

Analysis:
	This algorithm seems decent to me, in terms of speed it could run faster without the sleeps, but the sleeps also help reduce the chances of a race condition. The workers can sometimes get confused and think they still have stuff to do, but that's why I post to each semaphore before ending the scheduler, but other than that I feel like the responsibilities for the scheduler and timer are what they should be with the workers not doing anything.

	As for the output, mine appears to be about the same as the example in the lab document.

chris@chris-VirtualBox:/media/sf_SharedFolder/OS-EDF$ ./edf 3
Burst time for Thread 0: 10
Burst time for Thread 1: 15
Burst time for Thread 2: 5
Period for Thread 0: 30
Period for Thread 1: 40
Period for Thread 2: 50
How long do you want to execute this program (sec): 70
	Thread 0 is now being executed
1
2
3
4
5
6
7
8
9
10
	Thread 1 is now being executed
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
	Thread 2 is now being executed
26
27
28
29
30
	Thread 0 is now being executed
31
32
33
34
35
36
37
38
39
40
	Thread 1 is now being executed
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
	Thread 2 is now being executed
56
57
58
59
60
	Thread 0 is now being executed
61
62
63
64
65
66
67
68
69
Killed
