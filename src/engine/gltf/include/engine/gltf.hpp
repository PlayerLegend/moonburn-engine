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
    buffer(const json::object &root, const glb &glb);
};

enum class buffer_view_target : uint16_t
{
    UNSET = 0,
    ARRAY_BUFFER = 34962,
    ELEMENT_ARRAY_BUFFER = 34963
};

class buffer_view
{
  public:
    std::string name;
    const class buffer &buffer;
    offset byte_offset;
    offset byte_length;
    offset byte_stride;
    enum buffer_view_target target;
    buffer_view(const json::object &root, const gltf &gltf);
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
    SCALAR = 1,
    VEC2 = 2,
    VEC3 = 3,
    VEC4 = 4,
    MAT2 = 4,
    MAT3 = 9,
    MAT4 = 16
};

class accessor_sparse_indices
{
  public:
    const class buffer_view &buffer_view;
    offset byte_offset;
    enum component_type component_type;
    accessor_sparse_indices(const json::object &root, const gltf &gltf);
};

class accessor_sparse_values
{
  public:
    const class buffer_view &buffer_view;
    offset byte_offset;
    accessor_sparse_values(const json::object &root, const gltf &gltf);
};
class accessor_sparse
{
  public:
    size_t count;
    const accessor_sparse_indices indices;
    const accessor_sparse_values values;
    accessor_sparse(const json::object &root, const gltf &gltf);
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
    std::unique_ptr<accessor_sparse> sparse;
    accessor(const json::object &root, const gltf &gltf);
    bool normalized;

    size_t get_component_size() const
    {
        switch (component_type)
        {
        case component_type::BYTE:
        case component_type::UBYTE:
            return 1;
        case component_type::SHORT:
        case component_type::USHORT:
            return 2;
        case component_type::UINT:
        case component_type::FLOAT:
            return 4;
        default:
            throw exception::parse_error(
                "Invalid component type: " +
                std::to_string(static_cast<uint16_t>(component_type)));
        }
    }

    size_t get_attribute_size() const
    {
        return get_component_size() * static_cast<size_t>(type);
    }

    size_t get_stride() const
    {
        if (buffer_view.byte_stride != 0)
            return buffer_view.byte_stride;
        return get_attribute_size();
    }

    size_t get_byte_offset(size_t index) const
    {
        if (index >= count)
            throw exception::parse_error("Accessor index out of range: " +
                                         std::to_string(index));
        return buffer_view.byte_offset + byte_offset + index * get_stride();
    }

    class component_iterator
    {
        using component_t = union
        {
            int8_t i8;
            uint8_t u8;
            int16_t i16;
            uint16_t u16;
            uint32_t u32;
            float f32;
        };
        const component_t *position;
        size_t stride;
        const accessor &parent;

      public:
        component_iterator(const accessor &acc, size_t index)
            : position(reinterpret_cast<const component_t *>(
                  acc.buffer_view.buffer.contents.data() +
                  acc.get_byte_offset(index))),
              stride(acc.get_stride()), parent(acc)
        {
        }
        component_iterator &operator++()
        {
            position = reinterpret_cast<const component_t *>(
                reinterpret_cast<const uint8_t *>(position) + stride);
            return *this;
        }
        float operator*() const;
        operator uint32_t() const;

        operator std::vector<engine::gpu::arrays::position>() const;
        operator std::vector<engine::gpu::arrays::normal>() const;
        operator std::vector<engine::gpu::arrays::tangent>() const;
        operator std::vector<engine::gpu::arrays::texcoord>() const;
        operator std::vector<engine::gpu::arrays::color>() const;
        operator std::vector<engine::gpu::arrays::index>() const;
        // operator std::vector<engine::gpu::arrays::joints>() const; matches
        // color operator std::vector<engine::gpu::arrays::weights>() const;
        // matches color
    };
};

class image
{
  public:
    std::string name;
    const class buffer_view *buffer_view;
    std::string mime_type;
    std::string uri;
    image(const json::object &root, const gltf &gltf);
};

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

class sampler
{
  public:
    enum mag_filter mag_filter;
    enum min_filter min_filter;
    enum wrap_mode wrap_s;
    enum wrap_mode wrap_t;
    std::string name;
    sampler(const json::object &root);
};

class texture
{
  public:
    std::string name;
    const class image &source;
    const class sampler &sampler;
    texture(const json::object &root, const gltf &gltf);
};

class texture_info
{
  public:
    const class texture &texture;
    offset tex_coord;
    texture_info(const json::object &root, const gltf &gltf);
};

class pbr_metallic_roughness
{
  public:
    vec::fvec4 base_color_factor;
    std::optional<texture_info> base_color_texture;
    std::optional<texture_info> metallic_roughness_texture;
    float metallic_factor;
    float roughness_factor;
    pbr_metallic_roughness(const json::object &root, const gltf &gltf);
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
    class occlusion_texture_info : public texture_info
    {
      public:
        float strength;
        occlusion_texture_info(const json::object &root, const gltf &gltf);
    };
    class normal_texture_info : public texture_info
    {
      public:
        float scale;
        normal_texture_info(const json::object &root, const gltf &gltf);
    };

