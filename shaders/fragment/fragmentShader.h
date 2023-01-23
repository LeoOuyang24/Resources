#version 330 core

out vec4 fragColor;
in vec2 texCoord;
uniform sampler2D sprite;

void main()
{
    vec4 text = texture(sprite,texCoord);
    if (text.w == 0)
    {
        discard;
    }
    fragColor = text;//vec4(1,0,0,1);

}
