#include <iostream>
#include <dirent.h>  // Pour parcourir le répertoire
#include <cstring>
#include <cstdlib>
#include "image_ppm.h"

typedef unsigned char OCTET;


// Fonction pour parcourir un répertoire et lire jusqu'à 80 fichiers .pgm
void lire_images_dossier(const char* nom_dossier, OCTET* img_accumulateur, int nb_lignes, int nb_colonnes, int& compteur_images) {
    DIR *dir;
    struct dirent *ent;
    char chemin_image[256];
    int taille_image = nb_colonnes * nb_lignes;
    OCTET *img_temp;

    // Allocation de mémoire pour une image temporaire
    img_temp = (OCTET*) malloc(taille_image * sizeof(OCTET));
    if (img_temp == NULL) {
        std::cerr << "Erreur d'allocation de mémoire." << std::endl;
        exit(1);
    }

    // Ouvrir le répertoire
    if ((dir = opendir(nom_dossier)) != NULL) {
        // Parcourir tous les fichiers du répertoire
        while ((ent = readdir(dir)) != NULL && compteur_images < 80) {
            // Vérifier si le fichier a l'extension ".pgm"
            if (strstr(ent->d_name, ".pgm") != NULL) {
                // Créer le chemin complet vers le fichier
                snprintf(chemin_image, sizeof(chemin_image), "%s/%s", nom_dossier, ent->d_name);

                std::cout << "Lecture de l'image: " << chemin_image << std::endl;

                // Lire l'image et l'ajouter à l'accumulateur
                lire_image_pgm(chemin_image, img_temp, taille_image);

                // Ajouter l'image courante à l'accumulateur d'images moyennes
                for (int i = 0; i < taille_image; i++) {
                    img_accumulateur[i] += img_temp[i];
                }

                compteur_images++;  // Compter le nombre d'images lues
            }
        }
        closedir(dir);
    } else {
        // Erreur si le répertoire ne peut pas être ouvert
        perror("Erreur d'ouverture du répertoire");
        exit(1);
    }

    free(img_temp);
}

int main(int argc, char* argv[]){
    char cNomImgLue[250], cNomImgEcrite[250], cNomImgEcrite1[250], cNomImgEcrite2[250];
  int nH, nW, nTaille, compteur_images;
 	compteur_images = 0;
 	
  nH = 27;
  nW = 20;
  
  nTaille = nH*nW;
  
  if (argc != 3) 
     {
       printf("Usage: ImageIn.pgm ImageOut.pgm Seuil \n"); 
       exit (1) ;
     }
   
   sscanf (argv[1],"%s",cNomImgLue) ;
   sscanf (argv[2],"%s",cNomImgEcrite) ;
   
   

    OCTET *img_accumulateur;

    // Allocation de mémoire pour l'accumulateur
    img_accumulateur = (OCTET*) malloc(nTaille * sizeof(OCTET));
    if (img_accumulateur == NULL) {
        std::cerr << "Erreur d'allocation de mémoire." << std::endl;
        return 1;
    }

    // Initialiser l'accumulateur à zéro
    for (int i = 0; i < nTaille; i++) {
        img_accumulateur[i] = 0;
    }

    // Parcourir les images dans le dossier "classe1" et limiter à 80 images
    lire_images_dossier(cNomImgLue, img_accumulateur, nH, nW, compteur_images);

    // Calculer la moyenne
    for (int i = 0; i < nTaille; i++) {
        img_accumulateur[i] /= compteur_images;
    }

    // Vous pouvez maintenant utiliser img_accumulateur comme l'image moyenne
    std::cout << "Nombre d'images lues: " << compteur_images << std::endl;
    
    ecrire_image_pgm(cNomImgEcrite, img_accumulateur, nH, nW); // Écriture de l'image

    free(img_accumulateur);

    return 0;
}

