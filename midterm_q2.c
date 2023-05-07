/*
     Second Question of the Parallel Computing Exam:

          Rastgele siralama denemeleri kullanilarak, verilen bir diziye en yakin bicimde olusturulacak denemeler,
          dagitik bellekli sistemde modellenebilir. 

          Ornegin P dizisi sadece N, B, H, P karakterlerinden olusmaktadir ve asagidaki bicimde tanimlanmaktadir.
          (char P[24], char tipinde dinamik veya statik):
               P = [N, B, H, P, N, B, H, P, N, B, H, P, N, B, H, P, N, B, H, P, N, B, H, P]
          
          Burada P dizisi denemeler yoluyla ulasilmasi hedeflenen 24 elemandan olusan amac dizidir. 
          P dizisi dagitik sistemlerde bulunan tum islemcilerde, yukarida verilen siralama ile, ortak alanda tanimlanir.

          1) Sistemdeki tum islemcilerde, P ile ayni boyuta sahip bir deneme dizisi T olusturulmaktadir.
               Bir dongu icerisinde T'nin icerigi surekli olarak N, B, H, Q harfleri kullanilarak tamamen rastgele 
               bicimde yeniden olusturulacaktir.

          2) Her dongude P ve T'nin iceriklerinin ne kadar benzedigini gosteren bir benzerlik katsayisi 'r' hesaplanir.
               'r' katsayisi P ve T'nin ayni indise karsilik gelen esit degerli elemanlarinin sayisidir.
               
               Ornegin (dizi boyutlari basitlik icin azaltildi):
                    P = [N, B, H, P]
                    T = [P, B, P, P] icin r = 2.

                    P = [P, P, P, P, P, P, P, N]
                    T = [P, P, P, P, P, P, P, P] icin r = 7'dir. 

               Islemicilerde, her bir dongude, T'nin icerigi rastgele bicimde tekrar doldurulur ve olusturulan yeni T dizisi
               icin 'r' tekrar hesaplanir.

               Her 10^4 dongude bir, her bir islemci tarafindan indirgeme yoluyla toplanir (MPI_Reduce, MPI_MAX) ve 0 numarali
               islemcide ekrana yazdirilir.

               Islemcilerde en yuksek 'r' degerini veren diziler yerel Tmax dizilerinde saklanir.

          Yukarida ayrintilari verilen sureci MPI rutinleri ile dagitik bellekli sistemler icin C dili ile programlayiniz.

               * Kodu calistirmak icin komut istemine yazilmasi gerekenler:
                    $ mpicc filename.c -o filename -lm                              (-lm matematiksel kutuphaneleri bagliyor)
                    $ mpirun -np 4 ./filename

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#define P_BOYUTU 24
#define MAX_KARAKTER 4
#define MAX_ITERASYON 100000


int benzerlik_degeri_bul(char* P, char* T, int buyukluk) {
     // Benzerlik sabiti olan 'r' hesabi icin basit bir fonksiyon olusturuldu.
     int r = 0;
     for (int i = 0; i < buyukluk; i++) {
          if (P[i] == T[i]) {
               r++;
          }
     }
     return r;
}

int main(int argc, char* argv[]) {
    int rank, boyut;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &boyut);

    // Seed degeri 'time()' ile olusturulup rastgele sayilar elde edildi.
    srand(time(0) + rank);

    // 'P' ve 'T' array'leri olusturuldu.
    char P[P_BOYUTU];
    char T[P_BOYUTU];
    char Tmax[P_BOYUTU];
    int r, max_r = 0;

    // P array'i 'N', 'B', 'H', 'P' karakterleri ile deklare edildi.
    char karakterler[MAX_KARAKTER] = {'N', 'B', 'H', 'P'};
    for (int i = 0; i < P_BOYUTU; i++) {
        P[i] = karakterler[rand() % MAX_KARAKTER];
    }

    // 'T' array'i, 'P' array'i ile ayni buyuklukte, rastgele degerlerle tanimlandi. 
    for (int i = 0; i < P_BOYUTU; i++) {
        T[i] = karakterler[rand() % MAX_KARAKTER];
    }

    // Gerekli iterasyonlar gerceklestirildi.
    for (int iterasyon = 1; iterasyon <= MAX_ITERASYON; iterasyon++) {
        // Rastgele karakterlere sahip olan yeni bir 'T' array'i olusturuldu.
        for (int i = 0; i < P_BOYUTU; i++) {
            T[i] = karakterler[rand() % MAX_KARAKTER];
        }

        // 'r' degeri olculdu ve bunun ile 'Reduce' islemi uygulandi ve tum proseslerin arasinda maksimum benzerlik bulundu.
        r = benzerlik_degeri_bul(P, T, P_BOYUTU);
        MPI_Reduce(&r, &max_r, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

        // Maksimum benzerlik degerine sahip olan 'T' array'i 'Tmax' array'inde saklandi, her seferinde guncellenip kaybolmamali.
        if (r == max_r) {
            memcpy(Tmax, T, P_BOYUTU * sizeof(char));
        }

        // 'Tmax' array'i her 10^4 iterasyonda ana proses tarafindan ekrana bastirildi.
        if (iterasyon % 10000 == 0 && rank == 0) {
            printf("Iterasyon no: %d, r: %d, Tmax: ", iterasyon, r);
            for (int i = 0; i < P_BOYUTU; i++) {
                printf("%c ", Tmax[i]);
            }
            printf("\n");
        }
    }
    MPI_Finalize();
    return 0;
}
