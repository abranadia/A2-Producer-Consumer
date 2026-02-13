// producer.c - Producer process
// Name: Nadia Akpalu
// Section: W03

#include "buffer.h"

// Global variables for cleanup
shared_buffer_t* buffer = NULL;
sem_t* mutex = NULL;
sem_t* empty = NULL;
sem_t* full = NULL;
int shm_id = -1;

void cleanup() {
    // Detach shared memory
    if (buffer != NULL) {
        shmdt(buffer);
    }
    
    // Close semaphores (don't unlink - other processes may be using)
    if (mutex != SEM_FAILED) sem_close(mutex);
    if (empty != SEM_FAILED) sem_close(empty);
    if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig) {
    printf("\nProducer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        exit(1);
    }
    
    int producer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Seed random number generator
    srand(time(NULL) + producer_id);
    
    // Attach to shared memory (create if it doesn't exist)
    int created = 0;
    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        // Already exists - just get it
        shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
        if (shm_id == -1) {
            perror("Producer: shmget");
            exit(1);
        }
    } else {
        created = 1;
    }

    buffer = (shared_buffer_t*)shmat(shm_id, NULL, 0);
    if (buffer == (void*)-1) {
        perror("Producer: shmat");
        exit(1);
    }

    // If we created the shared memory, initialize it
    if (created) {
        memset(buffer, 0, sizeof(shared_buffer_t));
        buffer->head = 0;
        buffer->tail = 0;
        buffer->count = 0;

        // Clean up old named semaphores from a previous run (if any)
        sem_unlink(SEM_MUTEX);
        sem_unlink(SEM_EMPTY);
        sem_unlink(SEM_FULL);
    }

    // Open/Create semaphores
    // If this producer created the shared memory, also create semaphores with initial values.
    if (created) {
        mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0644, 1);
        empty = sem_open(SEM_EMPTY, O_CREAT | O_EXCL, 0644, BUFFER_SIZE);
        full  = sem_open(SEM_FULL,  O_CREAT | O_EXCL, 0644, 0);
    } else {
        mutex = sem_open(SEM_MUTEX, 0);
        empty = sem_open(SEM_EMPTY, 0);
        full  = sem_open(SEM_FULL,  0);
    }

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("Producer: sem_open");
        cleanup();
        exit(1);
    }
    
    printf("Producer %d: Starting to produce %d items\n", producer_id, num_items);
    
    // Main production loop
    for (int i = 0; i < num_items; i++) {
        // Create item
        item_t item;
        item.value = producer_id * 1000 + i;
        item.producer_id = producer_id;
        
        // Wait for an empty slot, then enter critical section
        sem_wait(empty);
        sem_wait(mutex);

        // Add item to buffer (circular buffer)
        buffer->buffer[buffer->head] = item;
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->count++;

        printf("Producer %d: Produced value %d\n", producer_id, item.value);
        
        // Exit critical section and signal that an item is available
        sem_post(mutex);
        sem_post(full);
        
        // Simulate production time
        usleep(rand() % 100000);
    }
    
    printf("Producer %d: Finished producing %d items\n", producer_id, num_items);
    cleanup();
    return 0;
}