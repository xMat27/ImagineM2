#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[]){

    char cNomImgLue[250];
    int nH, nW, nTaille, nB, nG, nR;
    int nvGris[256][4];

    for (int i=0; i < 256; i++){
        nvGris[i][0] = i;
        nvGris[i][1] = 0;
        nvGris[i][2] = 0;
        nvGris[i][3] = 0;
    }

    OCTET *ImgIn;

    sscanf (argv[1],"%s",cNomImgLue) ;

    lire_nb_lignes_colonnes_image_ppm(cNomImgLue, &nH, &nW);
    nTaille = nH * nW;

    int nTaille3 = nTaille * 3;
    allocation_tableau(ImgIn, OCTET, nTaille3);
    lire_image_ppm(cNomImgLue, ImgIn, nH * nW);

    for (int i=0; i < nTaille3; i+=3)
        for(int x=0; x<256; x++)
        {
        nR = ImgIn[i];
        nG = ImgIn[i+1];
        nB = ImgIn[i+2];
        if (nR == x) nvGris[x][1] += 1;
        if (nG == x) nvGris[x][2] += 1;
        if (nB == x) nvGris[x][3] += 1;
        }

    ofstream myfile;
    myfile.open("histoC.dat");
    for(int i=0; i < 256; i++){
        myfile << nvGris[i][0];
        myfile << " ";
        myfile << nvGris[i][1];
        myfile << " ";
        myfile << nvGris[i][2];
        myfile << " ";
        myfile << nvGris[i][3];
        myfile << "\n";
    }
    myfile.close();

    free(ImgIn);
}