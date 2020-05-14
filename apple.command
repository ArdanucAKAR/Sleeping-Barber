rm -rvf ./SleepingBarber
gcc sleeping-barber.c -o SleepingBarber -lpthread
clear
chmod +x ./SleepingBarber
echo "Derledikten sonra './SleepingBarber <Müşteri Sayısı> <Sandalye Sayısı> <Koltuk Sayısı>'"
./SleepingBarber 5 5 1