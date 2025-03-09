#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_MEMORY 0       
#define MAX_MEMORY 268435456  // 256 MB
#define MAX_COMMAND_LENGTH 256
#define MAX_PROCESS_NAME 32

// Global variables
void* memory;                 // Pointer to the memory pool
size_t total_memory;          // Total size of the memory pool

// Add these enums for clarity
typedef enum { // source in readme 
    FIRST_FIT = 'F',
    BEST_FIT = 'B',
    WORST_FIT = 'W'
} AllocationStrategy;

// Structure to represent a memory block
typedef struct MemoryBlock { // source in readme 
    size_t start_address;     // Starting address of the block
    size_t size;             // Size of the block
    char process_name[MAX_PROCESS_NAME];  // Name of process (empty if free)
    int is_free;             // 1 if block is free, 0 if allocated
    struct MemoryBlock* next;  // Pointer to next block
} MemoryBlock;

// Global pointer to the head of our memory block list
MemoryBlock* memory_blocks = NULL;

// Add these function prototypes after the global variables and before any function definitions
void init_memory_block();
int allocate_memory(const char* process_name, size_t size, AllocationStrategy strategy);
int release_memory(const char* process_name);
void print_memory_status();
void compact_memory(void); // causing error 

void initialize_memory(size_t memory_size) {
    memory = malloc(memory_size);

    if (memory == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        exit(-1);
    }
    total_memory = memory_size;

    // Initialize memory as one large free block
    memset(memory, 0, memory_size);

    // Initialize the memory block tracking
    init_memory_block();
}

void init_memory_block() {
    memory_blocks = malloc(sizeof(MemoryBlock));
    if (memory_blocks == NULL) {
        fprintf(stderr, "Failed to initialize memory block.\n");
        exit(1);
    }
    
    memory_blocks->start_address = 0;
    memory_blocks->size = total_memory;
    memory_blocks->is_free = 1;
    memory_blocks->process_name[0] = '\0';
    memory_blocks->next = NULL;
}

void process_command() {
    char cmd[MAX_COMMAND_LENGTH];
    char process_name[MAX_PROCESS_NAME];
    char command[MAX_COMMAND_LENGTH];
    
    size_t size;
    char strategy; // worst fit, best fit, first fit
    
    while (1) {
        printf("allocator>");
        fflush(stdout); // Flush the output buffer to ensure the prompt is displayed
        // source in readme 
        
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        // Remove trailing newline
        command[strcspn(command, "\n")] = 0;
        
        // Parse command
        if (strcmp(command, "X") == 0) {
            break;
        }
        else if (sscanf(command, "%s", cmd) == 1) {
            if (strcmp(cmd, "STAT") == 0) {
                print_memory_status();
            }
            else if (strcmp(cmd, "C") == 0) {
                compact_memory();
                // printf("Memory compaction completed.\n"); // in function
            }
            else if (strcmp(cmd, "RQ") == 0) {
                if (sscanf(command, "%s %s %zu %c", cmd, process_name, &size, &strategy) == 4) {
                    if (strategy == FIRST_FIT || strategy == BEST_FIT || strategy == WORST_FIT) {
                        if (allocate_memory(process_name, size, strategy)) {
                            printf("Allocated %zu bytes for process %s using %c strategy.\n", size, process_name, strategy);
                        }
                    } else {
                        printf("Invalid allocation strategy. Use B (best-fit), F (first-fit), or W (worst-fit).\n");
                    }
                } else {
                    printf("Invalid command. Format: RQ <process_name> <size> <strategy>.\n");
                }
            }
            else if (strcmp(cmd, "RL") == 0) {
                if (sscanf(command, "%s %s", cmd, process_name) == 2) {
                    if (release_memory(process_name)) {
                        printf("Released memory for process %s.\n", process_name);
                    }
                } else {
                    printf("Invalid command. Format: RL <process_name>.\n");
                }
            }
            else {
                printf("Invalid command or invalid memory allocation.\n");
            }
        }
    }
}

