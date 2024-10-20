// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"

int main(int argc, char* argv[])
{
  char cNomImgLue1[250], cNomImgLue2[250], cNomImgEcrite[250];
  int nH, nW, nTaille;
  
  if (argc != 4) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm Seuil \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue1) ;
   sscanf (argv[2],"%s",cNomImgLue2) ;
   sscanf (argv[3],"%s",cNomImgEcrite);

   OCTET *ImgSeuil, *ImgDilat, *ImgOut;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue1, &nH, &nW);
   nTaille = nH * nW;
   //lire_nb_lignes_colonnes_image_pgm(cNomImgLue2, &nH2, &nW2);
   //nTaille2 = nH2 * nW2;
  
   allocation_tableau(ImgSeuil, OCTET, nTaille);
   allocation_tableau(ImgDilat, OCTET, nTaille);
   lire_image_pgm(cNomImgLue1, ImgSeuil, nH * nW);
   lire_image_pgm(cNomImgLue2, ImgDilat, nH * nW);
   allocation_tableau(ImgOut, OCTET, nTaille);
	
   //   for (int i=0; i < nTaille; i++)
   // {
   //  if ( ImgIn[i] < S) ImgOut[i]=0; else ImgOut[i]=255;
   //  }


 for (int i=1; i < nH-1; i++)
   for (int j=1; j < nW-1; j++)
     {
       if ( ImgSeuil[i*nW+j] == ImgDilat[i*nW+j]) ImgOut[i*nW+j]=255; else ImgOut[i*nW+j]=0;
     }

   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgSeuil); free(ImgDilat); free(ImgOut);

   return 1;
}