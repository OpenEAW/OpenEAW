#pragma once

#include <khepri/math/color_rgba.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/math/vector3.hpp>

#include <cstdint>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Description of a mesh
 *
 * A mesh consists of consecutive sets of triangle-faces.
 */
struct MeshDesc final
{
    /// Integer size of indexes
    using Index = std::uint16_t;

    /// Describes a single vertex in the mesh
    struct Vertex
    {
        /// The vertex' position
        Vector3f position;

        /// The vertex' normal vector
        Vector3f normal;

        /// The vertex' tangent vector
        Vector3f tangent;

        /// The vertex' binormal vector
        Vector3f binormal;

        /// The vertex' texture coordinate
        Vector2f uv;

        /// The vertex' color
        ColorRGBA color;
    };

    /// The vertices in the mesh
    std::vector<Vertex> vertices;

    /**
     * \brief The indices of the faces in the mesh.
     *
     * Each face is defined by a consecutive triplet of indices into #vertices.
     */
    std::vector<Index> indices;
};

} // namespace khepri::renderer
