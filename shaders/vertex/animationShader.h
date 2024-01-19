#version 330 core

layout (location = 0) in vec2 values;
layout (location = 1) in vec4 rect;
layout (location = 2) in int depth;
layout (location = 3) in vec4 subsection;
layout (location = 4) in float radians;
layout (location = 5) in int effect;
layout (location = 6) in vec4 tint_;

#include "${resources_dir}/shaders/common/vertex_common.h"


layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};



out vec2 texCoord;
out vec4 tint;

void main()
{
    texCoord = getTransformed(effect,values,rect,radians,subsection,projection,view);


    tint = tint_;
}
