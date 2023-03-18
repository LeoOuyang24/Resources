#version 330 core
//the most basic vertex shader you can have, for textures
layout (location = 0) in vec4 aPos;
layout (location = 1) in float z;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, z, 1.0);
    texCoord = aPos.zw;
}
