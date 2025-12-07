#include "engine/gltf.hpp"
#include "engine/image.hpp"
#include <engine/exception.hpp>
#include <engine/filesystem.hpp>
#include <engine/gpu.hpp>
#include <engine/skel.hpp>
#include <engine/vec.hpp>
#include <engine/view3.hpp>
#include <unordered_map>

struct engine::view3::pipeline::forward::internal
{
    engine::gpu::skin gpu_skin;
    engine::filesystem::whitelist whitelist;
    engine::filesystem::cache_binary fs_bin;
    image::cache::rgba32 fs_image;
    gltf::gltf_cache fs_gltf;
    engine::gpu::cache::asset fs_asset;
    struct shader;
    std::unordered_map<std::string, shader> shaders;

    class pose_cache {
        size_t next;
    };

    class pose
    {
        skel::pose skel;
        std::vector<vec::fmat4> mat;
        gpu::skin gpu;

        bool is_on_gpu = false;

      public:
        void start(const skel::armature &arm)
        {
            is_on_gpu = false;
            skel.start(arm);
        }
        void accumulate(const skel::animation &anim, float time, float weight)
        {
            skel.accumulate(anim, time, weight);
        }
        skel::pose::slice finish()
        {
            return skel.append_matrices(mat);
        }
        void bind()
        {
            if (!is_on_gpu)
            {
                gpu = mat;
                mat.clear();
                skel.clear();
                gpu.bind();
                is_on_gpu = true;
            }
            gpu.bind();
        }
    };

    struct tasks
    {
        struct static_node
        {
            const gpu::asset::mesh &mesh;
            const vec::transform3 &transform;
            static_node(const gpu::asset::mesh &mesh,
                        const vec::transform3 &transform)
                : mesh(mesh), transform(transform)
            {
            }
        };

        struct pose_node
        {
            const gpu::asset::mesh &mesh;
            const vec::transform3 &transform;
            class pose &pose;
            skel::pose::slice slice;
            pose_node(const gpu::asset::mesh &_mesh,
                      const vec::transform3 &_transform,
                      class pose &_pose,
                      const skel::pose::slice &_slice)
                : mesh(_mesh), transform(_transform), pose(_pose), slice(_slice)
            {
            }
        };

        struct hold
        {
            gpu::cache::asset::reference ref;
            vec::transform3 transform;
            internal::pose pose; // need to cache gpu textures from this

            void
            add_node(const std::string &node_name,
                     const std::unordered_map<std::string, skel::pose::slice>
                         &armatures,
                     std::vector<static_node> &static_nodes,
                     std::vector<pose_node> &pose_nodes)
            {
                const gpu::asset &asset = *ref;
                const auto node_it = asset.objects.find(node_name);
                if (node_it == asset.objects.end())
                    return;
                const gpu::asset::object &asset_node = node_it->second;

                if (asset_node.skin)
                {
                    const auto skin_it = armatures.find(asset_node.skin_name);

                    if (skin_it == armatures.end())
                        static_nodes.emplace_back(asset_node.mesh, transform);
                    else
                        pose_nodes.emplace_back(asset_node.mesh,
                                                transform,
                                                pose,
                                                skin_it->second);
                }
                else
                    static_nodes.emplace_back(asset_node.mesh, transform);
            }

            hold(const struct object &obj,
                 gpu::cache::asset &cache,
                 std::vector<static_node> &static_nodes,
                 std::vector<pose_node> &pose_nodes)
                : transform(obj.transform)
            {
                ref = cache[obj.asset];

                std::unordered_map<std::string, skel::pose::slice> armatures;

                const gpu::asset &asset = *ref;

                for (const auto &[name, arm] : asset.armatures)
                {
                    pose.start(arm);

                    for (const view3::animation &in_anim : obj.animations)
                    {
                        const auto anim_it =
                            asset.animations.find(in_anim.name);

                        if (anim_it == asset.animations.end())
                            continue;

                        pose.accumulate(anim_it->second,
                                        in_anim.time,
                                        in_anim.weight);
                    }

                    armatures.emplace(name, pose.finish());
                }

                if (obj.nodes.has_value())
                    for (const std::string &in_node : obj.nodes.value())
                        add_node(in_node, armatures, static_nodes, pose_nodes);
                else
                    for (const auto &[name, node] : asset.objects)
                        add_node(name, armatures, static_nodes, pose_nodes);
            }
        };

        std::vector<hold> holds;
        std::vector<static_node> static_nodes;
        std::vector<pose_node> pose_nodes;

        void add_node(const struct object &obj, gpu::cache::asset &cache)
        {
            holds.emplace_back(obj, cache, static_nodes, pose_nodes);
        }

