#include <assert.h>
#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "Queue.h"
#include "PQueue.h"
#include "structs.h"
#include <stdbool.h>
#include <errno.h>
typedef struct MemoryBlock {
    int start;  // Start address of the block
    int end;     //End  address of the blocl
    int size;   // Size of the block
    int is_free; // true if free, false if allocated
    int is_Divided;
    struct MemoryBlock* left;  // Left child
    struct MemoryBlock* right; // Right child 
    int pid;//process id(from txt file)

} MemoryBlock;
MemoryBlock* initialize_memory(int total_size) {
    MemoryBlock* root = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    root->start = 0;
    root->end=total_size;
    root->size = total_size;
    root->is_free = 1;
    root->is_Divided = 0;
    root->pid=-1;
    root->left = root->right = NULL;
    return root;
}


MemoryBlock* allocate_memory(MemoryBlock* node, int size,int pid) 
{
    if (!node || !node->is_free) return NULL;

 
    int rounded_size = 1;//first possible memory block size
    while (rounded_size < size) {
        rounded_size *= 2;
    }

    printf("--------FOR SIZE %d at Node %d--------------\n",size,node->size);

    if (node->size == rounded_size && node->is_Divided!=1) //allocation of memory block
    {
        printf("check \n");
        node->pid=pid;
        node->is_free = 0;
         printf("Allocated %d bytes (requested %d bytes) for PID %d at start %d and end %d\n",
               node->size, size, pid, node->start,node->end);
        return node;
    }   

    if (node->size > rounded_size ) 
    {
        //Split the block if not already split
        if (!node->left && !node->right) {
            node->is_Divided=1;
            //Creating left memory block
            int half_size = node->size / 2;
            node->left = initialize_memory(half_size);
            node->left->start = node->start;
            node->left->end= node->start + half_size;    
            printf("Node:%d divided to Left Node:%d from %d to %d\n \n",node->size,node->left->size,node->left->start,node->left->end);


            //Create right child block
            node->right = initialize_memory(half_size);
            node->right->start = node->start + half_size;
            node->right->end= node->end;
            printf("Node:%d divided to Right Node:%d from %d to %d\n \n",node->size,node->right->size,node->right->start,node->right->end);

        }
 //Try allocating in left or right
    MemoryBlock* allocated = NULL;
        if (node->left && node->left->is_free != 0) 
        {
            printf("---Going Left----\n");
            allocated = allocate_memory(node->left, size, pid);
        }
        if (!allocated && node->right && node->right->is_free != 0)
         {
            printf("---Going Right----\n");
            allocated = allocate_memory(node->right, size, pid);

        }
        return allocated;
    }
    printf("No Memory Available");
    return NULL; // No suitable block found
}




int MergeBlocks(MemoryBlock* root) {
    if (root == NULL) {
        return 0;  // Nothing to merge
    }

    // If no children, nothing to merge
    if (root->left == NULL || root->right == NULL) {
        return 0;
    }

    // Ensure both children are leaf nodes and free
    if (root->left->is_free && root->right->is_free &&
        root->left->left == NULL && root->right->left == NULL) {
        
        printf("\nMerging blocks of size %d and %d with address from %d to %d\n", 
               root->left->size, root->right->size, root->left->start, root->right->end);
        
        free(root->left);
        free(root->right);
        root->left = NULL;
        root->right = NULL;
        root->is_free = 1;
        root->is_Divided = 0;
        return 1;  // Successful merge
    }

    // Recursively attempt merging on children
    int left_merge = MergeBlocks(root->left);
    int right_merge = MergeBlocks(root->right);
    
    // Reevaluate current node for merging after children are handled
    if (left_merge || right_merge) {
        return MergeBlocks(root);
    }

    return 0;  // Nothing merged
}


int deallocate_memory(MemoryBlock *root, int NodeId)
{
    if (root == NULL) {
        return 0; 
    }

    printf("\nROOT: %d\n", root->size);
    //Check and deallocate left child
    if (root->left && root->left->pid == NodeId) {
        printf("Deallocating Memory size: %d in left\n", root->left->size);
        root->left->is_free = 1;
        root->left->pid = -1;
          int m=MergeBlocks(root); // Attempt to merge after freeing
        printf("\nMERGING CODE %d\n",m);
        MergeBlocks(root); // Attempt to merge after freeing
        return 1; 
    }

    //Check and deallocate right child
    if (root->right && root->right->pid == NodeId) {
        printf("Deallocating Memory size: %d in right\n", root->right->size);
        root->right->is_free = 1;
        root->right->pid = -1;
        int m=MergeBlocks(root); // Attempt to merge after freeing
        printf("\nMERGING CODE %d\n",m);
        return 1; // Successful deallocation
    }

    //Recursively search in the left and right subtrees
    int dealloc = 0;
    if (root->left) {
        dealloc = deallocate_memory(root->left, NodeId);
    }
    if (!dealloc && root->right) { // Only search right if not already deallocated
        dealloc = deallocate_memory(root->right, NodeId);
    }

    MergeBlocks(root);

    return dealloc;
}




// // -----------Testing Memory-----------////
// int main() {
// //Initialize memory tree with 1024 bytes
//      MemoryBlock* memory_root = initialize_memory(1024);

//     // // Test allocations
//     //  allocate_memory(memory_root, 120, 1); // Allocate 120 bytes for PID 1
//     //  allocate_memory(memory_root, 64, 2);  // Allocate 64 bytes for PID 2
//     //  allocate_memory(memory_root, 512, 3); // Allocate 512 bytes for PID 3
//     //  allocate_memory(memory_root, 200, 4); // Allocate 200 bytes for PID 4
//     //  allocate_memory(memory_root, 200, 5); // Allocate 200 bytes for PID 4
//     allocate_memory(memory_root, 256, 0); 
//     allocate_memory(memory_root, 256, 1); 
//     deallocate_memory(memory_root,0);




//     return 0;
//  }