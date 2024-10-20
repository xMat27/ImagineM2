#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[]){

    char cNomImgLue[250];
    int ligne;
    int nH, nW, nTaille, num, taill;

    OCTET *ImgIn;

    sscanf (argv[1],"%s",cNomImgLue) ;
    sscanf (argv[2],"%d",&ligne) ;
    sscanf (argv[3],"%d",&num) ;

    lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
    nTaille = nH * nW;

    allocation_tableau(ImgIn, OCTET, nTaille);
    lire_image_pgm(cNomImgLue, ImgIn, nH * nW);

    if(ligne ==0){
        taill = nW;
    }
    else{
        taill = nH;
    }

    int nvGris[taill];
        for (int i=0; i < taill; i++){
            nvGris[i] = 0;
        }

    if(ligne == 1){
        for (int j=0; j < nW; j++)
        {
            nvGris[j] = ImgIn[num*nW+j];
        }
    }
    if(ligne == 0){
        for (int i=0; i < nH; i++)
        {
            nvGris[i] = ImgIn[i*nH+num];
        }
    }
    ofstream myfile;
    myfile.open("fich.dat");
    for(int i=0; i < taill; i++){
        myfile << i;
        myfile << " ";
        myfile << nvGris[i];
        myfile << "\n";
    }
    myfile.close();

    free(ImgIn);
}