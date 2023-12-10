#version 330 core

layout (location = 0) in vec4 values;
layout (location = 1) in vec4 rect;
layout (location = 2) in int depth;
layout (location = 3) in float radians;
layout (location = 4) in int effect;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

#include "${resources_dir}/shaders/common/vertex_common.h"

out vec2 texCoord;

void main()
{
    texCoord = getTransformed(effect,values,rect,radians,vec4(0,0,1,1),projection,view);

   // gl_Position.z = depth;

}
