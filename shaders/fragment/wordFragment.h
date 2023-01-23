#version 330 core

out vec4 fragColor;
in vec2 texCoord;
in vec4 shade;
uniform sampler2D sprite;

void main()
{
    vec4 text = texture(sprite,texCoord);
    if (text.r < .1)
    {
        discard;
    }
    fragColor = vec4(0,0,0,1);

}
