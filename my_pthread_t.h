// File:    my_pthread.c
// Author:  Yujie REN
// Date:    11/16/2017

// name: Baljit Kaur, Surekha Gurung, Krishna Patel
// username of iLab:
// iLab Server: top.cs.rutgers.edu

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* To use real pthread Library in Benchmark, you have to comment the USE_MY_PTHREAD macro */
#define USE_MY_PTHREAD 1

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>   
#include <signal.h>    
#include <unistd.h> 
#include <malloc.h>  
#include <stdbool.h>    
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ucontext.h>
#include <sys/types.h>
#include <string.h>
#include <sys/syscall.h>

#define MAX_THREADS2 200
#define LIBRARYREQ 0
#define THREADREQ 2

#define malloc(x) myallocate( x, __FILE__, __LINE__, THREADREQ);
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ);
#define P_SIZE sysconf(_SC_PAGE_SIZE)

/* STRUCTS */

typedef struct {
	// Define any fields you might need inside here.
} my_pthread_attr_t;

typedef struct my_pthread_t {
	int thread_id;          
	int dead; 				
	ucontext_t ctx;			
	void* args;				
	struct my_pthread_t* next_thread;  
	int pageNum;                		
} my_pthread_t; 

typedef struct node {

	my_pthread_t* thread; 
	int NodePrio;      
	double time2;     
	int action; // 0=nothing; 1=yield; 2=exit; 3=join; 
	ucontext_t* ctx;      
	time_t create;         
	my_pthread_t* joinThread;  
	struct node* next;    

} node;

typedef struct queue {
	int prio;        
	node* head;
	node* rear;
} queue;

typedef struct listOfThreads {
	my_pthread_t* head;
} listOfThreads;  

typedef struct listOfNodes {
	node* head;
} listOfNodes;   

typedef struct info {

	int calls;        
	int numThreads;   
	node* curr_node;   

	listOfNodes* join_list;   
	listOfThreads* all_threads;  

	ucontext_t* sched;       
	struct itimerval* timer;    

	queue* promoQueue[4];    
	queue* runQueue[5];       

} info;

/* globals */

static unsigned int* RB;
static unsigned int* d_Pages; 
static unsigned int* RB2;
static unsigned int* p_Pages;
static bool mall=0;
static char* memory;
static bool initMalloc=true;
static char* free_Space = NULL;
static int currrPage = 0;

static info* data = NULL;

//my_pthread_t* current_thread = NULL;

static int initialized = 0;
FILE *swapFile;

//my_pthread_t * curr_LowThread = 0;
//static my_pthread_t * thread_list[MAX_THREADS2];
//static my_pthread_t * run_list[MAX_THREADS2];
//static my_pthread_t * lowP_list[MAX_THREADS2];

/********************functions*******************/

void swap();
void check2();
void scheduler();
void maintenance();
void initQueues();
void initLists();
void initContext();
void ctxHandler();
void alrmHandle(int signum);
node* caluclatePlace(int place, node* tempNode);
//node* dequeue(int level, int choose);
double findTime(clock_t timeNow, node* temp);
void doTheRest(int* temp, node* tempNode);
node* dequeue(int level, int choose, queue* QTemp);
void createMain(ucontext_t* main_ctx);
void enqueue(int level, node* insertNode);
void insertByTime(queue* q, node* newNode);
void remove2(int* temp, int lev, node* tempNode, int j);
void change(int* temp, int lev, node* tempNode, int j);
node* list_funcs(int task, listOfNodes* tempList, my_pthread_t* tempThread);

////// MEM //////////
void max_mem(int t);
static bool freeBlock(unsigned int* block, unsigned int page, int memReq);
void disk_mem(int t);
void swapMem(int page, char* fFile);
int check_Sig(int checking, char *address);
int checkSig_mem(int sp , int c);
int dheckSig_disk(int t, int k);
static void sigHandler(int sig, siginfo_t *si, void *unused);
unsigned int* next( unsigned int* p);
void unlockPage(int page);
void lockPage(int page);
void swapPagesInMem(int page1, int page2);
void swapPagesInPageTable(int page1, int page2);
void freeHelper(unsigned int page,unsigned int* curr);
void merge(unsigned int* prev, unsigned int* curr, unsigned int* next);
int testAndIncrement(unsigned int* list, int size);
int find();
void max_mem(int t);
void disk_mem(int t);
static void manage();
void movePage(int num, int page);
void setPages();
void setDisk();
void initblock();
void check(int type);
void err(const char* file, int line);
void moveDisk(int y, int k);
void movePage(int num, int page);
static bool moveTO(int type, int num);
unsigned int getnum(unsigned int num_page, int i);
void* doREQ(size_t size, unsigned int* curr, unsigned int size2, const char* file, int line);
void* doStuff(unsigned int* prev_page, unsigned int* curr, size_t size, unsigned int extraSize );
void completeInit(int lev, int i, unsigned int num_page);
void Page_init(int i);
void* doPages(size_t size, int t_id, unsigned int extraSize, const char* file, int line);


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);
/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();
/* terminate a thread */
void my_pthread_exit(void *value_ptr);
/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);


// Mutex Decelarations:

typedef struct my_pthread_mutex_t {
	int flag;
} my_pthread_mutex_t;

typedef int my_pthread_mutexattr_t;

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

void* myallocate(size_t size, const char* file, int line, int caller);
void mydeallocate(void* ptr, const char* file, int line, int caller);

#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif

















