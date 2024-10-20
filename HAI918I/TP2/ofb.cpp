// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"
#include "AES.cpp"
#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250];
  int nH, nW, nTaille;

  
  if (argc != 3) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm Seuil \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite);

   OCTET *ImgIn, *ImgOut;
   OCTET *bloc, *blocX;
   OCTET *iv;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;
  
   allocation_tableau(ImgIn, OCTET, nTaille);
   allocation_tableau(bloc, OCTET, 16);
   allocation_tableau(blocX, OCTET, 16);
   allocation_tableau(iv, OCTET, 16);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
   allocation_tableau(ImgOut, OCTET, nTaille);

    const AESKeyLength keyLength = AESKeyLength::AES_128;
    unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    for (int i = 0; i < 16; i++)
    {
        iv[i] = i*2;
    }

    AES aesInstance(keyLength);

 for (int i=0; i < nH-1; i+=4)
   for (int j=0; j < nW-1; j+=4)
    {   
     for(int x = 0; x < 16; x++)
        {
        bloc[x] = ImgIn[(i+x/4)*nW+(j+x%4)];
        }
    unsigned char *blocC = aesInstance.EncryptECB(iv, 16, key);
    aesInstance.XorBlocks(bloc, blocC, blocX, 16);
    for(int y =0; y < 16; y++){
      iv[y] = blocC[y];
    }
    for(int x = 0; x < 16; x++)
        {
        ImgOut[(i+x/4)*nW+(j+x%4)] = (int)blocX[x];
        }
    }
    // ImgOut = ImgIn;

    aesInstance.AddNoiseToEncryptedImage(ImgOut, nTaille);
   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgIn); free(ImgOut);

   return 1;
}
