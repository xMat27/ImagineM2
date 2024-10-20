#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;

int main(int argc, char* argv[]){

    char cNomImgLue[250];
    int nH, nW, nTaille;
    float nvGris[256][2];

    float entropie =0;

    for (int i=0; i < 256; i++){
        nvGris[i][0] = i;
        nvGris[i][1] = 0;
    }

    OCTET *ImgIn;

    sscanf (argv[1],"%s",cNomImgLue) ;

    lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
    nTaille = nH * nW;

    allocation_tableau(ImgIn, OCTET, nTaille);
    lire_image_pgm(cNomImgLue, ImgIn, nH * nW);

    for (int i=0; i < nH; i++){
        for (int j=0; j < nW; j++){
            nvGris[ImgIn[i*nW+j]][1] += 1;
        }
    }

    for (int i=0; i < 256; i++){
        if(nvGris[i][1] != 0){
            entropie -= (nvGris[i][1]/nTaille) * log2(nvGris[i][1]/nTaille);
        }
    }




    // ofstream myfile;
    // myfile.open("histo.dat");
    // for(int i=0; i < 256; i++){
    //     myfile << nvGris[i][0];
    //     myfile << " ";
    //     myfile << nvGris[i][1];
    //     myfile << "\n";
    // }
    // myfile.close();

    printf("Entropie de l'image :  %f \n",  entropie);

    free(ImgIn);
}