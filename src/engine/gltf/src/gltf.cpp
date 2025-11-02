#include <cmath>
#include <engine/gltf.hpp>
#include <iostream>

namespace gltf
{
struct glb_header
{
  public:
    uint32_t magic;
    uint32_t version;
    uint32_t length;
};
struct glb_chunk
{
  public:
    uint32_t length;
    uint32_t type;
    const uint8_t data[];
};
struct glb_toc
{
    const glb_header *header;
    const glb_chunk *json;
    const glb_chunk *bin;
};

} // namespace gltf

#define GLB_MAGIC 0x46546C67
#define GLB_CHUNKTYPE_JSON 0x4E4F534A
#define GLB_CHUNKTYPE_BIN 0x004E4942

static struct gltf::glb_toc parse_toc(const engine::memory::const_view input)
{
    struct gltf::glb_toc toc;
    toc.header = (const gltf::glb_header *)&input.begin[0];

    if ((toc.header + 1) > (const gltf::glb_header *)&input.end[0])
        throw gltf::exception::parse_error("GLB too small for header");

    if (toc.header->magic != GLB_MAGIC)
        throw gltf::exception::parse_error("Invalid magic in GLB");

    toc.json = (const gltf::glb_chunk *)(toc.header + 1);

    if ((toc.json + 1) > (const gltf::glb_chunk *)&input.end[0])
        throw gltf::exception::parse_error("GLB too small for JSON chunk");

    if (toc.json->type != GLB_CHUNKTYPE_JSON)
        throw gltf::exception::parse_error("Invalid chunk type for GLB JSON");

    toc.bin = (const gltf::glb_chunk *)((const uint8_t *)(toc.json + 1) +
                                        toc.json->length);

    if (toc.bin > (const gltf::glb_chunk *)&input.end)
        throw gltf::exception::parse_error(
            "GLB JSON chunk size is out of bounds");

    if ((toc.bin + 1) > (const gltf::glb_chunk *)&input.end[0])
        throw gltf::exception::parse_error("GLB BIN header is out of bounds");

    if (toc.bin->type != GLB_CHUNKTYPE_BIN)
        throw gltf::exception::parse_error("Invalid chunk type for GLB BIN");

    if ((const uint8_t *)(toc.bin + 1) + toc.bin->length >
        (const uint8_t *)&input.end[0])
        throw gltf::exception::parse_error("GLB BIN data is out of bounds");

    return toc;
}

static class gltf::glb parse_glb(const engine::memory::const_view input)
{
    struct gltf::glb_toc toc = parse_toc(input);

    return gltf::glb(
        engine::memory::const_view{toc.json->data, toc.json->length},
        engine::memory::const_view{toc.bin->data, toc.bin->length});
}

gltf::asset::asset(const json::object &root)
{
    version = root.at("version");
    json::object::const_iterator generator_it = root.find("generator");
    if (generator_it != root.end())
        generator = generator_it->second;

    std::cout << "GLTF Asset version: " << version << "\n";
    std::cout << "GLTF Asset generator: " << generator << "\n";
}

gltf::asset::asset() : version(""), generator("") {}

static std::string get_string(const json::object &root,
                              const std::string &key,
                              const std::string &default_value)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return (std::string)it->second;
    return default_value;
}

static std::string get_string(const json::object &root, const std::string &key)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return (std::string)it->second;
    return "";
}

static bool
get_bool(const json::object &root, const std::string &key, bool default_value)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return it->second.as_bool();
    return default_value;
}

static gltf::offset get_offset(const json::object &root,
                               const std::string &key,
                               gltf::offset default_value)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return (gltf::offset)it->second.strict_int();
    return default_value;
}

static float
get_float(const json::object &root, const std::string &key, float default_value)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return (float)it->second.as_float();
    return default_value;
}

static vec::fvec4 array_to_fvec4(const json::array &array)
{
    if (array.size() != 4)
        throw gltf::exception::parse_error("Not a vec4");

    return vec::fvec4(array[0].as_float(),
                      array[1].as_float(),
                      array[2].as_float(),
                      array[3].as_float());
}

