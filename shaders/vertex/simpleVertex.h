#version 330 core
//the most basic vertex shader you can have
layout (location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
}
