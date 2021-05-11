#include "3190090-3190008-pizza.h"

int GetRandomNumber(int min, int max);

ResourceInfo* SetUpResources();
void DestroyResources(ResourceInfo* r);

void* TakeOrder(void* data);

int HandleOrderStage(int id, int resource_used_index, int amount, int workTime);

#define TELE 0
#define COOK 1
#define OVEN 2
#define DELIVERY 3

#define print(arg...) pthread_mutex_lock(&screen); printf(arg); pthread_mutex_unlock(&screen);

static unsigned int N_cust;
static unsigned int SEED;

static ResourceInfo* Resources;

static pthread_mutex_t screen;

int main(int argc, char** argv)
{
    if(argc == 3)
    {
        N_cust = atoi(argv[1]);
        if(N_cust == 0)
        {
            print("[ERROR] Number of customers is not corect! \n");
            exit(EXIT_FAILURE);
        }

        SEED = atoi(argv[2]);
        if(SEED == 0)
        {
            print("[ERROR] SEED is not corect! \n");
            exit(EXIT_FAILURE);
        }
    }
    else{
        print("[ERROR] Number of args must be 2 \n");
        exit(EXIT_FAILURE);
    }

    Resources = SetUpResources();

    int* ids = (int*)malloc(N_cust * sizeof(int));

    ThreadID* arr = (ThreadID*)malloc(N_cust * sizeof(ThreadID));

    for(int i = 0; i < N_cust; i++)
    {
        ids[i] = i + 1;
        int error_code = pthread_create(&arr[i], NULL, TakeOrder, &ids[i] );

        if(error_code != 0)
        {
            print("[ERROR] Thread %i failed to be created with code %i \n", i, error_code);
            exit(EXIT_FAILURE);
        }

        sleep(GetRandomNumber(T_OrderLow, T_OrderHigh));
    }
    
    for(int i = 0; i < N_cust; i++)
    {
        int error_code = pthread_join(arr[i], NULL);

        if(error_code != 0)
        {
            print("[ERROR] Thread %i failed to be Joined with code %i \n", i, error_code);
            exit(EXIT_FAILURE);
        }
    }

    DestroyResources(Resources);

    free(ids);
    free(arr);

    return 0;
}

void* TakeOrder(void* data)
{
    int id = *( (int*)data );
    int pizzas = GetRandomNumber(N_oderlow, N_orderhigh);

    HandleOrderStage(id, TELE, 1, GetRandomNumber(T_paymentlow, T_paymenthigh));

    HandleOrderStage(id, COOK, 1, T_prep * pizzas);

    HandleOrderStage(id, OVEN, pizzas, T_bake + T_pack * pizzas);

    HandleOrderStage(id, DELIVERY, 1, GetRandomNumber(T_dellow, T_delhigh));

    pthread_exit(NULL);
}

int HandleOrderStage(int id, int resource_used_index, int amount, int workTime)
{
    struct timespec startTime, endTime;
    double time;

    char* start;
    char* waiting;
    char* woriking;
    char* end;

    switch (resource_used_index)
    {
    case TELE:
        start    = "[THREAD %i] Going to the telephone stage \n";
        waiting  = "[THREAD %i] Waiting for telephone \n";
        woriking = "[THEAD %i]  has %i telephone on call, <%i> telephone left \n";
        end      = "[THREAD %i] End of telephone stage in %lf sec \n";
        break;
    case COOK:
        start    = "[THREAD %i] Going to the COOK stage \n";
        waiting  = "[THREAD %i] Waiting for cook \n";
        woriking = "[THEAD %i] %i pizzas being made, <%i> cooks left \n";
        end      = "[THREAD %i] End of COOK stage in %lf sec \n";
        break; 
    case OVEN:
        start    = "[THREAD %i] Going to the OVEN stage \n";
        waiting  = "[THREAD %i] Waiting for OVEN \n";
        woriking = "[THEAD %i] %i pizzas are in the oven, <%i> ovens left \n";
        end      = "[THREAD %i] End of OVEN stage in %lf sec \n";
        break;
    case DELIVERY:
        start    = "[THREAD %i] Going to the DELIVERY stage \n";
        waiting  = "[THREAD %i] Waiting for DELIVERY \n";
        woriking = "[THEAD %i] %i pizzas are being deliver, <%i> delivery left \n";
        end      = "[THREAD %i] End of DELIVERY stage in %lf sec \n";
        break;  
    default:
        print("[ERROR]resource_used_index = %i", resource_used_index);
        exit( EXIT_FAILURE );
        break;
    }

    if( clock_gettime( CLOCK_REALTIME, &startTime) == -1 ) {
      print( "clock gettime" );
      exit( EXIT_FAILURE );
    }
    // start

    print(start, id);
    pthread_mutex_lock(&Resources[resource_used_index].muxtex);

    while(Resources[resource_used_index].size < amount)
    {
        print(waiting, id);
        pthread_cond_wait(&Resources[resource_used_index].cond, &Resources[resource_used_index].muxtex);
    }
    Resources[resource_used_index].size -= amount;

    pthread_mutex_unlock(&Resources[resource_used_index].muxtex);

    print(woriking, id, amount, Resources[resource_used_index].size);
    sleep(workTime);

    pthread_mutex_lock(&Resources[resource_used_index].muxtex);

    Resources[resource_used_index].size += amount;
    pthread_cond_signal(&Resources[resource_used_index].cond);

    pthread_mutex_unlock(&Resources[resource_used_index].muxtex);

    // end
    if( clock_gettime( CLOCK_REALTIME, &endTime) == -1 ) {
      print( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    time = ( endTime.tv_sec - startTime.tv_sec )
          + ( endTime.tv_nsec - startTime.tv_nsec )
            / BILLION;
    
    print(end, id, time);

    return time;
}

int GetRandomNumber(int min, int max)
{
    return min + rand_r(&SEED) % ( (max+1) - min );
}

// NOTE: Check for error 
ResourceInfo* SetUpResources()
{   
    ResourceInfo* arr = (ResourceInfo*) malloc(4 * sizeof(ResourceInfo));

    arr[TELE]._MaxSize = N_tel;
    arr[TELE].size = N_tel;
    pthread_mutex_init(&arr[TELE].muxtex, NULL);
    pthread_cond_init(&arr[TELE].cond, NULL);

    arr[COOK]._MaxSize = N_cook;
    arr[COOK].size = N_cook;
    pthread_mutex_init(&arr[COOK].muxtex, NULL);
    pthread_cond_init(&arr[COOK].cond, NULL);

    arr[OVEN]._MaxSize = N_oven;
    arr[OVEN].size = N_oven;
    pthread_mutex_init(&arr[OVEN].muxtex, NULL);
    pthread_cond_init(&arr[OVEN].cond, NULL);

    arr[DELIVERY]._MaxSize = N_deliverer;
    arr[DELIVERY].size = N_deliverer;
    pthread_mutex_init(&arr[DELIVERY].muxtex, NULL);
    pthread_cond_init(&arr[DELIVERY].cond, NULL);

    return arr;
}
// NOTE: Check for error 
void DestroyResources(ResourceInfo* r)
{
    pthread_mutex_destroy(&r[TELE].muxtex);
    pthread_cond_destroy(&r[TELE].cond);

    pthread_mutex_destroy(&r[OVEN].muxtex);
    pthread_cond_destroy(&r[OVEN].cond);

    pthread_mutex_destroy(&r[DELIVERY].muxtex);
    pthread_cond_destroy(&r[DELIVERY].cond);

    free(r);
}

