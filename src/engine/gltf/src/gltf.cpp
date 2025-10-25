#include <engine/gltf.hpp>
#include <variant>
#include <unordered_map>
#include <engine/json.hpp>

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

class glb
{
  public:
    engine::memory::const_view json;
    engine::memory::const_view bin;
    glb(engine::memory::const_view _json, engine::memory::const_view _bin)
        : json(_json), bin(_bin)
    {
    }
};
} // namespace gltf

#define GLB_MAGIC 0x46546C67
#define GLB_CHUNKTYPE_JSON 0x4E4F534A
#define GLB_CHUNKTYPE_BIN 0x004E4942

struct gltf::glb_toc parse(const engine::memory::const_view input)
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

class gltf::glb parse_glb(const engine::memory::const_view input)
{
    struct gltf::glb_toc toc = parse(input);

    return gltf::glb(
        engine::memory::const_view{toc.json->data, toc.json->length},
        engine::memory::const_view{toc.bin->data, toc.bin->length});
}