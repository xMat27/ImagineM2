#ifndef LOADOFF_H
#define LOADOFF_H

#include <vector>
#include <string>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

struct OffVertex {
    float x, y, z;
    float nx, ny, nz;
};

struct OffFace {
    int vertexIndex1, vertexIndex2, vertexIndex3;
};

class loadOFF {
public:
    loadOFF();
    ~loadOFF();

    void loadOffFile(const std::string& filename);
    void drawOffGeometry(QOpenGLShaderProgram *program);
    void clearBuffers();

private:
    std::vector<OffVertex> vertices;
    std::vector<OffFace> faces;

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
};

#endif // LOADOFF_H
