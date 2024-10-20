// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/jmkdtree.h"




std::vector< Vec3 > positions;
std::vector< Vec3 > normals;

std::vector< Vec3 > positions2;
std::vector< Vec3 > normals2;


// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 640;
static unsigned int SCREENHEIGHT = 480;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static bool fullScreen = false;




// ------------------------------------------------------------------------------------------------------------
// i/o and some stuff
// ------------------------------------------------------------------------------------------------------------
void loadPN (const std::string & filename , std::vector< Vec3 > & o_positions , std::vector< Vec3 > & o_normals ) {
    unsigned int surfelSize = 6;
    FILE * in = fopen (filename.c_str (), "rb");
    if (in == NULL) {
        std::cout << filename << " is not a valid PN file." << std::endl;
        return;
    }
    size_t READ_BUFFER_SIZE = 1000; // for example...
    float * pn = new float[surfelSize*READ_BUFFER_SIZE];
    o_positions.clear ();
    o_normals.clear ();
    while (!feof (in)) {
        unsigned numOfPoints = fread (pn, 4, surfelSize*READ_BUFFER_SIZE, in);
        for (unsigned int i = 0; i < numOfPoints; i += surfelSize) {
            o_positions.push_back (Vec3 (pn[i], pn[i+1], pn[i+2]));
            o_normals.push_back (Vec3 (pn[i+3], pn[i+4], pn[i+5]));
        }

        if (numOfPoints < surfelSize*READ_BUFFER_SIZE) break;
    }
    fclose (in);
    delete [] pn;
}
void savePN (const std::string & filename , std::vector< Vec3 > const & o_positions , std::vector< Vec3 > const & o_normals ) {
    if ( o_positions.size() != o_normals.size() ) {
        std::cout << "The pointset you are trying to save does not contain the same number of points and normals." << std::endl;
        return;
    }
    FILE * outfile = fopen (filename.c_str (), "wb");
    if (outfile == NULL) {
        std::cout << filename << " is not a valid PN file." << std::endl;
        return;
    }
    for(unsigned int pIt = 0 ; pIt < o_positions.size() ; ++pIt) {
        fwrite (&(o_positions[pIt]) , sizeof(float), 3, outfile);
        fwrite (&(o_normals[pIt]) , sizeof(float), 3, outfile);
    }
    fclose (outfile);
}
void scaleAndCenter( std::vector< Vec3 > & io_positions ) {
    Vec3 bboxMin( FLT_MAX , FLT_MAX , FLT_MAX );
    Vec3 bboxMax( FLT_MIN , FLT_MIN , FLT_MIN );
    for(unsigned int pIt = 0 ; pIt < io_positions.size() ; ++pIt) {
        for( unsigned int coord = 0 ; coord < 3 ; ++coord ) {
            bboxMin[coord] = std::min<float>( bboxMin[coord] , io_positions[pIt][coord] );
            bboxMax[coord] = std::max<float>( bboxMax[coord] , io_positions[pIt][coord] );
        }
    }
    Vec3 bboxCenter = (bboxMin + bboxMax) / 2.f;
    float bboxLongestAxis = std::max<float>( bboxMax[0]-bboxMin[0] , std::max<float>( bboxMax[1]-bboxMin[1] , bboxMax[2]-bboxMin[2] ) );
    for(unsigned int pIt = 0 ; pIt < io_positions.size() ; ++pIt) {
        io_positions[pIt] = (io_positions[pIt] - bboxCenter) / bboxLongestAxis;
    }
}

void applyRandomRigidTransformation( std::vector< Vec3 > & io_positions , std::vector< Vec3 > & io_normals ) {
    srand(time(NULL));
    Mat3 R = Mat3::RandRotation();
    Vec3 t = Vec3::Rand(1.f);
    for(unsigned int pIt = 0 ; pIt < io_positions.size() ; ++pIt) {
        io_positions[pIt] = R * io_positions[pIt] + t;
        io_normals[pIt] = R * io_normals[pIt];
    }
}

