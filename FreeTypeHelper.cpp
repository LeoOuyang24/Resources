#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "FreeTypeHelper.h"

RenderProgram Font::wordProgram;
Font Font::tnr;

int Font::writeLength(std::string str)
{
    int length = 0;
    int size = str.size();
    for (int i = 0; i < size; ++i)
    {
        length += characters[str[i]].advance >> 6;
    }
    return length;
}

    void Font::init(int screenWidth, int screenHeight)
    {
        wordProgram.init("../../resources/shaders/vertex/wordVertex.h","../../resources/shaders/fragment/wordFragment.h");
        wordProgram.setMatrix4fv("projection", glm::value_ptr(RenderProgram::getOrtho()));

        tnr.init("../../resources/tnr.ttf");

        FontManager::addFont(tnr);
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
        GLfloat h = ch.size.y * vScale;
        totalWidth += (ch.advance)*hScale/64;
        maxHeight += (h-maxHeight)*(h > maxHeight);
    }
    return {totalWidth,maxHeight};
}

void Font::requestWrite(FontParameter&& param)
{
      //  std::cout << "Start: " << writeRequests.size() << std::endl;
    writeRequests.emplace_back(param);
   // std::cout << "End: " << writeRequests.size() << std::endl;
}

void Font::write(RenderProgram& p, const FontParameter& param)
{
    p.setVec4fv("textColor",param.color);
    p.use();
    glBindVertexArray(VAO);
    // Iterate through all characters
    std::string::const_iterator c;
    glm::vec2 center = {param.rect.x + param.rect.z/2, param.rect.y + param.rect.a/2};
    double x = param.rect.x;
    glm::vec2 dimen = getDimen(param.text,1,1);
//std::cout << length << std::endl;
    double scale = std::min(param.rect.z/dimen.x,param.rect.a/(maxVert.y + maxVert.x));
   // std::cout << scale << std::endl;
   // double maxHeight = (maxVert.x + maxVert.y);
   // PolyRender::requestRect({x,param.rect.y + maxVert.x, param.rect.z,param.rect.a - maxVert.x},{1,0,0,1},true,0,param.z);
    for (c = param.text.begin(); c != param.text.end(); c++)
    {
        Font::Character ch = characters[*c];
        GLfloat xpos = x +ch.bearing.x*scale;
        GLfloat ypos = (param.rect.y) + (maxVert.x - ch.bearing.y)*scale;
        glm::vec2 pos = rotatePoint({xpos,ypos},center,param.angle);
        GLfloat w = ch.size.x*scale;
        GLfloat h = (ch.size.y)*scale;
       /* if (ypos + h > maxY)
        {
            maxY = ypos+h;
        }*/
            // Update VBO for each character
       GLfloat vertices[6][4] = {
            { pos.x,     pos.y ,   0.0, 0.0 },
            { pos.x + w, pos.y,   1.0, 0.0 },
            { pos.x,     pos.y+h,  0.0, 1.0 },
            { pos.x + w, pos.y+h,  1.0, 1.0 }
        };
       // PolyRender::requestRect({x,param.rect.y + maxVert.x - ch.bearing.y, w,h},{1,0,0,1},false,0,param.z);

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
        x += (ch.advance >> 6)*scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Font::write(RenderProgram& p)
{
        //std::cout << "Writing: " << writeRequests.size() << std::endl;
    int size = writeRequests.size();
    for (int i = 0; i < size; ++i)
    {
        write(p,writeRequests[i]);
    }
}

void Font::clear()
{
    writeRequests.clear();
    //std::cout << writeRequests.size() << std::endl;
}

Font::~Font()
{
    clear();
    characters.clear();
}

std::vector<Font*> FontManager::fonts;

void FontManager::addFont(Font& font)
{
    fonts.push_back(&font);
}

void FontManager::update()
{
    int size = fonts.size();
    for (int i = 0; i < size; ++i)
    {
        fonts[i]->write(Font::wordProgram);
        fonts[i]->clear();
    }
}
/*glm::vec2 Font::write(RenderProgram& p,std::string text, GLfloat x, GLfloat y,GLfloat z, GLfloat scale,glm::vec3 color)
{
    // Activate corresponding render state
  //  p.use();
    //p.setVec3fv("textColor",color);


};*/

