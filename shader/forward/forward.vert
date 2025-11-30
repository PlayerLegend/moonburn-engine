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

out vs_out
{
    vec3 model_fragpos;
    vec2 texcoord;
    vec3 tangent_lightpos;
    vec3 tangent_viewpos;
    vec3 tangent_fragpos;
    vec3 tangent_normal;
};

mat4 get_skin_matrix_part(int index)
{
    int base = int(index) * 4; // 4 texels per mat4

    vec4 c0 = texelFetch(u_skin, ivec2(base + 0, 0), 0);
    vec4 c1 = texelFetch(u_skin, ivec2(base + 1, 0), 0);
    vec4 c2 = texelFetch(u_skin, ivec2(base + 2, 0), 0);
    vec4 c3 = texelFetch(u_skin, ivec2(base + 3, 0), 0);

    return mat4(c0, c1, c2, c3);
}

mat4 get_skin_matrix()
{
    return attribute_weights[0] * get_skin_matrix_part(0) +
           attribute_weights[1] * get_skin_matrix_part(1) +
           attribute_weights[2] * get_skin_matrix_part(2) +
           attribute_weights[3] * get_skin_matrix_part(3);
}

mat3 get_skin_normal_matrix(mat4 skin_matrix)
{
    // this will require uniform scaling of bones, I'll figure out a better
    // solution later
    return mat3(skin_matrix);
}

mat3 get_tbn_matrix(mat3 normal_matrix)
{
    vec3 tbn_t = normalize(normal_matrix * attribute_tangent.xyz);
    vec3 tbn_n = normalize(normal_matrix * attribute_normal);
    tbn_t = normalize(tbn_t - dot(tbn_t, tbn_n) * tbn_n);
    vec3 tbn_b = normalize(cross(tbn_n, tbn_t) * attribute_tangent.w);

    return transpose(mat3(tbn_t, tbn_b, tbn_n));
}

void vs_finish(vec3 position, mat3 normal_matrix)
{
    mat3 tbn = get_tbn_matrix(normal_matrix);

    model_fragpos = (u_model * vec4(position, 1.0)).xyz;
    texcoord = attribute_texcoord;
    tangent_lightpos = tbn * u_world_lightpos;
    tangent_viewpos = tbn * u_world_viewpos;
    tangent_fragpos = tbn * model_fragpos;
    tangent_normal = tbn * attribute_normal;

    gl_Position = u_mvp * vec4(position, 1.0);
}

void main()
{
    mat4 position_matrix;
    mat4 normal_matrix;

    if (u_skin_count > 0)
    {
        mat4 skin_matrix = get_skin_matrix();
        vec3 position = (skin_matrix * vec4(attribute_position, 1.0f)).xyz;

        vs_finish(position, u_normal * mat3(skin_matrix));
    }
    else
    {
        vs_finish(attribute_position, u_normal);
    }
}
