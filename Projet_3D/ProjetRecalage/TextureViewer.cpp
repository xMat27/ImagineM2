#include "TextureViewer.h"
#include "TextureDockWidget.h"
#include <cfloat>
#include <QFileDialog>
#include <QGLViewer/manipulatedCameraFrame.h>
#include <math.h>
#include <QVector3D>
#include <QDebug>
#include <vector>
#include <QLabel>
#include <QVBoxLayout>

#include "CImg.h"

//#include "img.h"

#include "img.inl"
//#include "mathematics.h"

//#include "ICP.h"

using namespace std;
using namespace qglviewer;
using namespace cimg_library;



struct Mat3D {
    float m[3][3];

    QVector3D operator*(const QVector3D& v) const {
        return {
            m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2],
            m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2],
            m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]
        };
    }
};

struct Point3D {
    float x, y, z; 
    unsigned char value; 
};

TextureViewer::TextureViewer(QWidget *parent):QGLViewer(parent){
}

// void TextureViewer::recalibrateMeshToImage() {
//     if (!meshOriginal || !image3D) {
//         qDebug() << "Aucun maillage ou image 3D disponible pour le recalage.";
//         return;
//     }

//     ICP::alignMeshToImage(*meshOriginal, *image3D, 20, 0.5); // 20 itérations, pondération λ = 0.5

//     qDebug() << "Recalage terminé.";
//     update();
// }

void TextureViewer::draw(){

    drawClippingPlane();

    glEnable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_BLEND);
    
    camera()->setSceneRadius(1000);

    texture->draw(camera());

    drawMesh();

    
}

void TextureViewer::drawClippingPlane(){
    
    glEnable(GL_LIGHTING);
    
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    

    glDisable(GL_LIGHTING);
    
    GLdouble equation[4];
    glGetClipPlane(GL_CLIP_PLANE0, equation);
    
    qreal p[] = {0.,-equation[3]/equation[1], 0.};
    qreal projP[3];
    camera()->getWorldCoordinatesOf(p, projP);
    
    
    qreal norm[] = {equation[0] + p[0], equation[1]+ p[1], equation[2]+ p[2]};
    qreal normResult[3];
    camera()->getWorldCoordinatesOf(norm, normResult);
    
    Vec3Df normal(normResult[0]-projP[0], normResult[1]-projP[1], normResult[2]-projP[2]);
    Vec3Df point(projP[0], projP[1],projP[2]);
    

}

