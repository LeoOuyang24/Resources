//library functions for vertex shaders


vec2 getTransformed(int effect, vec2 values, vec4 rect, float depth, float radians, vec4 subsection, mat4 projection, mat4 view)
{
    float z = (values.x + 1)/2.0f;
    float a = (values.y + 1)/2.0f;
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


    vec2 transformed = vec2(values.xy * rect.zw*.5);
    //transformed = vec2(cos(radians)*transformed.x + sin(radians),transformed.y );
    transformed = vec2(cos(radians)*transformed.x - sin(radians)*transformed.y,sin(radians)*transformed.x + cos(radians)*transformed.y);
    transformed += rect.xy + rect.zw*.5;

    gl_Position = projection*view*vec4(transformed,depth,1);

    vec2 texCoord = vec2( subsection.x + z*subsection.z, subsection.y + a*subsection.a);
    return texCoord;
}
