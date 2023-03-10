#version 330 core

out vec4 fragColor;
in vec2 texCoord;
uniform sampler2D sprite;

const float offset = 1.0/640 ;

float kernel[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
);
void main()
{
    vec4 text;//texture(sprite,TexCoords);
    for (int i = 0; i < 9 ; i++)
    {
        text += kernel[i]*(texture(sprite, vec2(texCoord.x + (i%3 - 1)*offset, texCoord.y + (1 - i/3)*offset)));// + vec4(.01,0,0,0);
    }
    //text= texture(sprite,texCoord);
    fragColor = text;

}
