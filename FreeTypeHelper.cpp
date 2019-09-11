#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "FreeTypeHelper.h"

RenderProgram Font::wordProgram;
Font Font::alef;
    void Font::init(int screenWidth, int screenHeight)
    {
        wordProgram.init("../../resources/shaders/vertex/wordVertex.h","../../resources/shaders/fragment/wordFragment.h");
        wordProgram.setMatrix4fv("projection", glm::value_ptr((glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f,-1.0f,1.0f))));

        alef.init("../../resources/Alef-Regular.ttf");
    }
    Font::Font(std::string source)
    {
        init(source);
    }
    void Font::init(std::string source)
    {

        font = source;
        FT_Library library;
        if (FT_Init_FreeType(&library))
        {
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        }
        FT_Face face;
        if (FT_New_Face(library, source.c_str(),0,&face))
        {
            std::cout << "Error loading font: " << source << std::endl;
        }
         FT_Set_Pixel_Sizes(face,0,32);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (GLubyte c = 0; c < 128; c++)
        {

            if ( FT_Load_Char(face,c,FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << " " << c << std::endl;
                continue;
            }
            // Generate texture
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // Now store character for later use
            Character character;
            character.letter = c;
            character.texture = texture;
            character.size =   glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            character.bearing =   glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            if (character.bearing.y > maxVert.x)
            {
                maxVert.x = character.bearing.y;
            }
            if (character.size.y - character.bearing.y > maxVert.y)
            {
                maxVert.y = character.size.y - character.bearing.y;
            }
           // character.bearing.x /= 64;
           // character.bearing.y /=64;
             character.advance =    face->glyph->advance.x;
            characters.insert(std::pair<GLchar, Character>(c, character));

        }
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1,&VBO);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
glm::vec2 Font::getDimen(std::string text, GLfloat hScale, GLfloat vScale)
{
    std::string::const_iterator c;
    double totalWidth = 0, maxHeight = 0;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];
        GLfloat h = ch.bearing.y * vScale;
        totalWidth += (ch.advance)*hScale/64;
        maxHeight += (h-maxHeight)*(h > maxHeight);
    }
    return {totalWidth,maxHeight};
}
void Font::write(RenderProgram& p, const FontParameter& param)
{
    p.setVec3fv("textColor",param.color);
    p.use();
    glBindVertexArray(VAO);
    // Iterate through all characters
    std::string::const_iterator c;
    double x = param.rect.x;
//    double maxY = y;
    //y = stanH - y;
    //glm::vec2 maxDimen = getDimen(param.text,param.rect.z,param.rect.a);
    double letterWidth = param.rect.z/param.text.size();
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    double maxHeight = (maxVert.x + maxVert.y);
    for (c = param.text.begin(); c != param.text.end(); c++)
    {
        Font::Character ch = characters[*c];
        GLfloat xpos = x +ch.bearing.x/ch.advance*letterWidth;
        GLfloat ypos = (param.rect.y) + (maxVert.x - ch.bearing.y)/maxHeight*param.rect.a;//We use 2*ch.size.y - ch.bearing.y because we need to use ch.size.y - bearing.y + ch.size.y because that is the total height the letter uses
        GLfloat w = ch.size.x/(ch.advance/64)*letterWidth;
        GLfloat h = (ch.size.y)/(maxHeight)*param.rect.a;
       /* if (ypos + h > maxY)
        {
            maxY = ypos+h;
        }*/
            // Update VBO for each character
       GLfloat vertices[6][4] = {
            { xpos,     ypos ,   0.0, 0.0 },
            { xpos + w, ypos,   1.0, 0.0 },
            { xpos,     ypos+h,  0.0, 1.0 },
            { xpos + w, ypos+h,  1.0, 1.0 }
        };

        // Render glyph texture over quad
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(float), vertices, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices),sizeof(float),&param.z);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,(void*)sizeof(vertices));
        glVertexAttribDivisor(1,1);
        glBindTexture(GL_TEXTURE_2D, ch.texture);


       // glDrawArrays(GL_TRIANGLES, 0, 6);
       glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,&indices);
        glBindBuffer(GL_ARRAY_BUFFER,0);
       // t.renderInstanced(,{{{200,200,64,64}}});
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += letterWidth; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
/*glm::vec2 Font::write(RenderProgram& p,std::string text, GLfloat x, GLfloat y,GLfloat z, GLfloat scale,glm::vec3 color)
{
    // Activate corresponding render state
  //  p.use();
    //p.setVec3fv("textColor",color);


};*/

