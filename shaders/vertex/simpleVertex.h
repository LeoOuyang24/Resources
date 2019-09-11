#version 330 core

layout (location = 0) in vec3 points;

uniform vec3 color;
uniform mat4 projection;

out vec2 texCoord;
out vec3 shade;

void main()
{
    gl_Position = projection*vec4(points.xyz,1);
    shade = color;
}
