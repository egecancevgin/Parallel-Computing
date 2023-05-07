// First question of the Parallel Computing midterm exam, Egecan Celik Evgin - 190701157

/*
     Bir vektorel islem surecinde, rank'i 0 olan Master islemcide olusturulacak olan vektorler 'double' veri tipinde,
     ve indislere bagli olarak x[k] = k^2 ve y[k] = 1 - k + k^3 seklinde tanimlanmistir. Ayrica k = 0, 1, ... 19 seklindedir.
     Fark vektoru e = x - y seklinde tanimlanmistir ve fark vektorunun normu asagidaki gibi hesaplanmaktadir:
          |e| = sqrt( sum( (x[i] - y[i])^2 ) ), toplam sembolunun indisi de `i`'dir.
     
     MPI rutinleri ve haberlesme altyapisini kullanarak x ve y vektorlerinin farkina air normu asagida verilen semantik
     aciklamayi kullanarak hesaplayiniz:
                                                  0 
                (Scatter x and y)               /    \   (Reduce |e|^2) 
                                              /        \ 
                                            /            \  
          0 (rank=0, print(x) and print(y)) - - - 1 - - - 0 - -> (Yerel hesaplari indirgeme yoluyla topla ve 0.
                                            \     .      /        Islemcide karekokunu al. print(|e|))
                                             \    .     /
                                              \   .   /
                                                \ .  /
                                                  N (Yerel |e|^2'i hesapla, print(|e|^2))

     I. Adim: Rank=0'da x, y vektorlerini olusturunuz, statik veya dinamik olabilir, bu vektorlerin iceriklerini ekrana islemci rank'i
     bilgisini de vererek yazdiriniz.

     II. Adim: Vektorleri tum islemcilere dagitiniz (scatter), toplam islemci sayisi vektor boyutlari ile orantili olmalidir
     (Ornegin size=10 icin calistirilabilir).

     III. Adim: Dagitilan vektorleri kullanarak her islemcide yerel |e|^2 toplamlarini hesaplayiniz. Her islemcide hesaplanan yerel
     |e|'leri ekrana islemci rank'i ile birlikte yazdiriniz.

     IV. Adim: Yerel olarak hesaplanan degerleri indirgeme yoluyla toplayarak (MPI_Reduce) sonucu 0 numarali islemciden ekrana yaziniz
     (print). Extradan bu degerin karekokunu de en sonda yaziniz.

     V. Adim: Kodunuzun MPI_Init ve MPI_Finalize arasinda kalan kisminin 10^q defa calistirilmasi sonucunda harcanan duvar saati zamanini
     olcunuz (herhangi bir cekirdek sayisi icin), (Benim bilgisayarimda q sayisi 5 olursa ~2.02 saniye suruyor),
     ve hem sureyi hem de q'yu print ediniz.

     Burada q, harcanan duvar saati zamanini 1 saniyenin uzerine cikaran bir degerdir. Sectiginiz q degerini ve cekirdek sayisini
     cevabinizda belirtiniz.

     * Kodu calistirmak icin komut istemine yazilmasi gerekenler:
          $ mpicc filename.c -o filename -lm
          $ mpirun -np 4 ./filename

*/

#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#define VEKTOR_BUYUKLUGU 20

int main(int argc, char** argv) {
     int rank, buyukluk;
     double x[VEKTOR_BUYUKLUGU], y[VEKTOR_BUYUKLUGU], e[VEKTOR_BUYUKLUGU];
     double lokal_toplam = 0.0, global_toplam = 0.0;
     
     // Gerekli sabit vektor buyuklukleri ve toplamlar deklare edildi, 'MPI' bu `rank` ve `size` degerleri ile baslatilmis oldu.
     MPI_Init(&argc, &argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_size(MPI_COMM_WORLD, &buyukluk);

     // I. Adim: Ana proses 'x' ve 'y' vektorlerini olusturdu ve iceriklerini 'rank' bilgisi ile ekrana bastirdi.
     if (rank == 0) {
         printf("Rank %d: x ve y vektorleri olusturuluyor...\n", rank);
         for (int k = 0; k < VEKTOR_BUYUKLUGU; k++) {
             x[k] = pow(k, 2);
             y[k] = 1 - k + pow(k, 3);
             printf("Rank %d: x[%d] = %.2f, y[%d] = %.2f\n", rank, k, x[k], k, y[k]);
          }
     }

    // II. Adim: 'x' ve 'y' vektorleri tum proseslere dagitildi, yani 'scatter' islemi yapildi.
    MPI_Scatter(x, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, x, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(y, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, y, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // III. Adim: Lokal |e|^2 toplamlari hesaplandi ve 'rank' bilgisi ile ekrana basildi.
    for (int k = 0; k < VEKTOR_BUYUKLUGU / buyukluk; k++) {
        e[k] = x[k] - y[k];
        lokal_toplam += pow(e[k], 2);
    }
    printf("Rank %d: |e|^2'nin lokal toplami = %.2f\n", rank, lokal_toplam);

    // IV. Adim: Lokal toplamlar 'Reduce' islemine tabii tutuldu ve ana proses global toplami ekrana bastirdi.
    MPI_Reduce(&lokal_toplam, &global_toplam, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("Rank %d: |e|^2'nin global toplami = %.2f\n", rank, global_toplam);
        printf("Global toplamin karekoku = %.2f\n", sqrt(global_toplam));
    }

    // V. Adim: Kod 10^q defa calistiriliyor ve duvar saati zamani bu spesifik 'q' degeri icin olculuyor.
    int q = 5;  // 'q' burada tanimlandi ve yuksek olmasinin sebebi kodun cok hizli olmasi, 1 saniyeye ulamasi zor.
    int iterasyonlar = pow(10, q);
    double baslangic_zamani = MPI_Wtime(); // Sayac baslatildi.

    for (int i = 0; i < iterasyonlar; i++) {
        // Ayni kod bastan tekrar calistiriliyor, ancak bu sefer output olusturulmuyor.
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &buyukluk);
        
        // I.adim
        if (rank == 0) {
          printf("\a");
          for (int k = 0; k < VEKTOR_BUYUKLUGU; k++) {
               x[k] = pow(k, 2);
               y[k] = 1 - k + pow(k, 3);
               printf("\a");
               }
          }
          
          // II. Adim
          MPI_Scatter(x, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, x, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
          MPI_Scatter(y, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, y, VEKTOR_BUYUKLUGU / buyukluk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
          
          // III. Adim
          for (int k = 0; k < VEKTOR_BUYUKLUGU / buyukluk; k++) {
               e[k] = x[k] - y[k];
               lokal_toplam += pow(e[k], 2);
          }
          printf("\a");
          
          // IV. Adim
          MPI_Reduce(&lokal_toplam, &global_toplam, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
          if (rank == 0) {
               printf("\a");
               } 
          }

    // Sayac tam olarak burada bitirilmis oldu ve toplam sure hesaplanmis oldu, ve ekrana bastirildi.
    double bitis_zamani = MPI_Wtime();
    double toplam_sure = bitis_zamani - baslangic_zamani;
    if (rank == 0) {
        printf("\nRank %d: %d iterasyon icin gecen toplam sure: %.2f saniye\n", rank, iterasyonlar, toplam_sure);
    }

    MPI_Finalize();
    return 0;
}
