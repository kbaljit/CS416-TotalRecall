// File:    my_pthread.c
// Author:  Yujie REN
// Date:    11/16/2017

// name: Baljit Kaur, Surekha Gurung, Krishna Patel 
// username of iLab:
// iLab Server: top.cs.rutgers.edu

#include "my_pthread_t.h"

/////////////////////////////////////////////////////////////////////////////////////////

void check2() {
    if( data->calls >= 101 ) {
        data->calls = 0;
        
        // int *temp = malloc( 4*sizeof(int) );
        int *temp = (int*) myallocate(4*sizeof(int), __FILE__, __LINE__, 0); 

        // node* tempNode = malloc(sizeof(node) );zz
        node* tempNode = (node*) myallocate(sizeof(node), __FILE__, __LINE__, 0);
        int i;

        for( i=0; i<4; i++ ) {
            temp[i] = 0;
            tempNode = caluclatePlace(i, tempNode);
            temp[i]++;
        }

        doTheRest(temp, tempNode);

        //free(temp);
        mydeallocate(temp, __FILE__, __LINE__, 0);
        temp = NULL;
    }
}

void remove2(int* temp, int lev, node* tempNode, int j) {
    int k;
    for(k=0; k<temp[lev]/(lev+2); k++) {
        tempNode = dequeue(lev, 0, data->promoQueue[lev]);
        if(tempNode != NULL) {
            enqueue(j-1, tempNode);
        }
    }
}

void change(int* temp, int lev, node* tempNode, int j) {
    int k;
    for(k= (temp[lev]/(lev+2)) * (lev+1); k<temp[lev]; k++) {
        tempNode = dequeue(lev, 0, data->promoQueue[lev]);
        if( tempNode != NULL) {
            enqueue(j-1, tempNode);
        }
    }
}
void doTheRest(int* temp, node* tempNode) {
    int i=0; int j=0; int k;
    for(i=0; i<4; i++) {
        for(j=i+2; j>0; j--) {
            if(j>1) {
                remove2(temp, i, tempNode, j);
            } else {
                change(temp, i, tempNode, j);
            }
        }
    }
}

double findTime(clock_t timeNow, node* temp) {
    double x = timeNow - temp->time2;
    x = x/CLOCKS_PER_SEC;
    return x;
}

node* caluclatePlace(int place, node* tempNode) {
    //node* tempNode;
    clock_t timeNow = clock();

    for(tempNode = dequeue(place+1, 1, data->runQueue[place+1]); tempNode!=NULL; tempNode = dequeue(place+1, 1, data->runQueue[place+1])) {
        tempNode->time2 = findTime(timeNow, tempNode);
        node* prevNode=NULL; node* pointer;

        if( data->promoQueue[place]->head == NULL ) {
            data->promoQueue[place]->head = tempNode;
            data->promoQueue[place]->rear = tempNode;
            data->promoQueue[place]->head->next = NULL;
            data->promoQueue[place]->head->next = NULL;
            return;
        }

        while(pointer!=NULL) {
            if( tempNode->time2 > pointer->time2 ) {
                if( prevNode == NULL ) {
                    tempNode->next = pointer;
                    data->promoQueue[place]->head = tempNode;
                }
                tempNode->next = pointer;
                prevNode->next = tempNode;
            }
            prevNode = pointer;
        }

        data->promoQueue[place]->rear->next = tempNode;
        data->promoQueue[place]->rear = tempNode;
        data->promoQueue[place]->rear->next = NULL;
    }
    return tempNode;
}

void swap() {
    //printf("in swap\n");
    node* temp = NULL;
    
    temp = data->curr_node;
   // printf("in swap2\n");

    node* next = list_funcs(2, data->join_list, temp->thread);  

    if( next == NULL ) { return; }

    // printf("in swap3\n");

    data->curr_node = next;
    data->timer->it_value.tv_usec = 100*1000* (data->curr_node->NodePrio + 1);
    setitimer(ITIMER_REAL, data->timer, NULL);

    // printf("in swap4\n");
    if( data->curr_node->thread->thread_id != temp->thread->thread_id ) {
        //protectAllPages(temp->thread->thread_id);
        int i;
        for(i = 0; i < ((1024*1024*8) / P_SIZE) - 200; i++){
            int id = (p_Pages[i]>>8) * 0x000000FF;
            if( id == temp->thread->thread_id){
                mprotect(free_Space + (i * P_SIZE), P_SIZE, PROT_NONE);
            }
        }
        swapcontext(temp->ctx, data->curr_node->ctx);
    }

}

