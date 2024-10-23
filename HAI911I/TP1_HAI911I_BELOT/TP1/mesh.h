#ifndef MESH_H
#define MESH_H
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>

class Mesh : protected QOpenGLFunctions
{
public:
    Mesh();
    ~Mesh();

    void initialize();
    void render();
    void clearMesh();

private:
    void setupMesh();


    QOpenGLBuffer m_vertexBuffer; // Vertex Buffer Object
    QOpenGLBuffer m_indexBuffer;  // Index Buffer Object
    QOpenGLVertexArrayObject m_vao; // Vertex Array Object

    QVector<float> vertices;
    QVector<unsigned int> indices;
};

#endif // MESH_H

