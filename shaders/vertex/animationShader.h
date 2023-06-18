#version 330 core

layout (location = 0) in vec4 values;
layout (location = 1) in vec4 rect;
layout (location = 2) in int depth;
layout (location = 3) in vec4 subsection;
layout (location = 4) in vec2 framesDimen; //total number of frames in the entire spritesheet
layout (location = 5) in int timeSince; //milliseconds since we started
layout (location = 6) in int fps;
layout (location = 7) in float radians;
layout (location = 8) in int effect;

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
    if (effect == 1)
    {
        z*=-1;
        z += 1;
    }
    else
    {
        if (effect == 2)
        {
            a*=-1;
        }
    }


    int frame = int(mod(fps*(timeSince/1000.0),subsection.z*subsection.a));
    float portionX = mod(frame,subsection.z)/framesDimen.x;
    float portionY = int(frame/subsection.z)/framesDimen.y;
    vec2 transformed = vec2(values.xy * rect.zw*.5);
    transformed = vec2(cos(radians)*transformed.x - sin(radians)*transformed.y,sin(radians)*transformed.x + cos(radians)*transformed.y);
    transformed += rect.xy + rect.zw*.5;
    gl_Position = projection*view*vec4(transformed,depth,1);
    texCoord = vec2( portionX + subsection.x + z/framesDimen.x,portionY + subsection.y + a/framesDimen.y);
}
