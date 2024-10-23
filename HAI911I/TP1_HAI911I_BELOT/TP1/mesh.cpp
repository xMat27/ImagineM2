#include "mesh.h"
#include <QtOpenGL>

Mesh::Mesh()
    : m_vertexBuffer(QOpenGLBuffer::VertexBuffer),
      m_indexBuffer(QOpenGLBuffer::IndexBuffer)
{

}

Mesh::~Mesh()
{
    m_vertexBuffer.destroy();
    m_indexBuffer.destroy();
    m_vao.destroy();
}

void Mesh::initialize()
{
    // Initialize VAO
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    setupMesh();
}

void Mesh::setupMesh()
{
    initializeOpenGLFunctions();
    // Cube vertices
    vertices = {
        // Positions       // Normals        // Texture Coords
        -0.5f, -0.5f, -0.5f, // Vertex 0
         0.5f, -0.5f, -0.5f, // Vertex 1
         0.5f,  0.5f, -0.5f, // Vertex 2
        -0.5f,  0.5f, -0.5f, // Vertex 3
        -0.5f, -0.5f,  0.5f, // Vertex 4
         0.5f, -0.5f,  0.5f, // Vertex 5
         0.5f,  0.5f,  0.5f, // Vertex 6
        -0.5f,  0.5f,  0.5f  // Vertex 7
    };

    // Indices for the cube
    indices = {
        0, 1, 2, 2, 3, 0,   // Face 1
        4, 5, 6, 6, 7, 4,   // Face 2
        0, 1, 5, 5, 4, 0,   // Face 3
        2, 3, 7, 7, 6, 2,   // Face 4
        0, 3, 7, 7, 4, 0,   // Face 5
        1, 2, 6, 6, 5, 1    // Face 6
    };

    // Initialize and bind the vertex buffer
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(vertices.constData(), vertices.size() * sizeof(float));

    // Initialize and bind the index buffer
    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(indices.constData(), indices.size() * sizeof(unsigned int));

    // Setup vertex attribute pointers
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    m_vertexBuffer.release();
    m_indexBuffer.release();
}

void Mesh::clearMesh()
{
    // Check if vertex buffer has been created before attempting to destroy it
    if (m_vertexBuffer.isCreated()) {
        m_vertexBuffer.destroy();  // Destroy the vertex buffer only if it exists
    }

    // Check if index buffer has been created before attempting to destroy it
    if (m_indexBuffer.isCreated()) {
        m_indexBuffer.destroy();  // Destroy the index buffer only if it exists
    }

    // Check if the VAO has been created before attempting to destroy it
    if (m_vao.isCreated()) {
        m_vao.destroy();  // Destroy the VAO only if it exists
    }

    // Clear the vertices and indices data
    vertices.clear();
    indices.clear();
}



void Mesh::render()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_vertexBuffer.bind();
    m_indexBuffer.bind();

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    m_indexBuffer.release();
    m_vertexBuffer.release();
}



