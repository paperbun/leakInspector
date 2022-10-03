/* 
    This tools just have a wrapper implementation for malloc and free
*/
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <cstdlib>
#include "leakInspector.h"

/*
    Address
    File_name
    LineNo
*/

using namespace std;

struct Node *head = NULL;


void dumpReport(){


    struct Node *temp = head;

    std::ofstream outfile;
    outfile.open("leakReport.txt", std::ios_base::out); // append instead of overwrite

    while(temp != NULL){
        outfile << "Size - " << temp -> alloc -> size << "     ";
        outfile << "Address - " << temp -> alloc -> address << "    ";
        outfile << endl;
        
        struct TraceInfo *traceInfo =  temp -> alloc ->traceInfo;
        for(int i = 0; i < traceInfo -> noOfsymbols; i++){
            for(int j = 0; traceInfo ->symbols[i][j] != '\0'; j++){
                outfile << traceInfo -> symbols[i][j];
            }
            outfile << endl;
        }
        
        outfile << endl;
        temp = temp -> next;
    }

    outfile.close();
}

/* Creates a node with allocInfo data i.e. lineNo, fileName, address, size */



struct Node* createNode(void *address, int size, struct TraceInfo* traceInfo){
   
    // Create a allocation info
    struct AllocInfo *alloc = NULL;
    
    alloc =  (struct AllocInfo*)malloc(sizeof(struct AllocInfo));

    alloc->address = address;
    alloc->size = size;
    alloc->traceInfo = traceInfo;
    
    // Create a list node
    struct Node *node = NULL;
    node = (struct Node*)malloc(sizeof(struct Node));

    node->alloc = alloc;
    node->next = NULL;

    return node;
}

void insertNode(void *address, int size, struct TraceInfo* traceInfo){

    struct Node *node = createNode(address, size, traceInfo);

    // Insert the node if head is already NULL
    if(head == NULL){
        head = node;
    }

    // If head is not NULL, insert the node at beginning.
    else {
        node -> next = head;
        head = node;        
    }
}


  void clearNode(struct Node *tempHead){

      int symbolsLength = tempHead -> alloc -> traceInfo -> noOfsymbols;

      // Clean all the symbols in the stack trace
      for(int i = 0; i < symbolsLength; i++){
          free(tempHead ->alloc ->traceInfo ->symbols[i]);
      }

      // Clean the Double pointer as well
      free(tempHead -> alloc -> traceInfo ->symbols);
      free(tempHead -> alloc -> traceInfo);
      free(tempHead -> alloc);

  }
  

// Delete the node if matches with given address.
void deleteNode(void* address){

    // Traverse till the end of the list and check if the node is present or not.
    // If present delete it.

    struct Node* tempHead = head;

    if(head == NULL)
        return;

    // Check the headNode first whether it can be matched with the Node to be removed
    if(tempHead -> alloc != NULL && tempHead -> alloc -> address == address){
        head = tempHead -> next;
        clearNode(tempHead);
        return;
    }


    while(tempHead -> next != NULL){

        if((tempHead -> next) -> alloc -> address == address){

            // Save the node for deletion
            struct Node *temp = tempHead -> next;

            // Update the next node;
            tempHead -> next = tempHead -> next -> next;

            clearNode(temp);
            return;
        }

        tempHead = tempHead -> next;
    }

}