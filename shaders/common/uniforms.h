//full list of things in the uniform buffer
//probably want to put this a bit further down from the rest of your includes

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
    vec2 screenDimen;
    float cameraZ;

};
