/**
 * Sean Hermes
 * CPSC 3220
 * Assignment 2
 * April 1, 2024
 * 
 * please compile and run with the following commands
 *      gcc sched.c
 *      ./a.out  -prio input.txt output.txt
 *                   or
 *      ./a.out -custom input.txt output.txt
 * 
 * NOTES
 * For my ready queue arbitrary sorting I have the task with the lowest task id
 * run first, so in the example given in the assignment description processes Z and A
 * both have priority 8 and (in custom) when they both have remaining time of 2 (or 1) I will have 
 * process Z run before A based on Z's task_ID being lower, I wanted to note this since my results 
 * slightly differ from the example output given because of this
 * 
 * I also wanted to note that when writing to my output file when ran on 
 * different machines (personal,SoC,vm) the spacing differed for each machine
 * So I tried to get it so the spacing is the most clear when on the SoC machines
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

/********************* Struct & Macros *********************/

#define LARGE_SPACER "-------------------------------------------------------------------------------------------------------\n"
#define SPACER "------------------------------------------------\n"
#define SMALL_SPACER "----------------------\n"

struct task {
    int task_id, 
    arrival_time,
    service_time,
    remaining_time,
    completion_time,
    response_time,
    wait_time,
    priority;
  struct task *next;
  struct task* readyNext;
  bool ready;
  bool exe;
  int incremented_priority;
};


/********************* Function Prototypes *********************/

/**
 * This funciton will parse the input file and store its contents
 * in a linked list that is created in the function. The function 
 * takes in the input file pointer and a task** to be the head
*/
void parseInputFile(FILE*, struct task**);

/**
 * This function will sort the linked list by taskID for 
 * printing the data in the tables to the output file 
*/
void sortBytaskID(struct task**);


/**
 * This functions reads all of the data from the linked list and
 * writes it to the ouput file
*/
void writeTables(FILE*, struct task*);

/**
 *  Used for development, prints current readyqueue at 
 *  the given time to stdout
*/
void printQueue(struct task *readyQueue,int time);

/**
 *  this function calculates the time window in which
 *  a process will be executing and returns true if the 
 *  task is current withing the time window of the given
 *  time
*/
bool inTimeWindow(struct task t, int time);

/**
 * This function removes any tasks in the ready queue
 * that are no longer supposed to be in the queue
*/
int removeTasks(struct task** readyQueue, int time);

/**
 * this function will sort the ready queue first by priority, and 
 * if two processes have the same priority it sorts them by time 
 * remaining
*/
void sortReadyQueue(struct task** readyQueue);

/**
 *  this function returns true if the input task, proc, is 
 *  currently in the ready queue
*/
bool inReadyQueue(struct task* readyQueue, struct task proc);

/**
 * This function writes the ready queue to the output file when 
 * there is more than one process in the queue
*/
 void writeReadyQueue(FILE* output, struct task* readyQueue);

/**
 * this function is called for each time tick to add, remove, and sort all
 * processes that should be executing and waiting in the readyQueue. The head 
 * of the queue is the current process executing.
 * This function returns the number of processes in the ready queue
*/
int updateReadyQueue(struct task** readyQueue, struct task* tasks, int time);

/**
 * This function increments processes currently waiting in the ready queue
*/
void incrementReadyQueue(struct task** readyQueue);

/**
 * This function will implement the priority scheduling and 
 * print the correct information to the output file and 
 * update the data in the linked list.
*/
void priority_scheduling(FILE*, struct task*);

/**
 * This function will implement the custom scheduling and 
 * print the correct information to the output file and 
 * update the data in the linked list.
*/
void custom_scheduling(FILE*, struct task*);


/********************* Function Definitions *********************/

void parseInputFile(FILE* input, struct task** tasks) {
    // variables used in function
    int arrivalTime, serviceTime, priority;
    int task_counter = 0;
    struct task* prev = *tasks;

    // loop through input 1 line at a time until eof
    while (fscanf(input, "%d %d %d", &arrivalTime, &serviceTime, &priority) == 3) {
        // allocate temp struct to store data
        struct task* temp = malloc(sizeof(struct task));
        assert(temp != NULL); 
        // insert data
        temp->arrival_time = arrivalTime;
        temp->service_time = serviceTime;
        temp->priority = priority;
        temp->task_id = ((('Y'- 65) + task_counter++) % 26) + 65;
        temp->remaining_time = serviceTime; 
        temp->completion_time = 0; 
        temp->response_time = 0; 
        temp->wait_time = 0; 
        temp->next = NULL;

        temp->readyNext = NULL;
        temp->incremented_priority = priority;

        // attach to LL
        if(prev == NULL) // set first node to head
            *tasks = temp; 
        else
            prev->next = temp;
        
        prev = temp; 
    }
} // parseInputFile