void TextureViewer::drawMesh() {
    glBegin(GL_TRIANGLES);
    for (const auto& triangle : triangles) {
        for (size_t i = 0; i < 3; ++i) {
            const qglviewer::Vec& vertex = vertices[triangle[i]];
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
    }
    glEnd();
}

void TextureViewer::init()
{

    texture = new Texture (QOpenGLContext::currentContext());
    
    // The ManipulatedFrame will be used as the clipping plane
    setManipulatedFrame(new ManipulatedFrame());
    
    // Enable plane clipping
    glEnable(GL_CLIP_PLANE0);
    
    //Set background color
    setBackgroundColor(QColor(255,255,255));
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    
    //Set blend parameters
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::vector<QVector3D> points;

    
    imageLoaded = false;
    
    cut = Vec3Df(0.,0.,0.),
    cutDirection = Vec3Df(1.,1.,1.);

    

}

void TextureViewer::clear(){
    texture->clear();
}

float TextureViewer::sampleAt(const QPoint& point) {
    return static_cast<float>(point.x() + point.y()); // À remplacer par un accès réel
}

std::vector<float> TextureViewer::getProfile(const QPoint& start, const QPoint& end, int steps) {
    std::vector<float> profile;

    float dx = (end.x() - start.x()) / static_cast<float>(steps);
    float dy = (end.y() - start.y()) / static_cast<float>(steps);

    for (int i = 0; i <= steps; ++i) {
        QPoint current(start.x() + dx * i, start.y() + dy * i);
        profile.push_back(sampleAt(current));
    }

    return profile;
}

float TextureViewer::computeDistance(const QVector3D& p1, const QVector3D& p2) {
    float dx = p2.x() - p1.x();
    float dy = p2.y() - p1.y();
    return std::sqrt(dx * dx + dy * dy);
}

float TextureViewer::computeDistanceToNormal(const QVector3D& point, const QVector3D& normal, const QVector3D& origin) {
    QVector3D vecToPoint = point - origin;
    return QVector3D::dotProduct(vecToPoint, normal.normalized());
}


void TextureViewer::updateCamera(const qglviewer::Vec & center, float radius){
    camera()->setSceneCenter(center);
    camera()->setSceneRadius(radius);
    
    camera()->showEntireScene();
}

void TextureViewer::calculateBoundingBox() {
    if (vertices.empty()) return;

    Vec minVertex = vertices[0];
    Vec maxVertex = vertices[0];

    for (const auto& vertex : vertices) {
        minVertex.x = std::min(minVertex.x, vertex.x);
        minVertex.y = std::min(minVertex.y, vertex.y);
        minVertex.z = std::min(minVertex.z, vertex.z);

        maxVertex.x = std::max(maxVertex.x, vertex.x);
        maxVertex.y = std::max(maxVertex.y, vertex.y);
        maxVertex.z = std::max(maxVertex.z, vertex.z);
    }

    boundingBoxMin = minVertex;
    boundingBoxMax = maxVertex;
}


void TextureViewer::scaleMeshToFitBoundingBox(float scaleFactor) {
    calculateBoundingBox(); // Assurez-vous que la boîte englobante est calculée

    // Calculez les dimensions de la boîte englobante
    Vec boxSize = boundingBoxMax - boundingBoxMin;

    // Calculez le facteur d'échelle pour que le maillage tienne dans une boîte de taille unitaire
    float scaleX = 1.0f / boxSize.x * scaleFactor;
    float scaleY = 1.0f / boxSize.y * scaleFactor;
    float scaleZ = 1.0f / boxSize.z * scaleFactor;

    // Appliquez le facteur d'échelle
    for (auto& vertex : vertices) {
        vertex.x = (vertex.x - boundingBoxMin.x) * scaleX;
        vertex.y = (vertex.y - boundingBoxMin.y) * scaleY;
        vertex.z = (vertex.z - boundingBoxMin.z) * scaleZ;
    }
}



void TextureViewer::open3DImage(const QString& fileName) {
    // Réinitialisation de l'objet texture et des indices de sous-domaines
    texture->clear();
    subdomain_indices.clear();
    std::vector<unsigned char> data;
    unsigned int nx, ny, nz;
    float dx, dy, dz;
    

    // Charger l'image 3D en fonction de son extension
    if (fileName.endsWith(".dim")) {
        openIMA(fileName, data, subdomain_indices, nx, ny, nz, dx, dy, dz);
    } else if (fileName.endsWith(".mhd")) {
        if (!openCImage(fileName, nx, ny, nz, dx, dy, dz, data, points)) {
            qWarning("Échec du chargement de l'image .mhd");
            return;
        }

        // Normalisation des données (conversion de float à unsigned char si nécessaire)
        std::vector<unsigned char> imageData(data.begin(), data.end());

        // Simuler les indices de sous-domaines (par exemple : 1 partout)
        subdomain_indices.resize(nx * ny * nz, 1);
    } else {
        qWarning("Format de fichier non pris en charge");
        return;
    }

    // Gérer la coloration des sous-domaines et la visibilité
    for (unsigned int i = 0; i < subdomain_indices.size(); i++) {
        int currentLabel = subdomain_indices[i];
        std::map<unsigned char, QColor>::iterator it = iColorMap.find(currentLabel);
        if (it == iColorMap.end()) {
            if (currentLabel == 0)
                iColorMap[currentLabel] = QColor(0, 0, 0);
            else
                iColorMap[currentLabel].setHsvF(0.98 * double(i) / subdomain_indices.size(), 0.8, 0.8);
        }
        iDisplayMap[currentLabel] = true;
    }

    // Construction de la texture
    texture->build(data, subdomain_indices, nx, ny, nz, dx, dy, dz, iColorMap);
    imageLoaded = true;

    // Mise à jour de la caméra
    qglviewer::Vec maxTexture(texture->getXMax(), texture->getYMax(), texture->getZMax());
    updateCamera(maxTexture / 2., maxTexture.norm());

    // Envoyer les paramètres de taille de grille à l'interface
    emit setMaxCutPlanes(texture->getWidth(), texture->getHeight(), texture->getDepth());
    emit setImageLabels();

    // Afficher les points récupérés
    qDebug() << "Nombre de points récupérés : " << points.size();
    for (const QVector3D& point : points) {
        qDebug() << "Point : (" << point.x() << ", " << point.y() << ", " << point.z() << ")";
    }
}


void TextureViewer::display2DProjection(const cimg_library::CImg<unsigned char>& projection) {
    // Convertir les données de la projection en texture OpenGL
    QImage image(projection.data(), projection.width(), projection.height(), QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image);

    // Utiliser un QLabel pour afficher la texture dans votre interface
    QLabel* label = new QLabel();
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);

    // Afficher dans une nouvelle fenêtre
    QWidget* viewer = new QWidget();
    viewer->setWindowTitle("Projection 2D");
    QVBoxLayout* layout = new QVBoxLayout(viewer);
    layout->addWidget(label);
    viewer->setLayout(layout);
    viewer->resize(800, 600);
    viewer->show();
}