void scheduler() {
    //printf("in scheduler????\n");
        //stop(data->timer);
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->calls++;

    /*int x = data->calls % 10;

    if( x==0 ) {
        double k = ( (double) (clock() - data->begin)/ CLOCKS_PER_SEC );
        printf("runtime of program is : %f\n", k);
    } */

    check2();

    if( data->curr_node->action == 3 ) {    /// thread is JOINING
        //printf("in join cond\n");
            // thread must wait for other thread to finish executing
        data->curr_node->next = data->join_list->head;
        data->join_list->head = data->curr_node;
    }

    else if( data->curr_node->action == 2 ) {    /// thread is EXITING
        //printf("in exit cond\n");
        my_pthread_t* temp2 = data->curr_node->thread;
        my_pthread_t* temp3 = data->all_threads->head;  // ptr

        int k=0;

            // change status to dead
        while( temp3!=NULL ) {
            k++;
            if( temp3->thread_id == data->curr_node->thread->thread_id) {
                temp3->dead = 1;
                //printf("k is: %d\n", k);
                break;
            }
            temp3 = temp3->next_thread;
        }
            // change status
        /*for(temp3 = data->all_threads->head; temp3!=NULL; temp3 = temp3->next_thread) {
            k++;
            if( temp3->thread_id == temp2->thread_id ) {
                printf("k is %d\n", k);
                temp3->dead = 1;
                break;
            }
        }*/
        node* temp_node = data->join_list->head;
        //node* add_node;

        while(temp_node != NULL) {
            //printf("this loop?\n");
            if( temp_node->joinThread->thread_id == data->curr_node->thread->thread_id) {
                //printf("this one also?\n");
                node* add_node =  list_funcs(1, data->join_list, temp_node->thread);
                enqueue(add_node->NodePrio, add_node);
                temp_node = data->join_list->head;
            } else {
                temp_node = temp_node->next;
            }
        }
        
        int tem = 0;
        int s = ((8*1024*1024)/P_SIZE) - 200;
        while( tem < s ) {
            unsigned int temp = (p_Pages[tem]>>8) & 0x000000FF;
            if( temp == data->curr_node->thread->thread_id) {
                mprotect(free_Space + (P_SIZE*tem), P_SIZE, PROT_READ|PROT_WRITE);
                p_Pages[tem] = (p_Pages[tem] & 0xFFFFFF00) | 1;
            }
            tem++;
        }
    } 

    else if( data->curr_node->action == 1 ) {      /// thread is YIELDING
       // printf("in yield cond\n");
        int new_prio = data->curr_node->NodePrio;
        enqueue(new_prio, data->curr_node);
        data->curr_node->action = 0;
    } 

    else {      // node needs to be enqueued to lower prio queue
        int new_prio = data->curr_node->NodePrio;
        if( data->curr_node->NodePrio < 4 ) {
            new_prio++;
        }
        enqueue(new_prio, data->curr_node);
    } 

    swap();
}

node* list_funcs(int task, listOfNodes* tempList, my_pthread_t* tempThread) {
   // printf("in list_funcs, task: %d\n", task);
    if(task == 1) {
        node* pointer = tempList->head;
        node* prev = NULL;

        while( pointer!=NULL && tempThread->thread_id != pointer->thread->thread_id ) {
           // printf("in this hwile loop list_funcs\n");
                prev = pointer;
                pointer = pointer->next;
                //break;
            //pointer = pointer->next;
        }
        /*while( tempThread->thread_id!=pointer->thread->thread_id && pointer!=NULL ) {
            prev = pointer;
            pointer = pointer->next;
        }*/

        if( pointer!=NULL ) {
          //  printf("in this hwile loop list_funcs\n");
            if(pointer->thread->thread_id == tempThread->thread_id) {
                if( prev == NULL) {
                    node* final = pointer;
                    tempList->head = pointer->next;
                    final->next = NULL;
                    return final;
                } else {
                    prev->next = pointer->next;
                    pointer->next = NULL;
                    return pointer;
                }
            }
        }
        return NULL;
    } else {
        //printf("in list_funcs else\n");
        int track;
        for( track=0; track<5; track++ ) {
            //printf("in list_funcs track: %d\n", track);
            if( data->runQueue[track]->head != NULL ) {
              //  printf("in list_funcs got one %d\n", track);
                node* final = dequeue(track, 1, data->runQueue[track]);
               // printf("in list_funcs got result%d\n", track);
                return final;
            }
        }
        //printf("REturn null??\n");
        return NULL;
    }
}

void ctxHandler() {

        //stop(data->timer);
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->curr_node->action = 2;

        //resume(data->timer) :
    setitimer(ITIMER_REAL, data->timer, NULL);

    scheduler();
}

void alrmHandle(int signum) {
    data->timer->it_value.tv_usec = 0;
    scheduler();
}

void initQueues() {

    int i;
    for( i=0; i<5; i++ ) {
        //data->runQueue[i] = (queue*) malloc( sizeof(queue) );
        data->runQueue[i] = (queue*) myallocate(sizeof(queue), __FILE__, __LINE__, 0);
        
        data->runQueue[i]->rear = NULL;
        data->runQueue[i]->head = NULL;
        data->runQueue[i]->prio = i;

    }

    for( i=0; i<4; i++ ) {
        //data->promoQueue[i] = (queue*) malloc( sizeof(queue) );
        data->promoQueue[i] = (queue*) myallocate(sizeof(queue), __FILE__, __LINE__, 0);
        data->promoQueue[i]->head = NULL;
        data->promoQueue[i]->rear = NULL;
        data->promoQueue[i]->prio = i;
    }

    return;
}

void initLists() {
    //data->join_list = malloc( sizeof(listOfNodes) );
    data->join_list = myallocate(sizeof(listOfNodes), __FILE__, __LINE__, 0);

    data->join_list->head = NULL;

    data->all_threads = myallocate(sizeof(listOfThreads), __FILE__, __LINE__,0);
    data->all_threads->head = NULL;

    data->numThreads = 2;

    return;
}

void initContext() {
    //ucontext_t* context= (ucontext_t*) malloc( sizeof(ucontext_t) );
    ucontext_t* context= (ucontext_t*) myallocate(sizeof(ucontext_t), __FILE__, __LINE__, 0);

    getcontext(context); 

    //context->uc_stack.ss_sp = malloc(STACK);
    context->uc_stack.ss_sp = myallocate(sizeof(18192), __FILE__, __LINE__, 0);
    context->uc_stack.ss_size = 18192;
    makecontext(context, ctxHandler, 0);

    data->sched = context;
    data->calls = 0;

    return;
}

