#version 330 core

layout (location = 0) in vec4 values;
layout (location = 1) in vec4 rect;
layout (location = 2) in int depth;
layout (location = 3) in float radians;
layout (location = 4) in float effect;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};


out vec2 texCoord;


void main()
{
    float z = values.z;
    float a = values.a;
    /*if (effect == 1)
    {
        z*=-1;
    }
    else
    {
        if (effect == 2)
        {
            a*=-1;
        }
    }*/
    vec2 transformed = vec2(values.xy * rect.zw*.5);
    transformed = vec2(cos(radians)*transformed.x - sin(radians)*transformed.y,sin(radians)*transformed.x + cos(radians)*transformed.y);
    transformed += rect.xy + rect.zw*.5;
    gl_Position = projection*view*vec4(transformed,depth,1);
   // gl_Position.z = depth;
    texCoord = vec2(z,a);

}
