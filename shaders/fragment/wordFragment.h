#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
void main()
{
    float a = texture(text, TexCoords).r;
    if (a < .3)
    {
        discard;
    }
        vec4 sampled = vec4(1.0, 1.0, 1.0,a );
        color = sampled*vec4(textColor,1);

}

