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
#include <algorithm>

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

/**
 * -------------------------------------------------------------------------------------
 * Assignment:           2
 * Due Date:             November 1, 2019
 * Name:                 Qian Yu
 * CCID:                 qian6
 * Student ID:           1546189
 * Instructor:           Omid Ardakanian
 * Lab Section:          Tuesday 1700 - 1950
 * -------------------------------------------------------------------------------------
 */


/** 
 * -------------------------------------------------------------------------------------
 * Global Variable Explaination:
 *
 * @ partVector: A vector of vector. If partition number is R, then, partVector has R vectors.
 * @ words: A priority_queue(MinHeap), store the key in ascending key
 * @ pool: A pointer to theadpool, reason: I want to access it in MR_emit.
 * @ myreduce: A function pointer to Reduce function. 
 *             Reason: I want to call Reduce inside MRProcessPartition.
 * @ num_partition: An integer to indicate the number of partition. 
 *             Reason: I want to use it in MR_emit
 * 
 * -------------------------------------------------------------------------------------
 */


auto cmp = [](string left, string right) { return left.compare(right)>=0?1:0; };
priority_queue<string,vector<string>,decltype(cmp)> words(cmp);

vector<vector<string>> partVector;
ThreadPool_t *pool;
int num_partition;
Reducer myreduce;

// The function is used to assist the built-in sort function.
bool cmp_file (char *left,char *right) { 
    struct stat sb_left,sb_right;
    if(stat(left, &sb_left) == -1 || stat(right, &sb_right)== -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    return (sb_left.st_size>sb_right.st_size); 
}

/**
 * Desciption: 
 * 1. Creating Mappers:
 *  - the master thread creates M mapper threads in the MR Run library function where M
 *  is an argument of this function
 *  - In the MR Run function, the master thread iterates over K input data splits 
 *  (represented by K files) and submits a job for each split to the thread pool. 
 *  If there is an idle mapper thread in the thread pool, it starts processing 
 *  the job right away by invoking the user-defined Map function with 
 *  the passed argument (i.e., the filename). 
 *
 * Parameters:
 *  @ num_files: The number of input files.
 *  @ filenames: A point array. Each element of this array points to a name of file.
 *  @ map: A function pointer which points to Map function. 
 *  @ num_mappers: The number of threads we want to use to execute map function.
 *  @ reduce: A function pointer which points to Reduce function. 
 *  @ num_reducers: The number of threads we want to use to execute reduce function.
 *
 * Return Value:
 *  void
 */

void MR_Run(int num_files, char *filenames[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers){

    pool = ThreadPool_create(10);
    num_partition = num_reducers;
    myreduce = reduce;
    for(int i = 0; i< num_partition ; i++){
        vector<string> s;
        partVector.push_back(s);
    }


    // Since my implementation should use the longest job first policy. 
    // We sort the file by their size (Descending order).
    sort(filenames,filenames+num_files-1,cmp_file);
    
    // A for loop to add the file into queue of task.
    for(int i=0; i<num_files;i++){
        ThreadPool_add_work(pool, (thread_func_t)map, filenames[i]);
        if(pthread_cond_signal(&(pool->cond)) != 0) 
		    cerr << "signal error "<<endl;
	}
    
    // The master thread has added all tasks. 
    // Call the destory method, and wait until child threads finish.
    ThreadPool_destroy(pool);

    for(int i = 0; i< num_partition ; i++){
        MR_ProcessPartition(i);
    } 
}


/**
 * Desciption: 
 *  when the MR Emit function is called by a mapper thread, it first determines
 *  where to write the given key/value pair by calling the MR Partition function.
 *
 * Parameters:
 *  @ key: The word.
 *  @ num_partitions: The number of partition.(a.k.a R)
 *
 * Return Value:
 *  void
 */

void MR_Emit(char *key, char *value){
    string myString = key;

    // index is the hash value
    int index = MR_Partition(key,num_partition);

    // Since I want to modify something in shared memory
    // I should use lock here
    pthread_mutex_lock(&pool->mutex);
    partVector.at(index).push_back(myString);
    pthread_mutex_unlock(&pool->mutex);
}


/**
 * Desciption: 
 *  The MR ProcessPartition library function invokes the user-defined Reduce 
 *  function on the next unprocessed key in the given partition in a loop
 *
 * Parameters:
 *  @ key: The word
 *  @ num_partitions: The number of partition(a.k.a R)
 *
 * Return Value:
 *  An unsigned long hash value.
 */

unsigned long MR_Partition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

/**
 * Desciption: 
 *  The MR Partition function takes a key and maps it to an integer between 0 
 *  and R − 1, which indicates the data structure the key/value pair must be written to. 
 *  This way MR Partition allows the MapReduce library to create R separated
 *  data structures, each containing a subset of keys and the value list associated with
 *  each of them. 
 *
 * Parameters:
 *  @ num_partitions: The number of partition(a.k.a R)
 *
 * Return Value:
 *  void
 */


void MR_ProcessPartition(int partition_number){
    vector<string> s = partVector.at(partition_number);

    vector<string>::iterator col;
    for(col = s.begin();col!=s.end();col++){
        words.push(*col);
    }
    while(!words.empty())  {
        string key =words.top();
        words.push(key);
        myreduce((char *)key.c_str(),partition_number);
    }
}



/**
 * Desciption: 
 * The MR GetNext
 * function returns the next value associated with the given key in the sorted 
 * partition or NULL when the key’s values have been processed completely
 *
 * Parameters:
 *  @ key: a word
 *  @ num_partitions: The number of partition(a.k.a R)
 *
 * Return Value:
 *  - A char* type string. Standing for the next word of key.
 *  - NUll, the key’s values have been processed completely
 */
char *MR_GetNext(char *key, int partition_number){
    string prev = key;
    words.pop();
    if(words.empty()){
        return NULL;
    }
    string next = words.top();
    if(prev.compare(next) == 0)
        return (char*)prev.c_str();
    else{
        return NULL;
    }
}