void subsample( std::vector< Vec3 > & i_positions , std::vector< Vec3 > & i_normals , float minimumAmount = 0.1f , float maximumAmount = 0.2f ) {
    std::vector< Vec3 > newPos , newNormals;
    std::vector< unsigned int > indices(i_positions.size());
    for( unsigned int i = 0 ; i < indices.size() ; ++i ) indices[i] = i;
    srand(time(NULL));
    std::random_shuffle(indices.begin() , indices.end());
    unsigned int newSize = indices.size() * (minimumAmount + (maximumAmount-minimumAmount)*(float)(rand()) / (float)(RAND_MAX));
    newPos.resize( newSize );
    newNormals.resize( newSize );
    for( unsigned int i = 0 ; i < newPos.size() ; ++i ) {
        newPos[i] = i_positions[ indices[i] ];
        newNormals[i] = i_normals[ indices[i] ];
    }
    i_positions = newPos;
    i_normals = newNormals;
}

bool save( const std::string & filename , std::vector< Vec3 > & vertices , std::vector< unsigned int > & triangles ) {
    std::ofstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    myfile << "OFF" << std::endl;

    unsigned int n_vertices = vertices.size() , n_triangles = triangles.size()/3;
    myfile << n_vertices << " " << n_triangles << " 0" << std::endl;

    for( unsigned int v = 0 ; v < n_vertices ; ++v ) {
        myfile << vertices[v][0] << " " << vertices[v][1] << " " << vertices[v][2] << std::endl;
    }
    for( unsigned int f = 0 ; f < n_triangles ; ++f ) {
        myfile << 3 << " " << triangles[3*f] << " " << triangles[3*f+1] << " " << triangles[3*f+2];
        myfile << std::endl;
    }
    myfile.close();
    return true;
}



// ------------------------------------------------------------------------------------------------------------
// rendering.
// ------------------------------------------------------------------------------------------------------------

void initLight () {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
}



void drawTriangleMesh( std::vector< Vec3 > const & i_positions , std::vector< unsigned int > const & i_triangles ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_triangles.size() / 3 ; ++tIt) {
        Vec3 p0 = i_positions[i_triangles[3*tIt]];
        Vec3 p1 = i_positions[i_triangles[3*tIt+1]];
        Vec3 p2 = i_positions[i_triangles[3*tIt+2]];

        Vec3 n = Vec3::cross(p1-p0 , p2-p0);
        n.normalize();
        glNormal3f( n[0] , n[1] , n[2] );
        glVertex3f( p0[0] , p0[1] , p0[2] );
        glVertex3f( p1[0] , p1[1] , p1[2] );
        glVertex3f( p2[0] , p2[1] , p2[2] );
    }
    glEnd();
}

void drawPointSet( std::vector< Vec3 > const & i_positions , std::vector< Vec3 > const & i_normals ) {
    glBegin(GL_POINTS);
    for(unsigned int pIt = 0 ; pIt < i_positions.size() ; ++pIt) {
        glNormal3f( i_normals[pIt][0] , i_normals[pIt][1] , i_normals[pIt][2] );
        glVertex3f( i_positions[pIt][0] , i_positions[pIt][1] , i_positions[pIt][2] );
    }
    glEnd();
}

void draw () {
    glPointSize(2); // for example...

    // glColor3f(0.8,0.8,1);
    // drawPointSet(positions , normals);

    // glColor3f(1,0.5,0.5);
    // drawPointSet(positions2 , normals2);
}








void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    glutPostRedisplay ();
}

void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;

    case 'w':
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if(polygonMode[0] != GL_FILL)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        break;

    default:
        break;
    }
    idle ();
}

void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle ();
}

void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}

void project(Vec3 inputPoint, const Vec3 &center, const Vec3 &normal, Vec3 &outputPoint, Vec3 &outputNormal){
    Vec3 v = inputPoint - center;
    float d = Vec3::dot(v, normal);
    outputPoint = inputPoint - d * normal;
    outputNormal = normal;
}

