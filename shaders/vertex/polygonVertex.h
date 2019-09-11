#version 330 core

layout (location = 0) in vec3 points;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 tri;

uniform mat4 projection;

out vec2 texCoord;
out vec4 shade;

void main()
{
    gl_Position = projection*vec4(points.xyz,1);
    shade = color;
}
