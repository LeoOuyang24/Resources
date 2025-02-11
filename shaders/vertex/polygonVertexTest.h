#version 330 core

layout (location = 0) in vec3 points;
layout (location = 1) in float z;

layout (location = 2) in vec4 color;
layout (location = 3) in float a;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};


out vec4 shade;

void main()
{
    gl_Position = projection*view*vec4(points.xy,z,1);
    shade = vec4(vec3(color),1-a);
}