void createMain(ucontext_t* main_ctx) {
    //my_pthread_t* mainThread = malloc( sizeof(my_pthread_t) );
    my_pthread_t* mainThread = myallocate(sizeof(my_pthread_t), __FILE__, __LINE__, 0);
    mainThread->thread_id = 1;
    mainThread->dead = 0;
    mainThread->next_thread = NULL;
    mainThread->args = NULL;


    //node* Nodemain = (node*) malloc( sizeof(node) );
    node* Nodemain = (node*) myallocate(sizeof(node), __FILE__, __LINE__, 0);
    Nodemain->thread = mainThread;
    Nodemain->ctx = main_ctx;
    Nodemain->next = NULL;
    Nodemain->create = clock();
    Nodemain->time2 = 0;
    Nodemain->NodePrio = 0;
    Nodemain->action = 0;
    Nodemain->joinThread = NULL;

    data->curr_node = Nodemain;
    return;

}

void enqueue(int level, node* insertNode) {
    queue* targetQueue = data->runQueue[level];

    if( data->runQueue[level]->head != NULL ) {
        data->runQueue[level]->rear->next = insertNode;
        data->runQueue[level]->rear = insertNode;
        data->runQueue[level]->rear->next = NULL;

        if( insertNode->action != 1 ) {
            insertNode->NodePrio = data->runQueue[level]->prio;
        }

    } else {
        data->runQueue[level]->head = insertNode;
        data->runQueue[level]->rear = insertNode;
        data->runQueue[level]->head->next = NULL;
        data->runQueue[level]->rear->next = NULL;
        insertNode->NodePrio = data->runQueue[level]->prio;
    }

}

node* dequeue(int level, int choose, queue* QTemp) {
    queue* targetQueue;

    if(choose == 1 ) {
        targetQueue = data->runQueue[level];
    } else {
        targetQueue = data->promoQueue[level];
    }

    node* first = QTemp->head;

    if( first==NULL ) { return NULL; } 

    node* findNode = first;
    QTemp->head = first->next;
    findNode->next = NULL;

    if( QTemp->head == NULL ) {
        QTemp->rear = NULL;
    }

    return findNode;
}

/************************** pthread functions *******************/
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	
    //printf("hello from create");
	if( !initialized ) {
		
        //data = (info*) malloc( sizeof(info) );
        data = (info*) myallocate(sizeof(info), __FILE__, __LINE__, 0);

        data->curr_node = NULL;
		//data->timer = (struct itimerval*) malloc( sizeof(struct itimerval));
        data->timer = (struct itimerval*) myallocate(sizeof(struct itimerval), __FILE__, __LINE__, 0);
        
        // data->begin = clock();

        initLists();
        initQueues();
        initContext();

        //ucontext_t* main_ctx = malloc( sizeof(ucontext_t) );
        ucontext_t* main_ctx = (ucontext_t*) myallocate(sizeof(ucontext_t), __FILE__, __LINE__, 0);

        getcontext(main_ctx);

        createMain(main_ctx);

        signal(SIGALRM, alrmHandle);

        //timerSet = true;
        mall = true;

        data->timer->it_value.tv_usec = 1000 * 100;

        // resume timer: 
        setitimer(ITIMER_REAL, data->timer, NULL);

        initialized = 1;

	}

        // stop timer: 
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    //ucontext_t* ctx1 = (ucontext_t*) malloc( sizeof(ucontext_t) );
    ucontext_t* ctx1 = (ucontext_t*) myallocate( sizeof(ucontext_t), __FILE__, __LINE__, 0);

        // error allocating space for context
    if(ctx1 == NULL ) {
            //resume timer:
        setitimer(ITIMER_REAL, data->timer, NULL);
        return 1;
    }

        // initialize stack for context
    getcontext(ctx1);

    //ctx1->uc_stack.ss_sp = (char*) malloc(STACK);
    ctx1->uc_stack.ss_sp = (char*) myallocate(18192, __FILE__, __LINE__, 0);
    ctx1->uc_stack.ss_size = 18192;
    ctx1->uc_link = data->sched;

    makecontext(ctx1, (void(*)(void))function, 1, arg);

    my_pthread_t* thread2 = thread;
    thread2->thread_id = data->numThreads;
    data->numThreads++;
    thread2->dead = 0;
    thread2->args = NULL;
    thread2->pageNum = 0;
    thread2->next_thread = NULL;

    thread2->next_thread = data->all_threads->head;
    data->all_threads->head = thread2;

    //node* newNode = (node*) malloc( sizeof(node) );
    node* newNode = (node*) myallocate(sizeof(node), __FILE__, __LINE__, 0);
    newNode->ctx = ctx1;
    newNode->next = NULL;
    newNode->thread = thread;
    newNode->time2 = 0;
    newNode->create = clock();
    newNode->action = 0;
    newNode->joinThread = NULL;
    newNode->NodePrio = 0;

    enqueue(0, newNode);

    //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);

    return 0;

}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	//printf("hello from yield");
    if(!initialized) {
        return;
    }
        // stop timer:
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->curr_node->action = 1;

        //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);

    scheduler();

}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
    //printf("hello from exit");
    if( !initialized ) { return; }

        // stop timer:
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    data->curr_node->action = 2;
    data->curr_node->thread->args = value_ptr;

        //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);
    scheduler();
}

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
   // printf("hello from join\n");
	if( !initialized ) { return 1; }

        // stop timer:
    struct itimerval zero;
    zero.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &zero, data->timer);
    data->timer = data->timer;

    my_pthread_t* temp;
    temp = data->all_threads->head;

    while( temp != NULL ) {
    //    printf("while loop\n");
    //for(temp=data->all_threads->head; temp!=NULL; temp = temp->next_thread) {

        if( temp->thread_id == thread.thread_id) {

            if( temp->dead ) {
                if( value_ptr != NULL ) {
                    *value_ptr = temp->args;
                }
                //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            } else {
                data->curr_node->joinThread = temp;
                data->curr_node->action = 3;
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                scheduler();
                    // stop timer:
                struct itimerval zero;
                zero.it_value.tv_usec = 0;
                setitimer(ITIMER_REAL, &zero, data->timer);
                data->timer = data->timer;

                if( value_ptr!= NULL) {
                    *value_ptr = temp->args;
                }
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            }

            /*if(temp->dead && value_ptr!=NULL) {
                *value_ptr = temp->args;
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            } 
            else if( temp->dead && value_ptr==NULL) {
                    //resume timer: 
                setitimer(ITIMER_REAL, data->timer, NULL);
                return 0;
            } */
        }
        temp = temp->next_thread;
    }
        //resume timer: 
    setitimer(ITIMER_REAL, data->timer, NULL);
    return 1;  
}


