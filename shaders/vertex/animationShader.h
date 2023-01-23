#version 330 core

layout (location = 0) in vec4 values;
layout (location = 3) in mat4 transform;
layout (location = 7) in float effect;
layout (location = 8) in vec4 color;
layout (location = 9) in float depth;
layout (location = 10) in vec4 portion;

uniform mat4 projection;
uniform mat4 view;

out vec2 texCoord;
out vec4 shade;


void main()
{
    float z = values.z;
    float a = values.a;
    float portionX = portion.x;
    float portionY = portion.y;
    if (effect == 1)
    {
        z*=-1;
        portionX += portion.z;
    }
    else
    {
        if (effect == 2)
        {
            a*=-1;
            portionY += portion.a;
        }
    }
    gl_Position = projection*view*transform*vec4(values.xy,depth,1);
   // gl_Position.z = depth;
    texCoord = vec2( portionX + z*portion.z,portionY + a*portion.a);
    shade = color;

}