void compact_memory() {
    MemoryBlock* current = memory_blocks;
    MemoryBlock* next_allocated = NULL;
    size_t next_free_addr = 0;
    
    // First pass: Move allocated blocks to their new positions
    while (current != NULL) {
        if (!current->is_free) {
            // Move the block's contents to the next available position
            if (current->start_address != next_free_addr) {
                // Move actual memory contents
                memmove(memory + next_free_addr, 
                       memory + current->start_address, 
                       current->size);
                current->start_address = next_free_addr;
            }
            next_free_addr += current->size;
        }
        current = current->next;
    }
    
    // Second pass: Merge all free blocks at the end
    current = memory_blocks;
    MemoryBlock* prev = NULL;
    
    // Find the last allocated block
    while (current != NULL) {
        if (!current->is_free) {
            prev = current;
            current = current->next;
        } else {
            // If this is the first block and it's free
            if (prev == NULL) {
                memory_blocks = current->next;
                free(current);
                current = memory_blocks;
            } else {
                // Remove this free block
                prev->next = current->next;
                free(current);
                current = prev->next;
            }
        }
    }
    
    // Create one large free block at the end if there's space left
    if (next_free_addr < total_memory) {
        MemoryBlock* free_block = malloc(sizeof(MemoryBlock));
        if (free_block == NULL) {
            fprintf(stderr, "Memory allocation failed during compaction.\n");
            return;
        }
        
        free_block->start_address = next_free_addr;
        free_block->size = total_memory - next_free_addr;
        free_block->is_free = 1;
        free_block->process_name[0] = '\0';
        free_block->next = NULL;
        
        // If there are no allocated blocks
        if (prev == NULL) {
            memory_blocks = free_block;
        } else {
            prev->next = free_block;
        }
    }
    
    printf("Memory compaction completed.\n");
}

void print_memory_status() {
    MemoryBlock* current = memory_blocks;
    
    if (current == NULL) {
        // If no blocks exist, print entire memory as unused
        printf("Addresses [0:%zu] Unused\n", total_memory - 1);
        return;
    }
    
    // Print first block if there's a gap at the start
    if (current->start_address > 0) {
        printf("Addresses [0:%zu] Unused\n", current->start_address - 1);
    }
    
    while (current != NULL) {
        // Print current blocka
        if (current->is_free) {
            printf("Addresses [%zu:%zu] Unused\n", 
                   current->start_address, 
                   current->start_address + current->size - 1);
        } else {
            printf("Addresses [%zu:%zu] Process %s\n", 
                   current->start_address, 
                   current->start_address + current->size - 1,
                   current->process_name);
        }
        
        // Check for gap between current and next block
        if (current->next != NULL) {
            size_t next_start = current->next->start_address;
            size_t expected_next = current->start_address + current->size;
            if (next_start > expected_next) {
                printf("Addresses [%zu:%zu] Unused\n", 
                       expected_next, 
                       next_start - 1);
            }
        }
        
        current = current->next;
    }
    
    // Check for gap at the end
    MemoryBlock* last = memory_blocks;
    while (last->next != NULL) {
        last = last->next;
    }
    
    size_t last_addr = last->start_address + last->size;
    if (last_addr < total_memory) {
        printf("Addresses [%zu:%zu] Unused\n", 
               last_addr, 
               total_memory - 1);
    }
}

// Add these function prototypes and implementations

// Helper function to find a block using First Fit strategy
MemoryBlock* find_first_fit(size_t size) {
    MemoryBlock* current = memory_blocks;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Helper function to find a block using Best Fit strategy
MemoryBlock* find_best_fit(size_t size) {
    MemoryBlock* current = memory_blocks;
    MemoryBlock* best_block = NULL;
    size_t smallest_difference = total_memory + 1;  // Initialize to larger than possible
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            size_t difference = current->size - size;
            if (difference < smallest_difference) {
                smallest_difference = difference;
                best_block = current;
            }
        }
        current = current->next;
    }
    return best_block;
}

