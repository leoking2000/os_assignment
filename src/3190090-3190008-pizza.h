#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define N_tel 3
#define N_cook 2
#define N_oven 10
#define N_deliverer 7

#define T_OrderLow 1
#define T_OrderHigh 5

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

#define ThreadID pthread_t

typedef struct ResourceInfo
{
    unsigned int size;
    unsigned int _MaxSize;

    pthread_mutex_t muxtex;
    pthread_cond_t cond;
} ResourceInfo;

#define TELE 0
#define COOK 1
#define OVEN 2
#define DELIVERY 3

typedef struct timespec timespec;

// this is used for the debug msg system
typedef enum State
{
    START,
    WAITING,
    WORKING,
    END
} State;

int GetRandomNumber(int min, int max);

ResourceInfo* SetUpResources();
void DestroyResources(ResourceInfo* r);

void LockResource(int id, int resource_used_index, int amount);
void UnLockResource(int id, int resource_used_index, int amount);

void* TakeOrder(void* data);

void PrintMsg(int id, int resource_used_index, State state);

timespec Now();
double TimePassedSince(timespec past); // return now() - past

double Max(double* arr, int len, double* is_ok);
double Avg(double* arr, int len, double* is_ok);

double* CreateArray(int len, double value);
void DestroyArray(double* arr);