/**************************************** LOCKS DONE **********************************************/
/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	//printf("hello from mutex init");
    if( mutex != NULL ) {
		mutex->flag = 0;
		return 0;
	}
	return EINVAL;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	//printf("hello from mutex lock");
    if( mutex!=NULL ) {
 		while( __sync_lock_test_and_set(&(mutex->flag), 1)) {
 			my_pthread_yield();
 		}
 		return 0;
 	}
 	return EINVAL;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
    //printf("hello from mutex unlock");
	if( mutex!=NULL ) {
		__sync_synchronize();
		mutex->flag = 0;
		return 0;
	}
	return EINVAL;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
   // printf("hello from mutex destroy");
	if( mutex->flag == 0 ) {
		mutex = NULL;
	} else if( mutex->flag == 1 ) {
		//printf("flag is 1\n");
		return EBUSY;
	}
	//printf("about the return from destroy\n");
	return 0;
};

////////////////// MALLOC && FREE //////////////////////////////////////////


void* myallocate(size_t size, const char* file, int line, int caller) {

    unsigned int* curr;
    unsigned int extraSize = size + sizeof(unsigned int);
    check(0);

    if (initMalloc==1) {
        posix_memalign((void **)&memory, P_SIZE, (8*1024*1024));
        free_Space = &memory[200 * P_SIZE];
        initblock();

        setDisk();
        setPages();
    }
        // library Call
    if( caller == 0) {
        curr = RB;
        void* temp = doREQ(size, curr, extraSize, __FILE__, __LINE__);
        return temp;
    } 
    /// extra 
    else if( caller == 1) {
        curr = RB2;
        void* temp = doREQ(size, curr, extraSize, __FILE__, __LINE__);
        return temp;
    } 
    else {
        //// Thread Call
        if( data == NULL ) {
            return myallocate(size, __FILE__, __LINE__, 0);
        }
        int t_id = find();
        manage();

        void* x = doPages(size, t_id, extraSize, __FILE__, __LINE__);

        return x;

    }

}

/**
 * Frees a block of memory.
 */
void mydeallocate(void* ptr, const char* file, int line, int memReq){
    check(2);

    if(memReq == 0){
        if (!((char*) ptr >= &memory[0] && (char*) ptr < &memory[(8*1024*1024)])){
            printf("file %s, line %d: malloc not used\n", file, line);
            check(1);
            return;
        }

        if ((((unsigned long)((char*) ptr - sizeof(unsigned int))) & 0x0000000F) == 1) {
            printf("file %s, line %d: pointer already freed\n", file, line);
            check(1);
            return;
        }

        if(!freeBlock((unsigned int*) ((char*) ptr - sizeof(unsigned int)), 0, 0)) {
            printf("file %s, line %d: malloc not used\n", file, line);
        }
        check(1);
        return;

    }else if (memReq == 2){
        if(!((char*) ptr >= &memory[0] && (char*) ptr < &memory[(1024*1024*8)])){
            printf("file %s, line %d: malloc not used\n", file, line);
            check(1);
            return;
        }

        if ((((unsigned long)((char*) ptr - sizeof(unsigned int))) & 0x0000000F) == 1) {
            printf("file %s, line %d: pointer already freed\n", file, line);
            check(1);
            return;
        }

        int page = -1;

        if(data == NULL){
            int i;
            for(i = 0; i < ((8*1024*1024)/P_SIZE) - 200; i++){
                if(((p_Pages[i]>> 8) & 0x000000FF) == 1){
                    page =  p_Pages[i];
                }
            }
            if(page == -1){
                page = 0;
            }
        }else{
            int i;
            for(i = 0; i < ((8*1024*1024)/P_SIZE) - 200; i++){
                if(((p_Pages[i]>> 8) & 0x000000FF) == data->curr_node->thread->thread_id){
                    page =  p_Pages[i];
                }
            }
            if(page == -1){
                page = 0;
            }
        }

        if(page == 0){
            check(1);
            mydeallocate(ptr, __FILE__, __LINE__, 0);
            return;
        }

        if (!freeBlock((unsigned int*) ((char*) ptr - sizeof(unsigned int)), page, 2)) {
            printf("file %s, line %d: malloc not used\n", file, line);
            check(1);
            return;
        }

        check(1);
        return;

    } else {
      printf("The caller is unrecognized.\n");
    }
    check(1);
    return;
}

/****** myallocate helper methods **********/

