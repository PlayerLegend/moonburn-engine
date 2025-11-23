#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_tangent;
layout(location = 3) in vec2 a_tex_coord;

uniform mat4 u_mat_m;
uniform mat4 u_mat_v;
uniform mat4 u_mat_p;

uniform mat4 u_mat_mv;
uniform mat3 u_mat_normal;

out vec3 v_view_pos;
out vec3 v_normal;    // view-space normal
out vec3 v_tangent;   // view-space tangent
out vec3 v_bitangent; // view-space bitangent
out vec2 v_tex_coord;

void main()
{
    v_view_pos = (u_mat_mv * vec4(a_position, 1.0)).xyz;

    v_normal = normalize(u_mat_normal * a_normal);
    v_tangent = normalize(u_mat_normal * a_tangent);
    v_bitangent = normalize(cross(v_normal, v_tangent));

    v_tex_coord = a_tex_coord;

    // Clip position
    gl_Position = u_projection * vec4(v_view_pos, 1.0);
}