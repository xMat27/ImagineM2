#include "loadOFF.h"

#include <fstream>
#include <sstream>
#include <QOpenGLFunctions>
#include <QDebug>

loadOFF::loadOFF()
    : arrayBuf(QOpenGLBuffer::VertexBuffer), indexBuf(QOpenGLBuffer::IndexBuffer)
{
}

loadOFF::~loadOFF()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}

void loadOFF::clearBuffers() {
    if (arrayBuf.isCreated()) {
        arrayBuf.destroy(); // Supprime le vertex buffer existant
    }
    if (indexBuf.isCreated()) {
        indexBuf.destroy(); // Supprime l'index buffer existant
    }

    vertices.clear(); // Vide le vecteur de sommets
    faces.clear();    // Vide le vecteur de faces
}

void loadOFF::loadOffFile(const std::string& filename) {
    clearBuffers(); // Libérer les anciens buffers

    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        qWarning( "Error: Unable to open file ");
        return;
    }

    // Lire l'en-tête du fichier OFF
    std::getline(file, line);
    if (line != "OFF") {
        qWarning( "Error: Not a valid OFF file" );
        return;
    }

    int numVertices, numFaces, numEdges;
    file >> numVertices >> numFaces >> numEdges;

    vertices.resize(numVertices);
    faces.clear();

    // Lire les sommets
    for (int i = 0; i < numVertices; ++i) {
        file >> vertices[i].x >> vertices[i].y >> vertices[i].z;
        vertices[i].nx = 0.0f;
        vertices[i].ny = 0.0f;
        vertices[i].nz = 0.0f;
    }

    // Lire les faces et effectuer la triangulation si nécessaire
    for (int i = 0; i < numFaces; ++i) {
        int numVerticesInFace;
        file >> numVerticesInFace;

        std::vector<int> faceIndices(numVerticesInFace);
        for (int j = 0; j < numVerticesInFace; ++j) {
            file >> faceIndices[j];
        }

        for (int j = 1; j < numVerticesInFace - 1; ++j) {
            OffFace face;
            face.vertexIndex1 = faceIndices[0];
            face.vertexIndex2 = faceIndices[j];
            face.vertexIndex3 = faceIndices[j + 1];
            faces.push_back(face);

            // Calcul des normales de la face
            QVector3D v1(vertices[face.vertexIndex1].x, vertices[face.vertexIndex1].y, vertices[face.vertexIndex1].z);
            QVector3D v2(vertices[face.vertexIndex2].x, vertices[face.vertexIndex2].y, vertices[face.vertexIndex2].z);
            QVector3D v3(vertices[face.vertexIndex3].x, vertices[face.vertexIndex3].y, vertices[face.vertexIndex3].z);

            QVector3D normal = QVector3D::crossProduct(v2 - v1, v3 - v1).normalized();

            vertices[face.vertexIndex1].nx += normal.x();
            vertices[face.vertexIndex1].ny += normal.y();
            vertices[face.vertexIndex1].nz += normal.z();

            vertices[face.vertexIndex2].nx += normal.x();
            vertices[face.vertexIndex2].ny += normal.y();
            vertices[face.vertexIndex2].nz += normal.z();

            vertices[face.vertexIndex3].nx += normal.x();
            vertices[face.vertexIndex3].ny += normal.y();
            vertices[face.vertexIndex3].nz += normal.z();
        }
    }

    // Normalisation des normales des sommets
    for (int i = 0; i < numVertices; ++i) {
        QVector3D normal(vertices[i].nx, vertices[i].ny, vertices[i].nz);
        normal.normalize();
        vertices[i].nx = normal.x();
        vertices[i].ny = normal.y();
        vertices[i].nz = normal.z();
    }

    // Allocation des nouveaux buffers OpenGL
    arrayBuf.create();
    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), vertices.size() * sizeof(OffVertex));

    indexBuf.create();
    indexBuf.bind();
    indexBuf.allocate(faces.data(), faces.size() * sizeof(OffFace));
}

void loadOFF::drawOffGeometry(QOpenGLShaderProgram *program) {
    arrayBuf.bind();
    indexBuf.bind();

    // Liaison de l'attribut position (vertex)
    int vertexLocation = program->attributeLocation("vertex");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offsetof(OffVertex, x), 3, sizeof(OffVertex));

    // Liaison de l'attribut normale
    int normalLocation = program->attributeLocation("normal");
    program->enableAttributeArray(normalLocation);
    program->setAttributeBuffer(normalLocation, GL_FLOAT, offsetof(OffVertex, nx), 3, sizeof(OffVertex));

    // Dessiner les triangles
    glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);

    program->disableAttributeArray(vertexLocation);
    program->disableAttributeArray(normalLocation);
}
