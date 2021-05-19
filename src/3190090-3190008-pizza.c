#include "3190090-3190008-pizza.h"

// if debug = 0 printmsg does not print debug info 
static char debug = 0; 

static unsigned int N_cust;
static unsigned int SEED;

static ResourceInfo* Resources; // the Resource array

static pthread_mutex_t screen;

#define print(arg...) pthread_mutex_lock(&screen); printf(arg); pthread_mutex_unlock(&screen);

static pthread_mutex_t Pack_person;

static unsigned int rev = 0;
static pthread_mutex_t rev_mutex;

static unsigned int failed = 0;
static pthread_mutex_t failed_mutex;

static double* Tele_wait;
static double* Order_wait;
static double* cool_wait;
static double* is_Failed;

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
    pthread_mutex_init(&rev_mutex, NULL);

    int* ids = (int*)malloc(N_cust * sizeof(int));
    ThreadID* arr = (ThreadID*)malloc(N_cust * sizeof(ThreadID));

    Tele_wait = CreateArray(N_cust, 0.0);
    Order_wait = CreateArray(N_cust, 0.0);
    cool_wait = CreateArray(N_cust, 0.0);
    is_Failed = CreateArray(N_cust, 1.0);

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

    printf("\n========================================\n");
    printf("Total Revenue: %i \n", rev);
    printf("Succesfull Orders: %i\n", N_cust - failed);
    printf("Failed Orders: %i\n", failed);
    printf("TELE WAIT: MAX = %lf | AVG = %lf \n", Max(Tele_wait, N_cust, NULL), Avg(Tele_wait, N_cust, NULL));
    printf("Order WAIT: MAX = %lf | AVG = %lf \n", Max(Order_wait, N_cust, is_Failed), Avg(Order_wait, N_cust, is_Failed));
    printf("Cool WAIT: MAX = %lf | AVG = %lf \n", Max(cool_wait, N_cust, is_Failed), Avg(cool_wait, N_cust, is_Failed));


    DestroyResources(Resources);
    pthread_mutex_destroy(&rev_mutex);

    DestroyArray(Tele_wait);
    DestroyArray(Order_wait);
    DestroyArray(cool_wait);
    DestroyArray(is_Failed);

    free(ids);
    free(arr);

    return 0;
}

void* TakeOrder(void* data)
{
    int id = *( (int*)data );

    timespec start = Now();
 
    LockResource(id, TELE, 1);

    Tele_wait[id - 1] = TimePassedSince(start);
    int pizzas = GetRandomNumber(N_oderlow, N_orderhigh);
    PrintMsg(id, TELE, WORKING);

    sleep(GetRandomNumber(T_paymentlow, T_paymenthigh));

    if(GetRandomNumber(1, 100) <= P_fail)
    {
        UnLockResource(id, TELE, 1);
        print("[THREAD %i] Has Failed payment!\n", id);

        pthread_mutex_lock(&failed_mutex);
        failed++;
        pthread_mutex_unlock(&failed_mutex);

        is_Failed[id - 1] = 0.0;

        pthread_exit(NULL);
    }

    print("[THREAD %i] Has paid and is ok!\n", id);

    pthread_mutex_lock(&rev_mutex);
    rev += C_pizza * pizzas;
    pthread_mutex_unlock(&rev_mutex);

    UnLockResource(id, TELE, 1);

    LockResource(id, COOK, 1);

    PrintMsg(id, COOK, WORKING);
    sleep(T_prep * pizzas);

    LockResource(id, OVEN, pizzas);
    UnLockResource(id, COOK, 1);

    PrintMsg(id, OVEN, WORKING);
    sleep(T_bake);
    timespec start_cool = Now();

    pthread_mutex_lock(&Pack_person);
    sleep(T_pack * pizzas);
    pthread_mutex_unlock(&Pack_person);
    
    print("[THREAD %i] Packing is over in %.2f minits\n",id, TimePassedSince(start));

    UnLockResource(id, OVEN, pizzas);

    LockResource(id, DELIVERY, 1);

    PrintMsg(id, DELIVERY, WORKING);
    int time = GetRandomNumber(T_dellow, T_delhigh);
    sleep(time);
    cool_wait[id - 1] = TimePassedSince(start_cool);
    Order_wait[id - 1] = TimePassedSince(start);
    print("[THREAD %i] Order is Delivered in %.2f minits\n",id, Order_wait[id - 1]);
    sleep(time);

    UnLockResource(id, DELIVERY, 1);
 
    //print("[THREAD %i]TIME: %lf\n", id, TimePassedSince(start));

    pthread_exit(NULL);
}