void SPSS(Vec3 inputPoint, Vec3 &outputPoint, Vec3 &outputNormal, const std::vector<Vec3> &positions, const std::vector<Vec3> &normals, const BasicANNkdTree &kdtree, int kerneltype, float radius, unsigned int nbIterations = 10, unsigned int knn = 200) {
    // Allouer de la mémoire pour les indices et les distances des voisins
    ANNidxArray id_nearest_neighbors = new ANNidx[knn];
    ANNdistArray square_distances_to_neighbors = new ANNdist[knn];

    // Initialiser la sortie
    outputPoint = inputPoint;
    outputNormal = Vec3(0.0f, 0.0f, 0.0f);

    for (unsigned int iter = 0; iter < nbIterations; ++iter) {
        // Trouver les knn plus proches voisins
        kdtree.knearest(inputPoint, knn, id_nearest_neighbors, square_distances_to_neighbors);

        // Variables pour accumuler les contributions pondérées
        Vec3 weightedSumPosition(0.0f, 0.0f, 0.0f);
        Vec3 weightedSumNormal(0.0f, 0.0f, 0.0f);
        float weightSum = 0.0f;

        for (unsigned int i = 0; i < knn; ++i) {
            float distance = sqrt(square_distances_to_neighbors[i]);

            // Calculer le poids en fonction du type de noyau
            float weight;
            if (kerneltype == 0) {
                // Noyau uniforme
                weight = 1.0f;
            } else {
                // Noyau gaussien
                weight = exp(-distance * distance / (2 * radius * radius));
            }

            // Accumuler les positions et normales pondérées
            weightedSumPosition += weight * positions[id_nearest_neighbors[i]];
            weightedSumNormal += weight * normals[id_nearest_neighbors[i]];
            weightSum += weight;
        }

        // // Calculer le point et la normale lissés
        // if (weightSum > 0.0f) {
        //     weightedSumPosition / weightSum;
        //     weightedSumNormal / weightSum;
        //     weightedSumNormal.normalize(); // Normaliser la normale
        // }
        weightedSumPosition /= weightSum;
        weightedSumNormal.normalize();

        project(inputPoint, weightedSumPosition, weightedSumNormal, outputPoint, outputNormal);
    }

    // Libérer la mémoire allouée pour les voisins
    delete[] id_nearest_neighbors;
    delete[] square_distances_to_neighbors;

}

void HPSS(Vec3 inputPoint, Vec3 &outputPoint, Vec3 &outputNormal, const std::vector<Vec3> &positions, const std::vector<Vec3> &normals, const BasicANNkdTree &kdtree, int kerneltype, float radius, unsigned int nbIterations = 10, unsigned int knn = 200) {
    outputPoint = inputPoint;
    outputNormal = Vec3(0.0f, 0.0f, 0.0f);

    // Allocation des tableaux pour les voisins et les distances
    ANNidxArray id_nearest_neighbors = new ANNidx[knn];
    ANNdistArray square_distances_to_neighbors = new ANNdist[knn];

    // Boucle d'itérations pour affiner la projection
    for (unsigned int it = 0; it < nbIterations; ++it) {
        // Trouver les k plus proches voisins
        kdtree.knearest(inputPoint, knn, id_nearest_neighbors, square_distances_to_neighbors);

        // Accumulateurs pour calculer le point projeté et la normale
        Vec3 sumPoint(0.0f, 0.0f, 0.0f);
        Vec3 sumNormal(0.0f, 0.0f, 0.0f);
        float weightSum = 0.0f;

        // Calcul de projection MLS basé sur les voisins trouvés
        for (unsigned int i = 0; i < knn; ++i) {
            int idx = id_nearest_neighbors[i];
            Vec3 neighbor = positions[idx];
            Vec3 normal = normals[idx];

            // Calculer le poids de chaque voisin selon le type de noyau
            float dist = sqrt(square_distances_to_neighbors[i]);
            float r = dist / radius;
            float weight = 0.0f;
            float s = 1.2;

            if (kerneltype == 0) { // Noyau gaussien
                weight = exp(-dist * dist / (radius * radius));
            } else if (kerneltype == 1) { // Noyau Wendland
                weight = pow((1.0f - r), 4) * (4.0f * r + 1.0f);
            } else if (kerneltype == 2) { // Noyau singulier
                weight = pow(1/r, s) * (4.0f * r + 1.0f);
            }

            // Utiliser dot pour calculer la projection sur le plan du voisin
            // float projectionDistance = Vec3::dot(outputPoint - neighbor, normal);
            sumPoint += weight * (neighbor);
            sumNormal += weight * normal;
            weightSum += weight;
        }

        // // Calculer le nouveau point projeté et la nouvelle normale
        // if (weightSum > 0.0f) {
        //     outputPoint = sumPoint / weightSum;
        //     outputNormal = sumNormal / weightSum;
        //     outputNormal.normalize();
        // } else {
        //     break; // Pas de contribution significative, arrêter
        // }

        sumPoint /= weightSum;
        sumNormal.normalize();

        project(inputPoint, sumPoint, sumNormal, outputPoint, outputNormal);
    }

    // Libération des tableaux alloués
    delete[] id_nearest_neighbors;
    delete[] square_distances_to_neighbors;
}


