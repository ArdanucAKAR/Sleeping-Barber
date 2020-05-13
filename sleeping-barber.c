#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "Library/semaphore/semaphoreApple.h"

#define CUSTOMER_LIMIT 10       /* Müşteriler için iş parçası sınırı */
#define CUT_TIME 1 /* Saç kesme için kullanılacak süre */

/* semaforlar */
#ifdef __APPLE__
sem_t* barbers;   /* berberler için semafor */
sem_t* customers; /* müşteriler için semafor */
sem_t* mutex;     /* berber koltuğuna karşılıklı münhasır erişim sağlar */
#else
sem_t barbers;   /* berberler için semafor */
sem_t customers; /* müşteriler için semafor */
sem_t mutex;     /* berber koltuğuna karşılıklı münhasır erişim sağlar */
#endif

/* değişkenler */
int chairCount = 0;         /* bekleme odasındaki sandalye sayısı */
int customerToBeServed = 0; /* hizmet edilecek müşteri kimliği */
int* seat;                 /* berber - müşteri arasında kimlik takası için */

int emptyChairCount = 0;            /* bekleme odasındaki boş sandalye sayısı */
int chairToBeSeated = 0;       /* müşterinin oturacağı sandalye kimliği */

int seatCount = 0;           /* berber koltuğu sayısı */
int customerCount = 0;          /* müşteri sayısı */

void Barber(void *count)
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

        dis_sem_wait(&barbers); /* uyuyan berberlerin kuyruğuna katıl */
        dis_sem_wait(&mutex);   /* koltuğa erişimi kilitle */

        /* hizmet edilecek müşterinin bekleyenlerin arasından seçilmesi */
        customerToBeServed = (++customerToBeServed) % chairCount;
        nextCustomer = customerToBeServed;
        customerID = seat[nextCustomer];
        seat[nextCustomer] = pthread_self();

        dis_sem_post(&mutex);     /* koltuğa erişim kilidini kaldır */
        dis_sem_post(&customers); /* seçilen müşteriyle ilgilen */

        printf("[Berber: %d]\t%d. müşterinin saçını kesmeye başladı.\n\n", s, customerID);
        sleep(CUT_TIME);
        printf("[Berber: %d]\t%d. müşterinin saçını kesmeyi bitirdi.\n\n", s, customerID);
    }
}

void Customer(void *count)
{
    int s = *(int*)count + 1;
    int chairTaken, barberID;

    sem_wait(&mutex); /* koltuğu korumak için erişimi kilitle */

    printf("[Müşteri: %d]\tdükkana geldi.\n", s);

    /* bekleme odasında boş sandalye varsa */
    if (emptyChairCount > 0)
    {
        emptyChairCount--;

        printf("[Müşteri: %d]\tbekleme salonunda bekliyor.\n\n", s);

        /* bekleme salonundan bir sandalye seçip otur */
        chairToBeSeated = (++chairToBeSeated) % chairCount;
        chairTaken = chairToBeSeated;
        seat[chairTaken] = s;

        dis_sem_post(&mutex);   /* koltuğa erişim kilidini kaldır */
        dis_sem_post(&barbers); /* uygun berberi uyandır */

        dis_sem_wait(&customers); /* bekleyen müşteriler kuyruğuna katıl */
        dis_sem_wait(&mutex);     /* koltuğu korumak için erişimi kilitle */

        /* berber koltuğuna geç */
        barberID = seat[chairTaken];
        emptyChairCount++;

        dis_sem_post(&mutex); /* koltuğa erişim kilidini kaldır */
    }
    else
    {
        dis_sem_post(&mutex); /* koltuğa erişim kilidini kaldır */
        printf("[Müşteri: %d]\tbekleme salonunda yer bulamadı. Dükkandan ayrılıyor.\n\n", s);
    }
    pthread_exit(0);
}

void Wait()
{
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 + 1) + 50000); /* 50000 - 250000 ms */
}

int main(int argc, char** args)
{
    if (argc != 4)
    {
        printf("\nKullanım Hatası!\nKullanım Şekli:\t uyuyan-berber <Müşteri Sayısı> <Sandalye Sayısı> <Koltuk Sayısı>\n\n");
        return EXIT_FAILURE;
    }

    customerCount = atoi(args[1]);
    chairCount = atoi(args[2]);
    seatCount = atoi(args[3]);
    emptyChairCount = chairCount;
    seat = (int*)malloc(sizeof(int) * chairCount);

    if (customerCount > CUSTOMER_LIMIT)
    {
        printf("\nMüşteri sınırı: %d\n\n", CUSTOMER_LIMIT);
        return EXIT_FAILURE;
    }

    printf("\n\nGirilen Müşteri Sayısı:\t\t%d", customerCount);
    printf("\nGirilen Sandalye Sayısı:\t%d", chairCount);
    printf("\nGirilen Berber Koltuğu Sayısı:\t%d\n\n", seatCount);

    pthread_t barber[seatCount], customer[customerCount]; /* iş parçaları */

    /* semaforların oluşturulması */
    dis_sem_init(&barbers, 0);
    dis_sem_init(&customers, 0);
    dis_sem_init(&mutex, 1);

    printf("\nBerber dükkanı açıldı.\n\n");

    /* berber iş parçalarının oluşturulması */
    for (int i = 0; i < seatCount; i++)
    {
        pthread_create(&barber[i], NULL, (void*)Barber, (void*)&i);
        sleep(1);
    }

    /* müşteri iş parçalarının oluşturulması */
    for (int i = 0; i < customerCount; i++)
    {
        pthread_create(&customer[i], NULL, (void*)Customer, (void*)&i);
        Wait(); /* müşterileri rastgele aralıklarda oluşturmak için */
    }

    /* dükkanı kapatmadan önce tüm müşteriler ile ilgilen */
    for (int i = 0; i < customerCount; i++)
    {
        pthread_join(customer[i], NULL);
    }

    printf("\nTüm müşterilere hizmet verildi. Berber dükkanı kapandı. Berberler dükkandan ayrıldı.\n\n");

    return EXIT_SUCCESS;
}