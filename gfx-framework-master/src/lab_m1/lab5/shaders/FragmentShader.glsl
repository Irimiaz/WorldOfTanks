#version 330

// Input
// TODO(student): Get values from vertex shader
in vec3 frag_color;
in vec3 frag_rand_color;
in vec3 frag_normal;
in vec2 tex_coord;


uniform sampler2D u_texture_0;
// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Write pixel out color
    out_color = texture(u_texture_0, tex_coord) - vec4(frag_rand_color, 0);
    if(out_color.a < 0.9)
    {
        discard;
    }
}
