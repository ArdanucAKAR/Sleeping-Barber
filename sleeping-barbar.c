#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "Library/semaphore/semaphore.h"

int main(int argc, char **args)
{
    return 0; //Değiştirelecek
}

void Wait()
{
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 + 1) + 50000); /* 50000 - 250000 ms */
}