// Helper function to find a block using Worst Fit strategy
MemoryBlock* find_worst_fit(size_t size) {
    MemoryBlock* current = memory_blocks;
    MemoryBlock* worst_block = NULL;
    size_t largest_difference = 0;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            size_t difference = current->size - size;
            if (difference > largest_difference) {
                largest_difference = difference;
                worst_block = current;
            }
        }
        current = current->next;
    }
    return worst_block;
}

// Helper function to split a block if necessary
void split_block(MemoryBlock* block, size_t size, const char* process_name) {
    // If the block is significantly larger than requested size, split it
    if (block->size > size + sizeof(MemoryBlock)) {
        MemoryBlock* new_block = malloc(sizeof(MemoryBlock));
        if (new_block == NULL) {
            fprintf(stderr, "Failed to allocate memory for new block.\n");
            return;
        }
        
        // Set up the new free block
        new_block->start_address = block->start_address + size;
        new_block->size = block->size - size;
        new_block->is_free = 1;
        new_block->process_name[0] = '\0';
        new_block->next = block->next;
        
        // Update the allocated block
        block->size = size;
        block->next = new_block;
    }
    
    // Update the allocated block's information
    block->is_free = 0;
    strncpy(block->process_name, process_name, MAX_PROCESS_NAME - 1);
    block->process_name[MAX_PROCESS_NAME - 1] = '\0';
}

// Function to allocate memory using the specified strategy
int allocate_memory(const char* process_name, size_t size, AllocationStrategy strategy) {
    MemoryBlock* chosen_block = NULL;
    
    // Find a suitable block based on the strategy
    switch (strategy) {
        case FIRST_FIT:
            chosen_block = find_first_fit(size);
            break;
        case BEST_FIT:
            chosen_block = find_best_fit(size);
            break;
        case WORST_FIT:
            chosen_block = find_worst_fit(size);
            break;
    }
    
    if (chosen_block == NULL) {
        printf("Error: No block found for allocation.\n");
        return 0;
    }
    
    split_block(chosen_block, size, process_name);
    return 1;
}

// Helper function to merge adjacent free blocks
void coalesce_free_blocks() {
    MemoryBlock* current = memory_blocks;
    
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            // Merge with next block
            current->size += current->next->size;
            MemoryBlock* temp = current->next;
            current->next = temp->next;
            free(temp);
            // Don't advance current as there might be more blocks to merge
        } else {
            current = current->next;
        }
    }
}

// Function to release memory for a process
/*
Your program will also need to keep track of which region of memory has been allocated to
which process. This is necessary to support the STAT command and is also needed when
memory is released via the RL command, as the process releasing memory is passed to this
command. If a partition being released is adjacent to an existing hole, be sure to combine the two
holes into a single hole.
*/
int release_memory(const char* process_name) {
    MemoryBlock* current = memory_blocks;
    int found = 0;
    
    // Find all blocks allocated to this process
    while (current != NULL) {
        if (!current->is_free && strcmp(current->process_name, process_name) == 0) {
            // Mark block as free
            current->is_free = 1;
            current->process_name[0] = '\0';
            found = 1;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("Error: Process %s not found.\n", process_name);
        return 0;
    }
    
    // Merge any adjacent free blocks
    coalesce_free_blocks();
    return 1;
}

int main(int argc, char *argv[]) {
    // Check if memory size argument is provided
    if (argc != 2) {
        fprintf(stderr, "Invalid input.");
        return -1;
    }

    // Convert argument to integer
    size_t memory_size = atol(argv[1]); // readme 

    // Validate memory size
    if (memory_size < MIN_MEMORY || memory_size > MAX_MEMORY) {
        fprintf(stderr, "Memory size must be between %d and %d bytes.\n", 
                MIN_MEMORY, MAX_MEMORY);
        return -1;
    }

    // Initialize the memory pool
    initialize_memory(memory_size);

    // Process commands until exit
    process_command();

    // Clean up
    free(memory);
    return 0;
}
