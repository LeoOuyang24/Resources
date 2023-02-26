#version 330 core

layout (location = 0) in vec2 point;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec4 shade;
out vec2 curPos;
flat out vec2 endPos;


void main()
{
    gl_Position = projection*view*vec4(point.xy,2,1);
    curPos = gl_Position.xy;
    endPos = curPos;
    shade = vec4(1,0,0,1);
}
