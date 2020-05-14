#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "Library/semaphore/semaphore.h"

#define CUSTOMER_LIMIT 10       /* Berber Dükkanı Müşteri Sınırı */
#define CUT_TIME 1              /* Berberin Saç Kesme Süresi */

/* semaforlar */
#ifdef __APPLE__
sem_t* barbers;                 /* MacOS için Berber Semafor */
sem_t* customers;               /* MacOS için Müşteri Semafor */
sem_t* mutex;                   /* Berber Koltuğuna Erişim */
#else
sem_t barbers;                  /* Berber Semafor */
sem_t customers;                /* Müşteri Semafor */
sem_t mutex;                    /* Berber Koltuğuna Erişim */
#endif

/* değişkenler */
int chairCount = 0;             /* Bekleme Koltuğu Sayısı */
int customerToBeServed = 0;     /* Hizmet Edilecek Müşteri Sayısı */
int* seat;                      /* Berber - Müşteri Arasında Kimlik Takası İçin */
int emptyChairCount = 0;        /* Bekleme Odasındaki Boş Koltuk Sayısı */
int chairToBeSeated = 0;        /* Müşterinin Oturacağı Sandalye Kimliği */
int seatCount = 0;              /* Berber Koltuğu Sayısı */
int customerCount = 0;          /* Müşteri Sayısı */

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

        dis_sem_wait(&barbers); /* Uyuyan Berberlerin Kuyruğuna Katıl */
        dis_sem_wait(&mutex);   /* Koltuğa Erişimi Kilitle */
        
        /* Hizmet Edilecek Müşterinin Bekleyenlerin Arasından Seçilmesi */
        customerToBeServed = (++customerToBeServed) % chairCount;
        nextCustomer = customerToBeServed;
        customerID = seat[nextCustomer];
        printf("[Berber: %d]\t%d. müşterinin saçını kesmeye başladı.\n\n", s, customerID);
        seat[nextCustomer] = pthread_self();
         sleep(CUT_TIME);
        printf("[Berber: %d]\t%d. müşterinin saçını kesmeyi bitirdi.\n\n", s, customerID);

        dis_sem_post(&mutex);     /* Koltuğa Erişim Kilidini Kaldır */
        dis_sem_post(&customers); /* Seçilen Müşteriyle İlgilen */

       
       
        
    }
}

void Customer(void *count)
{
    int s = *(int*)count + 1;
    int chairTaken, barberID;

    dis_sem_wait(&mutex); /* Koltuğu Korumak İçin Erişimi Kilitle */

    printf("[Müşteri: %d]\tdükkana geldi.\n", s);

    /* Bekleme Odasında Boş Sandalye Varsa */
    if (emptyChairCount > 0)
    {
        emptyChairCount--;

        printf("[Müşteri: %d]\tbekleme salonunda bekliyor.\n\n", s);

        /* Bekleme Salonundan Bir Sandalye Seçip Otur */
        chairToBeSeated = (++chairToBeSeated) % chairCount;
        chairTaken = chairToBeSeated;
        seat[chairTaken] = s;

        dis_sem_post(&mutex);   /* Koltuğa Erişim Kilidini Kaldır */
        dis_sem_post(&barbers); /* Uygun Berberi Uyandır */

        dis_sem_wait(&customers); /* Bekleyen Müşteriler Kuyruğuna Katıl */
        dis_sem_wait(&mutex);     /* Koltuğu Korumak İçin Erişimi Kilitle */

        /* Berber Koltuğuna Geç */
        barberID = seat[chairTaken];
        emptyChairCount++;

        dis_sem_post(&mutex); /* Koltuğa Erişim Kilidini Kaldır */
    }
    else
    {
        dis_sem_post(&mutex); /* Koltuğa Erişim Kilidini Kaldır */
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

    pthread_t barber[seatCount], customer[customerCount]; /* İş Parçaları */

    /* Semaforların Oluşturulması */
    dis_sem_init(&barbers, 0);
    dis_sem_init(&customers, 0);
    dis_sem_init(&mutex, 1);

    printf("\nBerber dükkanı açıldı.\n\n");

    /* Berber İş Parçalarının Oluşturulması */
    for (int i = 0; i < seatCount; i++)
    {
        pthread_create(&barber[i], NULL, (void*)Barber, (void*)&i);
        sleep(1);
    }

    /* Müşteri İş Parçalarının Oluşturulması */
    for (int i = 0; i < customerCount; i++)
    {
        pthread_create(&customer[i], NULL, (void*)Customer, (void*)&i);
        Wait(); /* Müşterileri Rastgele Aralıklarda Oluşturmak İçin */
    }

    /* Dükkanı Kapatmadan Önce Tüm Müşteriler İle İlgilen */
    for (int i = 0; i < customerCount; i++)
    {
        pthread_join(customer[i], NULL);
    }

    printf("\nTüm müşterilere hizmet verildi. Berber dükkanı kapandı. Berberler dükkandan ayrıldı.\n\n");

    return EXIT_SUCCESS;
}