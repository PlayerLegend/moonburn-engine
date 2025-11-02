#include <engine/gltf.hpp>
#include <filesystem>
#include <iostream>

void test_example1(const std::string &path)
{
    const std::string root = std::filesystem::path(path).parent_path().string();

    filesystem::whitelist wl(root);
    filesystem::cache_binary fs_bin(wl);
    image::rgba32_cache fs_img(wl);

    gltf::gltf_cache cache(wl, fs_bin, fs_img);

    gltf::gltf_cache::reference ref = cache[path];
    if (!ref)
    {
        std::cerr << "Failed to load glTF: " << path << "\n";
        std::exit(1);
    }
    const gltf::gltf &doc = *ref;

    std::vector<vec::fvec3> positions = doc.get_accessor(0);

    for (const vec::fvec3 &pos : positions)
    {
        std::cout << "Position: " << pos.x << ", " << pos.y << ", " << pos.z
                  << "\n";
    }

    std::vector<uint32_t> indices = doc.get_accessor(3);

    for (const uint32_t &index : indices)
    {
        std::cout << "Index: " << index << "\n";
    }

    // Asset checks
    const gltf::asset &asset = doc.get_asset();
    if (asset.version != "2.0")
    {
        std::cerr << "Unexpected asset.version: " << asset.version << "\n";
        std::exit(2);
    }
    if (asset.generator != "Khronos glTF Blender I/O v4.2.83")
    {
        std::cerr << "Unexpected asset.generator: " << asset.generator << "\n";
        std::exit(2);
    }

    // Basic structure counts / existence checks
    // accessors 0..3
    const gltf::accessor &acc0 = doc.get_accessor(0);
    const gltf::accessor &acc1 = doc.get_accessor(1);
    const gltf::accessor &acc2 = doc.get_accessor(2);
    const gltf::accessor &acc3 = doc.get_accessor(3);

    // Buffer views 0..3
    const gltf::buffer_view &bv0 = doc.get_buffer_view(0);
    const gltf::buffer_view &bv1 = doc.get_buffer_view(1);
    const gltf::buffer_view &bv2 = doc.get_buffer_view(2);
    const gltf::buffer_view &bv3 = doc.get_buffer_view(3);

    // Validate accessors match provided JSON
    if (acc0.count != 24 ||
        acc0.component_type != gltf::component_type::FLOAT ||
        acc0.type != gltf::attribute_type::VEC3 || &acc0.buffer_view != &bv0)
    {
        std::cerr << "Accessor 0 mismatch\n";
        std::exit(3);
    }
    if (acc1.count != 24 ||
        acc1.component_type != gltf::component_type::FLOAT ||
        acc1.type != gltf::attribute_type::VEC3 || &acc1.buffer_view != &bv1)
    {
        std::cerr << "Accessor 1 mismatch\n";
        std::exit(3);
    }
    if (acc2.count != 24 ||
        acc2.component_type != gltf::component_type::FLOAT ||
        acc2.type != gltf::attribute_type::VEC2 || &acc2.buffer_view != &bv2)
    {
        std::cerr << "Accessor 2 mismatch\n";
        std::exit(3);
    }
    if (acc3.count != 36 ||
        acc3.component_type != gltf::component_type::USHORT ||
        acc3.type != gltf::attribute_type::SCALAR || &acc3.buffer_view != &bv3)
    {
        std::cerr << "Accessor 3 mismatch\n";
        std::exit(3);
    }

    // Validate buffer view values (byteLength, byteOffset, target)
    if (bv0.byte_length != 288 || bv0.byte_offset != 0 ||
        bv0.target != gltf::buffer_view_target::ARRAY_BUFFER)
    {
        std::cerr << "BufferView 0 mismatch\n";
        std::exit(4);
    }
    if (bv1.byte_length != 288 || bv1.byte_offset != 288 ||
        bv1.target != gltf::buffer_view_target::ARRAY_BUFFER)
    {
        std::cerr << "BufferView 1 mismatch\n";
        std::exit(4);
    }
    if (bv2.byte_length != 192 || bv2.byte_offset != 576 ||
        bv2.target != gltf::buffer_view_target::ARRAY_BUFFER)
    {
        std::cerr << "BufferView 2 mismatch\n";
        std::exit(4);
    }
    if (bv3.byte_length != 72 || bv3.byte_offset != 768 ||
        bv3.target != gltf::buffer_view_target::ELEMENT_ARRAY_BUFFER)
    {
        std::cerr << "BufferView 3 mismatch\n";
        std::exit(4);
    }

    // Ensure buffer byte-range covers expected total (last offset + length ==
    // 840)
    const size_t final_end = bv3.byte_offset + bv3.byte_length;
    if (final_end != 840)
    {
        std::cerr << "Final buffer range end unexpected: " << final_end << "\n";
        std::exit(5);
    }

    // Mesh / primitive checks
    const gltf::mesh &mesh = doc.get_mesh(0);
    if (mesh.name != "Cube.001")
    {
        std::cerr << "Unexpected mesh.name: " << mesh.name << "\n";
        std::exit(2);
    }
    if (mesh.primitives.size() != 1)
    {
        std::cerr << "Unexpected primitive count: " << mesh.primitives.size()
                  << "\n";
        std::exit(2);
    }

    const gltf::mesh_primitive &prim = mesh.primitives[0];
    if (!prim.attributes.position || !prim.attributes.normal ||
        !prim.attributes.texcoord_0)
    {
        std::cerr << "Missing expected attribute pointers\n";
        std::exit(2);
    }
    if (!prim.indices)
    {
        std::cerr << "Missing indices accessor\n";
        std::exit(2);
    }

    // Validate attribute -> accessor mappings per JSON
    if (prim.attributes.position != &doc.get_accessor(0) ||
        prim.attributes.normal != &doc.get_accessor(1) ||
        prim.attributes.texcoord_0 != &doc.get_accessor(2) ||
        prim.indices != &doc.get_accessor(3))
    {
        std::cerr << "Primitive attribute/indices mapping mismatch\n";
        std::exit(6);
    }

    // Node checks
    const gltf::node &node0 = doc.get_node(0);
    if (node0.name != "Cube")
    {
        std::cerr << "Node 0 name unexpected: " << node0.name << "\n";
        std::exit(7);
    }
    if (node0.mesh == nullptr || node0.mesh != &doc.get_mesh(0))
    {
        std::cerr << "Node 0 mesh reference mismatch\n";
        std::exit(7);
    }

    std::cout << "glTF test_example1: OK\n";
}

