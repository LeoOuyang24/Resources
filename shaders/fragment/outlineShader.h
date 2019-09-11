#version 330 core

out vec4 fragColor;
in vec2 texCoord;

in vec3 shade;
uniform sampler2D sprite;

void main()
{
    float offset = .01;
    fragColor = texture(sprite,texCoord);
    float threshold = .5;
    vec2 left = vec2(texCoord.x - offset, texCoord.y);
    vec2 right = vec2(texCoord.x + offset, texCoord.y);
    vec2 up =vec2(texCoord.x, texCoord.y - offset);
    vec2 down = vec2(texCoord.x, texCoord.y + offset);
if (((left.x >= 0 && texture(sprite,left).a > threshold )|| (right.x <= 1 && texture(sprite,right).a > threshold )||
        (up.y >= 0 && texture(sprite,up).a > threshold) || ( down.y <= 1 &&  texture(sprite,down).a > threshold)) &&
        fragColor.a < threshold)
    {
        fragColor = vec4(shade,1);
    }
}