static vec::fvec4 get_fvec4(const json::object &root,
                            const std::string &key,
                            const vec::fvec4 &default_value)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return array_to_fvec4(it->second);
    return default_value;
}

static vec::fvec3 array_to_fvec3(const json::array &array)
{
    if (array.size() != 3)
        throw gltf::exception::parse_error("Not a vec3");

    return vec::fvec3(array[0].as_float(),
                      array[1].as_float(),
                      array[2].as_float());
}

static vec::fvec3 get_fvec3(const json::object &root,
                            const std::string &key,
                            const vec::fvec3 &default_value)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return array_to_fvec3(it->second);
    return default_value;
}

gltf::buffer::buffer(const json::object &root, const glb &glb)
    : name(get_string(root, "name")), uri(get_string(root, "uri"))
{
    if (uri.empty())
    {
        json::object::const_iterator byte_length_it = root.find("byteLength");
        if (byte_length_it == root.end())
            throw ::gltf::exception::parse_error(
                "Buffer missing byteLength and uri");

        const ::gltf::offset byte_length = byte_length_it->second.strict_int();
        if (byte_length > glb.bin.size())
            throw ::gltf::exception::parse_error(
                "Buffer byteLength exceeds GLB BIN chunk size");

        contents = engine::memory::allocation(glb.bin.begin,
                                              glb.bin.begin + byte_length);
    }
}

gltf::buffer_view::buffer_view(const json::object &root, const gltf &gltf)
    : buffer(gltf.get_buffer(root.at("buffer").strict_int())),
      byte_offset(get_offset(root, "byteOffset", 0)),
      byte_length(root.at("byteLength").strict_int()),
      byte_stride(get_offset(root, "byteStride", 0))
{
    json::object::const_iterator it = root.find("target");
    if (it != root.end())
        target = (enum buffer_view_target)it->second.strict_int();
    else
        target = buffer_view_target::UNSET;
}

static gltf::attribute_type parse_attribute_type(const std::string &name)
{
    if (name == "SCALAR")
        return gltf::attribute_type::SCALAR;
    if (name == "VEC2")
        return gltf::attribute_type::VEC2;
    if (name == "VEC3")
        return gltf::attribute_type::VEC3;
    if (name == "VEC4")
        return gltf::attribute_type::VEC4;
    if (name == "MAT2")
        return gltf::attribute_type::MAT2;
    if (name == "MAT3")
        return gltf::attribute_type::MAT3;
    if (name == "MAT4")
        return gltf::attribute_type::MAT4;
    throw gltf::exception::parse_error("Invalid attribute type: " + name);
}

gltf::accessor_sparse_indices::accessor_sparse_indices(const json::object &root,
                                                       const gltf &gltf)
    : buffer_view(gltf.get_buffer_view(root.at("bufferView").strict_int())),
      byte_offset(get_offset(root, "byteOffset", 0)),
      component_type((enum component_type)root.at("componentType").strict_int())
{
}

gltf::accessor_sparse_values::accessor_sparse_values(const json::object &root,
                                                     const gltf &gltf)
    : buffer_view(gltf.get_buffer_view(root.at("bufferView").strict_int())),
      byte_offset(get_offset(root, "byteOffset", 0))
{
}

gltf::accessor_sparse::accessor_sparse(const json::object &root,
                                       const gltf &gltf)
    : count(root.at("count").strict_int()),
      indices(accessor_sparse_indices(root.at("indices"), gltf)),
      values(accessor_sparse_values(root.at("values"), gltf))
{
}

static gltf::mag_filter get_mag_filter(const json::object &root,
                                       const std::string &key,
                                       gltf::mag_filter default_value)
{
    json::object::const_iterator it = root.find(key);

    if (it == root.end())
        return default_value;

    gltf::mag_filter result = (gltf::mag_filter)it->second.strict_int();

    switch (result)
    {
    case gltf::mag_filter::NEAREST:
    case gltf::mag_filter::LINEAR:
        return result;
    default:
        throw gltf::exception::parse_error("Invalid mag filter value");
    }
}