bool TextureViewer::openCImage(const QString& filename, unsigned int& nx, unsigned int& ny, unsigned int& nz, 
                                float& dx, float& dy, float& dz, std::vector<unsigned char>& imageData, 
                                std::vector<QVector3D>& points) {
    try {
        if (filename.endsWith(".mhd")) {
            // Charger les métadonnées du fichier .mhd
            IMG<unsigned char, float> img;
            img.load_metaimage(filename.toStdString().c_str());

            // Récupérer les dimensions de l'image
            nx = img.img.width();
            ny = img.img.height();
            nz = img.img.depth();

            // Récupérer l'espacement des voxels
            dx = img.voxelSize[0];
            dy = img.voxelSize[1];
            dz = img.voxelSize[2];

            // Copier les données dans un vecteur
            imageData.assign(img.img.data(), img.img.data() + nx * ny * nz);

            // Récupérer les points 3D (convertir les indices de voxels en coordonnées réelles)
            points.clear();
            for (unsigned int z = 0; z < nz; ++z) {
                for (unsigned int y = 0; y < ny; ++y) {
                    for (unsigned int x = 0; x < nx; ++x) {
                        unsigned char value = img.img(x, y, z);

                        // Convertir les indices (x, y, z) en coordonnées réelles 3D et utiliser QVector3D
                        QVector3D point;
                        point.setX(x * dx);
                        point.setY(y * dy);
                        point.setZ(z * dz);

                        // Ajouter le point à la liste des points
                        points.push_back(point);
                    }
                }
            }

            return true;
        } else {
            qWarning("Fichier non supporté : %s", filename.toStdString().c_str());
            return false;
        }
    } catch (const std::exception& e) {
        qWarning("Erreur lors du chargement de l'image : %s", e.what());
        return false;
    }
}





