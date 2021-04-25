#include "3190090-3190008-pizza.h"

static unsigned int N_cust;
static unsigned int SEED;

int GetRandomNumber(int min, int max);

ResourceInfo* SetUpResources();
void DestroyResources(ResourceInfo* r);

#define TELE 0
#define COOK 1
#define OVEN 2
#define DELIVERY 3

bool Take(ResourceInfo* r, unsigned int amount);
bool Give(ResourceInfo* r, unsigned int amount);

void* TakeOrder(void* data)
{
    Args* arg = (Args*)data;

    int id = arg->id;

    printf("[Thread %i]has started\n", id);

    int pizzas = GetRandomNumber(N_oderlow, N_orderhigh);
    pthread_mutex_lock(&arg->Resources[OVEN].muxtex);

    while(arg->Resources[OVEN].size < pizzas)
    {
        printf("Thread %i] is waiting\n", id);
        pthread_cond_wait(&arg->Resources[OVEN].cond, &arg->Resources[OVEN].muxtex);
    }
    arg->Resources[OVEN].size -= pizzas;

    pthread_mutex_unlock(&arg->Resources[OVEN].muxtex);

    printf("[Thread %i] %i pizzas are in the oven, <%i> ovens left \n", id, pizzas, arg->Resources[OVEN].size);
    sleep(pizzas);

    pthread_mutex_lock(&arg->Resources[OVEN].muxtex);

    arg->Resources[OVEN].size += pizzas;
    pthread_cond_signal(&arg->Resources[OVEN].cond);

    pthread_mutex_unlock(&arg->Resources[OVEN].muxtex);

    printf("[Thread %i]pizzas are ready\n", id);
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    if(argc == 3)
    {
        N_cust = atoi(argv[1]);
        if(N_cust == 0)
        {
            printf("[ERROR] Number of customers is not corect! \n");
            exit(EXIT_FAILURE);
        }

        SEED = atoi(argv[2]);
        if(N_cust == 0)
        {
            printf("[ERROR] SEED is not corect! \n");
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("[ERROR] Number of args must be 2 \n");
        exit(EXIT_FAILURE);
    }

    ResourceInfo* Resources = SetUpResources();

    Args* args = (Args*)malloc(N_cust * sizeof(Args));

    ThreadID* arr = (ThreadID*)malloc(N_cust * sizeof(ThreadID));

    for(int i = 0; i < N_cust; i++)
    {
        args[i].Resources = Resources;
        args[i].id = i + 1;
        
        int error_code = pthread_create(&arr[i], NULL, TakeOrder, &args[i] );

        if(error_code != 0)
        {
            printf("[ERROR] Thread %i failed to be created with code %i \n", i, error_code);
            exit(EXIT_FAILURE);
        }

        sleep(1);
    }
    
    for(int i = 0; i < N_cust; i++)
    {
        int error_code = pthread_join(arr[i], NULL);

        if(error_code != 0)
        {
            printf("[ERROR] Thread %i failed to be Joined with code %i \n", i, error_code);
            exit(EXIT_FAILURE);
        }
    }

    DestroyResources(Resources);

    free(Resources);
    free(args);
    free(arr);

    return 0;
}

int GetRandomNumber(int min, int max)
{
    return min + rand_r(&SEED) % ( (max+1) - min );
}

ResourceInfo* SetUpResources()
{   
    ResourceInfo* arr = (ResourceInfo*) malloc(4 * sizeof(ResourceInfo));

    arr[TELE]._MaxSize = N_tel;
    arr[TELE].size = N_tel;

    arr[COOK]._MaxSize = N_cook;
    arr[COOK].size = N_cook;

    arr[OVEN]._MaxSize = N_oven;
    arr[OVEN].size = N_oven;
    pthread_mutex_init(&arr[OVEN].muxtex, NULL);
    pthread_cond_init(&arr[OVEN].cond, NULL);

    arr[DELIVERY]._MaxSize = N_oven;
    arr[DELIVERY].size = N_oven;

    return arr;
}

void DestroyResources(ResourceInfo* r)
{
    pthread_mutex_destroy(&r[OVEN].muxtex);
    pthread_cond_destroy(&r[OVEN].cond);
}

bool Take(ResourceInfo* r, unsigned int amount)
{
    if(r->size - amount < 0)
    {
        return FALSE;
    }
    r->size = r->size - amount;
    return TRUE;
}

bool Give(ResourceInfo* r, unsigned int amount)
{
    if(r->size + amount > r->_MaxSize)
    {
        return FALSE;
    }
    r->size = r->size + amount;
    return TRUE;
}