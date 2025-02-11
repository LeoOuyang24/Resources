#version 330 core

//renders vertex based on screen coordinates
layout (location = 0) in vec2 values;
layout (location = 1) in vec4 rect;

out vec2 texCoord;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
    float cameraZ;
    vec2 screenDimen;
};

#include "${resources_dir}/shaders/common/vertex_common.h"


void main()
{

    float z = (values.x + 1)/2.0f;
    float a = (values.y + 1)/2.0f;
    texCoord = vec2(z, 1 - a); //1-a to not be upside down
    gl_Position = vec4(rect.xy + values.xy * rect.zw,0,1);

   // gl_Position.z = depth;

}