void TextureViewer::openOffMesh(const QString &fileName) {
    std::cout << "Opening " << fileName.toStdString() << std::endl;

    // open the file
    std::ifstream myfile;
    myfile.open(fileName.toStdString());
    if (!myfile.is_open())
    {
        std::cout << fileName.toStdString() << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;

    myfile >> magic_s;

    // check if it's OFF
    if( magic_s != "OFF" )
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        exit(1);
    }

    int n_vertices , n_faces , dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    // Clear any verticies
    vertices.clear();

    // Read the verticies
    for( int v = 0 ; v < n_vertices ; ++v )
    {
        float x , y , z;
        myfile >> x >> y >> z ;
        vertices.push_back( Vec( x , y , z ) );
    }

    // Clear any triangles
    triangles.clear();

    // Read the triangles
    for( int f = 0 ; f < n_faces ; ++f )
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;
        if( n_vertices_on_face == 3 )
        {
            unsigned int _v1 , _v2 , _v3;
            myfile >> _v1 >> _v2 >> _v3;
            triangles.push_back( {_v1, _v2, _v3} );
        }
        else if( n_vertices_on_face == 4 )
        {
            unsigned int _v1 , _v2 , _v3 , _v4;

            myfile >> _v1 >> _v2 >> _v3 >> _v4;
            triangles.push_back({_v1, _v2, _v3});
            triangles.push_back({_v1, _v3, _v4});
        }
        else
        {
            std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
            myfile.close();
            exit(1);
        }
    }

    scaleMeshToFitBoundingBox(40.0);
    update();

}

QVector3D computeCentroid(const std::vector<QVector3D>& points) {
    QVector3D centroid = {0, 0, 0};
    for (const auto& point : points) {
        centroid = centroid + point;
    }
    return centroid / static_cast<float>(points.size());
}

Mat3D computeOptimalRotation(const QVector3D& centroidA, const QVector3D& centroidB, const std::vector<QVector3D>& A, const std::vector<QVector3D>& B) {
    float Sxx = 0, Sxy = 0, Sxz = 0;
    float Syx = 0, Syy = 0, Syz = 0;
    float Szx = 0, Szy = 0, Szz = 0;

    for (size_t i = 0; i < A.size(); ++i) {
        QVector3D a = A[i] - centroidA;
        QVector3D b = B[i] - centroidB;

        Sxx += a[0] * b[0];
        Sxy += a[0] * b[1];
        Sxz += a[0] * b[2];

        Syx += a[1] * b[0];
        Syy += a[1] * b[1];
        Syz += a[1] * b[2];

        Szx += a[2] * b[0];
        Szy += a[2] * b[1];
        Szz += a[2] * b[2];
    }

    Mat3D rotation = {{{Sxx, Sxy, Sxz}, {Syx, Syy, Syz}, {Szx, Szy, Szz}}};


    float norm = std::sqrt(Sxx * Sxx + Syy * Syy + Szz * Szz);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            rotation.m[i][j] /= norm;
        }
    }
    return rotation;
}

void TextureViewer::applyICP(const std::vector<QVector3D>& targetPoints, std::vector<QVector3D>& sourcePoints, int maxIterations, float tolerance) {
    
    // if (vertices.empty()) {
    //     std::cerr << "No vertices loaded. Please load a mesh before running ICP." << std::endl;
    //     return;
    // }


    // std::vector<QVector3D> sourcePoints(vertices.size());
    // for (size_t i = 0; i < vertices.size(); ++i) {
    //     sourcePoints[i] = {vertices[i][0], vertices[i][1], vertices[i][2]};
    // }

    Mat3D rotation = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    QVector3D translation = {0, 0, 0};
    for (int iter = 0; iter < maxIterations; ++iter) {

        std::vector<QVector3D> closestPoints;
        for (const auto& source : sourcePoints) {
            float minDistance = std::numeric_limits<float>::max();
            QVector3D closest;

            for (const auto& target : targetPoints) {
                float dist = computeDistance(source, target);
                if (dist < minDistance) {
                    minDistance = dist;
                    closest = target;
                }
            }
            closestPoints.push_back(closest);
        }


        QVector3D centroidSource = computeCentroid(sourcePoints);
        QVector3D centroidTarget = computeCentroid(closestPoints);
        Mat3D optimalRotation = computeOptimalRotation(centroidSource, centroidTarget, sourcePoints, closestPoints);
        QVector3D optimalTranslation = centroidTarget - (optimalRotation * centroidSource);

        for (auto& source : sourcePoints) {
            source = optimalRotation * source + optimalTranslation;
        }
        
        rotation = optimalRotation;
        translation = optimalTranslation;

        // if (optimalTranslation.normalize() < tolerance) {
        //     std::cout << "ICP converged in " << iter + 1 << " iterations." << std::endl;
        //     break;
        // }
    }


    // for (size_t i = 0; i < vertices.size(); ++i) {
    //     vertices[i] = {sourcePoints[i][0], sourcePoints[i][1], sourcePoints[i][2]};
    // }

    update();
}