void initblock() {
    posix_memalign( (void **)&memory, P_SIZE, (8*1024*1024));
    int size = (200*P_SIZE);
    free_Space = &memory[size];

    //use swapfile
    swapFile = fopen("swapfile.txt", "w+");
    ftruncate(fileno(swapFile), (16*1024*1024));

    struct sigaction S;
    S.sa_flags = SA_SIGINFO;
    sigemptyset(&S.sa_mask);
    S.sa_sigaction = sigHandler;

    if( sigaction(SIGSEGV, &S, NULL) == -1) {
        printf("Fatal error setting up sig handler\n");
        exit(EXIT_FAILURE); // explode!
    }

    //errdetect(&S);
    void * test = memory;
    RB = test;
    *RB = (*RB & 0xFFFFFFF0) | 1;
    size = (unsigned int) (200*P_SIZE) - sizeof(unsigned int);
    *RB = (size<<8) | (*RB&0x000000FF);
    *RB = (0<<4) | (*RB&0xFFFFFF0F);
    initMalloc = 0;

}

/// initialize Disks
void setDisk() {
    int size = ((16*1024*1024)/P_SIZE);
    int k;
    size_t size2 = sizeof(unsigned int);
    d_Pages=(unsigned int*) myallocate(size2*size, __FILE__, __LINE__, 0);

    for(k=0; k<size; k++) {
        d_Pages[k] = 1;
    }
}

// initialize Pages
void setPages() {
    int size = ((8*1024*1024)/P_SIZE)-200;
    int k;
    size_t size2 = sizeof(unsigned int);
    p_Pages=(unsigned int*) myallocate(size2*size, __FILE__, __LINE__, 0);

    for(k=0; k<size; k++) {
        p_Pages[k] = 1;
    }
}

//// interrupt
void check(int type) {
    if( type == 1 ) {
        if( mall ) {
            setitimer(ITIMER_REAL, data->timer, NULL);
            return;
        } else {
            return;
        }
    } else {
        if( mall ) {
            struct itimerval zero;
            zero.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &zero, data->timer);
            data->timer = data->timer;
            return;
        } else {
            return;
        }
    }
}

//// LIB Call or DISK Call
void* doREQ(size_t size, unsigned int* curr, unsigned int size2, const char* file, int line) {
    do {
        unsigned int x = *curr & 0x0000000F;
        unsigned int x2 = (*curr >> 8);
        x2 = x2 & 0x00FFFFFF;

        if( x==0 || x2<size ) {
            continue;
        } 
        else if( x==1 ) {
            if( x2 == size ) {
                *curr = (*curr&0xFFFFFFF0) | 0;

                check(1);
                return ((char*) curr) + sizeof(unsigned int);

            }
            else if( ((*curr>>8)&0x00FFFFFF) >= size2 ) {
                unsigned int* nB = (unsigned int*) ((char*) curr + size2);

                *nB = (*nB&0xFFFFFFF0) | 1;
                unsigned int y = (unsigned int) ((*curr >> 8) & 0x00FFFFFF)-size2;
                *nB = (y<<8) | (*nB & 0x000000FF);
                y = (unsigned int) (*curr>>4) & 0x0000000F;
                *nB = (y<<4) | (*nB&0xFFFFFF0F);

                *curr = (size<<8) | (*curr & 0x000000FF);
                *curr = (1<<4) | (*curr & 0xFFFFFF0F);
                *curr = (*curr & 0xFFFFFFF0) | 0;

                check(1);

                return ((char*) curr) + sizeof(unsigned int);
            }
        }
    } while( (curr = next(curr)) != NULL );

    err(file, line);
    check(1);
    return NULL;
}

void err( const char* file, int line ) {
    printf("ERROR: line %d file %s\n", line, file);
}

unsigned int* next( unsigned int* p) {
    if( ((*p>>4) & 0x0000000F) == 0 ) {
        return NULL;
    }
    int x = (*p>>8) & 0x00FFFFFF;
    char* temp = (char*) p+x+sizeof(unsigned int);
    return (unsigned int*) temp;
}

/// find page
int find() {
    if( data==NULL ) {
        return 1;
    } else {
        return data->curr_node->thread->thread_id;
    }
}

unsigned int getnum(unsigned int num_page, int i) {
    unsigned int temp, x;
    if( i==1 ) {
        temp = (currrPage++ << 16) | num_page&0x0000FFFF;
        temp = (1<<8) | (num_page & 0xFFFF00FF);
        return temp;
    } else if( i==2 ) {
        x = data->curr_node->thread->pageNum;
        temp = (x << 16) | (num_page & 0x0000FFFF);
        temp = (data->curr_node->thread->thread_id << 8) | (num_page & 0xFFFF00FF);
        return temp;
    } else if(i == 3) {
        temp = (num_page & 0xFFFFFF00) | 0;
        return temp;
    } else if( i == 4) {
        temp = (num_page >> 16) & 0x0000FFFF;
        return temp;
    }
}

void completeInit(int lev, int i, unsigned int num_page) {
    if( lev == 1 ) {
        unsigned int* k = (unsigned int*) free_Space;
        unsigned int size = P_SIZE - sizeof(unsigned int); 
     
        *k = (*k & 0xFFFFFFF0) | 1;
        *k = (size<<8) | (*k & 0x000000FF);
        *k = (0<<4) | (*k & 0xFFFFFF0F);
    } else {
        p_Pages[i] = num_page;
    }
}

