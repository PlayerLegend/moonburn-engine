#pragma once
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/image.hpp>
#include <engine/json.hpp>
#include <engine/memory.hpp>
#include <engine/vec.hpp>
#include <optional>
#include <string>

namespace gltf
{
namespace exception
{
class base : public engine::exception
{
  public:
    base(const std::string &message) : engine::exception(message) {}
};
class parse_error : public base
{
  public:
    parse_error(const std::string &message) : base(message) {}
};
}; // namespace exception
class glb;
class gltf;

using offset = size_t;
class asset
{
  public:
    std::string version;
    std::string generator;
    asset(const json::object &root);
    asset();
};
class buffer
{
  public:
    std::string name;
    std::string uri;
    engine::memory::allocation contents;
};
class buffer_view
{
  public:
    std::string name;
    const class buffer &buffer;
    offset byte_offset;
    offset byte_length;
    offset byte_stride;
    offset target;
};
enum class component_type : uint16_t
{
    BYTE = 5120,
    UBYTE = 5121,
    SHORT = 5122,
    USHORT = 5123,
    UINT = 5125,
    FLOAT = 5126
};
enum class attribute_type : uint8_t
{
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4
};
class accessor_sparse_indices
{
  public:
    const class buffer_view &buffer_view;
    offset byte_offset;
    enum component_type component_type;
};
class accessor_sparse_values
{
  public:
    const class buffer_view &buffer_view;
    offset byte_offset;
};
class accessor_sparse
{
  public:
    size_t count;
    const accessor_sparse_indices &indices;
    const accessor_sparse_values &values;
};
class accessor
{
  public:
    std::string name;
    const class buffer_view &buffer_view;
    offset byte_offset;
    enum component_type component_type;
    attribute_type type;
    size_t count;
    accessor_sparse sparse;
};

class image
{
  public:
    std::string name;
    const class buffer_view *buffer_view;
    std::string mime_type;
    std::string uri;
};

class sampler
{
  public:
    enum class mag_filter : uint16_t
    {
        NEAREST = 9728,
        LINEAR = 9729
    };
    enum class min_filter : uint16_t
    {
        NEAREST = 9728,
        LINEAR = 9729,
        NEAREST_MIPMAP_NEAREST = 9984,
        LINEAR_MIPMAP_NEAREST = 9985,
        NEAREST_MIPMAP_LINEAR = 9986,
        LINEAR_MIPMAP_LINEAR = 9987
    };
    enum class wrap_mode : uint16_t
    {
        CLAMP_TO_EDGE = 33071,
        MIRRORED_REPEAT = 33648,
        REPEAT = 10497
    };
    mag_filter mag_filter;
    min_filter min_filter;
    wrap_mode wrap_s;
    wrap_mode wrap_t;
    std::string name;
};

class texture
{
  public:
    std::string name;
    const image &source;
    const class sampler sampler;
};

class texture_info
{
  public:
    const class texture &texture;
    offset tex_coord;
};

class material
{
  public:
    enum class alpha_mode : uint8_t
    {
        OPAQUE,
        MASK,
        BLEND
    };
    class pbr_metallic_roughness
    {
      public:
        vec::fvec4 base_color_factor;
        texture_info base_color_texture;
        texture_info metallic_roughness_texture;
        float metallic_factor;
        float roughness_factor;
    };
    class occlusion_texture
    {
      public:
        texture_info texture;
        float strength;
    };
    class normal_texture
    {
      public:
        texture_info texture;
        float scale;
    };

    std::string name;
    pbr_metallic_roughness pbr_metallic_roughness;
    occlusion_texture occlusion_texture;
    normal_texture normal_texture;
    texture_info emissive_texture;
    float emissive_factor[3];
    float alpha_cutoff;
    alpha_mode alpha_mode;
    bool double_sided;
};

class mesh
{
  public:
    class primitive
    {
      public:
        class target
        {
          public:
            const accessor &position;
            const accessor &normal;
            const accessor &tangent;
            const accessor &texcoord_0;
        };

        class attributes
        {
          public:
            const accessor &position;
            const accessor &normal;
            const accessor &tangent;
            const accessor &texcoord_0;
            const accessor &texcoord_1;
            const accessor &color_0;
        };
        enum class mode : uint8_t
        {
            POINTS = 0,
            LINES = 1,
            LINE_LOOP = 2,
            LINE_STRIP = 3,
            TRIANGLES = 4,
            TRIANGLE_STRIP = 5,
            TRIANGLE_FAN = 6
        };

        attributes attributes;
        const accessor &indices;
        mode mode;
        std::vector<target> targets;
    };
    std::string name;
    std::vector<primitive> primitives;
};
class skin;
class node
{
  public:
    std::string name;
    const class skin *skin;
    vec::transform3 transform;
    std::vector<const node *> children;
    const class mesh *mesh;
};
class skin
{
  public:
    std::string name;
    const accessor &inverse_bind_matrices;
    const node *skeleton;
    std::vector<const node *> joints;
};
class gltf
{
  public:
    class asset asset;
    std::vector<buffer> buffers;
    std::vector<buffer_view> buffer_views;
    std::vector<accessor> accessors;
    std::vector<image> images;
    std::vector<sampler> samplers;
    std::vector<texture> textures;
    std::vector<material> materials;
    std::vector<mesh> meshes;
    std::vector<node> nodes;
    std::vector<skin> skins;
    gltf(std::string path);
};
} // namespace gltf