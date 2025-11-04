#pragma once
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/image.hpp>
#include <engine/json.hpp>
#include <engine/memory.hpp>
#include <engine/vec.hpp>
#include <optional>
#include <string>

namespace skel
{

};

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
    engine::image::rgba32 get_image() const;
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
    float get_component_as_float(size_t attribute_index,
                                 size_t component_index) const;
    uint32_t get_component_as_index(size_t attribute_index,
                                    size_t component_index) const;

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
    size_t component_size;
    size_t attribute_size;
    size_t stride;

    size_t get_byte_offset(size_t attribute_index, size_t component_index) const
    {
        if (attribute_index >= count)
            throw exception::parse_error("Accessor index out of range: " +
                                         std::to_string(attribute_index));
        if (component_index >= (size_t)type)
            throw exception::parse_error(
                "Accessor component index out of range: " +
                std::to_string(component_index));

        return buffer_view.byte_offset + byte_offset +
               attribute_index * stride + component_index * component_size;
    }

    void dump_uint32(std::vector<uint8_t> &out) const;
    void dump_uint16(std::vector<uint8_t> &out) const;
    void dump_fvec3(std::vector<uint8_t> &out) const;
    void dump_i16vec2(std::vector<uint8_t> &out) const;
    void dump_i16vec4(std::vector<uint8_t> &out) const;
    void dump_u16vec2(std::vector<uint8_t> &out) const;
    void dump_u8vec4(std::vector<uint8_t> &out) const;
    void dump(std::vector<uint8_t> &out,
              enum component_type target_component_type,
              attribute_type target_attribute_type) const;

    operator std::vector<float>() const;
    operator std::vector<vec::fvec3>() const;
    operator std::vector<vec::fvec4>() const;
    operator std::vector<uint32_t>() const;
    operator std::vector<uint16_t>() const;
    operator std::vector<vec::i16vec2>() const;
    operator std::vector<vec::i16vec4>() const;
    operator std::vector<vec::u16vec2>() const;
    operator std::vector<vec::u8vec4>() const;
    operator std::vector<vec::fmat4>() const;
    operator std::vector<vec::cubicspline<vec::fvec3>>() const;
    operator std::vector<vec::cubicspline<vec::fvec4>>() const;
};

class image
{
  public:
    std::string name;
    const class buffer_view *buffer_view;
    std::string mime_type;
    std::string uri;
    engine::image::rgba32 contents;
    image(const json::object &root,
          const gltf &gltf,
          ::filesystem::cache_binary &cache);
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

class mesh_primitive
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
        const accessor *joints;
        const accessor *weights;
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
    mesh_primitive(const json::object &root, const gltf &gltf);
};
class mesh
{
  public:
    std::string name;
    std::vector<mesh_primitive> primitives;
    mesh(const json::object &root, const gltf &gltf);
};
class skin;
class node
{
  public:
    std::string name;
    const class skin *skin = NULL;
    vec::transform3 transform;
    std::vector<const node *> children;
    const class mesh *mesh = NULL;
    const node *parent = NULL;
    node(const json::object &root, const gltf &gltf);
    node() {};
};
class skin
{
  public:
    std::string name;
    const accessor *inverse_bind_matrices;
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

enum class animation_channel_path : uint8_t
{
    TRANSLATION,
    ROTATION,
    SCALE
};

enum class animation_sampler_interpolation : uint8_t
{
    LINEAR,
    STEP,
    CUBICSPLINE
};

class animation_sampler
{
  public:
    const accessor &input;
    const accessor &output;
    enum animation_sampler_interpolation interpolation;
    animation_sampler(const json::object &root, const gltf &gltf);
};

class animation_channel_target
{
  public:
    const class node *node = NULL;
    enum animation_channel_path path;
    animation_channel_target(const json::object &root, const gltf &gltf);
};

class animation_channel
{
  public:
    const animation_channel_target target;
    const animation_sampler &sampler;
    animation_channel(const json::object &root,
                      const gltf &gltf,
                      const std::vector<animation_sampler> &samplers);
};

class animation
{
  public:
    std::vector<animation_sampler> samplers;
    std::vector<animation_channel> channels;
    std::string name;
    const animation_sampler &get_sampler(size_t index) const
    {
        if (index >= samplers.size())
            throw ::gltf::exception::parse_error(
                "Animation sampler index out of range");
        return samplers[index];
    }
    const animation_channel &get_channel(size_t index) const
    {
        if (index >= channels.size())
            throw ::gltf::exception::parse_error(
                "Animation channel index out of range");
        return channels[index];
    }
    std::size_t get_sampler_index(const animation_sampler &sampler) const
    {
        size_t result = &sampler - samplers.data();
        if (result >= samplers.size())
            throw ::gltf::exception::parse_error(
                "Animation sampler not part of animation");
        return result;
    }
    animation(const json::object &root, const gltf &gltf);
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
    std::vector<animation> animations;

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
    const ::gltf::animation &get_animation(size_t index) const
    {
        if (index >= animations.size())
            throw ::gltf::exception::parse_error(
                "Animation index out of range");
        return animations[index];
    }

    gltf(const std::string &_path,
         ::filesystem::cache_binary &_fs_bin,
         engine::image::rgba32_cache &_fs_img);
};

class gltf_cache : public filesystem::cache<gltf,
                                            ::filesystem::cache_binary &,
                                            engine::image::rgba32_cache &>
{
    ::filesystem::cache_binary &fs_bin;
    engine::image::rgba32_cache &fs_img;

  protected:
    reference load(const std::string &path) override
    {
        return std::make_shared<gltf_cache::file_t>(path, fs_bin, fs_img);
    }

  public:
    gltf_cache(class filesystem::whitelist &wl,
               filesystem::cache_binary &_fs_bin,
               engine::image::rgba32_cache &_fs_img)
        : filesystem::cache<gltf,
                            ::filesystem::cache_binary &,
                            engine::image::rgba32_cache &>(wl),
          fs_bin(_fs_bin), fs_img(_fs_img)
    {
    }
};
}; // namespace gltf