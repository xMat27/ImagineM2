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




BasicANNkdTree kdtree;
std::vector< Vec3 > positions;
std::vector< Vec3 > normals;

std::vector< Vec3 > positions2;
std::vector< Vec3 > normals2;
Mat3 ICProtation;
Vec3 ICPtranslation; // USED TO ALIGN pointset 2 onto pointset 1

std::vector< Vec3 > positions3;
std::vector< Vec3 > normals3; // OUTPUT of alignment


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





// ------------------------------------
// ICP
// ------------------------------------

Vec3 project_orth( const Vec3 & x, const Vec3 & c , const Vec3 & n ){
    Vec3 cx = x-c;
    return x - Vec3::dot(cx, n)*n;
}


void HPSS(Vec3 input_point,
          Vec3 &output_point, Vec3 &output_normal,
          const std::vector<Vec3> &positions, const std::vector<Vec3> &normals,
          const BasicANNkdTree &kd_tree,
          unsigned int nb_iter = 10, uint knn = 20) {

    ANNidxArray id_nearest_neighbors = new ANNidx[knn];
    ANNdistArray square_distances_to_neighbors = new ANNdist[knn];
    Vec3 x_k = input_point, c, n;

    for (unsigned int it = 0; it < nb_iter; it++) {
        kd_tree.knearest(x_k, knn, id_nearest_neighbors, square_distances_to_neighbors);
        double sqrradius = square_distances_to_neighbors[knn-1];
        float total_w = 0.f;
        c = Vec3(0.f, 0.f, 0.f);
        n = Vec3(0.f, 0.f, 0.f);
        for (unsigned int k = 0; k < knn; k++) {
            int j = id_nearest_neighbors[k];
            Vec3 p_tilde = project_orth(x_k, positions[j], normals[j]);
            float w = std::exp(-(x_k - p_tilde).squareLength() / sqrradius);
            total_w += w;
            c += w * p_tilde;
            n += w * normals[j];
        }
        c /= total_w;
        n.normalize();

        x_k = project_orth(x_k, c, n);
    }

    output_point = x_k;
    output_normal = n;

    delete[] id_nearest_neighbors;
    delete[] square_distances_to_neighbors;
}

void ICP( std::vector<Vec3>& ps,  std::vector<Vec3>& nps,
         std::vector<Vec3>& qs,  std::vector<Vec3>& nqs,
         const BasicANNkdTree& qsKdTree, Mat3& rotation, Vec3& translation, unsigned int nIterations) {
    
    // Initialisation de la rotation et translation
    rotation = Mat3::Identity();
    translation = {0, 0, 0};

    
    for (unsigned int it = 0; it < nIterations; ++it) {

        // Mat3 M ();

        // for (size_t i = 0; i < ps.size(); ++i) {
        //     Vec3 nnPi = qs[qsKdTree.nearest(ps(i))];
        // }

        //HPSS
        
        std::vector<Vec3> correspondences(ps.size());
        std::vector<Vec3> correspondences_normals(ps.size());
        
        for (size_t i = 0; i < ps.size(); ++i) {
            HPSS(ps[i], correspondences[i], correspondences_normals[i], qs, nqs, qsKdTree);
        }

        //Sans HPSS

        // std::vector<Vec3> correspondences;
        // for (size_t i = 0; i < ps.size(); ++i) {
        //     correspondences.push_back(qs[qsKdTree.nearest(ps[i])]);
        // }
        
        
        Vec3 centroid_ps = {0, 0, 0};
        Vec3 centroid_qs = {0, 0, 0};
        
        for (size_t i = 0; i < ps.size(); ++i) {
            centroid_ps += ps[i];
            centroid_qs += correspondences[i];
        }
        centroid_ps /= static_cast<float>(ps.size());
        centroid_qs /= static_cast<float>(ps.size());

        
        std::vector<Vec3> centered_ps(ps.size());
        std::vector<Vec3> centered_qs(correspondences.size());
        for (size_t i = 0; i < ps.size(); ++i) {
            centered_ps[i] = ps[i] - centroid_ps;
            centered_qs[i] = correspondences[i] - centroid_qs;
        }

        
        Mat3 covariance = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        for (size_t i = 0; i < ps.size(); ++i) {
            covariance += Mat3::tensor(centered_ps[i], centered_qs[i]);
        } 

        
        Mat3 U, Vt;
        float sx, sy, sz;

        //covariance.SVD(U, sx, sy, sz, Vt);
        covariance.setRotation();
        

        // Mettre à jour la rotation et la translation
        rotation = covariance;
        translation = centroid_qs - covariance * centroid_ps;

        for( unsigned int pIt = 0 ; pIt < ps.size() ; ++pIt ) {
            ps[pIt] = ICProtation * qs[pIt] + ICPtranslation;
            nps[pIt] = ICProtation * nqs[pIt];
        }

        std::cout << "\tICP , iteration " << it << " done" << std::endl;
    }
}











// ------------------------------------
// i/o and some stuff
// ------------------------------------
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

