// test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include <dirent.h>
#include "image_ppm.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>


// void lire_images_dossier(const char* nom_dossier, OCTET* img_accumulateur, int nb_lignes, int nb_colonnes, int& compteur_images) {
//     DIR *dir;
//     struct dirent *ent;
//     char chemin_image[256];
//     int taille_image = nb_lignes * nb_colonnes;
//     OCTET *img_temp;

//     // Allocation de mémoire pour une image temporaire
//     img_temp = (OCTET*) malloc(taille_image * sizeof(OCTET));
//     if (img_temp == NULL) {
//         std::cerr << "Erreur d'allocation de mémoire." << std::endl;
//         exit(1);
//     }

//     // Ouvrir le répertoire
//     if ((dir = opendir(nom_dossier)) != NULL) {
//         // Parcourir tous les fichiers du répertoire
//         while ((ent = readdir(dir)) != NULL && compteur_images < 80) {
//             // Vérifier si le fichier a l'extension ".pgm"
//             if (strstr(ent->d_name, ".pgm") != NULL) {
//                 // Créer le chemin complet vers le fichier
//                 snprintf(chemin_image, sizeof(chemin_image), "%s/%s", nom_dossier, ent->d_name);

//                 std::cout << "Lecture de l'image: " << chemin_image << std::endl;

//                 // Lire l'image et l'ajouter à l'accumulateur
//                 lire_image_pgm(chemin_image, img_temp, taille_image);

//                 // Ajouter l'image courante à l'accumulateur d'images moyennes
//                 for (int i = 0; i < taille_image; i++) {
//                     img_accumulateur[i] += img_temp[i];
//                 }

//                 compteur_images++;  // Compter le nombre d'images lues
//             }
//         }
//         closedir(dir);
//     } else {
//         // Erreur si le répertoire ne peut pas être ouvert
//         perror("Erreur d'ouverture du répertoire");
//         exit(1);
//     }

//     free(img_temp);
// }

void maxPooling(float *M4, float *M2, int nH, int nW) {
    int nH2 = nH / 2;
    int nW2 = nW / 2;

    for (int i = 0; i < nH2; ++i) {
        for (int j = 0; j < nW2; ++j) {
            int x = i * 2;
            int y = j * 2;
            int maxVal = std::max({
                M4[x * nW + y], 
                M4[x * nW + y + 1],
                M4[(x + 1) * nW + y], 
                M4[(x + 1) * nW + y + 1]
            });
            M2[i * nW2 + j] = maxVal;
        }
    }

    // Si les dimensions sont impaires, gérer les dernières lignes et colonnes séparément
    if (nH % 2 != 0) {
        for (int j = 0; j < nW2; ++j) {
            M2[(nH2 - 1) * nW2 + j] = std::max(M4[(nH - 1) * nW + j * 2], M4[(nH - 1) * nW + j * 2 + 1]);
        }
    }
    if (nW % 2 != 0) {
        for (int i = 0; i < nH2; ++i) {
            M2[i * nW2 + (nW2 - 1)] = std::max(M4[i * 2 * nW + (nW - 1)], M4[(i * 2 + 1) * nW + (nW - 1)]);
        }
    }
}



void reLU(float *M, int size){
    for (int i=0; i < size; i++){
        M[i] = abs(std::max(M[i], 0.f));
    }
}

void convolution(float *c1, float *c2, float *filtre, int nH, int nW){
    for (int i=1; i < nH-1; i++){
        for (int j=1; j < nW-1; j++){
            c1[i*nW+j]= (c2[i*nW+j]*filtre[0]+c2[(i-1)*nW+j]*filtre[1]+c2[(i-1)*nW+(j-1)]*filtre[2]+c2[(i+1)*nW+j]*filtre[3]+c2[(i+1)*nW+(j-1)]*filtre[4]+c2[i*nW+(j-1)]*filtre[5]+c2[(i-1)*nW+(j+1)]*filtre[6]+c2[(i+1)*nW+(j+1)]*filtre[7]+c2[i*nW+(j+1)]*filtre[8]);
        }
    }
}

