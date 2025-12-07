#pragma once

#include <engine/vec.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace engine::view3
{
struct animation
{
    float weight;
    float time;
    std::string name;
};

struct object
{
    vec::transform3 transform;
    std::string shader;
    std::string asset;
    std::vector<animation> animations;
    std::optional<std::vector<std::string>> nodes;
};

namespace pipeline
{
class forward
{
    struct internal;
    std::unique_ptr<internal> internal;

  public:
    forward(const std::string &root);
    ~forward();
    void operator+=(const object &other);
    void draw(const vec::transform3 &camera_transform,
              const vec::perspective &camera_perspective);
};

} // namespace pipeline
}; // namespace engine::view3