#ifndef SUM_LIB_H
#define SUM_LIB_H

typedef struct {
    int *array;
    int begin;
    int end;
} SumArgs;

int Sum(const SumArgs *args);

#endif