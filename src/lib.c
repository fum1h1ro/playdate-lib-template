#include "lib.h"
#include "../include/external.h"

int internal_add(int a, int b)
{
    return a + b;
}

int external_add(int a, int b)
{
    return internal_add(a, b);
}