// New: test_example2 validates the larger JSON structure (accessors 0..12,
// bufferViews 0..12, meshes, node 3, scene 0)
void test_example2(const std::string &path)
{
    const std::string root = std::filesystem::path(path).parent_path().string();

    filesystem::whitelist wl(root);
    filesystem::cache_binary fs_bin(wl);
    image::rgba32_cache fs_img(wl);

    gltf::gltf_cache cache(wl, fs_bin, fs_img);

    gltf::gltf_cache::reference ref = cache[path];
    if (!ref)
    {
        std::cerr << "Failed to load glTF: " << path << "\n";
        std::exit(1);
    }
    const gltf::gltf &doc = *ref;

    // Asset checks
    const gltf::asset &asset = doc.get_asset();
    if (asset.version != "2.0" ||
        asset.generator != "Khronos glTF Blender I/O v4.2.83")
    {
        std::cerr << "Unexpected asset fields\n";
        std::exit(2);
    }

    // Accessors 0..12 (sample checks for representative indices)
    const gltf::accessor &acc0 = doc.get_accessor(0);
    const gltf::accessor &acc1 = doc.get_accessor(1);
    const gltf::accessor &acc2 = doc.get_accessor(2);
    const gltf::accessor &acc3 = doc.get_accessor(3);
    const gltf::accessor &acc4 = doc.get_accessor(4);
    const gltf::accessor &acc10 = doc.get_accessor(10);
    const gltf::accessor &acc12 = doc.get_accessor(12);

    if (!(acc0.count == 24 &&
          acc0.component_type == gltf::component_type::FLOAT &&
          acc0.type == gltf::attribute_type::VEC3))
    {
        std::cerr << "Accessor 0 mismatch\n";
        std::exit(3);
    }
    if (!(acc1.count == 24 &&
          acc1.component_type == gltf::component_type::FLOAT &&
          acc1.type == gltf::attribute_type::VEC3))
    {
        std::cerr << "Accessor 1 mismatch\n";
        std::exit(3);
    }
    if (!(acc2.count == 24 &&
          acc2.component_type == gltf::component_type::FLOAT &&
          acc2.type == gltf::attribute_type::VEC2))
    {
        std::cerr << "Accessor 2 mismatch\n";
        std::exit(3);
    }
    if (!(acc3.count == 36 &&
          acc3.component_type == gltf::component_type::USHORT &&
          acc3.type == gltf::attribute_type::SCALAR))
    {
        std::cerr << "Accessor 3 (indices) mismatch\n";
        std::exit(3);
    }
    if (!(acc4.count == 24 &&
          acc4.component_type == gltf::component_type::FLOAT &&
          acc4.type == gltf::attribute_type::VEC3))
    {
        std::cerr << "Accessor 4 mismatch\n";
        std::exit(3);
    }
    if (!(acc10.count == 24 &&
          acc10.component_type == gltf::component_type::FLOAT &&
          acc10.type == gltf::attribute_type::VEC3))
    {
        std::cerr << "Accessor 10 mismatch\n";
        std::exit(3);
    }
    if (!(acc12.count == 24 &&
          acc12.component_type == gltf::component_type::FLOAT &&
          acc12.type == gltf::attribute_type::VEC2))
    {
        std::cerr << "Accessor 12 mismatch\n";
        std::exit(3);
    }

    // Buffer views 0..12 (validate a few offsets/lengths and final range)
    const gltf::buffer_view &bv0 = doc.get_buffer_view(0);
    const gltf::buffer_view &bv1 = doc.get_buffer_view(1);
    const gltf::buffer_view &bv2 = doc.get_buffer_view(2);
    const gltf::buffer_view &bv3 = doc.get_buffer_view(3);
    const gltf::buffer_view &bv4 = doc.get_buffer_view(4);
    const gltf::buffer_view &bv12 = doc.get_buffer_view(12);

    if (bv0.byte_length != 288 || bv0.byte_offset != 0 ||
        bv0.target != gltf::buffer_view_target::ARRAY_BUFFER)
    {
        std::cerr << "BufferView 0 mismatch\n";
        std::exit(4);
    }
    if (bv1.byte_length != 288 || bv1.byte_offset != 288 ||
        bv1.target != gltf::buffer_view_target::ARRAY_BUFFER)
    {
        std::cerr << "BufferView 1 mismatch\n";
        std::exit(4);
    }
    if (bv2.byte_length != 192 || bv2.byte_offset != 576 ||
        bv2.target != gltf::buffer_view_target::ARRAY_BUFFER)
    {
        std::cerr << "BufferView 2 mismatch\n";
        std::exit(4);
    }
    if (bv3.byte_length != 72 || bv3.byte_offset != 768 ||
        bv3.target != gltf::buffer_view_target::ELEMENT_ARRAY_BUFFER)
    {
        std::cerr << "BufferView 3 mismatch\n";
        std::exit(4);
    }
    if (bv4.byte_offset != 840)
    {
        std::cerr << "BufferView 4 offset unexpected: " << bv4.byte_offset
                  << "\n";
        std::exit(4);
    }

    const size_t final_end = bv12.byte_offset + bv12.byte_length;
    if (final_end != 3144)
    {
        std::cerr << "Final buffer range end unexpected: " << final_end << "\n";
        std::exit(5);
    }

    // Mesh checks (0..3)
    const gltf::mesh &m0 = doc.get_mesh(0);
    const gltf::mesh &m1 = doc.get_mesh(1);
    const gltf::mesh &m2 = doc.get_mesh(2);
    const gltf::mesh &m3 = doc.get_mesh(3);

    if (m0.name != "Cube.004" || m1.name != "Cube.002" ||
        m2.name != "Cube.003" || m3.name != "Cube.001")
    {
        std::cerr << "Mesh name(s) unexpected\n";
        std::exit(6);
    }

    // Node / scene checks: node 3 should be "Cube" with children [1,2] and mesh
    // 3; scene 0 contains node 3
    const gltf::node &node3 = doc.get_node(3);
    if (node3.name != "Cube")
    {
        std::cerr << "Node 3 name unexpected: " << node3.name << "\n";
        std::exit(7);
    }
    if (node3.mesh == nullptr || node3.mesh != &doc.get_mesh(3))
    {
        std::cerr << "Node 3 mesh reference mismatch\n";
        std::exit(7);
    }
    if (node3.children.size() != 2 || node3.children[0] != &doc.get_node(1) ||
        node3.children[1] != &doc.get_node(2))
    {
        std::cerr << "Node 3 children mismatch\n";
        std::exit(8);
    }

    const gltf::scene &scene0 = doc.get_scene(0);
    if (scene0.name != "Scene" || scene0.nodes.size() != 1 ||
        scene0.nodes[0] != &doc.get_node(3))
    {
        std::cerr << "Scene 0 mismatch\n";
        std::exit(9);
    }

    std::cout << "glTF test_example2: OK\n";
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "gltf_test")
                  << " <path-to-glb-1> <path-to-glb-2>\n";
        return 1;
    }

    test_example1(argv[1]);
    test_example2(argv[2]);

    return 0;
}