static gltf::min_filter get_min_filter(const json::object &root,
                                       const std::string &key,
                                       gltf::min_filter default_value)
{
    json::object::const_iterator it = root.find(key);

    if (it == root.end())
        return default_value;

    gltf::min_filter result = (gltf::min_filter)it->second.strict_int();

    switch (result)
    {
    case gltf::min_filter::NEAREST:
    case gltf::min_filter::LINEAR:
    case gltf::min_filter::NEAREST_MIPMAP_NEAREST:
    case gltf::min_filter::LINEAR_MIPMAP_NEAREST:
    case gltf::min_filter::NEAREST_MIPMAP_LINEAR:
    case gltf::min_filter::LINEAR_MIPMAP_LINEAR:
        return result;
    default:
        throw gltf::exception::parse_error("Invalid min filter value");
    }
}

static gltf::wrap_mode get_wrap_mode(const json::object &root,
                                     const std::string &key,
                                     gltf::wrap_mode default_value)
{
    json::object::const_iterator it = root.find(key);

    if (it == root.end())
        return default_value;

    gltf::wrap_mode result = (gltf::wrap_mode)it->second.strict_int();

    switch (result)
    {
    case gltf::wrap_mode::CLAMP_TO_EDGE:
    case gltf::wrap_mode::MIRRORED_REPEAT:
    case gltf::wrap_mode::REPEAT:
        return result;
    default:
        throw gltf::exception::parse_error("Invalid wrap mode value");
    }
}

gltf::sampler::sampler(const json::object &root)
    : mag_filter(get_mag_filter(root, "magFilter", ::gltf::mag_filter::LINEAR)),
      min_filter(get_min_filter(root,
                                "minFilter",
                                ::gltf::min_filter::LINEAR_MIPMAP_LINEAR)),
      wrap_s(get_wrap_mode(root, "wrapS", ::gltf::wrap_mode::REPEAT)),
      wrap_t(get_wrap_mode(root, "wrapT", ::gltf::wrap_mode::REPEAT)),
      name(get_string(root, "name"))
{
}

