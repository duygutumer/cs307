#ifndef _HEADER_H
#define _HEADER_H
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>

using namespace std;

pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER; 

struct linked_list
{
    linked_list* right; // node to the right
    linked_list* left; // node to the left
    int size; // the number of bytes reserved 
    int index; // the starting address
    int id; // thread id
};

class HeapManager 
{
private:
    linked_list* head;
public:
    HeapManager();
    ~HeapManager();
    int initHeap(int size);
    int myMalloc(int ID, int size);
    int myFree(int ID, int index);
    void print();
};

HeapManager::HeapManager()
{
    head = new linked_list();
}

void HeapManager::print()
{
    pthread_mutex_lock(&printLock);
    linked_list* node = head;
    if(node->right == NULL && node->left == NULL) // base case: if it is one node
        cout << "[" << node->id << "]" << "[" << node->size << "]"<< "[" << node->index << "]" << endl;
    else
    {
        while(node != NULL)
        {
            cout << "[" << node->id << "]" << "[" << node->size << "]" << "[" << node->index << "]";
            if(node->right != NULL)
            {
                cout << "---";
            }
            
            node = node->right;
        }
        cout << endl;
    }  
    pthread_mutex_unlock(&printLock); 
}

int HeapManager::initHeap(int size) //Initializes the list with a single free node with given input size and start index 0. 
{
    //[-1][100][0]
    head->right = NULL;
    head->left = NULL;
    head->id = -1;
    head->size = size;
    head->index = 0;
    //print();
    return 1;
}

int HeapManager::myMalloc(int ID, int size)
{
    pthread_mutex_lock(&sharedLock);
    //[0][40][0]---[-1][60][40]
    linked_list* node = head;
    while(node != NULL && (node->id >= 0 || node->size < size))
    {
        node = node->right;
    }
    
    if(node == NULL)
    {
        cout << "Can not allocate, requested size " << size << " for thread " << ID << " is bigger than remaining size" << endl;
        
        pthread_mutex_unlock(&sharedLock);
        return -1;
    }

    cout << "Allocated for thread " << ID << endl;

    if((node->size) - size != 0)
    {

        if(node->right == NULL)
        {
            linked_list* freenode = new linked_list(); //since it has to be splited
            node->right = freenode; // create a new node and add it
            freenode->size = (node->size) - size;
            freenode->id = -1;
            freenode->index = (node->index) + size;
            freenode->right = NULL;
            freenode->left = node;
            // old node must be updated
            node->id = ID; 
            node->size = size;
        }
        else
        {
            linked_list* freenode = new linked_list(); //since it has to be splited
            
            freenode->size = (node->size) - size;
            freenode->id = -1;
            freenode->index = (node->index) + size;
            freenode->right = node->right;
            freenode->left = node;
            
            node->right->left = freenode;
            node->right = freenode;
            node->id = ID; 
            node->size = size;
        }
    }
    else
    {
        node->id = ID;
    }
    print();
    pthread_mutex_unlock(&sharedLock);
    return node->index; //return the start index of the newly allocated node
}

int HeapManager::myFree(int ID, int index)
{
    pthread_mutex_lock(&sharedLock);

    linked_list* node = head;
    while(node != NULL && !(node->id == ID && node->index == index))
    {
        node = node->right;
    }
    // check if this node exist or not
    if(node == NULL || node->id != ID || node->index != index)
    {
        pthread_mutex_unlock(&sharedLock);
        return -1;
    }
    
    // so there exist a node with the given ID and index
    
    // 4 CASES:

    // CASE-1: if it is the head node
    if(node->left == NULL && node->right != NULL)
    {
        //cout << "case1" << endl;
        // do not need to change starting address of the node since it is head
        node->id = -1;
        // check if its right node is also empty or not, if it is empty then merge them
        if(node->right->id == -1)
        {
            linked_list* to_delete = node->right;
            // do not need to change starting address of the node since it is head
            node->size = node->size + node->right->size; // add size
            
            //make connections
            if(node->right->right != NULL) // check if there exists only two nodes or more
                node->right->right->left = node; // if more then connect the left, too
            
            node->right = node->right->right;

            delete to_delete;
        }

    }
    // CASE-2: if it is in the middle
    else if(node->left != NULL && node->right != NULL)
    {
        //cout << "case2" << endl;
        node->id = -1; // make it free

        if(node->left->id == -1) // if its left node is free
        {
            linked_list* to_deleteL = node->left;
            node->index = node->left->index;
            node->size = node->size + node->left->size;
            
            if(node->left->left != NULL) // check if there exists only two nodes or more
                node->left->left->right = node; // if more then connect the right, too
            node->left = node->left->left;

            if(to_deleteL == head)
            {
                head = node;
            }
            delete to_deleteL;
        }

        if(node->right->id == -1) // if its right node is free
        {
            linked_list* to_deleteR = node->right;
            // do not need to change starting address of the node
            node->size = node->size + node->right->size; // add size
            
            if(node->right->right != NULL) // check if there exists only two nodes or more
                node->right->right->left = node; // if more then connect the left, too
            node->right = node->right->right;
            delete  to_deleteR;
        }

    }

    // CASE-3: if it is at the end
    else if(node->left != NULL && node->right == NULL)
    {
        //cout << "case3" << endl;
        node->id = -1;
        // check if its left node is also empty or not, if it is empty then merge them
        if(node->left->id == -1)
        {
            linked_list* to_delete = node->left;

            node->index = node->left->index;
            node->size = node->size + node->left->size; // add size
            
            //make connections
            
            if(node->left->left != NULL) // check if there exists only two nodes or more
                node->left->left->right = node; // if more then connect the left, too

            node->left = node->left->left;

            if(to_delete == head)
            {
                head = node;
            }
            delete to_delete;
        }
    }
    // CASE-4: if it is one node
    else // else if(node->left == NULL && node->right == NULL)
    {
        //cout << "case4" << endl;
        node->id = -1;
        node->index = 0;
    }

    cout << "Freed for thread " << ID << endl;

    print();
    pthread_mutex_unlock(&sharedLock);
    return 1;
}

HeapManager::~HeapManager()
{
    linked_list* node = head;
    while(node != NULL)
    {
        linked_list* deleted_node = node;
        node = node->right;
        delete deleted_node;
    }
    
}

#endif