void TextureViewer::loadOffMesh() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open OFF File", "", "OFF Files (*.off);;All Files (*)");
    if (!fileName.isEmpty()) {
        openOffMesh(fileName);
    }
}


void TextureViewer::recalage(){

    //applyICP(points, );

    std::cout << "Recalage"<< std::endl;
}



std::istream & operator>>(std::istream & stream, qglviewer::Vec & v)
{
    stream >>
            v.x >>
            v.y >>
            v.z;
    
    return stream;
}

void TextureViewer::selectIAll(){
    for(std::map<unsigned char, bool>::iterator it = iDisplayMap.begin() ; it != iDisplayMap.end(); ++it )
        iDisplayMap[it->first] = true;
    update();
}

void TextureViewer::discardIAll(){
    for(std::map<unsigned char, bool>::iterator it = iDisplayMap.begin() ; it != iDisplayMap.end(); ++it )
        iDisplayMap[it->first] = false;
    update();
}

void TextureViewer::setIVisibility(unsigned int i, bool visibility){
    if(iDisplayMap.find(i) != iDisplayMap.end())
        iDisplayMap[i] = visibility;
    update();
}


void TextureViewer::openIMA(const QString & fileName, std::vector<unsigned char> & data, std::vector<unsigned char> & labels,
                            unsigned int & nx , unsigned int & ny , unsigned int & nz, float & dx , float & dy , float & dz ){
    QString imaName = QString(fileName);
    
    imaName.replace(".dim", ".ima" );
    std::ifstream imaFile (imaName.toUtf8());
    if (!imaFile.is_open())
        return;
    
    std::ifstream dimFile (fileName.toUtf8());
    if (!dimFile.is_open())
        return;
    
    dimFile >> nx; dimFile >> ny; dimFile >> nz;
    
    string dummy, type;
    
    dimFile >> dummy;
    while (dummy.find("-type")==string::npos)
        dimFile >> dummy;
    
    dimFile >> type;
    
    while (dummy.find("-dx")==string::npos)
        dimFile >> dummy;
    
    dimFile >> dx;
    
    dimFile >> dummy;
    while (dummy.find("-dy")==string::npos)
        dimFile >> dummy;
    
    dimFile >> dy;
    
    dimFile >> dummy;
    while (dummy.find("-dz")==string::npos)
        dimFile >> dummy;
    
    dimFile >> dz;
    
    
    cout << "(nx,dx) = ( " << nx << " ; " << dx << " ) "<< endl;
    cout << "(ny,dy) = ( " << ny << " ; " << dy << " ) "<< endl;
    cout << "(nz,dz) = ( " << nz << " ; " << dz << " ) "<< endl;
    
    unsigned int size = nx*ny*nz;
    unsigned int sizeIn = size;
    
    if( type.find("S16")!=string::npos )
        sizeIn = size*2;
    if( type.find("FLOAT")!=string::npos )
        sizeIn = size*4;
    
    unsigned char * tempData = new unsigned char[sizeIn];
    
    imaFile.read((char*)tempData, sizeIn);
    
    data.clear();
    data.resize(size);
    
    if( type.find("S16")!=string::npos ){
        for(unsigned int i = 0, j=0 ; i < size ; i ++, j+=2){
            unsigned char value = (unsigned char)tempData[j];
            data[i] = value;
            if ( std::find(labels.begin(), labels.end(), value) == labels.end() )
                labels.push_back(value);
        }
    } else if( type.find("FLOAT")!=string::npos ){
        float * floatArray = (float*) tempData;
        
        for(unsigned int i = 0 ; i < size ; i ++){
            unsigned char value = (unsigned char)floatArray[i];
            data[i] = value;
            if ( std::find(labels.begin(), labels.end(), value) == labels.end() )
                labels.push_back(value);
        }
        delete [] floatArray;
    } else {
        for(unsigned int i = 0 ; i < size ; i ++){
            unsigned char value = (unsigned char)tempData[i];
            data[i] = value;
            if ( std::find(labels.begin(), labels.end(), value) == labels.end() )
                labels.push_back(value);
        }
    }
    
    delete [] tempData;
}

