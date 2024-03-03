#version 330 core

out vec4 fragColor;
in vec2 texCoord;
uniform sampler2D sprite;

void main()
{
    vec4 text = texture(sprite,texCoord);
    fragColor = text;//vec4(1,0,0,1);

}