void dualContouring(
    const BasicANNkdTree &kdtree,
    std::vector<Vec3> &i_positions,
    std::vector<Vec3> &i_normals,
    std::vector<unsigned int> &i_triangles,
    int gridResolution = 32
) {
    // Étape 1 : Définir la bounding box
    Vec3 boundingBoxMin(INFINITY, INFINITY, INFINITY);
    Vec3 boundingBoxMax(-INFINITY, -INFINITY, -INFINITY);

    // Étape 2 : Déterminer les bornes à partir des positions des points (i_positions)
    for (const auto& pos : i_positions) {
        boundingBoxMin[0] = std::min(boundingBoxMin[0], pos[0]);
        boundingBoxMin[1] = std::min(boundingBoxMin[1], pos[1]);
        boundingBoxMin[2] = std::min(boundingBoxMin[2], pos[2]);

        boundingBoxMax[0] = std::max(boundingBoxMax[0], pos[0]);
        boundingBoxMax[1] = std::max(boundingBoxMax[1], pos[1]);
        boundingBoxMax[2] = std::max(boundingBoxMax[2], pos[2]);
    }

    // Dimensions de la boundingBox
    Vec3 gridSize = boundingBoxMax - boundingBoxMin; // Taille de la boundingBox
    float gridSpacingX = gridSize[0] / (gridResolution - 1);
    float gridSpacingY = gridSize[1] / (gridResolution - 1);
    float gridSpacingZ = gridSize[2] / (gridResolution - 1);

    // Stocker les valeurs de la fonction implicite sur la grille
    std::vector<float> implicitGrid(gridResolution * gridResolution * gridResolution, 0.0f);

    // Échantillonnage de la fonction implicite sur la grille
    for (int x = 0; x < gridResolution; ++x) {
        for (int y = 0; y < gridResolution; ++y) {
            for (int z = 0; z < gridResolution; ++z) {
                // Coordonnées dans l'espace (ajustées avec la boundingBox)
                Vec3 projPoint, projNormal;
                Vec3 gridPoint(
                    boundingBoxMin[0] + x * gridSpacingX,
                    boundingBoxMin[1] + y * gridSpacingY,
                    boundingBoxMin[2] + z * gridSpacingZ
                );
                HPSS(gridPoint, projPoint, projNormal, i_positions, i_normals, kdtree,  0,  0.1f);
      
                //printf("%f\n", gridPoint[1]);
                implicitGrid[x + y * gridResolution + z * gridResolution * gridResolution] = Vec3::dot(gridPoint - projPoint, projNormal);
                //printf("%f", implicitGrid[x + y * gridResolution + z * gridResolution * gridResolution]);
            }
        }
    }

    // Parcours de chaque cellule de la grille
    for (int x = 0; x < gridResolution - 1; ++x) {
        for (int y = 0; y < gridResolution - 1; ++y) {
            for (int z = 0; z < gridResolution - 1; ++z) {
                // Indices des 8 sommets d'une cellule de la grille
                int idx0 = x + y * gridResolution + z * gridResolution * gridResolution;
                int idx1 = idx0 + 1;
                int idx2 = idx0 + gridResolution;
                int idx3 = idx2 + 1;
                int idx4 = idx0 + gridResolution * gridResolution;
                int idx5 = idx4 + 1;
                int idx6 = idx4 + gridResolution;
                int idx7 = idx6 + 1;

                // Récupérer les valeurs implicites aux sommets de la cellule
                float f0 = implicitGrid[idx0];
                float f1 = implicitGrid[idx1];
                float f2 = implicitGrid[idx2];
                float f3 = implicitGrid[idx3];
                float f4 = implicitGrid[idx4];
                float f5 = implicitGrid[idx5];
                float f6 = implicitGrid[idx6];
                float f7 = implicitGrid[idx7];

                // Vérifier si un changement de signe existe dans la cellule (indique une intersection)
                if ((f0 * f7 < 0) || (f1 * f6 < 0) || (f2 * f5 < 0) || (f3 * f4 < 0)) {

                    printf("ok\n");
                    // Calculer la position du sommet en minimisant l'erreur quadratique
                    Vec3 cubeCenter(
                        boundingBoxMin[0] + (x + 0.5f) * gridSpacingX,
                        boundingBoxMin[1] + (y + 0.5f) * gridSpacingY,
                        boundingBoxMin[2] + (z + 0.5f) * gridSpacingZ
                    );
                    Vec3 optimalVertex = cubeCenter;  // Pour simplifier, on prend le centre du cube ici

                    // Ajouter la position du sommet optimisé dans i_positions
                    unsigned int vertexIndex = i_positions.size();
                    i_positions.push_back(optimalVertex);

                    // Générer les triangles pour cette cellule (simplification)
                    Vec3 v0 = cubeCenter + Vec3(-gridSpacingX / 2, -gridSpacingY / 2, -gridSpacingZ / 2);
                    Vec3 v1 = cubeCenter + Vec3(gridSpacingX / 2, -gridSpacingY / 2, -gridSpacingZ / 2);
                    Vec3 v2 = cubeCenter + Vec3(-gridSpacingX / 2, gridSpacingY / 2, -gridSpacingZ / 2);
                    Vec3 v3 = cubeCenter + Vec3(gridSpacingX / 2, gridSpacingY / 2, -gridSpacingZ / 2);
                    Vec3 v4 = cubeCenter + Vec3(-gridSpacingX / 2, -gridSpacingY / 2, gridSpacingZ / 2);
                    Vec3 v5 = cubeCenter + Vec3(gridSpacingX / 2, -gridSpacingY / 2, gridSpacingZ / 2);
                    Vec3 v6 = cubeCenter + Vec3(-gridSpacingX / 2, gridSpacingY / 2, gridSpacingZ / 2);
                    Vec3 v7 = cubeCenter + Vec3(gridSpacingX / 2, gridSpacingY / 2, gridSpacingZ / 2);

                    // Stocker les indices des triangles dans i_triangles
                    // Exemple de triangles pour une face de la cellule
                    i_triangles.push_back(vertexIndex);
                    i_triangles.push_back(vertexIndex + 1);
                    i_triangles.push_back(vertexIndex + 2);

                    i_triangles.push_back(vertexIndex + 2);
                    i_triangles.push_back(vertexIndex + 1);
                    i_triangles.push_back(vertexIndex + 3);

                    i_triangles.push_back(vertexIndex + 4);
                    i_triangles.push_back(vertexIndex + 5);
                    i_triangles.push_back(vertexIndex + 6);

                    i_triangles.push_back(vertexIndex + 6);
                    i_triangles.push_back(vertexIndex + 5);
                    i_triangles.push_back(vertexIndex + 7);
                }
            }
        }
    }
}





