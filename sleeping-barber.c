#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "Library/semaphore/semaphore.h"


#define CUT_TIME 1          /* Saç kesme için kullanılacak süre */

/* semaforlar */
sem_t* barbers;                /* berberler için semafor */
sem_t* customers;               /* müşteriler için semafor */
sem_t* mutex;                    /* berber koltuğuna karşılıklı münhasır erişim sağlar */

/* değişkenler */
int chairCount = 0;         /* bekleme odasındaki sandalye sayısı */
int customerToBeServed = 0;  /* hizmet edilecek müşteri kimliği */
int* chair;                    /* berber - müşteri arasında kimlik takası için */


int main(int argc, char **args)
{
    return 0; //Değiştirelecek
}

void Barber(void* count)
{
    int s = *(int*)count + 1;
    int nextCustomer, customerID;

    printf("[Berber: %d]\tdukkana geldi.\n", s);

    while (1)
    {
        if (!customerID)
        {
            printf("[Barber: %d]\tuyumaya gitti.\n\n", s);
        }

        rk_sema_wait(&barbers);   /* uyuyan berberlerin kuyruğuna katıl */
        rk_sema_wait(&mutex);       /* koltuğa erişimi kilitle */

        /* hizmet edilecek müşterinin bekleyenlerin arasından seçilmesi */
        customerToBeServed = (++customerToBeServed) % chairCount;
        nextCustomer = customerToBeServed;
        customerID = chair[nextCustomer];
        chair[nextCustomer] = pthread_self();

        rk_sema_post(&mutex);       /* koltuğa erişim kilidini kaldır */
        rk_sema_post(&customers);  /* seçilen müşteriyle ilgilen */

        printf("[Berber: %d]\t%d. müşterinin saçını kesmeye başladı.\n\n", s, customerID);
        sleep(CUT_TIME);
        printf("[Berber: %d]\t%d. müşterinin saçını kesmeyi bitirdi.\n\n", s, customerID);
    }
}

void Wait()
{
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 + 1) + 50000); /* 50000 - 250000 ms */
}