void subsampleAlongRandomDirection( std::vector< Vec3 > & i_positions , std::vector< Vec3 > & i_normals  ) {
    Vec3 randomDir( -1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)) );
    randomDir.normalize();

    Vec3 bb(FLT_MAX,FLT_MAX,FLT_MAX),BB(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    for( unsigned int i = 0 ; i < i_positions.size() ; ++i ) {
        Vec3 p = i_positions[ i ];
        bb[0] = std::min<double>(bb[0] , p[0]);
        bb[1] = std::min<double>(bb[1] , p[1]);
        bb[2] = std::min<double>(bb[2] , p[2]);
        BB[0] = std::max<double>(BB[0] , p[0]);
        BB[1] = std::max<double>(BB[1] , p[1]);
        BB[2] = std::max<double>(BB[2] , p[2]);
    }

    double lambdaMin = FLT_MAX , lambdaMax = -FLT_MAX;
    lambdaMin = std::min<double>(Vec3::dot(Vec3(bb[0],bb[1],bb[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(BB[0],bb[1],bb[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(bb[0],BB[1],bb[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(BB[0],BB[1],bb[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(bb[0],bb[1],BB[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(BB[0],bb[1],BB[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(bb[0],BB[1],BB[2]) , randomDir) , lambdaMin);
    lambdaMin = std::min<double>(Vec3::dot(Vec3(BB[0],BB[1],BB[2]) , randomDir) , lambdaMin);

    lambdaMax = std::max<double>(Vec3::dot(Vec3(bb[0],bb[1],bb[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(BB[0],bb[1],bb[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(bb[0],BB[1],bb[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(BB[0],BB[1],bb[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(bb[0],bb[1],BB[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(BB[0],bb[1],BB[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(bb[0],BB[1],BB[2]) , randomDir) , lambdaMax);
    lambdaMax = std::max<double>(Vec3::dot(Vec3(BB[0],BB[1],BB[2]) , randomDir) , lambdaMax);

    double lambdaTarget = lambdaMin + ((float)(rand()) / (float)(RAND_MAX)) * (lambdaMax - lambdaMin);

    std::vector< Vec3 > newPos , newNormals;
    for( unsigned int i = 0 ; i < i_positions.size() ; ++i ) {
        Vec3 p = i_positions[ i ];
        double uRand = 0.0;// ((double)(rand()) / (double)(RAND_MAX));
        if( Vec3::dot(p,randomDir) < lambdaTarget + uRand ) {
            newPos.push_back( i_positions[ i ] );
            newNormals.push_back( i_normals[ i ] );
        }
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



// ------------------------------------

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




// ------------------------------------
// rendering.
// ------------------------------------

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

    glColor3f(0.8,0.8,1); // BLUE
    drawPointSet(positions , normals);

    glColor3f(0.8,1,0.5); // GREEN
    drawPointSet(positions2 , normals2);

    glColor3f(1,0.5,0.5); // RED
    drawPointSet(positions3 , normals3);
}






void performICP( unsigned int nIterations ) {
    positions3 = positions2;
    normals3 = normals2;

    ICP(positions3 , normals3 , positions , normals , kdtree , ICProtation , ICPtranslation , nIterations );
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

    case 'i':
        performICP(10);
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



int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("tp point processing : ICP");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);


    // ICP :
    {

        //Different

        // Load a first pointset, and build a kd-tree:
        loadPN("pointsets/dino_subsampled_extreme.pn" , positions , normals);
        kdtree.build(positions);

        // Load a second pointset :
        loadPN("pointsets/dino2_subsampled_extreme.pn" , positions2 , normals2);

        // Transform it slightly :
        srand(time(NULL));
        Mat3 rotation = Mat3::RandRotation(M_PI / 3); // PLAY WITH THIS PARAMETER !!!!!!
        Vec3 translation = Vec3( -1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)) );
        for( unsigned int pIt = 0 ; pIt < positions2.size() ; ++pIt ) {
            positions2[pIt] = rotation * positions2[pIt] + translation;
            normals2[pIt] = rotation * normals2[pIt];
        }

        // Initial solution for ICP :
        ICProtation = Mat3::Identity();
        ICPtranslation = Vec3(0,0,0);
        positions3 = positions2;
        normals3 = normals2;


        //Identique

        // loadPN("pointsets/dino_subsampled_extreme.pn" , positions , normals);
        // kdtree.build(positions);
        // positions2 = positions;
        // normals2 = normals;

        // srand(time(NULL));
        // Mat3 ICProtation = Mat3::RandRotation(M_PI / 3); // PLAY WITH THIS PARAMETER !!!!!!
        // Vec3 ICPtranslation = Vec3( -1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)) );
        // for( unsigned int pIt = 0 ; pIt < positions2.size() ; ++pIt ) {
        //      positions2[pIt] = ICProtation * positions2[pIt] + ICPtranslation;
        //      normals2[pIt] = ICProtation * normals2[pIt];
        // }
        // // Initial solution for ICP :
        // ICProtation = Mat3::Identity();
        // ICPtranslation = Vec3(0,0,0);
        // positions3 = positions2;
        // normals3 = normals2;

        
        //Partiel

        // // Load a first pointset, and build a kd-tree:
        // loadPN("pointsets/african_statue_partial_1.pn" , positions , normals);
        // kdtree.build(positions);

        // // Load a second pointset :
        // loadPN("pointsets/african_statue.pn" , positions2 , normals2);

        // // Transform it slightly :
        // srand(time(NULL));
        // Mat3 rotation = Mat3::RandRotation(M_PI / 3); // PLAY WITH THIS PARAMETER !!!!!!
        // Vec3 translation = Vec3( -1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)),-1.0 + 2.0 * ((double)(rand()) / (double)(RAND_MAX)) );
        // for( unsigned int pIt = 0 ; pIt < positions2.size() ; ++pIt ) {
        //     positions2[pIt] = rotation * positions2[pIt] + translation;
        //     normals2[pIt] = rotation * normals2[pIt];
        // }

        // // Initial solution for ICP :
        // ICProtation = Mat3::Identity();
        // ICPtranslation = Vec3(0,0,0);
        // positions3 = positions2;
        // normals3 = normals2;
    }





    glutMainLoop ();
    return EXIT_SUCCESS;
}