void sortBytaskID(struct task** tasks) {
    struct task* sorted_head = NULL; 

    //empty or single element lists
    if (*tasks == NULL || (*tasks)->next == NULL) {
        return;
    }

    while (*tasks != NULL) {
        struct task* current = *tasks; 
        *tasks = (*tasks)->next; 

        // sorted list is empty or current is less than head
        if (sorted_head == NULL || sorted_head->task_id >= current->task_id) {
            current->next = sorted_head;
            sorted_head = current;
        } 
        else {
            struct task* temp = sorted_head;
            // loop until end of list or smaller value found
            while (temp->next != NULL && temp->next->task_id < current->task_id) {
                temp = temp->next; 
            }
            // insert into sorted list
            current->next = temp->next;
            temp->next = current;
        }
    }

    *tasks = sorted_head;
} // sortBytaskID

void writeTables(FILE* output, struct task* tasks){
    struct task* temp = tasks;

    // write table 1
    fprintf(output,"\n\t\t\t\tarrival\t\tservice\t\tcompletion\tresponse\twait\n");
    fprintf(output,"tid\t\tprio\t\ttime\t\ttime\t\ttime\t\ttime\t\ttime\n");
    fprintf(output,LARGE_SPACER);

    
    // loop through LL and write data formatted correctly
    while(tasks != NULL){
        fprintf(output,"%c\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
        tasks->task_id,tasks->priority,tasks->arrival_time,
        tasks->service_time,
        tasks->completion_time,
        tasks->response_time,
        tasks->wait_time);
        tasks = tasks->next;
    }
    

    // write table 2
    fprintf(output,"\n");
    fprintf(output,"service\t\twait\ntime\t\ttime\n");
    fprintf(output,SMALL_SPACER);

    // loop through LL and write data formatted correctly
    while(temp != NULL){
        fprintf(output,"%d\t\t%d\n",temp->service_time,temp->wait_time);
        temp = temp->next;
    }
}//writeTables

void incrementReadyQueue(struct task** readyQueue){
    struct task* queue = *readyQueue;
    // process running is skipped
    queue = queue->readyNext;
    // increment wait time for waiting processes
    while(queue != NULL){ 
        queue->wait_time++;
        queue = queue->readyNext;
    }
} //incrementReadyQueue

void printQueue(struct task *readyQueue,int time){
    printf("%d\t",time);
    while (readyQueue != NULL){
        printf("%c%d %d\t", readyQueue->task_id, readyQueue->remaining_time, readyQueue->incremented_priority);
        readyQueue = readyQueue->readyNext;
    }
    puts("");
} //printQueue

bool inTimeWindow(struct task t, int time){
     if (t.arrival_time <= time && time < t.arrival_time + t.service_time + t.wait_time) 
        return true;
    return false;
} //inTimeWindow

int removeTasks(struct task** readyQueue, int time){
    struct task* current = *readyQueue;
    struct task* prev = NULL;
    int count = 0;
    while (current != NULL) {
        // if processes has finished running
         if(current->remaining_time == 0){ 
            if (prev == NULL) { // Removing the first node
                *readyQueue = current->readyNext;
            } else {
                prev->readyNext = current->readyNext;
            }
            current->ready = false;
        }
        else{
            count++;
        }
        prev = current;
        current = current->readyNext;
    }
    // return number of processes ready
    return count;
}

void sortReadyQueue(struct task** readyQueue) {
    // ist is empty or has a single element
    if (*readyQueue == NULL || (*readyQueue)->readyNext == NULL) {
        return; 
    }

    struct task* sorted = NULL; 
    struct task* current = *readyQueue;

    while (current != NULL) {
        struct task* next = current->readyNext; // Keep the next item to process
        struct task** sort = &sorted;

        // find the correct spot to insert
        while (*sort != NULL) {
            bool shouldBreak = false;
            if ((*sort)->incremented_priority > current->incremented_priority ||
                ((*sort)->incremented_priority == current->incremented_priority && (*sort)->remaining_time > current->remaining_time)||
                (*sort)->incremented_priority == current->incremented_priority && (*sort)->remaining_time == current->remaining_time 
                        &&(*sort)->task_id < current->task_id) {
                current->readyNext = *sort;
                *sort = current;
                shouldBreak = true;
               
            }
            if (shouldBreak) // leave if inserted
                 break;
            sort = &(*sort)->readyNext;
        }

        if (*sort == NULL) { // append to end of sorted list, if not already inserted
            current->readyNext = *sort;
            *sort = current;
        }

        current = next; // iterate
    }

    *readyQueue = sorted; // set readyQueue to sorted
} //sortReadyQueue

bool inReadyQueue(struct task* readyQueue, struct task proc){
    while(readyQueue != NULL){
        if(readyQueue->task_id == proc.task_id)
            return true;
        readyQueue = readyQueue->readyNext;
    }
    return false;
} //inReadyQueue

