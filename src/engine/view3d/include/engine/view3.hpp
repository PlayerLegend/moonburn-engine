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
    object() {};
};

namespace pipeline
{
class forward
{
    struct internal;
    std::unique_ptr<internal> *internal;

  public:
    void operator+=(object &&other);
    void draw();
};

} // namespace pipeline
}; // namespace engine::view3