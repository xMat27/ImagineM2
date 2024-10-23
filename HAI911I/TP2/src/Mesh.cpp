#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <cmath>

void Mesh::loadOFF (const std::string & filename) {
    std::ifstream in (filename.c_str ());
    if (!in)
        exit (EXIT_FAILURE);
    std::string offString;
    unsigned int sizeV, sizeT, tmp;
    in >> offString >> sizeV >> sizeT >> tmp;
    V.resize (sizeV);
    T.resize (sizeT);
    for (unsigned int i = 0; i < sizeV; i++)
        in >> V[i].p;
    int s;
    for (unsigned int i = 0; i < sizeT; i++) {
        in >> s;
        for (unsigned int j = 0; j < 3; j++)
            in >> T[i].v[j];
    }
    in.close ();
    recomputeNormals ();
}

void Mesh::recomputeNormals () {
    for (unsigned int i = 0; i < V.size (); i++)
        V[i].n = Vec3 (0.0, 0.0, 0.0);
    for (unsigned int i = 0; i < T.size (); i++) {
        Vec3 e01 = V[T[i].v[1]].p -  V[T[i].v[0]].p;
        Vec3 e02 = V[T[i].v[2]].p -  V[T[i].v[0]].p;
        Vec3 n = Vec3::cross (e01, e02);
        n.normalize ();
        for (unsigned int j = 0; j < 3; j++)
            V[T[i].v[j]].n += n;
    }
    for (unsigned int i = 0; i < V.size (); i++)
        V[i].n.normalize ();
}

void Mesh::compute_skinning_weights(Skeleton &skeleton) {
    // Parcourir chaque sommet du maillage
    for (unsigned int i = 0; i < V.size(); ++i) {
        MeshVertex &vertex = V[i];
        vertex.w.clear(); // Réinitialiser les poids pour ce sommet

        float totalWeight = 0.0f;
        std::vector<float> weights;

        // Calculer la distance entre ce sommet et chaque os du squelette
        for (unsigned int j = 0; j < skeleton.bones.size(); ++j) {
            const Bone &bone = skeleton.bones[j];

            Articulation & a0 = skeleton.articulations[ bone.joints[0] ];
            Articulation & a1 = skeleton.articulations[ bone.joints[1] ];
            //Vec3 bonePosition = (articulations[bone.joints[0]].p + articulations[bone.joints[1]].p) * 0.5;
            // Calculez la distance entre le sommet et l'os
            float dist = (vertex.p - 0.5 * (a0.p + a1.p) ).length();


            // Inverser la distance pour obtenir un poids
            float weight = 1.0f / pow(dist + 1e-5f, 4); // Ajoutez un epsilon pour éviter la division par zéro
            weights.push_back(weight);
            totalWeight += weight;
        }

        // Normaliser les poids pour que leur somme soit égale à 1
        for (unsigned int j = 0; j < weights.size(); ++j) {
            vertex.w.push_back(weights[j] / totalWeight);
        }
    }
}


void Mesh::draw( int displayedBone ) const {

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glBegin (GL_TRIANGLES);
    for (unsigned int i = 0; i < T.size (); i++)
        for (unsigned int j = 0; j < 3; j++) {
            const MeshVertex & v = V[T[i].v[j]];
            if( displayedBone >= 0 && displayedBone < v.w.size() ) {
                Vec3 rgb = HSVtoRGB( v.w[displayedBone], 0.8,0.8 );
                glColor3f( rgb[0], rgb[1], rgb[2] );
            } else glColor3f( 0.6, 0.6, 0.6 );
            glNormal3f (v.n[0], v.n[1], v.n[2]);
            glVertex3f (v.p[0], v.p[1], v.p[2]);
        }

    glEnd ();
}

void Mesh::drawTransformedMesh( SkeletonTransformation & transfo ) const {


    std::vector< Vec3 > new_positions( V.size() );
    std::vector< Vec3 > new_normals( V.size() );

    //---------------------------------------------------//
    //---------------------------------------------------//
    // code to change :
    for( unsigned int i = 0 ; i < V.size() ; ++i ) {
        Vec3 p(0.0, 0.0, 0.0); // Position transformée finale du sommet
        Vec3 n(0.0, 0.0, 0.0); // Normale transformée finale du sommet

        Vec3 initialP = V[i].p; // Position initiale du sommet
        Vec3 initialN = V[i].n; // Normale initiale du sommet

        // Parcourir tous les os influençant ce sommet
        for (unsigned int j = 0; j < V[i].w.size(); ++j) {
            float weight = V[i].w[j]; // Poids de skinning

            Mat3 rotation = transfo.bone_transformations[j].world_space_rotation;
            Vec3 translation = transfo.bone_transformations[j].world_space_translation;

            // Appliquer la transformation pondérée de l'os à la position
            p += weight * (rotation * initialP + translation);

            // Appliquer la transformation pondérée de l'os à la normale
            n += weight * (rotation * initialN);
        }

        // Normaliser la normale finale après transformation
        new_positions[i] = p;
        new_normals[i] =n ;
        new_normals[i].normalize();
    }

        
    
    //---------------------------------------------------//
    //---------------------------------------------------//
    //---------------------------------------------------//

    glEnable(GL_LIGHTING);
    glBegin (GL_TRIANGLES);
    for (unsigned int i = 0; i < T.size (); i++)
        for (unsigned int j = 0; j < 3; j++) {
            const MeshVertex & v = V[T[i].v[j]];
            Vec3 p = new_positions[ T[i].v[j] ];
            Vec3 n = new_normals[ T[i].v[j] ];
            glColor3f( 0.6, 0.6, 0.6 );
            glNormal3f (n[0], n[1], n[2]);
            glVertex3f (p[0], p[1], p[2]);
        }
    glEnd ();
}

/*! \brief Convert HSV to RGB color space

  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.

  \param fH Hue component, used as input, range: [0, 1]
  \param fS Hue component, used as input, range: [0, 1]
  \param fV Hue component, used as input, range: [0, 1]

  \param fR Red component, used as output, range: [0, 1]
  \param fG Green component, used as output, range: [0, 1]
  \param fB Blue component, used as output, range: [0, 1]

*/
Vec3 Mesh::HSVtoRGB( float fH, float fS, float fV) const {

    fH=(1.-fH)*0.65*360.;

    float fR, fG, fB;
    float fC = fV * fS; // Chroma
    float fHPrime = fmod(fH / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = fV - fC;

    if(0 <= fHPrime && fHPrime < 1) {
        fR = fC;
        fG = fX;
        fB = 0;
    } else if(1 <= fHPrime && fHPrime < 2) {
        fR = fX;
        fG = fC;
        fB = 0;
    } else if(2 <= fHPrime && fHPrime < 3) {
        fR = 0;
        fG = fC;
        fB = fX;
    } else if(3 <= fHPrime && fHPrime < 4) {
        fR = 0;
        fG = fX;
        fB = fC;
    } else if(4 <= fHPrime && fHPrime < 5) {
        fR = fX;
        fG = 0;
        fB = fC;
    } else if(5 <= fHPrime && fHPrime < 6) {
        fR = fC;
        fG = 0;
        fB = fX;
    } else {
        fR = 0;
        fG = 0;
        fB = 0;
    }

    fR += fM;
    fG += fM;
    fB += fM;
    return Vec3(fR,fG,fB);
}
