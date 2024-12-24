#include <assert.h>
#include"headers.h"
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
        // Split the block if not already split
        if (!node->left && !node->right) {
            node->is_Divided=1;
            //Creating left memory block
            int half_size = node->size / 2;
            node->left = initialize_memory(half_size);
            node->left->start = node->start;
            node->left->end= node->start + half_size;    
            printf("Node:%d divided to Left Node:%d from %d to %d\n \n",node->size,node->left->size,node->left->start,node->left->end);


            // Create right child block
            node->right = initialize_memory(half_size);
            node->right->start = node->start + half_size;
            node->right->end= node->end;
            printf("Node:%d divided to Right Node:%d from %d to %d\n \n",node->size,node->right->size,node->right->start,node->right->end);

        }
 // Try allocating in left or right
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




int MergeBlocks(MemoryBlock*root)
{
    printf("\nMerging size:%d\n",root->size);
    if(root== NULL)
    {
        return 0;
    }
    if ( root->left == NULL || root->right == NULL)//nothing to merge
    {
        printf("Childern Nothing to merge");
        return 0;
    }
    if( root->left->left || root->left->right || root->right->left || root->right->right)//children still unmerged
    {
        printf("Childern Still unmegred");
        return 0;
    }

    if ((root->left->is_free) && (root->right->is_free)) //leaves
    {
        printf("\nMerging blocks of size %d and %d   With address from %d to %d\n ", root->left->size, root->right->size,root->left->start,root->right->end);
            free(root->left);
            free(root->right);

            root->left = NULL;
            root->right = NULL;
            root->is_free = 1;
            root->is_Divided=0;
            printf("\nMERGEEE DONEEEE\n");
            return 1;
    }
return 0;
}

int deallocate_memory(MemoryBlock *root, int NodeId)
{
    if (root == NULL || !root->right && !root->left) {
        return 0; 
    }

    printf("\nROOT: %d\n", root->size);

    // Check and deallocate left child
    if (root->left && root->left->pid == NodeId) {
        printf("Deallocating Memory size: %d in left\n", root->left->size);
        root->left->is_free = 1;
        root->left->pid = -1;
        MergeBlocks(root); // Attempt to merge after freeing
        return 1; 
    }

    // Check and deallocate right child
    if (root->right && root->right->pid == NodeId) {
        printf("Deallocating Memory size: %d in right\n", root->right->size);
        root->right->is_free = 1;
        root->right->pid = -1;
        MergeBlocks(root); // Attempt to merge after freeing
        return 1; // Successful deallocation
    }

    // Recursively search in the left and right subtrees
    int dealloc = 0;
    if (root->left) {
        dealloc = deallocate_memory(root->left, NodeId);
    }
    if (!dealloc && root->right) { // Only search right if not already deallocated
        dealloc = deallocate_memory(root->right, NodeId);
    }
    printf("\nSTART MERGING AGAIN\n");
    MergeBlocks(root);
    printf("\nI AM DONE\n");
    return dealloc;
}