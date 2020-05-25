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
    double angle = 0;
    glm::vec4 color = {0,0,0,1};
    float z = 0;
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
    glm::vec2 maxVert; //x is max bearing.y, y is maximum space underneath the bearing. Sum of x and y is the maximum height we need
    std::string font = "";
    GLuint  VBO,VAO;
    int indices[6] = {
        0,1,2,
        3,1,2};
    std::vector<FontParameter> writeRequests;
    int writeLength(std::string str);
public:
    static void init(int screenWidth,int screenHeight); //initializes wordProgram and the default alef font
    static Font tnr; //the default alef font
    static RenderProgram wordProgram;
    Font(std::string source);
    Font()
    {

    }
    void init(std::string source);
    glm::vec2 getDimen(std::string text, GLfloat hScale, GLfloat vScale); //gets the dimensions on text printed if the text were to be printed. The height is based on the bearing rather than the actual character height
    void requestWrite(FontParameter&& param);
    void write(RenderProgram& p, const FontParameter& param); //Attempts to fit the string into the given rect
    void write(RenderProgram& p); //given a renderprogram, renders all the FontParameters in writeRequests
    void clear();
    ~Font();

    };

class FontManager
{
    static std::vector<Font*> fonts;
public:
    static void addFont(Font& font);
    static void update();
};


#endif // FREETYPEHELPER_H_INCLUDED
