#include <khepri/physics/collision_mesh.hpp>

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>

namespace khepri::physics {

namespace {
// Möller-Trumbore ray/triangle intersection algorithm
double intersect_distance(const Ray& ray, const Vector3f& v0, const Vector3f& v1,
                          const Vector3f& v2)
{
    const Vector3f e1 = v1 - v0;
    const Vector3f e2 = v2 - v0;

    // Calculate determinant
    auto h   = cross(ray.direction(), e2);
    auto det = dot(e1, h);

    constexpr auto MAX_PARALLEL_DETERMINANT = 0.00001;
    if (det < MAX_PARALLEL_DETERMINANT) {
        // If the determinant is negative the triangle is backfacing.
        // If the determinant is close to 0, the ray lies on or parallel to triangle.
        return -1;
    }
    auto inv_det = 1.0 / det;

    // First barycentric coordinate (u) of intersection point
    auto s = ray.start() - v0;
    auto u = inv_det * dot(s, h);
    if (u < 0.0 || u > 1.0) {
        // Intersection is outside the triangle.
        return -1;
    }

    // Second barycentric coordinate (v) of intersection point
    auto q = cross(s, e1);
    auto v = inv_det * dot(ray.direction(), q);
    if (v < 0.0 || u + v > 1.0) {
        // Intersection is outside the triangle.
        return -1;
    }

    // Get the distance along the ray of the intersection point
    auto d = inv_det * dot(e2, q);
    if (d < 0.0F) {
        // Intersection lies behind ray starting point
        return -1;
    }

    return d;
}
} // namespace

CollisionMesh::CollisionMesh(std::vector<Vector3f> vertices, std::vector<Index> indices)
    : m_vertices(std::move(vertices)), m_indices(std::move(indices))
{
    assert(m_indices.size() % 3 == 0);
}

double CollisionMesh::intersect_distance(const Ray& ray) const
{
    // Unoptimized implementation: iterate through the triangles and find the closest one that
    // intersects
    double min_distance = std::numeric_limits<double>::max();
    bool   found        = false;

    for (std::size_t i = 0; i < m_indices.size(); i += 3) {
        const double distance =
            physics::intersect_distance(ray, m_vertices[m_indices[i + 0]],
                                        m_vertices[m_indices[i + 1]], m_vertices[m_indices[i + 2]]);

        if (distance != -1) {
            min_distance = std::min(min_distance, distance);
            found        = true;
        }
    }

    return found ? min_distance : -1.0;
}

bool CollisionMesh::intersect(const Frustum& frustum) const
{
    // Every vertex has to be inside the frustum
    return std::all_of(m_vertices.begin(), m_vertices.end(),
                       [&](auto& v) { return frustum.inside(v); });
}

} // namespace khepri::physics
