#version 330 core
 //simple fragment to render a texture
out vec4 fragColor;
in vec2 texCoord;

uniform sampler2D sprite;

void main()
{
    fragColor = texture(sprite,texCoord);
}
