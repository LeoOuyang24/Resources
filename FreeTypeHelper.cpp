#include <iostream>

#include "FreeTypeHelper.h"
#include "geometry.h"

RenderProgram Font::wordProgram;
Font Font::tnr;


Font::Character::Character(char c, FT_Face& face) : Sprite()
{
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1,&VBO);
        glGenBuffers(1,&modVBO);

        glBindVertexArray(VAO);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D,texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    if ( FT_Load_Char(face,c,FT_LOAD_RENDER))
    {
        throw std::logic_error ("ERROR::FREETYTPE: Failed to load Glyph" + c);
    }
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );

   // glGenerateMipmap(GL_TEXTURE_2D);

    //std::cout << glGetError() << std::endl;
    //std::cout << texture << std::endl;
    letter = c;
    size =     glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
    bearing =  glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
    advance =    face->glyph->advance.x;

    loadVertices();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    //load("image.png",true);
}

const glm::vec2& Font::Character::getBearing()
{
    return bearing;
}
const glm::vec2& Font::Character::getSize()
{
    return size;
}
GLuint Font::Character::getAdvance()
{
    return advance;
}

Font::FontWrapper::FontWrapper(Character& sprite)
{
    spr = &sprite;
}

Font::Character& Font::FontWrapper::getCharacter()
{
    return *static_cast<Character*>(spr);
}

int Font::writeLength(std::string str)
{
    int length = 0;
    int size = str.size();
    for (int i = 0; i < size; ++i)
    {
        length += characters[str[i]]->getCharacter().getAdvance() >> 6;
    }
    return length;
}

    void Font::init(int screenWidth, int screenHeight)
    {
        wordProgram.init("../../resources/shaders/vertex/vertexShader.h","../../resources/shaders/fragment/wordFragment.h");
        wordProgram.setMatrix4fv("projection", glm::value_ptr(RenderProgram::getOrtho()));

        tnr.init("../../resources/tnr.ttf");
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
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (GLubyte c = 0; c < 128; c++)
        {

            // Now store character for later use
            Character* character = new Character(c,face);
            auto bearing = &(character->getBearing());
            if (bearing->y > maxVert.x)
            {
                maxVert.x = bearing->y;
            }
            auto size = &(character->getSize());
            if (size->y - bearing->y > maxVert.y)
            {
                maxVert.y = size->y - bearing->y;
            }
           // character.bearing.x /= 64;
           // character.bearing.y /=64;
           FontWrapper* fontWrapper = new FontWrapper(*character);
           SpriteManager::addSprite(*fontWrapper);
            characters[c] = std::unique_ptr<FontWrapper>(fontWrapper);
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
    auto end = text.end();
   // std::cout << hScale << " " << vScale << std::endl;
    for (c = text.begin(); c != end; ++c)
    {
        Character* ch = &(characters[*c].get()->getCharacter());
        GLfloat h = ch->getSize().y * vScale;
        totalWidth += (ch->getAdvance())*hScale/64;
        maxHeight += (h > maxHeight)*(h-maxHeight);
    }
    return {totalWidth,maxHeight};
}

void Font::requestWrite(const FontParameter& param)
{
    float scale;
    glm::vec4 absRect = absoluteValueRect(param.rect);
    glm::vec2 dimen = getDimen(param.text,1,1);
    if (param.rect.z < 0 )
    {
        scale = param.rect.a;
        absRect.z = dimen.x*scale;
        absRect.a = dimen.y*scale;

    }
    else
    {
        scale =  std::min((absRect.z/dimen.x),absRect.a/(maxVert.y + maxVert.x));
    }
    glm::vec2 center = {absRect.x + absRect.z/2, absRect.y + absRect.a/2};

    auto screenDimen = (RenderProgram::getScreenDimen()); //we need to find the dimensions of the screen vs the dimensions of the projection matrix and scale accordingly. We will assume that the ortho and screen dimen start at 0

    double x = absRect.x, y = absRect.y;
    switch (param.align)
    {
    case RIGHT:
        x += param.rect.z - getDimen(param.text,scale,1).x;
        break;
    case CENTER:
        x += param.rect.z/2 - getDimen(param.text,scale,1).x/2;
        break;
    }
    switch (param.vertAlign)
    {
    case VERTCENTER:
        y += param.rect.a/2 - getDimen(param.text,1,scale).y/2;
        break;
    case DOWN:
        y += param.rect.a - getDimen(param.text,1,scale).y;
        break;
    }
//    PolyRender::requestRect(absRect,{0,1,0,1},false,0,-1);
    //std::cout << length << std::endl;
      //  std::cout << "Start: " << writeRequests.size() << std::endl;
    int size = param.text.size();
    for (int i = 0; i < size; ++i)
    {
        char c = param.text[i];
        Character* ch = &(characters[c].get()->getCharacter());
        const glm::vec2* bearing = &ch->getBearing();
        const glm::vec2* chSize = &ch->getSize();
        GLfloat xpos = x +bearing->x*scale;
        GLfloat ypos = y+ (maxVert.x - bearing->y)*scale;
        //std::cout << c << " " << xpos<< " "<< ypos << std::endl;
        glm::vec2 pos = rotatePoint({xpos,ypos},center,param.angle);
        GLfloat w = chSize->x*scale;
        GLfloat h = (chSize->y)*scale;
      //  PolyRender::requestRect({pos.x,pos.y,w,h},{1,0,0,1},false,0,-1);
     // printRect({pos.x,pos.y,w,h});
        characters[c]->request({{pos.x,pos.y,w,h},0,NONE,param.color,&wordProgram,param.z});
        x += (ch->getAdvance() >> 6 )*scale;
    }

   // std::cout << "End: " << writeRequests.size() << std::endl;
}
void Font::write()
{
    /*auto end = characters.end();
    for (auto it = characters.begin(); it != end; ++it)
    {
   //     it->second.get()->render();
    //    it->second.get()->reset();
    }*/

}

Font::~Font()
{
   // requests.clear();
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
        fonts[i]->write();
       // fonts[i]->reset();
    }
}
/*glm::vec2 Font::write(RenderProgram& p,std::string text, GLfloat x, GLfloat y,GLfloat z, GLfloat scale,glm::vec3 color)
{
    // Activate corresponding render state
  //  p.use();
    //p.setVec3fv("textColor",color);


};*/

