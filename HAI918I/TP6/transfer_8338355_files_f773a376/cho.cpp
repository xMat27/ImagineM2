#include <GLFW/glfw3.h>
#include <iostream>
#include <GL/glu.h>
#include <fstream>
#include <algorithm> 
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

// Structure pour un sommet (3D)
struct Vertex {
    float x, y, z;
};

// Structure pour le maillage
struct Mesh {
    std::vector<Vertex> vertices;
};

struct VertexWithRadius {
    Vertex v;
    float radius;
    float azimuth;
    float elevation;
};


// Fonction pour calculer le barycentre du maillage
Vertex computeCentroid(const Mesh& mesh) {
    Vertex centroid = {0.0f, 0.0f, 0.0f};
    for (const Vertex& v : mesh.vertices) {
        centroid.x += v.x;
        centroid.y += v.y;
        centroid.z += v.z;
    }
    centroid.x /= mesh.vertices.size();
    centroid.y /= mesh.vertices.size();
    centroid.z /= mesh.vertices.size();
    return centroid;
}

// Convertir un sommet des coordonnées cartésiennes aux coordonnées polaires
void cartesianToPolar(const Vertex& v, const Vertex& centroid, float& radius, float& azimuth, float& elevation) {
    float dx = v.x - centroid.x;
    float dy = v.y - centroid.y;
    float dz = v.z - centroid.z;

    // Calcul du rayon (distance entre le sommet et le barycentre)
    radius = sqrt(dx * dx + dy * dy + dz * dz);

    // Calcul de l'azimut (angle autour de l'axe z)
    azimuth = acos(dz / radius);

    // Calcul de l'élévation (angle vertical par rapport au plan xy)
    elevation = acos(dx /sqrt (dx * dx) + dy * dy);
}

// Convertir des coordonnées polaires en coordonnées cartésiennes
Vertex polarToCartesian(float radius, float azimuth, float elevation, const Vertex& centroid) {
    Vertex v;
    v.x = centroid.x + radius * sin(elevation) * cos(azimuth);
    v.y = centroid.y + radius * sin(elevation) * sin(azimuth);
    v.z = centroid.z + radius * sin(elevation);
    return v;
}


// Fonction d'initialisation de GLFW et création de fenêtre
GLFWwindow* initOpenGLWindow(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "Erreur : Impossible d'initialiser GLFW" << std::endl;
        return nullptr;
    }

    // Créer la fenêtre
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Erreur : Impossible de créer la fenêtre GLFW" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glViewport(0, 0, width, height);
    
    return window;
}

void drawMesh(const Mesh& mesh) {
    glBegin(GL_POINTS);  // Commencer à dessiner des points

    for (const Vertex& v : mesh.vertices) {
        glVertex3f(v.x, v.y, v.z);  // Placer chaque sommet
    }

    glEnd();  // Terminer de dessiner
}

std::string textToBinary(const std::string& text) {
    std::string binaryMessage;
    for (char c : text) {
        for (int i = 7; i >= 0; --i) {
            binaryMessage += ((c >> i) & 1) ? '1' : '0';
        }
    }
    return binaryMessage;
}

std::string binaryToText(const std::string& binaryMessage) {
    std::string text;
    for (size_t i = 0; i < binaryMessage.length(); i += 8) {
        char c = 0;
        for (int j = 0; j < 8; ++j) {
            c = (c << 1) | (binaryMessage[i + j] - '0');
        }
        text += c;
    }
    return text;
}

// Fonction pour charger un fichier .obj
bool loadObjMesh(const std::string& filename, Mesh& mesh) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << std::endl;
        return false;
    }

    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        // On lit uniquement les lignes de sommets (commencent par 'v')
        if (prefix == "v") {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
    }

    return true;
}

