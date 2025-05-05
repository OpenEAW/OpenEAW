#pragma once

#include <khepri/math/frustum.hpp>
#include <khepri/math/ray.hpp>
#include <khepri/math/vector3.hpp>

#include <cstdint>
#include <vector>

namespace khepri::physics {

/**
 * \brief Collision mesh
 *
 * A collision mesh is an optimized data structure for collision detection.
 */
class CollisionMesh final
{
public:
    /// Type of vertex indices
    using Index = std::uint16_t;

    /**
     * Constructs a collision mesh from vertices and faces
     * \param[in] vertices the vertices of the collision mesh
     * \param[in] indices the indices of the triangle faces of the collision mesh
     *
     * Every consecutive set of three indices is a triangle face. Each index in \a indices must
     * be a valid index into \a vertices.
     */
    CollisionMesh(std::vector<Vector3f> vertices, std::vector<Index> indices);

    /**
     * \brief Returns the distance along the ray of the first intersection with this mesh, if
     * any.
     *
     * Returns a negative number if there is no intersection or if the origin is inside the
     * collision mesh.
     */
    [[nodiscard]] double intersect_distance(const Ray& ray) const;

    /// Checks if this collision mesh intersects, even partially, the specified frustum.
    [[nodiscard]] bool intersect(const Frustum& frustum) const;

private:
    std::vector<Vector3f> m_vertices;
    std::vector<Index>    m_indices;
};

} // namespace khepri::physics
