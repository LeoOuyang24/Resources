#ifndef FREETYPEHELPER_H_INCLUDED
#define FREETYPEHELPER_H_INCLUDED

#include "glew.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <map>
#include "render.h"




struct FontParameter //Represents all the information  required to call the write function given to a font
{
    std::string text = "";
    glm::vec4 rect = {0,0,0,0};
    float z = 0;
    glm::vec3 color = {0,0,0};
};

class Font
{
    class Character //represents a character in a font
    {
    public:
        char letter;
        GLuint texture = -1;
        glm::vec2 size; //the dimensions of the character
        glm::vec2 bearing; //the bearing, or margins of the character
        GLuint advance; //the total width the character takes up, including the character width, the horizontal bearing, and the space from the next character
    };
    std::map<GLchar,Character> characters;
    glm::vec2 maxVert; //x is max bearing.y, y is max size - bearing.y. Sum of x and y is the maximum height we need
    std::string font = "";
    GLuint  VBO,VAO;
    int indices[6] = {
        0,1,2,
        3,1,2};
public:
    static RenderProgram wordProgram;
    Font(std::string source);
    Font()
    {

    }
    void init(std::string source);
    glm::vec2 getDimen(std::string text, GLfloat hScale, GLfloat vScale); //gets the dimensions on text printed if the text were to be printed. The height is based on the bearing rather than the actual character height
    void write(RenderProgram& p, const FontParameter& param);
    ~Font()
    {
        characters.clear();
    }
    static void init(int screenWidth,int screenHeight); //initializes wordProgram and the default alef font
    static Font alef; //the default alef font
    };


#endif // FREETYPEHELPER_H_INCLUDED
