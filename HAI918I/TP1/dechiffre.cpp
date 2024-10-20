// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, n, r;
  
  if (argc != 3) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite);

   float nvGris[256][2];
    float entropie =0;
    float entropMin = 8;
    

    for (int i=0; i < 256; i++){
        nvGris[i][0] = i;
        nvGris[i][1] = 0;
    }
   

   OCTET *ImgIn, *ImgOut, *ImgTemp;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;
  
   allocation_tableau(ImgIn, OCTET, nTaille);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
   allocation_tableau(ImgTemp, OCTET, nTaille);
   allocation_tableau(ImgOut, OCTET, nTaille);

   for(int K = 1; K < 256; K++){
        srand(K);
        entropie =0;

        for (int i=0; i < 256; i++){
            nvGris[i][1] = 0;
        }
        

        ImgTemp[0] = (ImgIn[0] - rand()%256)%256;
        for (int i=1; i < nTaille; i++){
            r = rand()%256;
            ImgTemp[i]= (ImgIn[i] - ImgIn[i-1] - r)%256;
        }

        for (int i=0; i < nH; i++){
            for (int j=0; j < nW; j++){
                nvGris[ImgTemp[i*nW+j]][1] += 1;
            }
        }

        for (int i=0; i < 256; i++){
            if(nvGris[i][1] != 0){
                entropie -= (nvGris[i][1]/nTaille) * log2(nvGris[i][1]/nTaille);
            }
        }

        if(entropie < entropMin){
            for (int i=0; i < nTaille; i++){
                ImgOut[i]= ImgTemp[i];
            }
            n = K;
            entropMin = entropie;
        }

   }


//  for (int i=0; i < nH; i++)
//    for (int j=0; j < nW; j++)
//      {
//        if ( ImgIn[i*nW+j] < S) ImgOut[i*nW+j]=0; else ImgOut[i*nW+j]=255;
//      }

    printf("La clé est :  %d \n",  n);
    printf("Entropie de l'image :  %f \n",  entropMin);
   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgIn); free(ImgOut);

   return 1;
}