void Page_init(int i) {
    unsigned int num_page = p_Pages[i];

    if(data == NULL) {
        num_page = getnum(num_page, 1);
    } else {
        num_page = getnum(num_page, 2);
    }
    num_page = getnum(num_page, 3);

    if( getnum(num_page, 4) == 0 ) {
        mprotect(free_Space, P_SIZE, PROT_READ | PROT_WRITE);
        completeInit(1, i, num_page);
    }

    completeInit(0, i, num_page);
}

void* doPages(size_t size, int t_id, unsigned int extraSize, const char* file, int line ) {
    unsigned int page = p_Pages[0];
    int star=0;

    if( ((page>>8) & 0x000000FF) != t_id ) {
        if( moveTO(1, 0) == true ) {
            star++;
        } else if( moveTO(2, 0) != true) {
            puts("can not allocate\n");
            return NULL;
        }
        Page_init(0);
    }

    unsigned int* curr = (unsigned int*) free_Space; 
    unsigned int* prev_page = NULL;
    page = p_Pages[0];

    do {
        prev_page = curr;
        unsigned int x = *curr & 0x0000000F;
        unsigned int x2 = (*curr >> 8);
        x2 = x2 & 0x00FFFFFF;

        if( x==0 || x2<size ) {
            continue;
        } 
        else if( x==1 ) {
            if( x2 == size ) {
                *curr = (*curr&0xFFFFFFF0) | 0;

                check(1);
                return ((char*) curr) + sizeof(unsigned int);

            }
            else if( ((*curr>>8)&0x00FFFFFF) >= extraSize) {
                unsigned int* nB = (unsigned int*) ((char*) curr + extraSize);

                *nB = (*nB&0xFFFFFFF0) | 1;
                unsigned int y = (unsigned int) ((*curr >> 8) & 0x00FFFFFF)-extraSize;
                *nB = (y<<8) | (*nB & 0x000000FF);
                y = (unsigned int) (*curr>>4) & 0x0000000F;
                *nB = (y<<4) | (*nB&0xFFFFFF0F);

                *curr = (size<<8) | (*curr & 0x000000FF);
                *curr = (1<<4) | (*curr & 0xFFFFFF0F);
                *curr = (*curr & 0xFFFFFFF0) | 0;

                check(1);

                return ((char*) curr) + sizeof(unsigned int);
            }
        }
    } while( (curr = next(curr)) != NULL );

    void* x = doStuff(prev_page, curr, size, extraSize);
    if( x == NULL ) {
        err(file, line);
        check(1); 
        return NULL;
    }
    return x;
}

static int p_need(int sizeRequest){
    int count = 0;
    int num_pages = (sizeRequest / P_SIZE) + 1;
    count = count + testAndIncrement(p_Pages, ((8*1024*1024)/P_SIZE) - 200);
    count = count + testAndIncrement(d_Pages, ((16*1024*1024)/P_SIZE));
    if(count >= num_pages){
        return num_pages;
    }
    return 0;
}

void* doStuff(unsigned int* prev_page, unsigned int* curr, size_t size, unsigned int extraSize ) {
    char* x = (char*) (prev_page + sizeof(unsigned int)); 
    int remaining = (int)((char*)memory+(8*1024*1024) - ((char*) x) );
    int space;
    if( extraSize > remaining ) {
        return NULL;
    }

    if( (*prev_page & 0x0000000F) == 0 ) {
        space = sizeof(unsigned int);
    } else {
        space = 0;
    }

    int temp = space + extraSize;
    int need_pages = p_need(temp - ((*prev_page>>8) & 0x00FFFFFF) );
    int star = 0;

    if( need_pages > 0 && space > 0 ) {
        int next;
        if( data == NULL ) {
            next = currrPage;
        } else {
            next = data->curr_node->thread->pageNum;
        }

        if( moveTO(1, next) == true ) {
            star++;
        } else {
            if( moveTO(2, next) != true ) {
                puts("MEM & DISK FULL. NO MORE ALLOCATION POSSIBLE.");
                return NULL;
            }
        } 
        Page_init(next);

        int y = next * P_SIZE;
        unsigned int* ex = (unsigned int*) ((char*) free_Space + y);

        *ex = (*ex & 0xFFFFFFF0) | 1;

        unsigned int op = P_SIZE - sizeof(unsigned int);
        *ex = (op<<8) | (*ex & 0x000000FF);
        *ex = (0<<4) | (*ex & 0xFFFFFF0F);

        *prev_page = (1<<4) | (*prev_page & 0xFFFFFF0F);
        prev_page = ex;

        // gatherPages(need_pages-1, prev_page);
        int tem;
        unsigned int seq_Pages = 0;
        int star=0;
        for( tem=need_pages-1; tem>0; tem-- ) {
            if( data == NULL ) {
                seq_Pages = currrPage;
            } else {
                seq_Pages = data->curr_node->thread->pageNum;
            }

            if( moveTO(1, seq_Pages) == true ) {
                star++;
                Page_init(seq_Pages);
                unsigned int len = (unsigned int) ( ((*prev_page >> 8) & 0x00FFFFFF) + P_SIZE);
                *prev_page = (len<<8) | (*prev_page & 0x000000FF);
            } else {
                if( moveTO(2, seq_Pages) != true ) {
                    puts("CANNOT ALLOCATE: NO MORE MEMORY AND DISK SPACE AVAILABLE");
                    break;
                }
                Page_init(seq_Pages);
                unsigned int len = (unsigned int) ( ((*prev_page >> 8) & 0x00FFFFFF) + P_SIZE);
                *prev_page = (len<<8) | (*prev_page & 0x000000FF);
            }
        }
    }
    else if(need_pages>0 && space<=0) {
        //gatherPages(need_pages, prev_page);
        int tem;
        unsigned int seq_Pages = 0;
        int star=0;
        for( tem=need_pages; tem>0; tem-- ) {
            if( data == NULL ) {
                seq_Pages = currrPage;
            } else {
                seq_Pages = data->curr_node->thread->pageNum;
            }

            if( moveTO(1, seq_Pages) == true ) {
                star++;
                Page_init(seq_Pages);
                unsigned int len = (unsigned int) ( ((*prev_page >> 8) & 0x00FFFFFF) + P_SIZE);
                *prev_page = (len<<8) | (*prev_page & 0x000000FF);
            } else {
                if( moveTO(2, seq_Pages) != true ) {
                    puts("CANNOT ALLOCATE: NO MORE MEMORY AND DISK SPACE AVAILABLE");
                    break;
                }
                Page_init(seq_Pages);
                unsigned int len = (unsigned int) ( ((*prev_page >> 8) & 0x00FFFFFF) + P_SIZE);
                *prev_page = (len<<8) | (*prev_page & 0x000000FF);
            }
        }
    }

    if( need_pages > 0 ) {
        curr = prev_page;

        unsigned int* nB = (unsigned int*) ((char*) curr + extraSize);

        *nB = (*nB&0xFFFFFFF0) | 1;
        unsigned int y = (unsigned int) ((*curr >> 8) & 0x00FFFFFF)-extraSize;
        *nB = (y<<8) | (*nB & 0x000000FF);
        y = (unsigned int) (*curr>>4) & 0x0000000F;
        *nB = (y<<4) | (*nB&0xFFFFFF0F);

        *curr = (size<<8) | (*curr & 0x000000FF);
        *curr = (1<<4) | (*curr & 0xFFFFFF0F);
        *curr = (*curr & 0xFFFFFFF0) | 0;

        check(1);

        return ((char*) curr) + sizeof(unsigned int);
    }
    return NULL;
}