        void clear()
        {
            holds.clear();
            static_nodes.clear();
            pose_nodes.clear();
        }
    };

    struct shader
    {
        struct tasks tasks;

        gpu::shader::program pose_depth_prepass;
        gpu::shader::program pose_draw;
        gpu::shader::program static_depth_prepass;
        gpu::shader::program static_draw;

        shader(const gpu::shader::vertex &static_vert,
               const gpu::shader::vertex &pose_vert,
               const gpu::shader::fragment &frag)
            : pose_depth_prepass(&pose_vert, nullptr),
              pose_draw(&pose_vert, &frag),
              static_depth_prepass(&static_vert, nullptr),
              static_draw(&static_vert, &frag)
        {
        }

        shader(const std::string &name, engine::filesystem::cache_binary &fs)
            : shader(gpu::shader::vertex(fs, name + ".static.vert"),
                     gpu::shader::vertex(fs, name + ".pose.vert"),
                     gpu::shader::fragment(fs, name + ".frag"))
        {
        }
    };

    void add_object(const engine::view3::object &obj)
    {
        const auto shader_it = shaders.find(obj.shader);

        if (shader_it == shaders.end())
        {
            const auto added_it =
                shaders.emplace(obj.shader, shader(obj.shader, fs_bin));
            added_it.first->second.tasks.add_node(obj, fs_asset);
        }
        else
        {
            shader_it->second.tasks.add_node(obj, fs_asset);
        }
    }

    void load_shaders(const std::vector<std::string> &paths)
    {
        for (const std::string &path : paths)
            shaders.emplace(path, shader(path, fs_bin));
    }

    void draw_static(const vec::transform3 &camera_transform,
                     const vec::perspective &camera_perspective,
                     gpu::shader::program &program,
                     const std::vector<tasks::static_node> &nodes)
    {
        if (nodes.empty())
            return;

        const vec::transform3 *transform = nullptr;

        program.bind();
        program.set_view_perspective(camera_transform, camera_perspective);

        for (const tasks::static_node &node : nodes)
        {
            if (transform != &node.transform)
            {
                transform = &node.transform;
                program.set_model_transform(node.transform);
            }

            node.mesh.draw(program);
        }
    }

    void draw_pose(const vec::transform3 &camera_transform,
                   const vec::perspective &camera_perspective,
                   gpu::shader::program &program,
                   const std::vector<tasks::pose_node> &nodes)
    {
        if (nodes.empty())
            return;

        const vec::transform3 *transform = nullptr;
        const pose *pose = nullptr;

        program.bind();
        program.set_view_perspective(camera_transform, camera_perspective);

        for (const tasks::pose_node &node : nodes)
        {
            if (transform != &node.transform)
            {
                transform = &node.transform;
                program.set_model_transform(node.transform);
            }

            if (pose != &node.pose)
            {
                pose = &node.pose;
                node.pose.bind();
            }
            program.set_skin_slice(node.slice);

            node.mesh.draw(program);
        }
    }

    void draw(const vec::transform3 &camera_transform,
              const vec::perspective &camera_perspective)
    {
        gpu::state::forward::start_depth_pass();

        for (auto &[name, shader] : shaders)
        {
            draw_static(camera_transform,
                        camera_perspective,
                        shader.static_depth_prepass,
                        shader.tasks.static_nodes);

            draw_pose(camera_transform,
                      camera_perspective,
                      shader.pose_depth_prepass,
                      shader.tasks.pose_nodes);
        }

        gpu::state::forward::start_draw_pass();

        for (auto &[name, shader] : shaders)
        {
            draw_static(camera_transform,
                        camera_perspective,
                        shader.static_draw,
                        shader.tasks.static_nodes);

            draw_pose(camera_transform,
                      camera_perspective,
                      shader.pose_draw,
                      shader.tasks.pose_nodes);

            shader.tasks.clear();
        }
    }

    internal(const std::string &root)
        : whitelist(root), fs_bin(whitelist), fs_image(whitelist),
          fs_gltf(whitelist, fs_bin, fs_image), fs_asset(whitelist, fs_gltf)
    {
    }
};

engine::view3::pipeline::forward::forward(const std::string &root)
    : internal(std::make_unique<struct internal>(root))
{
}

engine::view3::pipeline::forward::~forward() = default;

void engine::view3::pipeline::forward::operator+=(const object &other)
{
    internal->add_object(other);
}

void engine::view3::pipeline::forward::draw(
    const vec::transform3 &camera_transform,
    const vec::perspective &camera_perspective)
{
    internal->draw(camera_transform, camera_perspective);
}