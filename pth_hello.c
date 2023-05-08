#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 

// Defining necessary variables and signatures
const int MAX_THREADS = 64;
int thread_count;  
void usage(char* prog_name);
void *hello(void* rank);

/*
To run:
          $ gcc -o pth_hello pth_hello.c -pthread
          $ ./pth_hello 4
*/


int main(int argc, char* argv[]) {
          long thread; 
          pthread_t* thread_handles;
          // Checking whether the user has provided the correct number of input arguments in the shell
          if (argc != 2)
                    usage(argv[0]);
          
          // Converting the user's input to an integer and store it in 'thread_count', which means the total thread number
          thread_count = strtol(argv[1], NULL, 10);  
          
          // Checking whether the input is valid
          if (thread_count <= 0 || thread_count > MAX_THREADS)
                    usage(argv[0]);
          
          // Allocating memory for our 'thread_handles' array
          thread_handles = malloc (thread_count*sizeof(pthread_t)); 
          
          // Create threads
          for (thread = 0; thread < thread_count; thread++)  
                    pthread_create(&thread_handles[thread], NULL, hello, (void*) thread);  

          printf("Hello from the main thread\n");

          // Wait for all threads to finish, similar to MPI Barrier
          for (thread = 0; thread < thread_count; thread++) 
                    pthread_join(thread_handles[thread], NULL); 

          // Freeing the memory allocated for 'thread_handles'
          free(thread_handles);
          return 0;
}


void *hello(void* rank) {
          // Returning a greeting message that carries current thread ID
          long my_rank = (long) rank; 
          printf("Hello from thread %ld of %d\n", my_rank, thread_count);
          return NULL;
}


void usage(char* prog_name) {
          // Checking whether the user has provided the correct input arguments to the program
          fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
          fprintf(stderr, "0 < number of threads <= %d\n", MAX_THREADS);
          exit(0);
}
