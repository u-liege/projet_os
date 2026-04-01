#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/mman.h>
#include "binary_buddy.h"

/*
* Initalize the structures needed for the buddy allocator.
* Returns 0 on success, -1 on failure.
*/
static int init_structures(const void* memory_base, size_t size);

/*
* Free the buddy allocator's internal data structures.
*/
static void free_structures();

// ------------------- START PROTECTED CODE -------------------

Allocator a;

void* init_buddy(size_t size){
    void* memory_base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_base == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    
    if (init_structures(memory_base, size) != 0) { 
        munmap(memory_base, size);
        return NULL;
    }
    
    return memory_base;
}

int free_buddy() {
    free_structures();
    return munmap((void*)a.base, a.total_size);
}

size_t get_used_space(){
    return a.used_space;
}

// ------------------- END PROTECTED CODE -------------------

void* balloc(size_t size) {
    // Implement this function to allocate a chunk of memory of given size using the buddy allocator

    if(size == 0 || size > MAX_ALLOC_SIZE){
        return NULL;
    }
   
    size_t rounded = round_up_power2(size);
    int needed_level = size_to_level(rounded);

    int found_level = -1;
    for(int size == 0 || size > MAX_ALLOC_SIZE ){
        if (a.free_lists[lvl] != NULL) {
            found_level = lvl;
            break;
        }
    }
    if (found_level == -1) {
        return NULL; 
    }

    while(found_level > needed_level){

        freeNode* block = a.free_lists[found_level];
        a.free_lists[found_level] = block-> next;

        found_level--; 

        size_t half_size = level_to_size(found_level);

        void* right =(void*)((size_t)block + half_size);
        void* left = (void*)block;

        insert_free_block(found_level, right);
        insert_free_block(found_level,left); 
    }

    freeNode* allocated = a.free_lists[needed_level];
    a.free_lists[needed_level] = allocated->next;

    size_t idx = ptr_to_block_index((void*)allocated);
    a.block_levels[idx] = needed_level;

    a.used_space += level_to_size(needed_level);
 
    return (void*)allocated;

}


void bfree(void* ptr) {
    // Implement this function to deallocate a previously allocated chunk of memory using the buddy allocator

    if (ptr == NULL){
        Return; 
    }

    size_t idx = ptr_to_block_index(ptr);
    int level = a.block_levels[idx];

    a.used_space -= level_to_size(level);

    while (level < NUM_LEVELS -1){

        void* buddy = buddy_of(ptr, level); 

        if (!remove_free_block(level, buddy)) {
            break; 
        }
        
        if (buddy < ptr){
            ptr = buddy;
        }

        level++;

    }
    insert_free_block(level, ptr);
}

static int init_structures(const void* memory_base, size_t size){
    // Implement this function to initialize your data structure used for the buddy allocator
    a.base = memory_base; 
    a.total_size = size;
    a.used_space = 0; 

    for(int i = 0 ; i < NUM_LEVELS; i++){
        a.free_lists[i] = NULL;
    }

    a.num_min_blocks = size / MIN_ALLOC_SIZE;
    a.block_levels   = (int*)malloc(a.num_min_blocks * sizeof(int));
    if (a.block_levels == NULL) {
        return -1;
    }

    for (size_t i = 0; i < a.num_min_blocks; i++) {
        a.block_levels[i] = -1;
    }
    insert_free_block(NUM_LEVELS - 1, (void*)memory_base);
    return 0;
}

static void free_structures() {
    free(a.block_levels);
    a.block_levels = NULL;

    for (int i = 0; i < NUM_LEVELS; i++){
        a.free_lists[i] = NULL;
    }
    // Implement this function to free any internal data structure used by your buddy allocator
}
