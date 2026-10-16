#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child process (PID: %d) is exiting...\n", getpid());
        exit(0); // Ребенок завершается, но родитель его не "ждет"
    } 
    else if (pid > 0) {
        printf("Parent process (PID: %d). Child PID: %d\n", getpid(), pid);
        printf("Parent is sleeping for 15 seconds. Check 'ps' now!\n");
        
        // Родитель спит 15 секунд, не вызывая wait(). 
        // В это время ребенок будет в состоянии зомби
        sleep(15); 

        printf("Parent woke up. Now reaping the zombie...\n");
        wait(NULL); // Убираем зомби
        printf("Zombie reaped. Parent exiting.\n");
    } 
    else {
        perror("Fork failed");
        exit(1);
    }

    return 0;
}
