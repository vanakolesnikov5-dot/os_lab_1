#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h> 
#include <errno.h>
#include <getopt.h>
#include "multmod.h"
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};

struct ThreadArgs {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
};


bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) return false;
  if (errno != 0) return false;
  *val = i;
  return true;
}

// Общается с одним конкретным сервером
void* ServerWorker(void* arg) {
  struct ThreadArgs* args = (struct ThreadArgs*)arg;
  args->result = 1; // По умолчанию (если вдруг соединение упадет)

  struct hostent *hostname = gethostbyname(args->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", args->server.ip);
    return NULL;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(args->server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) return NULL;

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection to %s:%d failed\n", args->server.ip, args->server.port);
    close(sck);
    return NULL;
  }

  // Отправляем задачу
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &args->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &args->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &args->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    close(sck);
    return NULL;
  }

  // Получаем ответ (поток блокируется здесь)
  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) > 0) {
    memcpy(&args->result, response, sizeof(uint64_t));
    printf("Server %s:%d calculated part [%lu, %lu] -> %lu\n", 
           args->server.ip, args->server.port, args->begin, args->end, args->result);
  }

  close(sck);
  return NULL;
}

int main(int argc, char **argv) {
  uint64_t k = UINT64_MAX;
  uint64_t mod = UINT64_MAX;
  char servers_file[255] = {'\0'}; 

  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1) break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0: ConvertStringToUI64(optarg, &k); break;
      case 1: ConvertStringToUI64(optarg, &mod); break;
      case 2: memcpy(servers_file, optarg, strlen(optarg)); break;
      }
    } break;
    }
  }

  if (k == UINT64_MAX || mod == UINT64_MAX || !strlen(servers_file)){
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(servers_file, "r");
  if (!file) return 1;

  unsigned int servers_num = 0;
  char line[256];
  while (fgets(line, sizeof(line), file)) servers_num++;
  rewind(file);

  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  for (unsigned int i = 0; i < servers_num; i++) {
    fgets(line, sizeof(line), file);
    sscanf(line, "%[^:]:%d", to[i].ip, &to[i].port);
  }
  fclose(file);

  pthread_t *threads = malloc(sizeof(pthread_t) * servers_num);
  struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs) * servers_num);

  uint64_t chunk = k / servers_num;
  uint64_t remainder = k % servers_num;
  uint64_t current_start = 1;

  for (unsigned int i = 0; i < servers_num; i++) {
    uint64_t current_chunk = chunk + ((uint64_t)i < remainder ? 1 : 0);
    
    args[i].server = to[i];
    args[i].begin = current_start;
    args[i].end = current_start + current_chunk - 1;
    args[i].mod = mod;
    args[i].result = 1;
    
    current_start = args[i].end + 1;

    // Создаем поток для каждого сервера
    if (pthread_create(&threads[i], NULL, ServerWorker, (void*)&args[i]) != 0) {
        fprintf(stderr, "Failed to create thread for server %d\n", i);
    }
  }

  // Ждем завершения всех потоков и собираем результаты
  uint64_t total_result = 1;
  for (unsigned int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    total_result = MultModulo(total_result, args[i].result, mod);
  }

  printf("\nFinal answer: %lu\n", total_result);

  free(threads);
  free(args);
  free(to);
  return 0;
}