    std::string name;
    class std::optional<class pbr_metallic_roughness> pbr_metallic_roughness;
    class std::optional<class occlusion_texture_info> occlusion_texture;
    class std::optional<class normal_texture_info> normal_texture;
    class std::optional<class texture_info> emissive_texture;
    vec::fvec3 emissive_factor;
    float alpha_cutoff;
    alpha_mode alpha_mode;
    bool double_sided;

    material(const json::object &root, const gltf &gltf);
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
            const accessor *position;
            const accessor *normal;
            const accessor *tangent;
            target(const json::object &root, const gltf &gltf);
        };

        class attributes
        {
          public:
            const accessor *position;
            const accessor *normal;
            const accessor *tangent;
            const accessor *texcoord_0;
            const accessor *texcoord_1;
            const accessor *color_0;
            attributes(const json::object &root, const gltf &gltf);
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
        const accessor *indices = NULL;
        mode mode;
        std::vector<target> targets;
        primitive(const json::object &root, const gltf &gltf);
    };
    std::string name;
    std::vector<primitive> primitives;
    mesh(const json::object &root, const gltf &gltf);
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
    node(const json::object &root, const gltf &gltf);
    node();
};
class skin
{
  public:
    std::string name;
    const accessor &inverse_bind_matrices;
    const node *skeleton;
    std::vector<const node *> joints;
    skin(const json::object &root, const gltf &gltf);
};

class glb
{
  public:
    engine::memory::const_view json;
    engine::memory::const_view bin;
    glb(engine::memory::const_view _json, engine::memory::const_view _bin)
        : json(_json), bin(_bin)
    {
    }
    glb(engine::memory::const_view _glb);
};

class scene
{
  public:
    std::string name;
    std::vector<const node *> nodes;
    scene(const json::object &root, const gltf &gltf);
};

class gltf
{
    class asset asset;
    std::vector<buffer> buffers;
    std::vector<::gltf::buffer_view> buffer_views;
    std::vector<accessor> accessors;
    std::vector<image> images;
    std::vector<sampler> samplers;
    std::vector<texture> textures;
    std::vector<material> materials;
    std::vector<mesh> meshes;
    std::vector<node> nodes;
    std::vector<skin> skins;
    std::vector<scene> scenes;

  public:
    const ::gltf::asset &get_asset() const
    {
        return asset;
    }
    const ::gltf::buffer &get_buffer(size_t index) const
    {
        if (index >= buffers.size())
            throw ::gltf::exception::parse_error("Buffer index out of range");
        return buffers[index];
    }
    const ::gltf::buffer_view &get_buffer_view(size_t index) const
    {
        if (index >= buffer_views.size())
            throw ::gltf::exception::parse_error(
                "Buffer view index out of range");
        return buffer_views[index];
    }
    const ::gltf::accessor &get_accessor(size_t index) const
    {
        if (index >= accessors.size())
            throw ::gltf::exception::parse_error("Accessor index out of range");
        return accessors[index];
    }
    const class ::gltf::image &get_image(size_t index) const
    {
        if (index >= images.size())
            throw ::gltf::exception::parse_error("Image index out of range");
        return images[index];
    }
    const ::gltf::sampler &get_sampler(size_t index) const
    {
        if (index >= samplers.size())
            throw ::gltf::exception::parse_error("Sampler index out of range");
        return samplers[index];
    }
    const ::gltf::texture &get_texture(size_t index) const
    {
        if (index >= textures.size())
            throw ::gltf::exception::parse_error("Texture index out of range");
        return textures[index];
    }
    const ::gltf::material &get_material(size_t index) const
    {
        if (index >= materials.size())
            throw ::gltf::exception::parse_error("Material index out of range");
        return materials[index];
    }
    const ::gltf::mesh &get_mesh(size_t index) const
    {
        if (index >= meshes.size())
            throw ::gltf::exception::parse_error("Mesh index out of range");
        return meshes[index];
    }
    const ::gltf::node &get_node(size_t index) const
    {
        if (index >= nodes.size())
            throw ::gltf::exception::parse_error("Node index out of range");
        return nodes[index];
    }
    const ::gltf::skin &get_skin(size_t index) const
    {
        if (index >= skins.size())
            throw ::gltf::exception::parse_error("Skin index out of range");
        return skins[index];
    }
    const ::gltf::scene &get_scene(size_t index) const
    {
        if (index >= scenes.size())
            throw ::gltf::exception::parse_error("Scene index out of range");
        return scenes[index];
    }

    gltf(const std::string &_path,
         ::filesystem::cache_binary &_fs_bin,
         ::image::rgba32_cache &_fs_img);
};

class gltf_cache : public filesystem::cache<gltf,
                                            ::filesystem::cache_binary &,
                                            ::image::rgba32_cache &>
{
    ::filesystem::cache_binary &fs_bin;
    ::image::rgba32_cache &fs_img;

  protected:
    reference load(const std::string &path) override
    {
        return std::make_shared<gltf_cache::file_t>(path, fs_bin, fs_img);
    }

  public:
    gltf_cache(class filesystem::whitelist &wl,
               filesystem::cache_binary &_fs_bin,
               ::image::rgba32_cache &_fs_img)
        : filesystem::cache<gltf,
                            ::filesystem::cache_binary &,
                            ::image::rgba32_cache &>(wl),
          fs_bin(_fs_bin), fs_img(_fs_img)
    {
    }
};
}; // namespace gltf