#version 330 core

layout (location = 0) in vec2 values;
/*layout (location = 1) in vec4 rect;
layout (location = 2) in int depth;
layout (location = 3) in float radians;
layout (location = 4) in int effect;*/

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
    float cameraZ;
    vec2 screenDimen;
};

uniform sampler1D sprite;

#include "${resources_dir}/shaders/common/vertex_common.h"

out vec2 texCoord;

void main()
{
    int ind = (0/2)*7; //index of the first float in our data
    vec4 rect = vec4(texture(sprite,ind).r,texture(sprite,ind+4).r, 100,100);
    //rect = vec4(100,100,100,100);
    float depth = texture(sprite,ind + 4).r;
    float radians = texture(sprite, ind + 5).r;
    float effect = texture(sprite, ind + 6).r;

    //vec4 rect = vec4(100);

    texCoord = getTransformed(0,values,rect,1, 0,vec4(0,0,1,1),projection,view);

   // gl_Position.z = depth;

}
