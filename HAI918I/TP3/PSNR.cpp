// // test_couleur.cpp : Seuille une image en niveau de gris

#include <stdio.h>
#include "image_ppm.h"
#include <list>
#include <algorithm>
#include <iostream>
#include <vector>

struct couleurPoint {
    int r, g, b; 
};

float distance(couleurPoint c1, couleurPoint centroide){

    return sqrt(pow(c1.r-centroide.r, 2) + pow(c1.g-centroide.g, 2) + pow(c1.b-centroide.b, 2));

} 

bool sameColor(couleurPoint p1, couleurPoint p2){
	if(p1.r == p2.r && p1.g == p2.g && p1.b == p2.b){
		return true; 
	}
	return false; 
}

int main(int argc, char* argv[])
{
	char cNomImgLue[250], cNomImgLue2[250];
	int nH, nW, nTaille;
	
	if (argc != 3) 
	{
		printf("Usage: ImageIn.pgm ImageIn2.pgm \n"); 
		exit (1) ;
	}
	
	sscanf (argv[1],"%s",cNomImgLue) ;
	sscanf (argv[2],"%s",cNomImgLue2) ;

	OCTET *ImgIn, *ImgIn2;
	
	lire_nb_lignes_colonnes_image_pgm(cNomImgLue, &nH, &nW);
	nTaille = nH * nW;

	allocation_tableau(ImgIn, OCTET, nTaille);
	allocation_tableau(ImgIn2, OCTET, nTaille);
	lire_image_pgm(cNomImgLue, ImgIn, nH * nW);
	lire_image_pgm(cNomImgLue2, ImgIn2, nH * nW);


	float EQM = 0, PSNR;

	for (int i=0; i < nTaille; i++) {
		EQM += pow(ImgIn[i] - ImgIn2[i] , 2);

	}

	EQM/= nTaille;

	PSNR = 10*log10(pow(255, 2)/EQM);

	std::cout << EQM << " " << PSNR << std::endl;


	
	free(ImgIn);

	return 1;
}


// test_couleur.cpp : Seuille une image couleur 



