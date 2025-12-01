#version 330 core

out vec4 FragColor;

in vs_out
{
    vec3 model_fragpos;
    vec2 texcoord;
    vec3 tangent_lightpos;
    vec3 tangent_viewpos;
    vec3 tangent_fragpos;
    vec3 tangent_normal;
}
vs_in;

uniform sampler2D u_tex_color;
uniform sampler2D u_tex_metallic_roughness;
uniform sampler2D u_tex_normal;

uniform float u_time;

float get_diffuse(float intensity, vec3 lightpos)
{
    // vec3 normal = vs_in.tangent_normal;
    vec3 normal = texture(u_tex_normal, vs_in.texcoord).xyz;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 lightdelta = lightpos - vs_in.tangent_fragpos;
    float lightdistance = length(lightdelta);
    vec3 lightdir = normalize(lightdelta);

    return max(intensity * dot(normal, lightdir) / lightdistance, 0.0);
}

void main()
{
    // FragColor.xyz = get_diffuse(5, vs_in.tangent_viewpos) *
    //                 texture(u_tex_color, vs_in.texcoord).rgb;

    FragColor.xyz = texture(u_tex_color, vs_in.texcoord).rgb;

    // FragColor.xyz = texture(u_tex_normal, vs_in.texcoord).xyz;
    // vec3 normal = texture(u_tex_normal, vs_in.texcoord).xyz;
    // normal = normalize(normal * 2.0 - 1.0);

    // FragColor.xyz = vs_in.tangent_viewpos;
    // FragColor.xyz = ( vs_in.tangent_normal * vec3(0,0,1) ) * vec3(1,1,1);
    // FragColor.xyz = vec3(1,0,0);
    FragColor.w = 1;

    // FragColor = vec4(1,0,0,1);
}
