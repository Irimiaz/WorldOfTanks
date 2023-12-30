#version 330

// Input
// TODO(student): Get vertex attributes from each location
layout(location = 0) in vec3 v_position;
layout(location = 3) in vec3 v_normal;
layout(location = 2) in vec2 v_texture;
layout(location = 1) in vec3 v_color;


// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform float Health;

// Output
// TODO(student): Output values to fragment shader
out vec3 frag_color;
out vec3 frag_normal;
out vec3 frag_rand_color;
out vec2 tex_coord;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 712.5453);
}

void main()
{
    // TODO(student): Send output to fragment shader
    frag_color = v_color;
    frag_normal = v_normal;
    frag_rand_color = vec3((5 - Health) * 0.03,(5 - Health) * 0.03, (5 - Health) * 0.03);

    // TODO(student): Compute gl_Position
    tex_coord = v_texture;
    vec4 my_pos = vec4(v_position, 1);
    
    my_pos.x = my_pos.x - rand(vec2(my_pos.z, my_pos.y)) * 20 *(1 - Health/5);
    my_pos.y = my_pos.y - rand(vec2(my_pos.x, my_pos.z)) * 20 *(1 - Health/5);
    my_pos.z = my_pos.z - rand(vec2(my_pos.x, my_pos.y)) * 20 *(1 - Health/5);
    my_pos = Model * my_pos;
    gl_Position = Projection * View * my_pos;

}