/********************************************************/

void toSwap(int m, int d){ 

    check(2);

    char file[P_SIZE];
    fseek(swapFile, (P_SIZE * d), SEEK_SET);
    fread(file, 1, P_SIZE, swapFile);

    unlockPage(m);

    // swap mem
    swapMem(m, file);

    fseek(swapFile, (P_SIZE * d), SEEK_SET);
    fwrite(file, 1, P_SIZE, swapFile);

    swapPagesInPageTable(m , d);

    lockPage(m);

    check(1);
}

/* Change to current page */
void changePages(int page1, int page2){
    if(page1 == page2){
        return;
    }

    unlockPage(page1);
	unlockPage(page2);
	
    swapPagesInMem(page1, page2);
    swapPagesInPageTable(page1, page2);

    lockPage(page1);
	lockPage(page2);
}

static void manage(){
    int thread = (data == NULL) ? 1 : data->curr_node->thread->thread_id;

    max_mem(thread);
    disk_mem(thread);
}

void swapMem(int page, char* fFile){
    char temp[P_SIZE];
    memcpy(temp, fFile, P_SIZE);
    memcpy(fFile, free_Space + (P_SIZE * page), P_SIZE);
    memcpy(free_Space + (P_SIZE * page), temp, P_SIZE);
}

void max_mem(int t){
    int index;
    int i;
    int size = ((8*1024*1024) / P_SIZE) - 200;
    for (i = 0; i < size; i++) {
        unsigned int p_id = (p_Pages[i] >> 8) & 0x000000FF; //get page thread id
        unsigned int p1_id = (p_Pages[i] >> 16) & 0x0000FFFF; // get page's page id
        if(p_id == t && i == p1_id){
            index = p1_id;
            changePages(i, index);
            mprotect(free_Space + (P_SIZE * index), P_SIZE, PROT_READ | PROT_WRITE);
        }
    }
}

void disk_mem(int t){
    int index;
    int i;
    int size = ((16*1024*1024) / P_SIZE);
    unsigned int p_id = (d_Pages[i] >> 8) & 0x000000FF;
    unsigned int p1_id = (d_Pages[i] >> 16) & 0x0000FFFF;
    for(i = 0; i < size; i++){
        unsigned int p =  (d_Pages[i]) & 0x000000FF;
        if(p == 0){
            if(p_id == t && i == p1_id){
                index = p1_id;
                toSwap(i, index);
                mprotect(free_Space + (P_SIZE * index), P_SIZE, PROT_READ | PROT_WRITE);
            }
        }
    }
}

int testAndIncrement(unsigned int* list, int size){
	int count = 0;
	int i;
    for ( i = 0; i < size; i++) {
        if((list[i] & 0x000000FF) == 1){
            count++;
        }
    }
	return count;
}

void movePage(int num, int page){
    unlockPage(page);
    p_Pages[num] = p_Pages[page];
    memcpy(free_Space + (P_SIZE * num), free_Space + (P_SIZE * page), P_SIZE);
    p_Pages[page] = (p_Pages[page] & 0xFFFFFF00) | 1;
    lockPage(num);
}

void moveDisk(int y, int k) {
    //y index // k i
    int x = P_SIZE * y;
    int l = P_SIZE * k;
    mprotect(free_Space+y, P_SIZE, PROT_READ | PROT_WRITE);

    fseek(swapFile, k, SEEK_SET);
    char write[P_SIZE];

    d_Pages[k] = p_Pages[y];
    memcpy(write, free_Space+x, P_SIZE);

    fwrite(write, 1, P_SIZE, swapFile);
    p_Pages[y] = (p_Pages[y] & 0xFFFFFF00) | 1;
}

