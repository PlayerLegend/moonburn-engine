#version 330 core

in vec3 v_view_pos;
in vec3 v_normal;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec2 v_tex_coord;

uniform sampler2D u_map_albedo;
uniform sampler2D u_map_normal;
uniform sampler2D
    u_map_metallic_roughness; // R = metallic, G = roughness, B = ao (optional)
uniform sampler2D u_map_emission;

uniform vec3 u_albedo_color;

uniform bool u_use_map_albedo;
uniform bool u_use_map_normal;
uniform bool u_use_map_mr;
uniform bool u_use_map_emission;

layout(location = 0) out vec3 g_position;
layout(location = 1) out vec3 g_normal;
layout(location = 2) out vec3 g_albedo;

void main()
{
    vec3 albedo = u_albedo_color;

    if (u_use_map_albedo)
    {
        albedo *= texture(u_map_albedo, v_tex_coord).rgb;
    }

    vec3 N = normalize(v_normal);

    if (u_use_map_normal)
    {
        vec3 map_n = texture(u_map_normal, v_tex_coord).rgb;
        vec3 tspace_n = normalize(map_n * 2.0 - 1.0);
        mat3 tbn = mat3(normalize(v_tangent), normalize(v_bitangent), N);
        N = normalize(tbn * tspace_n);
    }

    g_position = vec4(v_view_pos, 1.0); // view-space position
    g_normal = N;
    g_albedo = vec4(albedo, 1.0);
}