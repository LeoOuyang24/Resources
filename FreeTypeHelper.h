#ifndef FREETYPEHELPER_H_INCLUDED
#define FREETYPEHELPER_H_INCLUDED

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>
#include <memory>
#include <list>

#include "geometry.h"
#include "render.h"

enum Align
{
    LEFT,
    CENTER,
    RIGHT
};

enum VertAlign
{
    UP,VERTCENTER,DOWN
};


struct FontParameter //Represents all the information  required to call the write function given to a font
{
    std::string text = "";
    glm::vec4 rect = {0,0,0,0}; //if the width is negative, the height is assumed to be the font size
    glm::vec4 color = {0,0,0,1};
    double angle = 0;
    float z = 0;
    Align align = LEFT;
    VertAlign vertAlign = UP;
};


class Character : public Sprite//represents a character in a font
{
    char letter;
protected:
    glm::ivec2 size; //the dimensions of the character
    glm::ivec2 bearing; //the bearing, or margins of the character
    GLuint advance; //the total width the character takes up, including the character width, the horizontal bearing, and the space from the next character
    Character(char c); //use this constructor if loading the texture is handled by some child class constructor
public:
    Character(char c, FT_Face& face);
    const glm::ivec2& getBearing();
    const glm::ivec2& getSize();
    GLuint getAdvance();

};

class Font;
struct FontGlobals
{
    static Font tnr; //the default alef font
    static std::unique_ptr<BasicRenderPipeline> wordProgram;

    static void init(int screenWidth,int screenHeight); //initializes wordProgram and the default alef font
};

class Font
{
    glm::vec2 maxVert = {0,0}; //x is max bearing.y, y is maximum space underneath the bearing. Sum of x and y is the maximum height we need
    std::string font = "";
protected:
    std::unordered_map<GLchar,std::unique_ptr<Character>> characters;
    int writeLength(std::string str);
public:
    Font(std::string source);
    Font()
    {

    }
    //initializes the font
    //you can specify any child class of Character or use Character itself
    template <typename T = Character>
    void init(std::string source)
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
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (GLubyte c = 0; c < 128; c++)
        {

            // Now store character for later use
            T* character = new T(c,face);
            glm::ivec2 bearing = (character->getBearing());
            if (bearing.y > maxVert.x)
            {
                maxVert.x = bearing.y;
            }
            glm::ivec2 size = (character->getSize());
            if (size.y - bearing.y > maxVert.y)
            {
                maxVert.y = size.y - bearing.y;
            }
            characters[c] = std::unique_ptr<Character>(character);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(library);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }
    glm::vec2 getDimen(std::string text, GLfloat hScale, GLfloat vScale); //gets the dimensions on text printed if the text were to be printed. The height is based on the bearing rather than the actual character height
    virtual void requestWrite(const FontParameter& param, BasicRenderPipeline& pipeline = *FontGlobals::wordProgram); //idk, I feel like the pipeline should not be part of the FontParameter. I can't explain why, just a gut feeling
    Character& getChar(GLchar c)
    {
        return *characters[c].get();
    }
    void clear();
    ~Font();

    };


#endif // FREETYPEHELPER_H_INCLUDED
