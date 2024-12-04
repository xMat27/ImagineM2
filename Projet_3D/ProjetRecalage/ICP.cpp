#include "ICP.h"
#include <Eigen/SVD>

void ICP::alignMeshToImage(Mesh& mesh, const Image3D& image, int iterations, float lambda) {
    Eigen::Matrix3f R = Eigen::Matrix3f::Identity(); 
    Eigen::Vector3f T = Eigen::Vector3f::Zero();    

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<Eigen::Vector3f> correspondences;

        
        for (const auto& vertex : mesh.getVertices()) {
            Eigen::Vector3f projected = projectOntoImage(vertex, image);
            correspondences.push_back(projected);
        }

        Eigen::Matrix3f R_new;
        Eigen::Vector3f T_new;
        minimizeError(mesh.getVertices(), correspondences, R_new, T_new);

        
        R = R_new * R;
        T = R_new * T + T_new;

        
        mesh.applyTransformation(R, T);
    }
}

Eigen::Vector3f ICP::projectOntoImage(const Eigen::Vector3f& point, const Image3D& image) {
    
    int x = std::round(point.x() / image.getVoxelSizeX());
    int y = std::round(point.y() / image.getVoxelSizeY());
    int z = std::round(point.z() / image.getVoxelSizeZ());

    
    if (!image.isValidVoxel(x, y, z)) {
        return point;
    }

    
    Eigen::Vector3f gradient = image.getGradient(x, y, z);
    float distance = image.getDistance(x, y, z);

    
    return point - distance * gradient.normalized();
}

void ICP::minimizeError(const std::vector<Eigen::Vector3f>& source, 
                        const std::vector<Eigen::Vector3f>& target, 
                        Eigen::Matrix3f& rotation, 
                        Eigen::Vector3f& translation) {
    
    Eigen::Vector3f centroidSource = Eigen::Vector3f::Zero();
    Eigen::Vector3f centroidTarget = Eigen::Vector3f::Zero();

    for (size_t i = 0; i < source.size(); ++i) {
        centroidSource += source[i];
        centroidTarget += target[i];
    }
    centroidSource /= source.size();
    centroidTarget /= target.size();

   
    Eigen::MatrixXf centeredSource(source.size(), 3);
    Eigen::MatrixXf centeredTarget(target.size(), 3);

    for (size_t i = 0; i < source.size(); ++i) {
        centeredSource.row(i) = source[i] - centroidSource;
        centeredTarget.row(i) = target[i] - centroidTarget;
    }

    
    Eigen::Matrix3f H = centeredSource.transpose() * centeredTarget;

    
    Eigen::JacobiSVD<Eigen::Matrix3f> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3f U = svd.matrixU();
    Eigen::Matrix3f V = svd.matrixV();

    
    rotation = V * U.transpose();

    
    if (rotation.determinant() < 0) {
        V.col(2) *= -1;
        rotation = V * U.transpose();
    }

    
    translation = centroidTarget - rotation * centroidSource;
}