int main(int argc, char* argv[])
{
  char cNomImgLue[250], cNomImgEcrite[250], cNomImgEcrite1[250], cNomImgEcrite2[250];
  int nH, nW, nTaille;

  
  if (argc != 3) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm Seuil \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite) ;
   //srand(27);
   printf("%s\n", cNomImgEcrite);

    float c1[900], c2[900], c3[900], c4[900], c5[900], c6[225], c7[225], c8[225], c9[225], c10[225], c11[169], c12[169], c13[169], c14[169], c15[169], c16[169], c17[169], c18[169], c19[169], c20[169], c21[169], c22[169], c23[169], c24[169], c25[169];
    float c26[36], c27[36], c28[36], c29[36], c30[36], c31[36], c32[36], c33[36], c34[36], c35[36], c36[36], c37[36], c38[36], c39[36], c40[36];
    float vect[540], vectRand1[540], vectRand2[540], classe1, classe2;

    for (int i=0; i < 540; i++){
        vectRand1[i] = abs((double) rand() / (RAND_MAX));  
        vectRand2[i] = abs((double) rand() / (RAND_MAX)); 
    } 
   
   OCTET *ImgIn;
   
   lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
   nTaille = nH * nW;

   float filtre1[9] = {0,-1,0,-1,4,-1,0,-1,0};
   float filtre2[9] = {0, -1.0f/8.0f, 0, -1.0f/8.0f, 4.0f/8.0f, -1.0f/8.0f, 0, -1.0f/8.0f, 0};
   float filtre3[9] = {1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f, 2.0f/16.0f, 4.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f, 2.0f/16.0f, 1.0f/16.0f};
   float filtre4[9] = {1.0f/8.0f, 2.0f/8.0f, 1.0f/8.0f, 2.0f/8.0f, 4.0f/8.0f, 2.0f/8.0f, 1.0f/8.0f, 2.0f/8.0f, 1.0f/8.0f};
   float filtre5[9] = {6, 4, 0, 9, 2, 6, 8, 4, 5};
   float filtre6[9] = {-1, 0, -1, -2, 0, 2, 1, 0, 1};
   float filtre7[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
   float filtre8[9] = {6, 4, 0, 9, 2, 6, 8, 4, 5};

  
   allocation_tableau(ImgIn, OCTET, nTaille);
   lire_image_pgm(cNomImgLue, ImgIn, nH * nW);

   //printf("nH = %i\n", nH);
   classe1 =0;
   classe2 =0;

   printf("%s\n", cNomImgEcrite);


    
    for (int i=1; i < nH-1; i++){
        for (int j=1; j < nW-1; j++){
            c1[i*nW+j]= (ImgIn[i*nW+j]*filtre1[0]+ImgIn[(i-1)*nW+j]*filtre1[1]+ImgIn[(i-1)*nW+(j-1)]*filtre1[2]+ImgIn[(i+1)*nW+j]*filtre1[3]+ImgIn[(i+1)*nW+(j-1)]*filtre1[4]+ImgIn[i*nW+(j-1)]*filtre1[5]+ImgIn[(i-1)*nW+(j+1)]*filtre1[6]+ImgIn[(i+1)*nW+(j+1)]*filtre1[7]+ImgIn[i*nW+(j+1)]*filtre1[8]);
        }
    }

    for (int i=1; i < nH-1; i++){
        for (int j=1; j < nW-1; j++){
            c2[i*nW+j]= (ImgIn[i*nW+j]*filtre2[0]+ImgIn[(i-1)*nW+j]*filtre2[1]+ImgIn[(i-1)*nW+(j-1)]*filtre2[2]+ImgIn[(i+1)*nW+j]*filtre2[3]+ImgIn[(i+1)*nW+(j-1)]*filtre2[4]+ImgIn[i*nW+(j-1)]*filtre2[5]+ImgIn[(i-1)*nW+(j+1)]*filtre2[6]+ImgIn[(i+1)*nW+(j+1)]*filtre2[7]+ImgIn[i*nW+(j+1)]*filtre2[8]);
        }
    }

    for (int i=1; i < nH-1; i++){
        for (int j=1; j < nW-1; j++){
            c3[i*nW+j]= (ImgIn[i*nW+j]*filtre3[0]+ImgIn[(i-1)*nW+j]*filtre3[1]+ImgIn[(i-1)*nW+(j-1)]*filtre3[2]+ImgIn[(i+1)*nW+j]*filtre3[3]+ImgIn[(i+1)*nW+(j-1)]*filtre3[4]+ImgIn[i*nW+(j-1)]*filtre3[5]+ImgIn[(i-1)*nW+(j+1)]*filtre3[6]+ImgIn[(i+1)*nW+(j+1)]*filtre3[7]+ImgIn[i*nW+(j+1)]*filtre3[8]);
        }
    }
    printf("%s\n", cNomImgEcrite);

    for (int i=1; i < nH-1; i++){
        for (int j=1; j < nW-1; j++){
            c4[i*nW+j]= (ImgIn[i*nW+j]*filtre4[0]+ImgIn[(i-1)*nW+j]*filtre4[1]+ImgIn[(i-1)*nW+(j-1)]*filtre4[2]+ImgIn[(i+1)*nW+j]*filtre4[3]+ImgIn[(i+1)*nW+(j-1)]*filtre4[4]+ImgIn[i*nW+(j-1)]*filtre4[5]+ImgIn[(i-1)*nW+(j+1)]*filtre4[6]+ImgIn[(i+1)*nW+(j+1)]*filtre4[7]+ImgIn[i*nW+(j+1)]*filtre4[8]);
        }
    }
    printf("%s\n", cNomImgEcrite);

    strcpy(cNomImgEcrite1, cNomImgEcrite);

    for (int i=1; i < nH-1; i++){
        for (int j=1; j < nW-1; j++){
            c5[i*nW+j]= (ImgIn[i*nW+j]*filtre5[0]+ImgIn[(i-1)*nW+j]*filtre5[1]+ImgIn[(i-1)*nW+(j-1)]*filtre5[2]+ImgIn[(i+1)*nW+j]*filtre5[3]+ImgIn[(i+1)*nW+(j-1)]*filtre5[4]+ImgIn[i*nW+(j-1)]*filtre5[5]+ImgIn[(i-1)*nW+(j+1)]*filtre5[6]+ImgIn[(i+1)*nW+(j+1)]*filtre5[7]+ImgIn[i*nW+(j+1)]*filtre5[8]);
        }
    }
    printf("ok\n");
    printf("%s\n", cNomImgEcrite1);
    
    nH -=2;
    nW -=2;
    //printf("nH = %i\n", nH);

    
    reLU(c1, nH*nW);
    reLU(c2, nH*nW);
    reLU(c3, nH*nW);
    reLU(c4, nH*nW);
    reLU(c5, nH*nW);
    
    printf("%s\n", cNomImgEcrite1);

     //printf("%f\n", c5[25]);


    maxPooling(c1, c6, nH, nW);
    maxPooling(c2, c7, nH, nW);
    maxPooling(c3, c8, nH, nW);
    maxPooling(c4, c9, nH, nW);
    maxPooling(c5, c10, nH, nW);


    printf("%s\n", cNomImgEcrite1);
    nH /=2;
    nW /=2;

    //printf("nH = %i\n", nH);

    convolution(c11, c6, filtre6, nH, nW);
    convolution(c12, c6, filtre7, nH, nW);
    convolution(c13, c6, filtre8, nH, nW);
    convolution(c14, c7, filtre6, nH, nW);
    convolution(c15, c7, filtre7, nH, nW);
    convolution(c16, c7, filtre8, nH, nW);
    convolution(c17, c8, filtre6, nH, nW);
    convolution(c18, c8, filtre7, nH, nW);
    convolution(c19, c8, filtre8, nH, nW);
    convolution(c20, c9, filtre6, nH, nW);
    convolution(c21, c9, filtre7, nH, nW);
    convolution(c22, c9, filtre8, nH, nW);
    convolution(c23, c10, filtre6, nH, nW);
    convolution(c24, c10, filtre7, nH, nW);
    convolution(c25, c10, filtre8, nH, nW);

    //printf("nH = %i\n", nH);

    nH -=2;
    nW -=2;

    //printf("nH = %i\n", nH);

    
    reLU(c11, nH*nW);
    reLU(c12, nH*nW);
    reLU(c13, nH*nW);
    reLU(c14, nH*nW);
    reLU(c15, nH*nW);
    reLU(c16, nH*nW);
    reLU(c17, nH*nW);
    reLU(c18, nH*nW);
    reLU(c19, nH*nW);
    reLU(c20, nH*nW);
    reLU(c21, nH*nW);
    reLU(c22, nH*nW);
    reLU(c23, nH*nW);
    reLU(c24, nH*nW);
    reLU(c25, nH*nW);

     

    maxPooling(c11, c26, nH, nW);
    maxPooling(c12, c27, nH, nW);
    maxPooling(c13, c28, nH, nW);
    maxPooling(c14, c29, nH, nW);
    maxPooling(c15, c30, nH, nW);
    maxPooling(c16, c31, nH, nW);
    maxPooling(c17, c32, nH, nW);
    maxPooling(c18, c33, nH, nW);
    maxPooling(c19, c34, nH, nW);
    maxPooling(c20, c35, nH, nW);
    maxPooling(c21, c36, nH, nW);
    maxPooling(c22, c37, nH, nW);
    maxPooling(c23, c38, nH, nW);
    maxPooling(c24, c39, nH, nW);
    maxPooling(c25, c40, nH, nW);


    nH /=2;
    nW /=2;

    //printf("nH = %i\n", nH);
    

    for (int i=0; i < 36; i++){
        vect[i] = c26[i];  
    } 
    for (int i=36; i < 72; i++){
        vect[i] = c27[i];  
    } 
    for (int i=72; i < 108; i++){
        vect[i] = c28[i];  
    } 
    for (int i=108; i < 144; i++){
        vect[i] = c29[i];  
    } 
    for (int i=144; i < 180; i++){
        vect[i] = c30[i];  
    } 
    for (int i=180; i < 216; i++){
        vect[i] = c31[i];  
    } 
    for (int i=216; i < 252; i++){
        vect[i] = c32[i];  
    } 
    for (int i=252; i < 288; i++){
        vect[i] = c33[i];  
    } 
    for (int i=288; i < 324; i++){
        vect[i] = c34[i];  
    } 
    for (int i=324; i < 360; i++){
        vect[i] = c35[i];  
    } 
    for (int i=360; i < 396; i++){
        vect[i] = c36[i];  
    } 
    for (int i=396; i < 432; i++){
        vect[i] = c37[i];  
    } 
    for (int i=432; i < 468; i++){
        vect[i] = c38[i];  
    } 
    for (int i=468; i < 504; i++){
        vect[i] = c39[i];  
    } 
    for (int i=504; i < 540; i++){
        vect[i] = c40[i];  
    } 

    // std::ofstream myfile;
    // myfile.open("CNN.dat");
    // for(int i=0; i < 540; i++){
    //     myfile << vect[i];
    //     myfile << " ";
    //     myfile << vectRand1[i];
    //     myfile << " ";
    //     myfile << vectRand2[i];
    //     myfile << "\n";
    // }
    //myfile.close();
    
    
	//Résultats pour chaque classe
    for (int i=0; i < 540; i++){
        classe1 += vect[i] * vectRand1[i]; 
        classe2 += vect[i] * vectRand2[i]; 
    }

    for (int i=0; i < 540; i++){
        if(vect[i] < 0){
        	vect[i] = 0;
        } 
    }
    
    for (int i=0; i < 540; i++){
        if(vect[i] > 5000){
        	vect[i] = 5000;
        } 
    }
   
   int imgWidth = 20;
    int imgHeight = 27;
    OCTET *ImgOut; // Pour stocker l'image de sortie
    allocation_tableau(ImgOut, OCTET, imgHeight * imgWidth); // Allocation mémoire pour l'image

    // Trouver les valeurs min et max dans le vecteur pour normaliser
    float minVal = *std::min_element(vect, vect + 540);
    float maxVal = *std::max_element(vect, vect + 540);

    // Transformation des coefficients en niveaux de gris
    for (int i = 0; i < imgHeight * imgWidth; i++) {
        // Normaliser la valeur
        float normalizedValue = (vect[i] - minVal) / (maxVal - minVal) * 255;
        ImgOut[i] = static_cast<OCTET>(std::round(normalizedValue)); // Convertir en entier
    }

    // Sauvegarder l'image sous format PGM

     printf("ok\n");
     printf("%s\n", cNomImgEcrite1);
     printf("ok\n");
    
    ecrire_image_pgm(cNomImgEcrite1, ImgOut, imgHeight, imgWidth); // Écriture de l'image
    
    printf("ok\n");

    // Libération de la mémoire
    free(ImgOut);
    free(ImgIn); 
   //printf("%f\n", vectRand1[5] );

    if (classe1 + classe2 != 0) {
    printf("%f\n", classe1 / (classe1 + classe2));
    printf("%f\n", classe2 / (classe1 + classe2));
	} else {
    printf("Erreur : Division par zéro détectée lors du calcul des classes.\n");
	}

    std::ofstream myfile;
    myfile.open("CNN.dat", std::ios::app);
    
    myfile << classe1 / (classe1 + classe2);
    myfile << " ";
    myfile << classe2 / (classe1 + classe2);
    myfile << "\n";
    myfile.close();

   return 1;
}
