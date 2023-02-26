#version 330 core

layout (location = 0) in vec3 points;
layout (location = 1) in vec4 color;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};


out vec4 shade;

void main()
{
    gl_Position = projection*view*vec4(points.xyz,1);
    shade = color;
}
