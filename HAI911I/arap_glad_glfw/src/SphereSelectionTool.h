#ifndef SphereSelectionTool_H
#define SphereSelectionTool_H

#include "OpenGL.h"
#include <cmath>

struct SphereSelectionTool {
    int xCenter, yCenter;   // Centre de la sphère (point de clic)
    float radius;           // Rayon de la sphère
    bool isAdding;          // Ajouter ou enlever des points
    bool isActive;

    SphereSelectionTool() : xCenter(0), yCenter(0), radius(0), isActive(false) {}

    // Initialiser la sphère avec un point de clic
    void initSphere(int x, int y) {
        xCenter = x;
        yCenter = y;
        radius = .1f;  // Commencer avec un rayon nul
    }

    // Mettre à jour le rayon en fonction du mouvement de la molette
    void updateRadius(float deltaRadius) {
        radius = std::max(0.1f, radius + deltaRadius);  // Ajuster le rayon, éviter un rayon négatif ou nul
    }

    void updateSphere(int x , int y) {
        xCenter = x;
        yCenter = y;
    }

    // Vérifier si un point est dans la sphère de sélection
    bool contains(float x, float y) const {
        // On récupère la position dans l'espace écran
        float viewport[4]; 
        glGetFloatv(GL_VIEWPORT, viewport);
        float w = viewport[2], h = viewport[3];

        // Conversion des coordonnées écran vers [0,1] pour comparaison avec x, y
        float xNormalized = (float)xCenter / w;
        float yNormalized = 1.f - (float)yCenter / h;

        // Calcul de la distance entre le point et le centre
        float dist = std::sqrt((x - xNormalized) * (x - xNormalized) +
                               (y - yNormalized) * (y - yNormalized));

        // Retourner vrai si le point est dans la sphère
        return dist * dist <= radius * radius;
    }

    // Dessiner la sphère de sélection
    
    void draw() {
        if (!isActive) return;

        float viewport[4]; 
        glGetFloatv(GL_VIEWPORT, viewport);
        float w = viewport[2], h = viewport[3];

        // Calculer les coordonnées du centre et le rayon
        // Normalisation des coordonnées du clic
        float centerX = (float)xCenter / w; // Coordonnée x normalisée
        float centerY = 1.f - (float)yCenter / h; // Coordonnée y normalisée

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);

        // Remplir le cercle
        glColor4f(1, 1, 1, 0.4f); // Couleur blanche transparente
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY); // Centre du cercle
        for (int i = 0; i <= 100; ++i) { // Dessiner 100 segments
            float angle = 2.0f * M_PI * float(i) / float(100); // Angle
            float x = radius * cos(angle); // Coordonnée x
            float y = radius * sin(angle); // Coordonnée y
            glVertex2f(centerX + x, centerY + y); // Ajouter le sommet
        }
        glEnd();

        glLineWidth(2.0);

        // Déterminer la couleur selon le mode d'ajout ou de suppression
        if (isAdding)
            glColor4f(0.1, 0.1, 1.f, 0.5f); // Couleur bleue pour ajout
        else
            glColor4f(1.0, 0.1, 0.1, 0.5f); // Couleur rouge pour suppression

        // Dessiner le contour du cercle
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= 100; ++i) { // Dessiner 100 segments
            float angle = 2.0f * M_PI * float(i) / float(100); // Angle
            float x = radius * cos(angle); // Coordonnée x
            float y = radius * sin(angle); // Coordonnée y
            glVertex2f(centerX + x, centerY + y); // Ajouter le sommet
        }
        glEnd();

        glEnable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }


};

#endif // SphereSelectionTool_H
