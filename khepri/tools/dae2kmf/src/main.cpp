#include <khepri/io/file.hpp>
#include <khepri/renderer/io/kmf.hpp>
#include <khepri/renderer/model_desc.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {
constexpr auto PROGRAM_NAME = "dae2kmf";

void check_required(const cxxopts::ParseResult& result, const std::vector<std::string>& required)
{
    for (const auto& r : required) {
        if (result.count(r) == 0) {
            throw std::runtime_error("missing option '" + r + "'");
        }
    }
}

struct MeshInfo
{
    /// The mesh to export
    const aiMesh* mesh;

    /// The hierarchy node it's attached to
    const aiNode* node;
};

struct SceneInfo
{
    std::vector<MeshInfo> meshes;
};

void collect_scene_info(const aiScene& scene, const aiNode& node, SceneInfo& info)
{
    const auto meshes       = gsl::span<aiMesh*>(scene.mMeshes, scene.mNumMeshes);
    const auto mesh_indices = gsl::span<unsigned int>(node.mMeshes, node.mNumMeshes);

    for (unsigned int mesh_index : mesh_indices) {
        if (mesh_index < scene.mNumMeshes) {
            info.meshes.push_back({meshes[mesh_index], &node});
        }
    }

    // Recurse
    const auto children = gsl::span<aiNode*>(node.mChildren, node.mNumChildren);
    for (const auto* child : children) {
        collect_scene_info(scene, *child, info);
    }
}

auto get_combined_transformation(const aiNode& node) noexcept
{
    aiMatrix4x4 transform{};
    for (const auto* pNode = &node; pNode != nullptr; pNode = pNode->mParent) {
        transform = pNode->mTransformation * transform;
    }
    return transform;
}

khepri::renderer::ModelDesc create_model(const aiScene& scene)
{
    std::vector<khepri::renderer::MeshDesc> meshes;

    // Walk the scene hierarchy and find all items we want to export
    SceneInfo scene_info;
    collect_scene_info(scene, *scene.mRootNode, scene_info);

    for (const auto& mesh_info : scene_info.meshes) {
        const auto& src = *mesh_info.mesh;
        if (src.mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
            throw std::runtime_error("mesh with non-triangles encountered");
        }

        // Calculate the combined transformation for this node
        const auto transform = get_combined_transformation(*mesh_info.node);

        khepri::renderer::MeshDesc mesh;

        mesh.vertices.resize(src.mNumVertices);

        const auto src_vertices = gsl::span<aiVector3D>(src.mVertices, src.mNumVertices);
        const auto src_normals  = gsl::span<aiVector3D>(src.mNormals, src.mNumVertices);

        for (std::size_t iv = 0; iv < src_vertices.size(); ++iv) {
            const auto v = transform * src_vertices[iv];

            // Ignore translation for normal
            const auto n = (aiMatrix3x3(transform) * src_normals[iv]).Normalize();

            mesh.vertices[iv] = {{v.x, v.y, v.z}, {n.x, n.y, n.z}};
        }

        const auto& src_faces = gsl::span<aiFace>(src.mFaces, src.mNumFaces);

        mesh.indices.resize(src_faces.size() * 3);
        for (unsigned int x = 0, j = 0; x < src_faces.size(); ++x, j += 3) {
            const auto& face    = src_faces[x];
            const auto& indices = gsl::span<unsigned int>(face.mIndices, face.mNumIndices);

            mesh.indices[j + 0] = indices[0];
            mesh.indices[j + 1] = indices[1];
            mesh.indices[j + 2] = indices[2];
        }

        meshes.push_back(std::move(mesh));
    }

    return {std::move(meshes)};
}

template <class Callable>
void ignore_exceptions(const Callable& callable)
{
    try {
        callable();
    } catch (...) {
    }
}
} // namespace

int main(int argc, char* argv[])
{
    try {
        using namespace std::literals;

        cxxopts::Options options(
            PROGRAM_NAME, "Converts a Collada model file (.dea) to a Khepri model file (.kmf)");
        options.positional_help("INPUT OUTPUT");

        auto adder = options.add_options();
        adder("h,help", "display this help and exit");
        adder("i,input", "Input file", cxxopts::value<std::filesystem::path>());
        adder("o,output", "Output file", cxxopts::value<std::filesystem::path>());

        options.parse_positional({"input", "output"});
        auto result = options.parse(argc, argv);
        if (result.count("help") != 0) {
            std::cout << options.help({"", "Group"}) << "\n";
            return 0;
        }

        check_required(result, {"input", "output"});
        const auto input_path  = result["input"].as<std::filesystem::path>();
        const auto output_path = result["output"].as<std::filesystem::path>();

        // Import the file
        Assimp::Importer imp;

        // assimp will try to fix the "up" definition to be UP_Y by applying a root transform.
        // We just want to import the model as is, so ignore whatever "up" direction the file
        // indicates.
        imp.SetPropertyInteger(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, 1);

        const auto* scene =
            imp.ReadFile(input_path.string(), aiProcessPreset_TargetRealtime_Quality);
        if (scene == nullptr) {
            throw std::runtime_error("failed to load file: "s + imp.GetErrorString());
        }

        // Convert the imported scene to a Khepri model
        auto model = create_model(*scene);

        // Write out the model
        khepri::io::File output(output_path, khepri::io::OpenMode::read_write);
        khepri::renderer::io::write_kmf(model, output);
    } catch (const std::exception& e) {
        ignore_exceptions([&] { std::cerr << "error: " << e.what() << '\n'; });
        return 1;
    } catch (...) {
        ignore_exceptions([&] { std::cerr << "unknown error\n"; });
        return 1;
    }
    return 0;
}