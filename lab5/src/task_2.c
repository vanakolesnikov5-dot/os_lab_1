#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

long long global_result = 1; 
int mod_val = 1;  
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; 

// Структура для передачи аргументов в поток
typedef struct {
    int start;
    int end;
} ThreadArgs;

void* calc_factorial(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    long long local_result = 1;

    // Поток считает факториал для своего кусочка (от start до end)
    for (int i = args->start; i <= args->end; i++) {
        local_result = (local_result * i) % mod_val;
    }

    // КРИТИЧЕСКАЯ СЕКЦИЯ: обновляем общий результат
    pthread_mutex_lock(&mut);
    global_result = (global_result * local_result) % mod_val;
    pthread_mutex_unlock(&mut);

    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    int k = 0, pnum = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            k = atoi(argv[i + 1]); // Берем число после флага -k
            i++; // Пропускаем это число в следующем шаге цикла
        } else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(argv[i] + 7); // Берем число после =
        } else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod_val = atoi(argv[i] + 6); // Берем число после =
        }
    }

    if (k < 0 || pnum < 1 || mod_val < 1) {
        printf("Ошибка в аргументах. Пример: ./task2 -k 10 --pnum=4 --mod=10\n");
        return 1;
    }
    
    if (k == 0 || k == 1) {
        printf("Результат: 1\n");
        return 0;
    }

    pthread_t threads[pnum];
    int chunk_size = k / pnum;
    int remainder = k % pnum;
    int current_start = 1;

    for (int i = 0; i < pnum; i++) {
        ThreadArgs* args = malloc(sizeof(ThreadArgs));
        args->start = current_start;
        
        int current_chunk = chunk_size + (i < remainder ? 1 : 0);
        args->end = current_start + current_chunk - 1;
        
        current_start = args->end + 1;

        if (pthread_create(&threads[i], NULL, calc_factorial, (void*)args) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Факториал %d! по модулю %d = %lld\n", k, mod_val, global_result);

    return 0;
}