// Fonction pour charger un fichier .off
bool loadOffMesh(const std::string& filename, Mesh& mesh) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);  // Lire la première ligne ("OFF")
    
    if (line != "OFF") {
        std::cerr << "Erreur : Le fichier n'est pas un fichier OFF valide." << std::endl;
        return false;
    }

    int numVertices, numFaces, numEdges;
    file >> numVertices >> numFaces >> numEdges;  // Lire les nombres de sommets, faces et arêtes
    
    // Lire les sommets
    for (int i = 0; i < numVertices; ++i) {
        Vertex v;
        file >> v.x >> v.y >> v.z;
        mesh.vertices.push_back(v);
    }

    // On ignore les faces et arêtes pour cet exemple, car seul les sommets nous intéressent pour l'encodage
    return true;
}



void embedMessageInMesh(Mesh& mesh, const std::string& message) {

    Vertex centroid = computeCentroid(mesh);
    // Convertir le message en binaire
    std::string binaryMessage = textToBinary(message);
    
    // Paramètre epsilon pour les petits déplacements
    float alpha = 0.0001f;
    int numClasses = binaryMessage.size();

    std::vector<VertexWithRadius> verticesWithRadius;
    // Parcourir les sommets du maillage pour encoder le message
    int bitIndex = 0;
    for (Vertex& v : mesh.vertices) {
        if (bitIndex >= binaryMessage.size()) {
            break;  // Si tous les bits du message ont été encodés, on arrête
        }

        // Convertir les coordonnées du sommet en coordonnées polaires
        float radius, azimuth, elevation;
        cartesianToPolar(v, centroid, radius, azimuth, elevation);
        verticesWithRadius.push_back({v, radius, azimuth, elevation});
    }

    float minRadius = std::numeric_limits<float>::max();
    float maxRadius = std::numeric_limits<float>::lowest();
    for (const auto& vertex : verticesWithRadius) {
        minRadius = std::min(minRadius, vertex.radius);
        maxRadius = std::max(maxRadius, vertex.radius);
    }

    // Normaliser les valeurs de radius
    for (auto& vertex : verticesWithRadius) {
        vertex.radius = (vertex.radius - minRadius) / (maxRadius - minRadius);  // Normalisation
    }

    std::sort(verticesWithRadius.begin(), verticesWithRadius.end(),
              [](const VertexWithRadius& a, const VertexWithRadius& b) {
                  return a.radius < b.radius;
              });

    size_t verticesPerClass = verticesWithRadius.size() / numClasses;
    for (int i = 0; i < numClasses; ++i) {
        // Sélectionner les sommets de la classe i
        size_t startIdx = i * verticesPerClass;
        size_t endIdx = (i + 1) * verticesPerClass;

        // Calculer la moyenne du radius pour cette classe
        float radiusSum = 0.0f;
        for (size_t j = startIdx; j < endIdx; ++j) {
            radiusSum += verticesWithRadius[j].radius;
        }
        float averageRadius = radiusSum / (endIdx - startIdx);

        // Encoder le bit
        if (binaryMessage[i] == '1') {
            float k = 1.0f;
            while (averageRadius > 0.5f + alpha) {
                printf("%f\n", averageRadius);
                k -= (1 - 2 * alpha)/(1 + 2 * alpha);
                for (size_t j = startIdx; j < endIdx; ++j) {
                    verticesWithRadius[j].radius = pow(verticesWithRadius[j].radius, k);
                }

                // Recalculer la moyenne après la multiplication
                radiusSum = 0.0f;
                for (size_t j = startIdx; j < endIdx; ++j) {
                    radiusSum += verticesWithRadius[j].radius;
                }
                averageRadius = radiusSum / (endIdx - startIdx);  // Nouvelle moyenne du radius
            }
        } else {
            float k = 1.0f;
            while (averageRadius < 0.5f - alpha) {
                printf("%f\n", averageRadius);
                k += (1 + 2 * alpha)/(1 - 2 * alpha);
                for (size_t j = startIdx; j < endIdx; ++j) {
                    verticesWithRadius[j].radius = pow(verticesWithRadius[j].radius, k);
                }

                // Recalculer la moyenne après la multiplication
                radiusSum = 0.0f;
                for (size_t j = startIdx; j < endIdx; ++j) {
                    radiusSum += verticesWithRadius[j].radius;
                }
                averageRadius = radiusSum / (endIdx - startIdx);  // Nouvelle moyenne du radius
            }
        }

        for (size_t j = startIdx; j < endIdx; ++j) {
            VertexWithRadius& vertex = verticesWithRadius[j];
            vertex.radius = averageRadius;  // Mettre à jour le radius avec la moyenne calculée
            for (auto& vertex : verticesWithRadius) {
                vertex.radius = vertex.radius * (maxRadius - minRadius) + minRadius;  // Dénormalisation
            }

            // Reconversion en coordonnées cartésiennes
            vertex.v = polarToCartesian(vertex.radius, vertex.azimuth, vertex.elevation, centroid);  // Azimut et élévation sont fixes
        }
    }

    // Mettre à jour le maillage avec les nouveaux sommets
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        mesh.vertices[i] = verticesWithRadius[i].v;
    }
}

