#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> // time
#define MAX_LIMIT 1000

typedef enum {false, true} bool;

typedef struct ticket_lock 
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    unsigned long queue_head, queue_tail;
} ticket_lock_t;

#define TICKET_LOCK_INITIALIZER { PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER }

char **myMatrix;
int MatrixSize;
int TotalSize; // the # of square in the matrix
bool STOP_THREADS = false;
char winner = ' ';

void ticket_lock(ticket_lock_t *ticket)
{
    unsigned long queue_me;

    pthread_mutex_lock(&ticket->mutex);
    queue_me = ticket->queue_tail++;
    while (queue_me != ticket->queue_head)
    {
        pthread_cond_wait(&ticket->cond, &ticket->mutex);
    }
    pthread_mutex_unlock(&ticket->mutex);
}

void ticket_unlock(ticket_lock_t *ticket)
{
    pthread_mutex_lock(&ticket->mutex);
    ticket->queue_head++;
    pthread_cond_broadcast(&ticket->cond);
    pthread_mutex_unlock(&ticket->mutex);
}

ticket_lock_t m; // create a mutex 

void *mythread(void *arg) 
{
    int add = 0; // it is about to finf # of square for both threads

    if (MatrixSize%2 == 1) // if it is even like 4x4 then we don't add anythin but if it is odd then add 
        add = 1;

    TotalSize = MatrixSize*MatrixSize; // calculate total matrix size (global)

    for(int i = 0; i < (((MatrixSize*MatrixSize)/2) + add)  && (TotalSize) > 0; i++)
    {
        ticket_lock(&m);

        if (STOP_THREADS) 
            pthread_exit(NULL); 

        int x = rand() % MatrixSize; // 0-n
        int y = rand() % MatrixSize; // 0-n
        char val;
        
        //taking values
        if(arg == "x") // for writing in matrix 'x'
        {
            val ='x';
        }
        else // for writing in matrix 'o'
        {
            val ='o';
        }
        
        //put them in matrix 
        if (myMatrix[x][y] == ' ')
        {
            myMatrix[x][y] = val;
            TotalSize--; // increment square size
        }
        else
        {
            while(myMatrix[x][y] != ' ' && TotalSize > 0) // if matrix is full we have to continue
            {
                x = rand() % MatrixSize; // 0-n
                y = rand() % MatrixSize; // 0-n
            
            }

            myMatrix[x][y] = val;
            TotalSize--; // increment square size
        }
        
        // for output string
        if(arg == "x")
        {
             printf("Player x played on: ");
        }
        else
        {
            printf("Player o played on: ");
        }
        
        printf("(%d,%d)\n", x, y); // print result (x,y)
        int randomTime = rand() % 1000; 
        usleep(randomTime);

        /// Check if there is a winner
        bool rowsame;
        char k;
        // check rows
        for (int i = 0; i < MatrixSize; i++) //row
        {
            rowsame = true;
            for (int j = 0; j < MatrixSize; j++) //column
            {
                if (j == 0 && myMatrix[i][j] != ' ') 
                {
                    k = myMatrix[i][j]; 
                    continue;
                }
                if (myMatrix[i][j] != k)
                {
                    // no winner on this row
                    rowsame = false;
                    break;
                }
            }
            if (rowsame)
            {
                // winner is k
                winner = k;
                STOP_THREADS = true;
            }
        }

        // check column
        for (int i = 0; i < MatrixSize; i++) //row
        {
            rowsame = true;
            for (int j = 0; j < MatrixSize; j++) //column
            {
                if (j == 0 && myMatrix[j][i] != ' ') 
                {
                    k = myMatrix[j][i]; 
                    continue;
                }
                if (myMatrix[j][i] != k)
                {
                    // no winner on this row
                    rowsame = false;
                    break;
                }
            }
            if (rowsame)
            {
                // winner is k
                winner = k;
                STOP_THREADS = true;
            }
        }

        //diagonal 1
        rowsame = true;
        for (int i = 0; i < MatrixSize; i++) //row
        {
            if (i == 0 && myMatrix[i][i] != ' ') 
            {
                k = myMatrix[i][i]; 
                continue;
            }
            if (myMatrix[i][i] != k)
            {
                // no winner on this row
                rowsame = false;
                break;
            }
            
        }
        if (rowsame)
        {
            // winner is k
            winner = k;
            STOP_THREADS = true;
        }

        //diagonal 2
        rowsame = true;
        for (int i = 0; i < MatrixSize; i++)
        {
            if (i == 0 && myMatrix[i][MatrixSize-1-i] != ' ')
            {
                k = myMatrix[i][MatrixSize-1-i];
                continue;
            }
            if (myMatrix[i][MatrixSize-1-i] != k)
            {
                rowsame = false;
                break;
            }
        }
        if (rowsame)
        {
            winner = k;
            STOP_THREADS = true;
        }

        ticket_unlock(&m);

        // check if it is full
        if (STOP_THREADS) 
            pthread_exit(NULL); 
    }
}


int main() 
{
    srand(time(NULL));

    pthread_t childX, childO; // create two child
    
    char sizeM[MAX_LIMIT]; // for taking num of square
    
    printf("Board Size: "); 
    scanf("%[^\n]%*c", sizeM); // get the input from the user

    char num[MAX_LIMIT]; // taking the number from the user

    int i = 0;
    while(sizeM[i]!='x') // since we are looking n from nxn
    {
        num[i] = sizeM[i]; // get the number 
        i++;
    }

    int sizeMatrix = atoi(num); // get the n 

    MatrixSize = sizeMatrix; // since MatrixSize is global I need it later for func
    
    // create a matrix in heap
    myMatrix = (char **)malloc(sizeMatrix * sizeof(char *));
    for (i = 0; i < sizeMatrix; i++)
         myMatrix[i] = (char *)malloc(sizeMatrix * sizeof(char));

     // initialize all matrix elements as empty
    for(int i = 0; i < sizeMatrix; i++) 
    {
      for(int j = 0; j < sizeMatrix; j++) 
      {
         myMatrix[i][j] = ' ';
      }
    }

    // I have to lock unclock childx and childo because I want them one by one and first "x" should come
    ticket_lock(&m);
    pthread_create(&childX, NULL, mythread, "x");
    ticket_unlock(&m);

    ticket_lock(&m);
    pthread_create(&childO, NULL, mythread, "o");
    ticket_unlock(&m);

    while (TotalSize > 0 && STOP_THREADS == false) 
    {
        continue;
    }
    
    STOP_THREADS = true;

    pthread_join(childX, NULL);
    pthread_join(childO, NULL);
    
    printf("Game end\n");
    if (winner == ' ')
    {
        printf("It is a tie\n");
    }
    else 
    {
        if (winner == 'x') 
            printf("Winner is X\n");
        else 
            printf("Winner is O\n");
    }

    // print matrix in here
    for(int i = 0; i < sizeMatrix; i++) 
    {
      for(int j = 0; j < sizeMatrix; j++) 
      {
         printf("[%c] ", myMatrix[i][j]);
         if(j == MatrixSize - 1)
         {
            printf("\n");
         }
      }
    }

    return 0;
}
