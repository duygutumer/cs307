#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h> //For writing the output to the output.txt

int main (int argc, char *argv[]) {
    
    int fd[2];
    pipe(fd);

    printf("I’m SHELL process, with PID:%d - Main command is: man ls | grep [-]m[^,] > output.txt \n", (int) getpid());
    
    int man_process = fork(); // fork() for man

    if (man_process < 0 ) {

        // fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);

    } else if (man_process == 0){

        printf("I’m MAN process, with PID:%d - My command is: man ls \n", (int) getpid());
    
        char *myargs[3];
        myargs[0] = strdup("man"); // program: man (manual)
        myargs[1] = strdup("ls"); // argument: ls
        myargs[2] = NULL;
        
        close(fd[0]);
        close(STDOUT_FILENO);
        dup(fd[1]);
        
        execvp(myargs[0], myargs);
        //write(fd[1], string, (strlen(string)+1));
        
    } else {
        // parent goes down this path (main)
        int grep_process = fork(); // fork() for grep

        if (grep_process < 0) {

            // fork failed
            fprintf(stderr, "fork failed\n");
            exit(1);

        } else if (grep_process == 0){

            printf("I’m GREP process, with PID:%d - My command is: grep [-]m[^,] \n", (int) getpid());
            
            close(fd[1]);
            close(STDIN_FILENO);
            dup(fd[0]);
            
            char *myargs[3];
            myargs[0] = strdup("grep"); // program: grep 
            myargs[1] = strdup("[-]m[^,]"); // argument: -m
            myargs[2] = NULL;
            
            //For writing the output to the output.txt
            int writeF = open("output.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(writeF, 1);
            execvp(myargs[0], myargs);
            

        } else {

            close(fd[1]);

            int man_wait = wait(NULL);
            int grep_wait = wait(NULL);
            printf("I’m SHELL process, with PID:%d - execution is completed, you can find the results in output.txt \n", (int) getpid());
            //printf("hello, I am parent of %d (man_wait:%d) (pid:%d)\n", man_process, man_wait, (int) getpid());
            //printf("hello, I am parent of %d (grep_wait:%d) (pid:%d)\n", grep_process, grep_wait, (int) getpid());
                      

        }
        

}
return 0;
}