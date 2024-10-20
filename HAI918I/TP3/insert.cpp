// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, k, pk;

  
  if (argc != 4) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm Seuil \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite);
   sscanf (argv[3],"%d",&k);


   OCTET *ImgIn, *ImgOut;
   pk = pow(2,k);
   int *key;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;
  
   allocation_tableau(ImgIn, OCTET, nTaille);
   allocation_tableau(key, int, nTaille);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
   allocation_tableau(ImgOut, OCTET, nTaille);
   srand(12);

   for(int i = 0; i < nTaille; i++){
        key[i] = rand()%2;
        //printf("%d",key[i]);
   }


    for(int i = 0; i < nTaille; i++){
        if(ImgIn[i] >= pk && (int)ImgIn[i]%(2*pk) >= pk){
            if(key[i]==1){
                ImgOut[i] = ImgIn[i];
            }
            else{
                ImgOut[i] = ImgIn[i] - pk;
                if(ImgOut[i] < 0){printf("ERREUR");}
            }
        }else{
            if(key[i]==1){
                ImgOut[i] = ImgIn[i] + pk;
                if(ImgOut[i] > 255){printf("ERREUR");}
            }
            else{
                ImgOut[i] = ImgIn[i];
            }
        }
    }


   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgIn); free(ImgOut);

   return 1;
}
