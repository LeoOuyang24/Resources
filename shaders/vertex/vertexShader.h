#version 330 core

layout (location = 0) in vec4 values;
layout (location = 1) in mat4 transform;
layout (location = 5) in float depth;
layout (location = 6) in float effect;


uniform mat4 projection;
uniform mat4 view;

out vec2 texCoord;


void main()
{
    float z = values.z;
    float a = values.a;
    if (effect == 1)
    {
        z*=-1;
    }
    else
    {
        if (effect == 2)
        {
            a*=-1;
        }
    }
    gl_Position = projection*view*transform*vec4(values.xy,depth,1);
   // gl_Position.z = depth;
    texCoord = vec2(z,a);

}