int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("tp point processing");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);


    {
        // Load a first pointset, and build a kd-tree:
        loadPN("pointsets/igea.pn" , positions , normals);

        BasicANNkdTree kdtree;
        kdtree.build(positions);

        // Create a second pointset that is artificial, and project it on pointset1 using MLS techniques:
        // positions2.resize( 1000 );
        // normals2.resize(positions2.size());
        // for( unsigned int pIt = 0 ; pIt < positions2.size() ; ++pIt ) {
        //     positions2[pIt] = Vec3(
        //                 -0.6 + 1.2 * (double)(rand())/(double)(RAND_MAX),
        //                 -0.6 + 1.2 * (double)(rand())/(double)(RAND_MAX),
        //                 -0.6 + 1.2 * (double)(rand())/(double)(RAND_MAX)
        //                 );
        //     positions2[pIt].normalize();
        //     positions2[pIt] = 0.6 * positions2[pIt];
        // }

        // // PROJECT USING MLS (HPSS and APSS):
        // for (unsigned int pIt = 0; pIt < positions2.size(); ++pIt) {
        // Vec3 outputPoint, outputNormal;
        // HPSS(positions2[pIt], outputPoint, outputNormal, positions, normals, kdtree, 0, 0.1f);
        // positions2[pIt] = outputPoint;
        // normals2[pIt] = outputNormal;
        // }
        // TODO

        std::vector<unsigned int> triangles;
        dualContouring(kdtree, positions, normals, triangles, 32);

        drawTriangleMesh(positions, triangles);
    }



    glutMainLoop ();
    return EXIT_SUCCESS;
}

