#version 330 core

layout(location = 0) in vec3 attribute_position;
layout(location = 1) in vec3 attribute_normal;
layout(location = 2) in vec4 attribute_tangent;
layout(location = 3) in vec2 attribute_texcoord;
layout(location = 4) in uvec4 attribute_joints;
layout(location = 5) in vec4 attribute_weights;

uniform mat4 u_mvp;
uniform mat3 u_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform vec3 u_world_viewpos;
uniform vec3 u_world_lightpos;
uniform sampler2D u_skin;
uniform int u_skin_count;

mat4 get_skin_matrix(uint index)
{
    ivec2 ts = textureSize(u_skin, 0);
    int base = int(index) * 4; // 4 texels per mat4
    vec4 c0 = texelFetch(u_skin, ivec2(base + 0, 0), 0);
    vec4 c1 = texelFetch(u_skin, ivec2(base + 1, 0), 0);
    vec4 c2 = texelFetch(u_skin, ivec2(base + 2, 0), 0);
    vec4 c3 = texelFetch(u_skin, ivec2(base + 3, 0), 0);

    return mat4(c0, c1, c2, c3);
}

out vs_out
{
    vec3 model_fragpos;
    vec2 texcoord;
    vec3 tangent_lightpos;
    vec3 tangent_viewpos;
    vec3 tangent_fragpos;
    vec3 tangent_normal;
};

mat3 tbn_matrix()
{
    vec3 tbn_t = normalize(u_normal * attribute_tangent.xyz);
    vec3 tbn_n = normalize(u_normal * attribute_normal);
    tbn_t = normalize(tbn_t - dot(tbn_t, tbn_n) * tbn_n);
    vec3 tbn_b = normalize(cross(tbn_n, tbn_t) * attribute_tangent.w);

    return mat3(tbn_t, tbn_b, tbn_n);
}

vec3 skin_position_part(int i)
{
    vec4 skin =
        get_skin_matrix(attribute_joints[i]) * vec4(attribute_position, 1.0f);

    return attribute_weights[i] * skin.xyz;
}

vec3 skin_position()
{
    if (u_skin_count > 0)
        return skin_position_part(0) + skin_position_part(1) +
               skin_position_part(2) + skin_position_part(3);
    else
        return attribute_position;
}

void main()
{
    mat3 tbn = transpose(tbn_matrix());

    vec3 position = skin_position();
    
    model_fragpos = (u_model * vec4(position, 1.0)).xyz;
    texcoord = attribute_texcoord;
    tangent_lightpos = tbn * u_world_lightpos;
    tangent_viewpos = tbn * u_world_viewpos;
    tangent_fragpos = tbn * model_fragpos;
    tangent_normal = tbn * attribute_normal;

    gl_Position = u_mvp * vec4(position, 1.0);
}
