// Standard Library and IO:
#include <cstdio>
#include <cstdlib>
#include <iostream>

// C Head File
#include <stdio.h>
#include <stdlib.h>

// Data Structure:
#include <vector>
#include <string>
#include <set>
#include <cstring>
#include <queue>

// POSIX
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include <pthread.h>

// Exception:
#include <assert.h>

// Head Files:
#include "mapreduce.h"
#include "threadpool.h"

using namespace std;



/*
* 
* A C style constructor for creating a new ThreadPool object
* Parameters:
*     num - The number of threads to create
* Return:
*     ThreadPool_t* - The pointer to the newly created ThreadPool object
*/
ThreadPool_t *ThreadPool_create(int num){
	ThreadPool_t* tp = (ThreadPool_t*) malloc(sizeof(ThreadPool_t));
	if(tp == NULL){
		cerr<<"Malloc error - tp"<<endl;
	}

	// Mutex and Cond:
	// Initializing: 
	if( pthread_mutex_init(&tp->mutex,NULL)!=0 ||
		pthread_cond_init(&tp->cond,NULL) != 0){
			cerr << "Initialization Error" << endl;
			return NULL;
		} 

	// Set Default Value
	// is_finish is false at the begining
	tp->is_finish = false;

	ThreadPool_work_queue_t* twq = (ThreadPool_work_queue_t*) malloc(sizeof(ThreadPool_work_queue_t));
	if(twq == NULL){
		cerr << "Malloc Error - twq " <<endl;
	}
	tp->queue_task = twq;

	// Create Vector of threads:
	for (int i = 0; i < num; ++i){
		pthread_t *threads2 = new pthread_t;
		tp->threads.push_back(threads2);
		if( pthread_create(threads2,nullptr, 
				(void *(*)(void *))Thread_run, tp)!=0){
					cerr << "Creation Error" <<endl;
					ThreadPool_destroy(tp);
					return NULL;
				}
	}

	return tp;
}

/**
 * Description: 
 * 		If there is an idle mapper thread in the thread pool,
 * 		it starts processing the job right away by invoking 
 * 		the user-defined Map function with the passed argument 
 * 		(i.e., the filename). Otherwise, the job is added to a
 * 		queue to be processed later by an idle mapper thread
 * 		Add a task to the ThreadPool's task queue
 * Parameters:
 *     tp   - The ThreadPool object to add the task to
 *     func - The function pointer that will be called in the thread
 *     arg  - The arguments for the function
 * Return:
 *     true  - If successful
 *     false - Otherwise
 */

bool ThreadPool_add_work(ThreadPool_t *tp, thread_func_t func, void *arg){
	ThreadPool_work_t *task = (ThreadPool_work_t*) malloc(sizeof(ThreadPool_work_t));
	if(task == NULL || tp == NULL){
		cerr << "malloc error "<<endl;
		return false;
	}
	if(pthread_mutex_lock(&(tp->mutex)) != 0) {
		return false;
	}
	task -> func = func;
	task -> arg = arg;
	tp->queue_task->qTask.push(*task);
	free(task);

	if(pthread_mutex_unlock(&(tp->mutex)) != 0){
		return false;
	}
	return true;
}


/**
* Get a task from the given ThreadPool object
* Parameters:
*     tp - The ThreadPool object being passed
* Return values:
*     ThreadPool_work_t* - The next task to run
*/

ThreadPool_work_t *ThreadPool_get_work(ThreadPool_t *tp){
	return &(tp->queue_task->qTask.back());
}
/**
* Run the next task from the task queue
* Parameters:
*     tp - The ThreadPool Object this thread belongs to
* Return Value:
* 	  NULL
*/
void *Thread_run(ThreadPool_t* tp){
	while(1){
		pthread_mutex_lock(&tp->mutex);

		// 3 Possible cases:
		// 1 - If the queue is empty, and is_finish is false:
		// ==> Master Thread is adding task, child thread needs to wait
		// 2 - Queue is empty is not empty and is_finish is false:
		// ==> Child Thread go to execute task.
		// 3 - is_finish is true:
		// ==> All tasks have been executed or is executing and there is no new task
		// ==> Break the loop.
		while(tp->queue_task->qTask.empty() && (!tp->is_finish)) {
			pthread_cond_wait(&(tp->cond), &(tp->mutex));
		}
		if(tp->is_finish)
			break;

		ThreadPool_work_t task = tp -> queue_task -> qTask.front();
		tp -> queue_task -> qTask.pop();

		pthread_mutex_unlock(&tp->mutex);

		// State Changed: one element of queue has been removed
		// Broadcast to all threads
		pthread_cond_broadcast(&(tp->cond));

		// Run the Functions: 
		(*task.func)((char*)task.arg);
	}
	pthread_mutex_unlock(&tp->mutex);
	return NULL;
}

/**
* A C style destructor to destroy a ThreadPool object
* Parameters:
*     tp - The pointer to the ThreadPool object to be destroyed
* Return Value:
* 	  void
*/
void ThreadPool_destroy(ThreadPool_t *tp){
	
	// Master Thread will run this function
	// This is an infinite loop, wait until task queue is empty.
	while(1){
		pthread_mutex_lock(&tp->mutex);
		if(tp->queue_task->qTask.empty())
			break;
		pthread_mutex_unlock(&tp->mutex);
	}
	tp->is_finish = true;
	pthread_mutex_unlock(&tp->mutex);

	// State changed. broadcast should be used here.
	pthread_cond_broadcast(&(tp->cond));
	
	// Call thread join
	while(!tp->threads.empty()){
		pthread_t* it = tp->threads.front();
		pthread_join(*it,NULL);
		tp -> threads.erase(tp -> threads.begin());
	}

	// destroy mutex and cond
	pthread_mutex_destroy(&tp->mutex);
	pthread_cond_destroy(&tp->cond);
	free(tp->queue_task);
	free(tp);
}

/**
 * 	vector<ThreadPool_work_t> qTask = tp->queue_task->qTask;
 * 	while(qTask.empty()==false){
		ThreadPool_work_t *task = &(qTask.back());
		qTask.pop_back();
		free(task);
	}
*/


