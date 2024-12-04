#ifndef TEXTUREVIEWER_H
#define TEXTUREVIEWER_H

#include <QKeyEvent>
#include <QGLViewer/qglviewer.h>

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <algorithm>
#include "Texture.h"

namespace cimg_library {
    template<typename T> class CImg; // Déclaration anticipée
}


class TextureViewer : public QGLViewer
{
    Q_OBJECT

public :
    TextureViewer(QWidget *parent = nullptr);
    
    std::vector<float> getProfile(const QPoint& start, const QPoint& end, int steps = 100);
    float sampleAt(const QPoint& point);
    float computeDistance(const QVector3D& p1, const QVector3D& p2);
    float computeDistanceToNormal(const QVector3D& point, const QVector3D& normal, const QVector3D& origin);
    void applyICP(const std::vector<QVector3D>& targetPoints, std::vector<QVector3D>& sourcePoints, int maxIterations = 10, float tolerance = 1e-6);
    void display2DProjection(const cimg_library::CImg<unsigned char>& projection);
    void open3DImage(const QString & fileName);
    bool openCImage(const QString& filename, unsigned int& nx, unsigned int& ny, unsigned int& nz, float& dx, float& dy, float& dz, std::vector<unsigned char>& imageData, std::vector<QVector3D>& points);
    void openOffMesh(const QString & fileName);
    void loadOffMesh();
    void recalage();
    void calculateBoundingBox();
    void scaleMeshToFitBoundingBox(float scaleFactor);
    void openColors(const QString & fileName);
    void selectIAll();
    void discardIAll();
    void setIVisibility(unsigned int i, bool visibility);
    const std::map<unsigned char, QColor> & getIColorMap()const {return iColorMap;}
    void getImageSubdomainIndices(std::vector<unsigned char> & _subdomain_indices){ _subdomain_indices = subdomain_indices;}

protected :

    Texture * texture;

    bool imageLoaded;

    virtual void draw();
    virtual void init();

    virtual QString helpString() const;
    virtual void keyPressEvent(QKeyEvent *e);

    void drawClippingPlane();
    void drawMesh();

    void clear();
    void updateCamera(const qglviewer::Vec & center, float radius);


    void openIMA(  const QString & filename,std::vector<unsigned char> & data, std::vector<unsigned char> & labels,
                   unsigned int & nx , unsigned int & ny , unsigned int & nz, float & dx , float & dy , float & dz );

    Vec3Df cut;
    Vec3Df cutDirection;

    std::map<unsigned char, bool> iDisplayMap;
    std::map<unsigned char, QColor> iColorMap;
    std::vector<unsigned char> subdomain_indices;

    std::vector<qglviewer::Vec> vertices;
    std::vector<std::array<size_t,3>> triangles;
    qglviewer::Vec boundingBoxMin;
    qglviewer::Vec boundingBoxMax;


public slots:

    void setXCut(float _x);
    void setYCut(float _y);
    void setZCut(float _z);

    void invertXCut();
    void invertYCut();
    void invertZCut();

    void setXCutDisplay(bool _xCutDisplay);
    void setYCutDisplay(bool _yCutDisplay);
    void setZCutDisplay(bool _zCutDisplay);

    void recompileShaders();

signals:
    void setMaxCutPlanes(int _xMax, int _yMax, int _zMax);
    void setImageLabels();

};

#endif
