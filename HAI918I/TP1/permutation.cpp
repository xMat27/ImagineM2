// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"

int occupe(int r, OCTET *ImgOut, int nTaille){
        if(r >= nTaille){
            return occupe(0, ImgOut, nTaille);
        }
        if ( ImgOut[r] == 0){
            return r;
        }
        else{
            return occupe(r+1, ImgOut, nTaille);
        }
   }

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, n, r, K;
  
  if (argc != 4) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm Clé \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite);
   sscanf (argv[3],"%d",&K);

   OCTET *ImgIn, *ImgOut;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;
  
   allocation_tableau(ImgIn, OCTET, nTaille);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
   allocation_tableau(ImgOut, OCTET, nTaille);
   srand(K);
	
    for (int i=0; i < nTaille; i++){
        ImgOut[i]=0;
    }
    for (int i=0; i < nTaille; i++){
        n = ImgIn[i];
        r = rand()%nTaille;
        ImgOut[occupe(r, ImgOut, nTaille)]= n;
    }


//  for (int i=0; i < nH; i++)
//    for (int j=0; j < nW; j++)
//      {
//        if ( ImgIn[i*nW+j] < S) ImgOut[i*nW+j]=0; else ImgOut[i*nW+j]=255;
//      }

   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgIn); free(ImgOut);

   return 1;
}
