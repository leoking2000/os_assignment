#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ThreadID pthread_t

#define N_tel 3
#define N_cook 2
#define N_oven 10
#define N_deliverer 7

#define T_OrderLow 1
#define T_OrderHigh 1

#define N_oderlow 1
#define N_orderhigh 5

#define T_paymentlow 1
#define T_paymenthigh 2

#define C_pizza 10

#define P_fail 5

#define T_prep 1
#define T_bake 10
#define T_pack 2

#define T_dellow 5
#define T_delhigh 15

typedef struct ResourceInfo
{
    unsigned int size;
    unsigned int _MaxSize;

    pthread_mutex_t muxtex;
    pthread_cond_t cond;
} ResourceInfo;

typedef struct Args
{
    ResourceInfo* Resources;
    int id;
} Args;


#define bool unsigned char
#define TRUE 1
#define FALSE 0

