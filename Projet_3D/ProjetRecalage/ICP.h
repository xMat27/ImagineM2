#ifndef ICP_H
#define ICP_H

#include <Eigen/Dense>
#include <vector>
#include "Mesh.h"
#include "Image3D.h"

class ICP {
public:
    static void alignMeshToImage(Mesh& mesh, const Image3D& image, int iterations = 10, float lambda = 0.1);

private:
    static Eigen::Vector3f projectOntoImage(const Eigen::Vector3f& point, const Image3D& image);
    static void minimizeError(const std::vector<Eigen::Vector3f>& source, 
                              const std::vector<Eigen::Vector3f>& target, 
                              Eigen::Matrix3f& rotation, 
                              Eigen::Vector3f& translation);
};

#endif // ICP_H