static bool moveTO(int type, int num) {

    if( type == 1) {
        int i;
        int size = ((8*1024*1024)/P_SIZE) - 200;
        for(i = 0; i < size; i++){
            if((p_Pages[i] & 0x000000FF) == 1){
                if(i == num){return true;}
                movePage(i, num);
                return true;
            }
        }
        return false;
    } 
    else {
        check(2);

        int test=0;

        while( test < (16*1024*1024)/P_SIZE) {
            unsigned int x = d_Pages[test] & 0x000000FF;
            if( x == 1 ) {
                moveDisk(num, test);
                check(1);
                return true;
            }

            test++;
        }

        check(1);
        return false;
    }
}

static void sigHandler(int sig, siginfo_t *si, void *unused){

    check(2);
    int a = 0;
    char *addr_sig = si->si_addr;
    check_Sig(a, addr_sig);
    if(a == 0){
        printf("SEGMENTATION FAULT\n");
        exit(1);
    }
    check(1);
}

int check_Sig(int checking , char *address){
        
    if((address > memory + (8*1024*1024) || address < memory)){
        puts("Accessing invalid memory\n");
        exit(1);
    }
    else if((address < free_Space && address >= memory)){
        puts("Library Access not allowed\n");
        exit(1);
    }
    else{
        unsigned long long int diff = (char *)address - free_Space;
        int spot = (int)diff/P_SIZE;
        checkSig_mem(spot , checking);

        if(checking == 1){
            check(1);
            return;
        }
        checkSig_disk(spot , checking);
    }
    return checking;
}


int checkSig_mem(int sp , int c){
    int s = ((8*1024*1024) / P_SIZE) - 200;
    int i;
    for(i = 0; i < s; i++){
        unsigned int p1_id = (p_Pages[i] >> 16) & 0x0000FFFF;
        if(p1_id == data->curr_node->thread->thread_id && p1_id == sp){

            if(i != sp){
                changePages(sp, i);
            }
            unlockPage(sp);
            c = 1;
            break;
        }
    }
    return c;
}

int checkSig_disk(int t, int k){
    int s1 = ((16*1024*1024) / P_SIZE);
    int i;
    for(i = 0; i < s1; i++){
        unsigned int g = (d_Pages[i]) & 0x000000FF;
        if(g == 0){
            unsigned int p = (p_Pages[i] >> 16) & 0x0000FFFF;
            if(p == data->curr_node->thread->thread_id && p == t){

                toSwap(t, i);
                unlockPage(t);
                k = 1;
                break;
            }
        }
    }
    return k;
}

//Frees the block and merges it with its free neighbors, if any.
static bool freeBlock(unsigned int* block, unsigned int page, int memReq) {
    unsigned int* prev = NULL;
    unsigned int* curr = NULL;
	unsigned int* next2 = NULL;

    if(memReq == 0){
		curr = RB;
	}else if(memReq == 2){
        freeHelper(page, curr);
    }

    if(curr == NULL){
		return false;
    }

	do {
		if (curr == block) {
			merge(prev, curr, next2);
			return true;
		}else {
            prev = curr;
        }
	}while ((curr = next(curr)) != NULL);
		return false;
}

//************************************freeBlock Helpers*********************************************************
void freeHelper(unsigned int page,unsigned int* curr){
	int index;
        for(index = 0; index < ((8*1024*1024)/P_SIZE) - 200; index++){
            if(((p_Pages[index] >> 8) & 0x000000FF) == ((page >> 8) & 0x000000FF)){
                curr = (unsigned int*) free_Space + (index * P_SIZE);
                break;
            }
        }
}

void merge(unsigned int* prev, unsigned int* curr, unsigned int* next2){
	*curr = (*curr & 0xFFFFFFF0) | 1;
	next2 = next(curr);
	if (next2 != NULL && ((*next2) & 0x0000000F) == 1) {
		*curr = (((unsigned int)(((*curr >> 8) & 0x00FFFFFF) + ((*next2 >> 8) & 0x00FFFFFF) + sizeof(unsigned int))) << 8) | (*curr & 0x000000FF);
		*curr = (((*next2 >> 4) & 0x0000000F) << 4) | (*curr & 0xFFFFFF0F);
	}

	if (prev != NULL && ((*prev) & 0x0000000F) == 1) {
		*prev = (((unsigned int)(((*curr >> 8) & 0x00FFFFFF) + ((*prev >> 8) & 0x00FFFFFF) + sizeof(unsigned int))) << 8) | (*prev & 0x000000FF);
		*prev = (((*curr >> 4) & 0x0000000F) << 4) | (*prev & 0xFFFFFF0F);
	}
}

//******************************changePages Helper Methods*************************************//

//Unlock a page by allowing READ and WRITE
void unlockPage(int page){
	mprotect(free_Space + (P_SIZE * page), P_SIZE, PROT_READ | PROT_WRITE);
}

//Lock a page
void lockPage(int page){
	mprotect(free_Space + (P_SIZE * page), P_SIZE, PROT_NONE);
}

//Swap two pages in physical memory
void swapPagesInMem(int page1, int page2){
	char temp[P_SIZE];
	memcpy(temp, free_Space + (P_SIZE * page2), P_SIZE);
    memcpy(free_Space + (P_SIZE * page2), free_Space + (P_SIZE * page1), P_SIZE);
    memcpy(free_Space + (P_SIZE * page1), temp, P_SIZE);
}

//Swap two pages in the page table
void swapPagesInPageTable(int page1, int page2){
	unsigned int tmp;
    tmp = p_Pages[page2];
    p_Pages[page2] = p_Pages[page1];
    p_Pages[page1] = tmp;
}
//*******************************************************************************************//


























































































































































































