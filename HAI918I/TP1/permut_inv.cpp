// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"

int occupe(int r, int* Table, int nTaille){
        if(r >= nTaille){
            return occupe(0, Table, nTaille);
        }
        if ( Table[r] == -1){
            return r;
        }
        else{
            return occupe(r+1, Table, nTaille);
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
   int Table[nTaille];
   srand(K);
	
    for (int i=0; i < nTaille; i++){
        Table[i] = -1;
    }
    for (int i=0; i < nTaille; i++){
        r = rand()%nTaille;
        //n = ImgIn[occupe(r, Table, nTaille)];
        while(Table[r] != -1){
            if(r == nTaille -1) r = 0;
            else r++;
        }
        Table[r] = r;
        ImgOut[i]= ImgIn[r];
    }

   ecrire_image_pgm(cNomImgEcrite, ImgOut,  nH, nW);
   free(ImgIn); free(ImgOut);

   return 1;
}

// #include <stdio.h>
// #include "image_ppm.h"

// int main(int argc, char* argv[])
// {
//     char cNomImgLue[250], cNomImgEcrite[250];
//     int nH, nW, nTaille, r, K;

//     if (argc != 4)
//     {
//         printf("Usage: ImageIn.pgm ImageOut.pgm Clé \n");
//         exit(1);
//     }

//     sscanf(argv[1], "%s", cNomImgLue);
//     sscanf(argv[2], "%s", cNomImgEcrite);
//     sscanf(argv[3], "%d", &K);

//     OCTET* ImgIn, * ImgOut;
//     lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
//     nTaille = nH * nW;

//     allocation_tableau(ImgIn, OCTET, nTaille);
//     lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
//     allocation_tableau(ImgOut, OCTET, nTaille);

//     // Initialiser le générateur de nombres pseudo-aléatoires avec la clé K
//     srand(K);

//     // Tableau pour suivre les positions de l'image d'origine
//     int* pos_origine = (int*)malloc(nTaille * sizeof(int));
//     for (int i = 0; i < nTaille; i++) {
//         pos_origine[i] = -1; // Initialisation à -1
//     }

//     // Suivre les positions aléatoires utilisées lors du chiffrement
//     for (int i = 0; i < nTaille; i++) {
//         r = rand() % nTaille;
//         while (pos_origine[r] != -1) { // Chercher une position non utilisée
//             r = (r + 1) % nTaille;
//         }
//         pos_origine[r] = i; // Associer la position générée à l'indice d'origine
//     }

//     // Reconstituer l'image originale
//     for (int i = 0; i < nTaille; i++) {
//         ImgOut[pos_origine[i]] = ImgIn[i];
//     }

//     ecrire_image_pgm(cNomImgEcrite, ImgOut, nH, nW);
//     free(ImgIn);
//     free(ImgOut);
//     free(pos_origine);

//     return 1;
// }


