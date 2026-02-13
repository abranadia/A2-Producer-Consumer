
// buffer.h - Shared definitions for Producer/Consumer (Bounded Buffer)

#ifndef BUFFER_H
#define BUFFER_H

// Needed for some POSIX functions (like usleep) when compiling with -std=c11
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

// ===== Required includes for both producer and consumer =====
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <semaphore.h>
#include <fcntl.h>

#include <string.h>
#include <signal.h>
#include <time.h>

//  Constants for shared memory and semaphores 
#define BUFFER_SIZE 10
#define SHM_KEY 0x1234

// Named POSIX semaphores (must match producer.c / consumer.c)
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL  "/sem_full"

// Student Info 
// Name: Nadia Akpalu
// Section: W03

// Data Types 

// Item stored in the buffer
typedef struct {
    int value;        // Data value produced/consumed
    int producer_id;  // Which producer created this item
} item_t;

// Shared circular buffer stored in shared memory
typedef struct {
    item_t buffer[BUFFER_SIZE];
    int head;   // Next write position (producer)
    int tail;   // Next read position (consumer)
    int count;  // Number of items currently in the buffer
} shared_buffer_t;

#endif // BUFFER_H

