#pragma once

#include "material_desc.hpp"

#include <khepri/math/color_rgba.hpp>
#include <khepri/math/vector3.hpp>
#include <khepri/math/vector4.hpp>

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace openglyph::renderer {

/**
 * A description of a Glyph model
 */
class Model
{
public:
    /// Type of vertex indices
    using Index = std::uint16_t;

    /**
     * The master vertex type.
     *
     * This contains all possible data of the model's vertices.
     * Which parts are used depends on the material.
     */
    struct Vertex
    {
        khepri::Vector3f  position; ///< Position (in object space)
        khepri::Vector3f  normal;   ///< Normal vector (in tangent space)
        khepri::Vector2f  uv[4];    ///< Texture coordinates (in tangent space)
        khepri::Vector3f  tangent;  ///< Tangent vector (in tangent space)
        khepri::Vector3f  binormal; ///< Binormal vector (in tangent space)
        khepri::ColorRGBA color;    ///< Color (in linear color space)
    };

    /**
     * Description of a material part of a mesh.
     * A mesh is split into one or more "sub-meshes", each for a specific material.
     */
    struct Material
    {
        /// Value for a material parameter
        using ParamValue =
            std::variant<std::int32_t, float, khepri::Vector3f, khepri::Vector4f, std::string>;

        /// A material parameter
        struct Param
        {
            std::string name;  ///< The parameter's name
            ParamValue  value; ///< The parameter's value
        };

        /// The material's name
        std::string name;

        /// The material's parameters
        std::vector<Param> params;

        /// The mesh's vertices for this material
        std::vector<Vertex> vertices;

        /// The mesh's vertex indices for this material
        std::vector<Index> indices;
    };

    /**
     * Description of a mesh.
     * A mesh is a single "object" in a model that is controlled a whole.
     * A mesh has various properties that determine how and when it is rendered.
     */
    struct Mesh
    {
        /**
         * @brief The meshes' name
         */
        std::string name;

        /**
         * @brief The mesh's LoD (level-of-detail) level.
         *
         * Multiple variants of a mesh with more or fewer vertices can be stored in a model. Each
         * mesh has the same name, but a different LoD level. A higher level means more detail.
         *
         * When rendering a model, a version of the mesh with lower detail can be rendered if the
         * model is further away from the camera, thus saving on data that needs to be processed.
         */
        unsigned int lod;

        /**
         * @brief The mesh's Alt (alternative) level.
         *
         * Multiple variants of a mesh in different states can be stored in a model. This is used,
         * for instance, to store "broken" versions of a mesh when the object it belongs to is at
         * low health.
         */
        unsigned int alt;

        /**
         * @brief Initial visibility of the mesh
         *
         * Meshes can become visible or invisible depending on in-game events. This denotes the
         * default visibility of the mesh. Certain meshes are always invisible (like collision
         * boxes).
         */
        bool visible;

        /**
         * @brief The materials of the mesh
         *
         * A mesh is split into one or more "sub-meshes", each for a specific material.
         */
        std::vector<Material> materials;
    };

    /**
     * The meshes in the model
     */
    std::vector<Mesh> meshes;
};

} // namespace openglyph::renderer