void TextureViewer::setXCut(float _x){
    texture->setXCut(_x*texture->getWidth());
    cut[0] =_x*texture->dx()*texture->getWidth();
    update();
}

void TextureViewer::setYCut(float _y){
    texture->setYCut(_y*texture->getHeight());
    cut[1] =_y*texture->dy()*texture->getHeight();
    update();
}

void TextureViewer::setZCut(float _z){
    texture->setZCut(_z*texture->getDepth());
    cut[2] =_z*texture->dz()*texture->getDepth();
    update();
}

void TextureViewer::invertXCut(){
    texture->invertXCut();
    cutDirection[0] *= -1;
    update();
}

void TextureViewer::invertYCut(){
    texture->invertYCut();
    cutDirection[1] *= -1;
    update();
}

void TextureViewer::invertZCut(){
    texture->invertZCut();
    cutDirection[2] *= -1;
    update();
}

void TextureViewer::setXCutDisplay(bool _xCutDisplay){
    texture->setXCutDisplay(_xCutDisplay);
    update();
}

void TextureViewer::setYCutDisplay(bool _yCutDisplay){
    texture->setYCutDisplay(_yCutDisplay);
    update();
}

void TextureViewer::setZCutDisplay(bool _zCutDisplay){
    texture->setZCutDisplay(_zCutDisplay);
    update();
}

void TextureViewer::recompileShaders() {
    texture->recompileShaders();
    update();
}

void TextureViewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_R :update(); break;
    default : QGLViewer::keyPressEvent(e);
}
}

QString TextureViewer::helpString() const
{
    QString text("<h2>S i m p l e V i e w e r</h2>");
    text += "Use the mouse to move the camera around the object. ";
    text += "You can respectively revolve around, zoom and translate with the three mouse buttons. ";
    text += "Left and middle buttons pressed together rotate around the camera view direction axis<br><br>";
    text += "Pressing <b>Alt</b> and one of the function keys (<b>F1</b>..<b>F12</b>) defines a camera keyFrame. ";
    text += "Simply press the function key again to restore it. Several keyFrames define a ";
    text += "camera path. Paths are saved when you quit the application and restored at next start.<br><br>";
    text += "Press <b>F</b> to display the frame rate, <b>A</b> for the world axis, ";
    text += "<b>Alt+Return</b> for full screen mode and <b>Control+S</b> to save a snapshot. ";
    text += "See the <b>Keyboard</b> tab in this window for a complete shortcut list.<br><br>";
    text += "Double clicks automates single click actions: A left button double click aligns the closer axis with the camera (if close enough). ";
    text += "A middle button double click fits the zoom of the camera and the right button re-centers the scene.<br><br>";
    text += "A left button double click while holding right button pressed defines the camera <i>Revolve Around Point</i>. ";
    text += "See the <b>Mouse</b> tab and the documentation web pages for details.<br><br>";
    text += "Press <b>Escape</b> to exit the TextureViewer.";
    return text;
}