int updateReadyQueue(struct task** readyQueue, struct task* tasks, int time){
    int count = 0;

    // remove tasks that are no longer ready
    count = removeTasks(readyQueue, time);
   
    // get tail to the back of the readyQueue or empty queue
    struct task* tail = *readyQueue;
    while (tail != NULL && tail->readyNext != NULL){
        tail = tail->readyNext;
    }
   

    // append ready tasks from tasks
    while (tasks != NULL) {
        if (inTimeWindow(*tasks, time) && !inReadyQueue(*readyQueue,*tasks)) { // Task is ready
            if (tail == NULL) { // Queue is empty
                *readyQueue = tasks; 
                tail = tasks; 
            } else { // Queue is not empty
                tail->readyNext = tasks; 
                tail = tasks;
            }
            tasks->ready = true; 
            tasks->readyNext = NULL;
            count++;
        } 
        tasks = tasks->next;   
    }
    // sort ready queue
    sortReadyQueue(readyQueue); 
    return count;
} //updateReadyQueue

void writeReadyQueue(FILE* output, struct task* readyQueue){
    readyQueue = readyQueue->readyNext;
    while(readyQueue != NULL){
        fprintf(output,"\t%c%d %d", readyQueue->task_id, readyQueue->remaining_time, readyQueue->incremented_priority);
        readyQueue = readyQueue->readyNext;
    }
    fprintf(output,"\n");
 }//writeReadyQueue

void priority_scheduling(FILE* output, struct task* tasks){
    // prrint headers
    fprintf(output, "Priority Scheduling Results:\nTime\tCPU\tPriority\tReady Queue (tid/rst)\n");
    fprintf(output,SPACER);

    struct task* readyQueue = NULL;
    int time = 0;
    int num = 0;
       
    while (tasks != NULL){
        //update readyQueue after each tick
        
         num = updateReadyQueue(&readyQueue, tasks, time);
       
        if (num > 1){ // waiting tasks
            incrementReadyQueue(&readyQueue);   // increment waiting tasks
            fprintf(output, "%d\t%c%d\t%d\t",time,readyQueue->task_id,readyQueue->remaining_time--,readyQueue->priority);
            writeReadyQueue(output,readyQueue);
    
        }
        else if (num == 1) { // one task
                
            fprintf(output,"%d\t%c%d\t%d\t\t--\n",time,readyQueue->task_id,readyQueue->remaining_time--,readyQueue->priority);
        }
        else{ // no tasks
            if(tasks->next != NULL)
                fprintf(output,"%d\t\t\t\t--\t\n",time);  
        }

        // if task is finished move to the next one
        if(tasks->remaining_time == 0){
            tasks->completion_time = tasks->service_time + tasks->wait_time + tasks->arrival_time;
            tasks->response_time = tasks->service_time + tasks->wait_time;
            tasks = tasks->next;
        }
       // increment time
        time++;
    }
}// priority_scheduling

void custom_scheduling(FILE* output, struct task* tasks){
    // write headers
    fprintf(output, "Custom(preemptive) Scheduling Results:\nTime\tCPU\tPriority\tReady Queue (tid/rst)\n");
    fprintf(output,SPACER);


    struct task* readyQueue = NULL;
    int time = 0;
    int num = 0;
       
    while (tasks != NULL){
        //update readyQueue after each tick
        
        num = updateReadyQueue(&readyQueue, tasks, time);
        if (num > 1){ // tasks waiting
            incrementReadyQueue(&readyQueue);
            fprintf(output, "%d\t%c%d\t%d\t",time,readyQueue->task_id,readyQueue->remaining_time--,readyQueue->incremented_priority++);
            writeReadyQueue(output,readyQueue);
        }
        else if (num == 1) { // one tasks
            fprintf(output,"%d\t%c%d\t%d\t\t--\n",time,readyQueue->task_id,readyQueue->remaining_time--,readyQueue->incremented_priority++);
        }   
        else{ // no tasks
            if(tasks->next != NULL)
                fprintf(output,"%d\t\t\t\t--\t\n",time);  
            
        }
       
       // task has finished running
       if(tasks->remaining_time == 0){
            tasks->completion_time = tasks->service_time + tasks->wait_time + tasks->arrival_time;
            tasks->response_time = tasks->service_time + tasks->wait_time;
            tasks = tasks->next;
       }
        time++;
    }


}// custom_scheduling

/********************* Main *********************/
int main(int argc, char** argv) {
    bool prio; 

    // Check for correct number of arguments
    assert(argc == 4);

    // Parse the scheduling type
    if (strcmp(argv[1], "-prio") == 0) {
        prio = true;
    } 
    else if (strcmp(argv[1], "-custom") == 0) {
        prio = false;
    } 
    else {
        exit(1); // Invalid flag
    }

    // open files
    FILE* input = fopen(argv[2], "r");
    FILE* output = fopen(argv[3], "w");
    
    // check if files opened correctly 
    assert(input != 0);
    assert(output != 0);

    struct task* tasks = NULL; 

    parseInputFile(input, &tasks);


    if(prio){
        priority_scheduling(output,tasks);
    }
    else{
       custom_scheduling(output,tasks);
    }
    
    sortBytaskID(&tasks);
     writeTables(output, tasks);

    // close files
    fclose(input);
    fclose(output);


    // free allocated memory
    while (tasks != NULL) {
        struct task* temp = tasks;
        tasks = tasks->next;
        free(temp);
    }

    return 0;
} // main