gltf::image::image(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name")), mime_type(get_string(root, "mimeType")),
      uri(get_string(root, "uri"))
{
    json::object::const_iterator buffer_view_it = root.find("bufferView");
    if (buffer_view_it != root.end())
        buffer_view =
            &gltf.get_buffer_view(buffer_view_it->second.strict_int());
}

gltf::texture::texture(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name")),
      source(gltf.get_image(root.at("source").strict_int())),
      sampler(gltf.get_sampler(root.at("sampler").strict_int()))
{
}

gltf::texture_info::texture_info(const json::object &root, const gltf &gltf)
    : texture(gltf.get_texture(root.at("index").strict_int())),
      tex_coord(get_offset(root, "texCoord", 0))
{
}

static std::optional<gltf::texture_info>
get_optional_texture_info(const json::object &root,
                          const std::string &key,
                          const gltf::gltf &gltf)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return gltf::texture_info(it->second, gltf);
    return std::nullopt;
}

gltf::pbr_metallic_roughness::pbr_metallic_roughness(const json::object &root,
                                                     const gltf &gltf)
    : base_color_factor(
          get_fvec4(root, "baseColorFactor", vec::fvec4(1, 1, 1, 1))),
      base_color_texture(
          get_optional_texture_info(root, "baseColorTexture", gltf)),
      metallic_roughness_texture(
          get_optional_texture_info(root, "metallicRoughnessTexture", gltf)),
      metallic_factor(get_float(root, "metallicFactor", 1.0f)),
      roughness_factor(get_float(root, "roughnessFactor", 1.0f))
{
}

static enum gltf::material::alpha_mode parse_alpha_mode(const std::string &name)
{
    if (name == "OPAQUE")
        return gltf::material::alpha_mode::OPAQUE;
    if (name == "MASK")
        return gltf::material::alpha_mode::MASK;
    if (name == "BLEND")
        return gltf::material::alpha_mode::BLEND;
    throw gltf::exception::parse_error("Invalid alpha mode: " + name);
}

gltf::material::material(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name")),
      emissive_factor(get_fvec3(root, "emissiveFactor", vec::fvec3(0, 0, 0))),
      alpha_cutoff(get_float(root, "alphaCutoff", 0.5f)),
      alpha_mode(parse_alpha_mode(get_string(root, "alphaMode", "OPAQUE"))),
      double_sided(get_bool(root, "doubleSided", false))
{
    using pbr_metallic_roughness_t = class ::gltf::pbr_metallic_roughness;
    using occlusion_t = class ::gltf::material::occlusion_texture_info;
    using normal_t = class ::gltf::material::normal_texture_info;

    json::object::const_iterator _pbr_metallic_roughness =
        root.find("pbrMetallicRoughness");

    if (_pbr_metallic_roughness != root.end())
        pbr_metallic_roughness.emplace(
            pbr_metallic_roughness_t(_pbr_metallic_roughness->second, gltf));

    json::object::const_iterator _occlusion_texture =
        root.find("occlusionTexture");

    if (_occlusion_texture != root.end())
        occlusion_texture.emplace(
            occlusion_t(_occlusion_texture->second, gltf));

    json::object::const_iterator _normal_texture = root.find("normalTexture");
    if (_normal_texture != root.end())
        normal_texture.emplace(normal_t(_normal_texture->second, gltf));

    json::object::const_iterator _emissive_texture =
        root.find("emissiveTexture");
    if (_emissive_texture != root.end())
        emissive_texture.emplace(texture_info(_emissive_texture->second, gltf));
}

static const gltf::accessor *get_optional_accessor(const json::object &root,
                                                   const std::string &key,
                                                   const gltf::gltf &gltf)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
        return &gltf.get_accessor(it->second.strict_int());
    return nullptr;
}

gltf::mesh_primitive::target::target(const json::object &root,
                                      const gltf &gltf)
    : position(get_optional_accessor(root, "POSITION", gltf)),
      normal(get_optional_accessor(root, "NORMAL", gltf)),
      tangent(get_optional_accessor(root, "TANGENT", gltf))
{
}

gltf::mesh_primitive::attributes::attributes(const json::object &root,
                                              const gltf &gltf)
    : position(get_optional_accessor(root, "POSITION", gltf)),
      normal(get_optional_accessor(root, "NORMAL", gltf)),
      tangent(get_optional_accessor(root, "TANGENT", gltf)),
      texcoord_0(get_optional_accessor(root, "TEXCOORD_0", gltf))
{
}

static enum gltf::mesh_primitive::mode
get_mode(const json::object &root,
         const std::string &key,
         enum gltf::mesh_primitive::mode default_value)
{
    json::object::const_iterator it = root.find(key);

    if (it == root.end())
        return default_value;

    enum gltf::mesh_primitive::mode result =
        (enum gltf::mesh_primitive::mode)it->second.strict_int();

    switch (result)
    {
    case gltf::mesh_primitive::mode::POINTS:
    case gltf::mesh_primitive::mode::LINES:
    case gltf::mesh_primitive::mode::LINE_LOOP:
    case gltf::mesh_primitive::mode::LINE_STRIP:
    case gltf::mesh_primitive::mode::TRIANGLES:
    case gltf::mesh_primitive::mode::TRIANGLE_STRIP:
    case gltf::mesh_primitive::mode::TRIANGLE_FAN:
        return result;
    default:
        throw gltf::exception::parse_error("Invalid primitive mode value");
    }
}

using attributes_t = class gltf::mesh_primitive::attributes;
gltf::mesh_primitive::mesh_primitive(const json::object &root, const gltf &gltf)
    : attributes(attributes_t(root.at("attributes"), gltf)),
      mode(get_mode(root, "mode", ::gltf::mesh_primitive::mode::TRIANGLES))
{
    json::object::const_iterator indices_it = root.find("indices");

    if (indices_it != root.end())
        indices = &gltf.get_accessor(indices_it->second.strict_int());

    json::object::const_iterator targets_it = root.find("targets");

    if (targets_it != root.end())
    {
        const json::array &targets_array = targets_it->second;

        for (const json::object &target : targets_array)
            targets.push_back(::gltf::mesh_primitive::target(target, gltf));
    }
}

gltf::mesh::mesh(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name"))
{
    json::object::const_iterator primitives_it = root.find("primitives");
    if (primitives_it != root.end())
    {
        const json::array &primitives_array = primitives_it->second;

        for (const json::object &primitive : primitives_array)
            primitives.push_back(::gltf::mesh_primitive(primitive, gltf));
    }
}

::gltf::material::occlusion_texture_info::occlusion_texture_info(
    const json::object &root,
    const gltf &gltf)
    : texture_info(root, gltf), strength(get_float(root, "strength", 1.0f))
{
}

::gltf::material::normal_texture_info::normal_texture_info(
    const json::object &root,
    const gltf &gltf)
    : texture_info(root, gltf), scale(get_float(root, "scale", 1.0f))
{
}

gltf::node::node(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name")),
      transform(
          vec::transform3(get_fvec3(root, "translation", vec::fvec3(0, 0, 0)),
                          get_fvec4(root, "rotation", vec::fvec4(0, 0, 0, 1)),
                          get_fvec3(root, "scale", vec::fvec3(1, 1, 1))))
{
    json::object::const_iterator skin_it = root.find("skin");
    if (skin_it != root.end())
        skin = &gltf.get_skin(skin_it->second.strict_int());

    json::object::const_iterator mesh_it = root.find("mesh");
    if (mesh_it != root.end())
        mesh = &gltf.get_mesh(mesh_it->second.strict_int());

    json::object::const_iterator children_it = root.find("children");
    if (children_it != root.end())
    {
        const json::array &children_array = children_it->second;

        for (const json::value &child : children_array)
            children.push_back(&gltf.get_node(child.strict_int()));
    }
}

::gltf::glb::glb(engine::memory::const_view _glb)
{
    struct glb_toc toc = parse_toc(_glb);
    json = engine::memory::const_view{toc.json->data, toc.json->length};
    bin = engine::memory::const_view{toc.bin->data, toc.bin->length};
}

static size_t get_component_size(enum gltf::component_type component_type)
{
    switch (component_type)
    {
    case gltf::component_type::BYTE:
    case gltf::component_type::UBYTE:
        return 1;
    case gltf::component_type::SHORT:
    case gltf::component_type::USHORT:
        return 2;
    case gltf::component_type::UINT:
    case gltf::component_type::FLOAT:
        return 4;
    default:
        throw gltf::exception::parse_error(
            "Invalid component type: " +
            std::to_string(static_cast<uint16_t>(component_type)));
    }
}

::gltf::accessor::accessor(const json::object &root, const gltf &gltf)
    : buffer_view(gltf.get_buffer_view(root.at("bufferView").strict_int())),
      byte_offset(get_offset(root, "byteOffset", 0)),
      component_type(
          (enum component_type)root.at("componentType").strict_int()),
      type(parse_attribute_type(get_string(root, "type"))),
      count(root.at("count").strict_int()),
      normalized(get_bool(root, "normalized", false)),
      component_size(get_component_size(component_type)),
      attribute_size(component_size * (size_t)type),
      stride(buffer_view.byte_stride ? buffer_view.byte_stride : attribute_size)
{
    switch (component_type)
    {
    case component_type::BYTE:
    case component_type::UBYTE:
    case component_type::SHORT:
    case component_type::USHORT:
    case component_type::UINT:
    case component_type::FLOAT:
        break;
    default:
        throw ::gltf::exception::parse_error(
            "Invalid component type: " +
            std::to_string(static_cast<uint16_t>(component_type)));
    }

    json::object::const_iterator sparse_it = root.find("sparse");
    if (sparse_it != root.end())
        sparse = std::unique_ptr<accessor_sparse>(
            new accessor_sparse(sparse_it->second, gltf));
}

::gltf::scene::scene(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name"))
{
    json::object::const_iterator nodes_it = root.find("nodes");
    if (nodes_it != root.end())
    {
        const json::array &nodes_array = nodes_it->second;

        for (const json::value &node : nodes_array)
            nodes.push_back(&gltf.get_node(node.strict_int()));
    }
}

static const json::array *get_optional_array(const json::object &root,
                                             const std::string &key)
{
    json::object::const_iterator it = root.find(key);
    if (it != root.end())
    {
        const json::array &array = it->second;
        return &array;
    }
    return nullptr;
}

enum gltf::animation_sampler_interpolation
parse_animation_sampler_interpolation(const std::string &name)
{
    if (name == "LINEAR")
        return gltf::animation_sampler_interpolation::LINEAR;
    if (name == "STEP")
        return gltf::animation_sampler_interpolation::STEP;
    if (name == "CUBICSPLINE")
        return gltf::animation_sampler_interpolation::CUBICSPLINE;
    throw gltf::exception::parse_error(
        "Invalid animation sampler interpolation: " + name);
}

::gltf::animation_sampler::animation_sampler(const json::object &root,
                                             const gltf &gltf)
    : input(gltf.get_accessor(root.at("input").strict_int())),
      output(gltf.get_accessor(root.at("output").strict_int())),
      interpolation(parse_animation_sampler_interpolation(
          get_string(root, "interpolation", "LINEAR")))
{
}

enum gltf::animation_channel_path
parse_animation_channel_path(const std::string &name)
{
    if (name == "translation")
        return gltf::animation_channel_path::TRANSLATION;
    if (name == "rotation")
        return gltf::animation_channel_path::ROTATION;
    if (name == "scale")
        return gltf::animation_channel_path::SCALE;
    throw gltf::exception::parse_error("Invalid animation channel path: " +
                                       name);
}

::gltf::animation_channel_target::animation_channel_target(
    const json::object &root,
    const gltf &gltf)
    : path(parse_animation_channel_path(get_string(root, "path")))
{
    json::object::const_iterator node_it = root.find("node");
    if (node_it != root.end())
        node = &gltf.get_node(node_it->second.strict_int());
}

::gltf::animation_channel::animation_channel(
    const json::object &root,
    const gltf &gltf,
    const std::vector<animation_sampler> &samplers)
    : target(animation_channel_target(root.at("target"), gltf)),
      sampler(samplers.at(root.at("sampler").strict_int())),
      name(get_string(root, "name"))
{
}

::gltf::animation::animation(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name"))
{
    json::object::const_iterator samplers_it = root.find("samplers");
    if (samplers_it != root.end())
    {
        const json::array &samplers_array = samplers_it->second;

        samplers.reserve(samplers_array.size());

        for (const json::object &sampler : samplers_array)
            samplers.push_back(::gltf::animation_sampler(sampler, gltf));
    }

    json::object::const_iterator channels_it = root.find("channels");
    if (channels_it != root.end())
    {
        const json::array &channels_array = channels_it->second;

        channels.reserve(channels_array.size());

        for (const json::object &channel : channels_array)
            channels.push_back(
                ::gltf::animation_channel(channel, gltf, samplers));
    }
}

::gltf::skin::skin(const json::object &root, const gltf &gltf)
    : name(get_string(root, "name"))
{
    json::object::const_iterator inverse_bind_matrices_it =
        root.find("inverseBindMatrices");
    if (inverse_bind_matrices_it != root.end())
        inverse_bind_matrices =
            &gltf.get_accessor(inverse_bind_matrices_it->second.strict_int());

    json::object::const_iterator skeleton_it = root.find("skeleton");
    if (skeleton_it != root.end())
        skeleton = &gltf.get_node(skeleton_it->second.strict_int());

    const json::array &joints_array = root.at("joints");
    for (const json::value &joint : joints_array)
        joints.push_back(&gltf.get_node(joint.strict_int()));
}

::gltf::gltf::gltf(const std::string &_path,
                   ::filesystem::cache_binary &fs_bin,
                   ::image::rgba32_cache &fs_img)
{
    const filesystem::cache_binary::reference glb_ref = fs_bin[_path];
    const filesystem::allocation &glb_alloc = *glb_ref;
    glb glb(parse_glb(glb_alloc));
    json::object root(json::parse_memory(_path, glb.json));
    this->asset = ::gltf::asset(root.at("asset"));

    const json::array *_buffers = get_optional_array(root, "buffers");
    const json::array *_buffer_views = get_optional_array(root, "bufferViews");
    const json::array *_accessors = get_optional_array(root, "accessors");
    const json::array *_images = get_optional_array(root, "images");
    const json::array *_samplers = get_optional_array(root, "samplers");
    const json::array *_textures = get_optional_array(root, "textures");
    const json::array *_materials = get_optional_array(root, "materials");
    const json::array *_meshes = get_optional_array(root, "meshes");
    const json::array *_nodes = get_optional_array(root, "nodes");
    const json::array *_skins = get_optional_array(root, "skins");
    const json::array *_scenes = get_optional_array(root, "scenes");

    if (_buffers)
        buffers.reserve(_buffers->size());
    if (_buffer_views)
        buffer_views.reserve(_buffer_views->size());
    if (_accessors)
        accessors.reserve(_accessors->size());
    if (_images)
        images.reserve(_images->size());
    if (_samplers)
        samplers.reserve(_samplers->size());
    if (_textures)
        textures.reserve(_textures->size());
    if (_materials)
        materials.reserve(_materials->size());
    if (_meshes)
        meshes.reserve(_meshes->size());
    if (_nodes)
        nodes.reserve(_nodes->size());
    if (_skins)
        skins.reserve(_skins->size());
    if (_scenes)
        scenes.reserve(_scenes->size());

    if (_buffers)
        for (const json::object &buffer : *_buffers)
            buffers.push_back(::gltf::buffer(buffer, glb));

    if (_buffer_views)
        for (const json::object &buffer_view : *_buffer_views)
            buffer_views.push_back(::gltf::buffer_view(buffer_view, *this));

    if (_accessors)
        for (const json::object &accessor : *_accessors)
            accessors.push_back(::gltf::accessor(accessor, *this));

    if (_images)
        for (const json::object &image : *_images)
            images.push_back(::gltf::image(image, *this));

    if (_samplers)
        for (const json::object &sampler : *_samplers)
            samplers.push_back(::gltf::sampler(sampler));

    if (_textures)
        for (const json::object &texture : *_textures)
            textures.push_back(::gltf::texture(texture, *this));

    if (_materials)
        for (const json::object &material : *_materials)
            materials.push_back(::gltf::material(material, *this));

    if (_meshes)
        for (const json::object &mesh : *_meshes)
            meshes.push_back(::gltf::mesh(mesh, *this));

    if (_nodes)
        for (const json::object &node : *_nodes)
            nodes.push_back(::gltf::node(node, *this));

    if (_scenes)
        for (const json::object &scene : *_scenes)
            scenes.push_back(::gltf::scene(scene, *this));
}

float gltf::accessor::get_component_as_float(size_t attribute_index,
                                             size_t component_index) const
{
    const uint8_t *ptr = buffer_view.buffer.contents.data() +
                         get_byte_offset(attribute_index, component_index);

    if (component_type == component_type::FLOAT)
        return *reinterpret_cast<const float *>(ptr);

    if (!normalized)
        throw exception::parse_error(
            "Attempted to read non-normalized component as float");

    switch (component_type)
    {
    case component_type::BYTE:
        return std::fmax(
            static_cast<float>(*reinterpret_cast<const int8_t *>(ptr)) / 127.0f,
            -1.0f);
    case component_type::UBYTE:
        return static_cast<float>(*reinterpret_cast<const uint8_t *>(ptr)) /
               255.0f;
    case component_type::SHORT:
        return std::fmax(
            static_cast<float>(*reinterpret_cast<const int16_t *>(ptr)) /
                32767.0f,
            -1.0f);
    case component_type::USHORT:
        return static_cast<float>(*reinterpret_cast<const uint16_t *>(ptr)) /
               65535.0f;
    default:
        throw exception::parse_error(
            "Invalid component type for normalized conversion: " +
            std::to_string(static_cast<uint16_t>(component_type)));
    }
}

uint32_t gltf::accessor::get_component_as_index(size_t attribute_index,
                                                size_t component_index) const
{
    if (normalized)
        throw exception::parse_error(
            "Attempted to read normalized component as an index");

    const uint8_t *ptr = buffer_view.buffer.contents.data() +
                         get_byte_offset(attribute_index, component_index);

    switch (component_type)
    {
    case component_type::UBYTE:
        return *reinterpret_cast<const uint8_t *>(ptr);
    case component_type::USHORT:
        return *reinterpret_cast<const uint16_t *>(ptr);
    case component_type::UINT:
        return *reinterpret_cast<const uint32_t *>(ptr);
    default:
        throw exception::parse_error(
            "Invalid component type for index conversion: " +
            std::to_string(static_cast<uint16_t>(component_type)));
    }
}

static int16_t i16_from_float(float value)
{
    return static_cast<int16_t>(std::round(value * 32767.0f));
}

static uint16_t u16_from_float(float value)
{
    return static_cast<uint16_t>(std::round(value * 65535.0f));
}

static uint8_t u8_from_float(float value)
{
    return static_cast<uint8_t>(std::round(value * 255.0f));
}

::gltf::accessor::operator std::vector<vec::fvec3>() const
{
    if (type != attribute_type::VEC3)
        throw exception::parse_error("Accessor type is not VEC3, cannot "
                                     "convert to std::vector<vec::fvec3>");

    std::vector<vec::fvec3> result;
    result.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        result.push_back(vec::fvec3(get_component_as_float(i, 0),
                                    get_component_as_float(i, 1),
                                    get_component_as_float(i, 2)));
    }

    return result;
}

::gltf::accessor::operator std::vector<uint32_t>() const
{
    if (type != attribute_type::SCALAR)
        throw exception::parse_error("Accessor type is not SCALAR, cannot "
                                     "convert to std::vector<uint32_t>");

    std::vector<uint32_t> result;
    result.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        result.push_back(get_component_as_index(i, 0));
    }

    return result;
}

::gltf::accessor::operator std::vector<vec::i16vec2>() const
{
    if (type != attribute_type::VEC2)
        throw exception::parse_error("Accessor type is not VEC2, cannot "
                                     "convert to std::vector<vec::i16vec2>");

    std::vector<vec::i16vec2> result;

    result.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        result.push_back(
            vec::i16vec2(i16_from_float(get_component_as_float(i, 0)),
                         i16_from_float(get_component_as_float(i, 1))));
    }
    return result;
}

::gltf::accessor::operator std::vector<vec::i16vec4>() const
{
    if (type != attribute_type::VEC4)
        throw exception::parse_error("Accessor type is not VEC4, cannot "
                                     "convert to std::vector<vec::i16vec4>");

    std::vector<vec::i16vec4> result;

    result.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        result.push_back(
            vec::i16vec4(i16_from_float(get_component_as_float(i, 0)),
                         i16_from_float(get_component_as_float(i, 1)),
                         i16_from_float(get_component_as_float(i, 2)),
                         i16_from_float(get_component_as_float(i, 3))));
    }
    return result;
}

::gltf::accessor::operator std::vector<vec::u16vec2>() const
{
    if (type != attribute_type::VEC2)
        throw exception::parse_error("Accessor type is not VEC2, cannot "
                                     "convert to std::vector<vec::u16vec2>");

    std::vector<vec::u16vec2> result;

    result.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        result.push_back(
            vec::u16vec2(u16_from_float(get_component_as_float(i, 0)),
                         u16_from_float(get_component_as_float(i, 1))));
    }
    return result;
}

::gltf::accessor::operator std::vector<vec::u8vec4>() const
{
    if (type != attribute_type::VEC4)
        throw exception::parse_error("Accessor type is not VEC4, cannot "
                                     "convert to std::vector<vec::u8vec4>");

    std::vector<vec::u8vec4> result;

    result.reserve(count);

    if (normalized)
    {
        for (size_t i = 0; i < count; i++)
        {
            result.push_back(
                vec::u8vec4(u8_from_float(get_component_as_float(i, 0)),
                            u8_from_float(get_component_as_float(i, 1)),
                            u8_from_float(get_component_as_float(i, 2)),
                            u8_from_float(get_component_as_float(i, 3))));
        }
    }
    else
    {
        for (size_t i = 0; i < count; i++)
        {
            result.push_back(vec::u8vec4(get_component_as_index(i, 0),
                                         get_component_as_index(i, 1),
                                         get_component_as_index(i, 2),
                                         get_component_as_index(i, 3)));
        }
    }
    
    return result;
}
