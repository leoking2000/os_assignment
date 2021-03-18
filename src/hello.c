#include <stdio.h>
#include "utilities.h"

int main()
{
    printf("Hello world!!!\n");

    vec2d v1 = { 5.0f, 1.0f };
    vec2d v2 = { 6.0f, 3.0f}; 

    vec2d k = add(v1, v2);
    printf("(%.1f , %.1f)\n",k.x,k.y);

    return 0;
}