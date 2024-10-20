// test_couleur.cpp : Seuille une image couleur 

#include <stdio.h>
#include "image_ppm.h"
#include <math.h>   

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille, nbr, nbrG, nbrB, EQM;

  nbr = 0;

  
  if (argc != 3) 
     {
       printf("Usage: ImageIn.ppm ImageOut.ppm Seuil \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite);

   OCTET *ImgIn, *ImgOut;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;
  
   allocation_tableau(ImgIn, OCTET, nTaille);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
   allocation_tableau(ImgOut, OCTET, nTaille);
   lire_image_pgm(cNomImgEcrite, ImgOut, nH * nW);
	
   for (int i=0; i < nTaille; i++)
     {
       nbr +=pow(ImgIn[i]-ImgOut[i],2);
     }

     nbr = nbr/nTaille;

     EQM = nbr;

     printf("PSNR = %f \n", 10*log10(255*255/EQM));
   free(ImgIn);
   return 1;
}

//RGB = 29.740
//YCbCr = 21.789