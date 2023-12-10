//library functions for vertex shaders


vec2 getTransformed(int effect, vec4 values, vec4 rect, float radians, vec4 subsection, mat4 projection, mat4 view)
{
     float z = values.z;
    float a = values.a;
    if (effect == 1)
    {
        z*=-1;
        z += 1;
    }
    else
    {
        if (effect == 2)
        {
            a*=-1;
        }
    }


    /*int frame = int(mod(fps*(timeSince/1000.0),framesDimen.x*framesDimen.y));
    float portionX = mod(frame,framesDimen.x)/framesDimen.x;
    float portionY = int(frame/framesDimen.x)/framesDimen.y;*/

    vec2 transformed = vec2(values.xy * rect.zw*.5);
    transformed = vec2(cos(radians)*transformed.x - sin(radians)*transformed.y,sin(radians)*transformed.x + cos(radians)*transformed.y);
    transformed += rect.xy + rect.zw*.5;

    gl_Position = projection*view*vec4(transformed,depth,1);

    vec2 texCoord = vec2( subsection.x + z*subsection.z, subsection.y + a*subsection.a);
    return texCoord;
}
