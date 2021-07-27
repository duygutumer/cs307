#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> // time

sem_t waitA;
sem_t waitB;
sem_t driverSem;

//size of A and B
int numA = 0;
int numB = 0;

//num of people inside of team A and B
int inA = 0;
int inB = 0;

//condition variable for while loop
int con = 0;

// two threads for doing job and printing
pthread_mutex_t mutex; //doing job (increment team A and B)
pthread_mutex_t mutex2; //printing

void *teamA()
{   
    int id = rand(); //create a random id for threads
    int driver = 0;
    
    pthread_mutex_lock(&mutex);

    while(con != 0) 
    {
        //wait for desired 4 people to print their results
    }

    pthread_mutex_lock(&mutex2);
    printf("Thread ID: %d, Team: A, I am looking for a car\n",id);
    pthread_mutex_unlock(&mutex2);

    inA += 1; // increment the number of people waiting for the bus 
    
    if (inA >= 4) //if there are 4 people who wait the bus
    {
        // send them inside the bus
        sem_post(&waitA);
        sem_post(&waitA);
        sem_post(&waitA);
        sem_post(&waitA);
        driver = 1; //we are looking for a driver
        con = 1;
        inA -= 4; //decrease the number of people who are waiting for the bus 
    }
    else if (inA >= 2 && inB >= 2) //if there are 4 people(2-2) who wait the bus
    {
        sem_post(&waitA);
        sem_post(&waitA);
        sem_post(&waitB);
        sem_post(&waitB);
        driver = 1;
        con = 1;
        inA -= 2; //decrease the number of Team A people who are waiting for the bus 
        inB -= 2; //decrease the number of Team B people who are waiting for the bus 
    }
    pthread_mutex_unlock(&mutex);

    //if the number of people waiting is less than 4
    sem_wait(&waitA); // it is like a barrier for threads

    pthread_mutex_lock(&mutex2);
    printf("Thread ID: %d, Team: A, I have found a spot in a car\n",id);
    pthread_mutex_unlock(&mutex2);

    sem_post(&driverSem);


    if (driver == 1)
    {
        //since the driver should print at the end of four people
        sem_wait(&driverSem);
        sem_wait(&driverSem);
        sem_wait(&driverSem);
        sem_wait(&driverSem);
        pthread_mutex_lock(&mutex2);
        printf("Thread ID: %d, Team: A, I am the captain and driving the car\n",id);
        pthread_mutex_unlock(&mutex2);
        con = 0; // make con 0 so we will wait after these 4 people will be gone
    }

}
void *teamB() //same as A  
{   
    int id = rand();
    int driver = 0;

    pthread_mutex_lock(&mutex);
    while(con != 0) {}

    pthread_mutex_lock(&mutex2);
    printf("Thread ID: %d, Team: B, I am looking for a car\n",id);
    pthread_mutex_unlock(&mutex2);

    inB += 1;
    if (inB >= 4)
    {
        sem_post(&waitB);
        sem_post(&waitB);
        sem_post(&waitB);
        sem_post(&waitB);
        driver = 1;
        con = 1;
        inB -= 4;
    }
    else if (inB >= 2 && inA >= 2)
    {
        sem_post(&waitB);
        sem_post(&waitB);
        sem_post(&waitA);
        sem_post(&waitA);
        driver = 1;
        con = 1;
        inB -= 2;
        inA -= 2;
    }
    pthread_mutex_unlock(&mutex);

    sem_wait(&waitB);
    
    pthread_mutex_lock(&mutex2);
    printf("Thread ID: %d, Team: B, I have found a spot in a car\n",id);
    pthread_mutex_unlock(&mutex2);
    
    sem_post(&driverSem);

    if (driver == 1)
    {
        sem_wait(&driverSem);
        sem_wait(&driverSem);
        sem_wait(&driverSem);
        sem_wait(&driverSem);
        pthread_mutex_lock(&mutex2);
        printf("Thread ID: %d, Team: B, I am the captain and driving the car\n",id);
        pthread_mutex_unlock(&mutex2);
        con = 0;
    }
    
}
int main(int argc, char *argv[])
{   
    // ./a.out 2 3 => to do 
    int sizeOfA = atoi(argv[1]); // take first one
    int sizeOfB = atoi(argv[2]); // take second one

    if((sizeOfA%2 == 0) && (sizeOfB%2 == 0) && ((sizeOfA+sizeOfB)%4 == 0))
    {
        //printf("%d, %d\n",sizeOfA,sizeOfB);
        numA = sizeOfA;
        numB = sizeOfB;

        pthread_t A[sizeOfA], B[sizeOfB];

        //initialize threads
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutex2, NULL);
        
        //initialize semaphores
        sem_init(&waitA, 0, 0);
        sem_init(&waitB, 0, 0);
        sem_init(&driverSem, 0, 0);

        //create sizeOfA threads 
        for(int i = 0; i < sizeOfA; i++) {
            pthread_create(&A[i], NULL, (void *)teamA, NULL);
        }
        //create sizeOfB threads 
        for(int i = 0; i < sizeOfB; i++) {
            pthread_create(&B[i], NULL, (void *)teamB, NULL);
        }

        for(int i = 0; i < sizeOfA; i++) {
            pthread_join(A[i], NULL);
        }
        for(int i = 0; i < sizeOfB; i++) {
            pthread_join(B[i], NULL);
        }

        // Destroy what you create
        pthread_mutex_destroy(&mutex);
        pthread_mutex_destroy(&mutex2);
        sem_destroy(&waitA);
        sem_destroy(&waitB);
        sem_destroy(&driverSem);
    }
    
    printf("The main terminates\n");
    return 0;
    
}