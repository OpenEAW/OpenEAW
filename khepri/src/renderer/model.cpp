#include <khepri/renderer/model_desc.hpp>

#include <algorithm>

namespace khepri::renderer {
namespace {
auto compute_bounding_sphere(const std::vector<MeshDesc>& meshes)
{
    // We center the bounding sphere at 0,0,0 in object space because that's
    // the pivot point for rotations. That means we don't need to consider the
    // rotation of the object when applying its bounding sphere.
    const Vector3 center(0, 0, 0);

    // Degenerate sphere by default
    double radius = 0.0;

    if (!meshes.empty()) {
        for (const auto& mesh : meshes) {
            for (const auto& v : mesh.vertices) {
                radius = std::max(radius, (v.position - center).length());
            }
        }
    }
    return Sphere(center, radius);
}

auto create_collision_mesh(const std::vector<MeshDesc>& meshes)
{
    std::vector<Vector3f>                      vertices;
    std::vector<physics::CollisionMesh::Index> indices;

    if (!meshes.empty()) {
        vertices.reserve(meshes[0].vertices.size());
        for (const auto& v : meshes[0].vertices) {
            vertices.push_back(v.position);
        }
        indices = meshes[0].indices;
    }

    return physics::CollisionMesh(std::move(vertices), std::move(indices));
}
} // namespace

ModelDesc::ModelDesc(std::vector<MeshDesc> meshes)
    : m_meshes(std::move(meshes))
    , m_bounding_sphere(compute_bounding_sphere(m_meshes))
    , m_collision_mesh(create_collision_mesh(m_meshes))
{}

} // namespace khepri::renderer