void LockResource(int id, int resource_used_index, int amount)
{
    pthread_mutex_lock(&Resources[resource_used_index].muxtex);

    while(Resources[resource_used_index].size < amount)
    {
        PrintMsg(id, resource_used_index, WAITING);
        pthread_cond_wait(&Resources[resource_used_index].cond, &Resources[resource_used_index].muxtex);
    }
    Resources[resource_used_index].size -= amount;

    pthread_mutex_unlock(&Resources[resource_used_index].muxtex);
}

void UnLockResource(int id, int resource_used_index, int amount)
{
    pthread_mutex_lock(&Resources[resource_used_index].muxtex);

    Resources[resource_used_index].size += amount;
    PrintMsg(id, resource_used_index, END);
    pthread_cond_signal(&Resources[resource_used_index].cond);

    pthread_mutex_unlock(&Resources[resource_used_index].muxtex);
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

    pthread_mutex_init(&screen, NULL);
    pthread_mutex_init(&Pack_person, NULL);
    pthread_mutex_init(&rev_mutex, NULL);
    pthread_mutex_init(&failed_mutex, NULL);

    return arr;
}

void DestroyResources(ResourceInfo* r)
{
    pthread_mutex_destroy(&r[TELE].muxtex);
    pthread_cond_destroy(&r[TELE].cond);

    pthread_mutex_destroy(&r[COOK].muxtex);
    pthread_cond_destroy(&r[COOK].cond);

    pthread_mutex_destroy(&r[OVEN].muxtex);
    pthread_cond_destroy(&r[OVEN].cond);

    pthread_mutex_destroy(&screen);
    pthread_mutex_destroy(&Pack_person);

    pthread_mutex_destroy(&r[DELIVERY].muxtex);
    pthread_cond_destroy(&r[DELIVERY].cond);
    pthread_mutex_destroy(&rev_mutex);
    pthread_mutex_destroy(&failed_mutex);

    free(r);
}

void PrintMsg(int id, int resource_used_index, State state)
{
    if(debug == 0) return;

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
        end      = "[THREAD %i] End of telephone stage, <%i> telephone left \n";
        break;
    case COOK:
        start    = "[THREAD %i] Going to the COOK stage \n";
        waiting  = "[THREAD %i] Waiting for cook \n";
        woriking = "[THEAD %i] %i pizzas being made, <%i> cooks left \n";
        end      = "[THREAD %i] End of COOK stage, <%i> cooks left\n";
        break; 
    case OVEN:
        start    = "[THREAD %i] Going to the OVEN stage \n";
        waiting  = "[THREAD %i] Waiting for OVEN \n";
        woriking = "[THEAD %i] %i pizzas are in the oven, <%i> ovens left \n";
        end      = "[THREAD %i] End of OVEN stage in %lf sec, <%i> ovens left \n";
        break;
    case DELIVERY:
        start    = "[THREAD %i] Going to the DELIVERY stage \n";
        waiting  = "[THREAD %i] Waiting for DELIVERY \n";
        woriking = "[THEAD %i] %i pizzas are being deliver, <%i> delivery left \n";
        end      = "[THREAD %i] End of DELIVERY stage, <%i> delivery left\n";
        break;  
    default:
        print("[ERROR]resource_used_index = %i , in thread %i\n", resource_used_index, id);
        exit( EXIT_FAILURE );
        break;
    }

    switch(state)
    {
    case START:
        print(start, id);
        break;
    case WAITING:
        print(waiting, id);
        break;
    case WORKING:
        print(woriking, id, Resources[resource_used_index].size);
        break;
    case END:
        print(end, id, Resources[resource_used_index].size);
    }

}

timespec Now()
{
    timespec now;

    if( clock_gettime( CLOCK_REALTIME, &now) == -1 ) {
      print( "clock gettime" );
      exit( EXIT_FAILURE );
    }

    return now;
}
double TimePassedSince(timespec startTime)
{
    timespec endTime = Now();

    double time = ( endTime.tv_sec - startTime.tv_sec );

    return time / 60.0;
}

double Max(double* arr, int len, double* is_ok)
{
    double max = arr[0];
    for(int i = 1; i < len; i++)
    {
        if(is_ok != NULL)
        {
            if(is_ok[i] == 0.0) continue;
        }

        if(max < arr[i])
        {
            max = arr[i];
        }
    }
    return max;
}

double Avg(double* arr, int len, double* is_ok)
{
    double sum = 0.0;
    for(int i = 0; i < len; i++)
    {
        if(is_ok != NULL)
        {
            if(is_ok[i] == 0.0) continue;
        }

        sum += arr[i];
    }

    return sum / (len - failed);
}

double* CreateArray(int len, double value)
{
    double* arr = (double*)malloc(len * sizeof(double));
    if(arr == NULL)
    {
        printf("Maloc Failed!!!!\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < len; i++)
    {
        arr[i] = value;
    }
    return arr;
}

void DestroyArray(double* arr)
{
    free(arr);
}