std::string extractMessageFromMesh(const Mesh& mesh, const Mesh& originalMesh, size_t messageLength) {
    std::string binaryMessage;
    
    // Paramètre seuil pour déterminer les déplacements
    float epsilonThreshold = 0.00001f;  // Plus petit que epsilon utilisé dans l'insertion
    
    // Lire les premiers sommets pour extraire le message
    for (size_t i = 0; i < messageLength * 8; ++i) {
        const Vertex& v = mesh.vertices[i];
        const Vertex& vOriginal = originalMesh.vertices[i];
        
        float displacement = v.x - vOriginal.x;
        if (displacement > epsilonThreshold) {
            binaryMessage += '1';
        } else if (displacement < -epsilonThreshold) {
            binaryMessage += '0';
        } else {
            std::cerr << "Erreur : Aucun déplacement détecté à l'indice " << i << "." << std::endl;
            return "";
        }
    }

    return binaryToText(binaryMessage);
}


int main() {
    // Exemple de maillage simple avec 16 sommets
    Mesh meshOriginal;

     // Charger un fichier OFF
    std::string filenameOff = "Draco.off";
    if (!loadOffMesh(filenameOff, meshOriginal)) {
        std::cerr << "Erreur lors du chargement du fichier OFF." << std::endl;
        return -1;
    }

    // Copier le maillage pour pouvoir comparer après insertion
    Mesh meshWithMessage = meshOriginal;

    // Message à insérer
    std::string message = "ok";
    
    // Insérer le message
    embedMessageInMesh(meshWithMessage, message);

    // Extraire le message du maillage (en comparant avec l'original)
    std::string extractedMessage = extractMessageFromMesh(meshWithMessage, meshOriginal, message.length());

    GLFWwindow* window = initOpenGLWindow(800, 600, "Visualisation du maillage");
    if (!window) return -1;

    // Boucle principale d'affichage
    while (!glfwWindowShouldClose(window)) {
        // Effacer l'écran
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Configurer la projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);

        // Configurer la vue
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0.0, 0.0, 5.0,  // Position de la caméra
                  0.0, 0.0, 0.0,  // Point regardé
                  0.0, 1.0, 0.0); // Axe Y orienté vers le haut

        // Dessiner les deux maillages avec des couleurs différentes
        // glColor3f(1.0, 0.0, 0.0);  // Couleur rouge pour le maillage original
        // drawMesh(meshOriginal);

        glColor3f(0.0, 1.0, 0.0);  // Couleur verte pour le maillage modifié
        drawMesh(meshWithMessage);

        // Afficher à l'écran
        glfwSwapBuffers(window);
        glfwPollEvents();  // Traiter les événements
    }

    // Fermer GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // Afficher les résultats
    if (!extractedMessage.empty()) {
        std::cout << "Message inséré : " << message << std::endl;
        std::cout << "Message extrait : " << extractedMessage << std::endl;
    } else {
        std::cerr << "Erreur lors de l'extraction du message." << std::endl;
    }

    return 0;
}
