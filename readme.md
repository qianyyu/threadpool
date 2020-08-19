# Assignment 2  —  MapReduce Library
The second assignment(a.k.a MapReduce Library) aims to write some functions for MapReduce. To use the MapReduce infrastructure, developers need to write just a little bit of code in addition to their program. They do not need to worry about how to parallelize their program; the MapReduce runtime will make this happen!
# Some Related Question about this assignment:


### How to store the intermediate key/value pairs
- I choose to use global variable to store the intermediate key/value pairs, since this is probably the most straightforward way to implement it.

### The time complexity analysis of MR Emit and MR GetNext functions
- MR_Emit:

	Suppose there are **n** words in total, the average length of words is **k**.
	  
	For each word, the function will call MR_partition () and push_back()
	  
	MR_Partition() need to calculate the summation of ascii value of each character in the word. It takes O(k).
	  
	pushback is to insert an element at end of vector. It takes O(1)
	  
	For n words, It takes O(kn) in total.
	  
	  
- MR_GetNext:

	I choose to use min Heap (priority queue) to implement this algorithm.
	  
	Suppose There are n words in total.
	  
	For min Heap, top() function and pop() function takes O(logn)
	  
	If I want to traversal all words, I have to call pop() function n times.
	  
	That is, for n words, It takes O(nlogn) In total.

### The data structure used to implement the task queue in the thread pool library
- I choose to use __queue__ (the queue in C++ STL) as the data structure for my task queue. The assignment ask us to use longest job first policy, and I always push the longest job into queue first. Thus, If I want to pop the longest job out, I need to pop the first element out. This task needs a first in first out data structure.  
### Implementation of the thread pool library

### The synchronization primitives I used
- pthread_mutex_init
- pthread_cond_init
- pthread_mutex_lock
- pthread_mutex_unlock
- pthread_cond_wait
- pthread_cond_broadcast
- pthread_mutex_destroy
- pthread_cond_destroy


### How I tested the correctness of your code
For the MapReduce Function part:
- I download the test case and script files from discussion forum, which is very helpful. 


For the Thread pool part:
- This part is hard to test by using test cases, because you need to think about deadlock, conflict .etc. Thus, I wrote many different possible case by hand. Making sure there is no bug.

### The file I have uploaded:
- distwc.cc 
- mapreduce.cc 
- mapreduce.h 
- threadpool.cc 
- threadpool.h 
- readme.md 
- Makefile

### Sources:
[ CMPUT 379 Lab 4](https://eclass.srv.ualberta.ca/pluginfile.php/5376221/mod\_resource/content/2/379%20Lab%204.pdf) ( Lab TAs and/or Instructor, n.d.)

[ CMPUT 379 Lab 5](https://eclass.srv.ualberta.ca/pluginfile.php/5382577/mod\_resource/content/2/379%20Lab%205.pdf) ( Lab TAs and/or Instructor, n.d.)

[ CMPUT 379 Lab 6](https://eclass.srv.ualberta.ca/pluginfile.php/5397652/mod\_resource/content/1/379%20Lab%206.pdf) ( Lab TAs and/or Instructor, n.d.)

[Test Cases and Scripts](https://eclass.srv.ualberta.ca/mod/forum/discuss.php?d=1243730) (Jihoon Og)

[ C++ priority queue ]()(https://en.cppreference.com/w/cpp/container/priority\_queue) 

