#include <iostream>

#include "FreeTypeHelper.h"
#include "geometry.h"
#include "resourcesMaster.h"

std::unique_ptr<BasicRenderPipeline> Font::wordProgram;
Font Font::tnr;


Font::Character::Character(char c, FT_Face& face) : Sprite()
{
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

    //transluscent = true;

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

int Font::writeLength(std::string str)
{
    int length = 0;
    int size = str.size();
    for (int i = 0; i < size; ++i)
    {
        length += characters[str[i]]->getAdvance() >> 6;
    }
    return length;
}

    void Font::init(int screenWidth, int screenHeight)
    {
        std::string source = templateShader(stripComments(readFile(ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/vertex/betterShader.h").first), true,
                                            {"vec4 color"},
                                            {"vec4 shade"},
                                            {"shade = color"});
        wordProgram = std::unique_ptr<BasicRenderPipeline>(new BasicRenderPipeline({LoadShaderInfo{source,GL_VERTEX_SHADER,false},
                                                                                   LoadShaderInfo{ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/fragment/wordFragment.h",GL_FRAGMENT_SHADER,true}}));

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
           //SpriteManager::addSprite(*character);
            characters[c] = std::unique_ptr<Character>(character);
        }


        FT_Done_Face(face);
        FT_Done_FreeType(library);
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
        Character* ch = (characters[*c].get());
        GLfloat h = ch->getSize().y * vScale;
        totalWidth += (ch->getAdvance())*hScale/64;
        maxHeight += (h > maxHeight)*(h-maxHeight);
    }
    return {totalWidth,maxHeight};
}

void Font::requestWrite(const FontParameter& param, BasicRenderPipeline& pipeline)
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

    auto screenDimen = (ViewPort::getScreenDimen()); //we need to find the dimensions of the screen vs the dimensions of the projection matrix and scale accordingly. We will assume that the ortho and screen dimen start at 0

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
        Character* ch = (characters[c].get());
        const glm::vec2* bearing = &ch->getBearing();
        const glm::vec2* chSize = &ch->getSize();
        GLfloat xpos = x +bearing->x*scale;
        GLfloat ypos = y+ (maxVert.x - bearing->y)*scale;
        glm::vec2 pos = rotatePoint({xpos,ypos},center,param.angle);
        GLfloat w = chSize->x*scale;
        GLfloat h = (chSize->y)*scale;

        //SpriteManager::request({*wordProgram,characters[c].get()},param.z,false,glm::vec4(pos.x,pos.y,w,h));
        SpriteManager::requestSprite({pipeline,characters[c].get()},glm::vec4(pos.x,pos.y,w,h),param.z,0,0,param.color);
      //  PolyRender::requestRect({pos.x,pos.y,w,h},{1,0,0,1},false,0,-1);
     // printRect({pos.x,pos.y,w,h});
        //SpriteManager::request(*characters[c],{{pos.x,pos.y,w,h},0,param.z});
        //characters[c]->request({{pos.x,pos.y,w,h},0,NONE,param.color,&wordProgram,param.z});
        x += (ch->getAdvance() >> 6 )*scale;
    }

   // std::cout << "End: " << writeRequests.size() << std::endl;
}

Font::~Font()
{
   // requests.clear();
    characters.clear();
}

/*FontManager::FontManager(std::string vectorShader, std::string fragmentShader) : transFish("projects/trans_fish.png")
{
    program.init(vectorShader,fragmentShader,1);
}

void FontManager::request(Font& font, std::string str, const SpriteParameter& request)
{
    for (int i =0; i < str.size(); ++i)
    {
        requests.push_front({&font.getChar(str[i]),request});
        //requests.push_front({&transFish,request});
    }
    //requests.push_front(request);
}

void FontManager::update()
{
    std::vector<float> data;
    data.resize(32);
    int  i= 0;
    while (requests.size() > 0)
    {
       auto it = requests.begin();
       int floats = i*it->first->getFloats();
       if (floats + it->first->getFloats() >= data.size()) //not enough room, gotta resize
       {
           data.resize((data.size())*2); //double size every time, hopefully will limit resize calls.
       }

       it->first->loadData(&data[0],it->second,floats);
       ++i;
       if (requests.size() == 1 || std::next(it)->first != it->first) //render all current sprite parameters in one go
       {
            //it->first->draw(program,&data[0],i);
            i = 0;
       }
       requests.pop_front();
       // fonts[i]->reset();
    }
    /*auto end = params.end();
    int i= 0;
    for (auto it = params.begin(); it != end; ++it)
    {
       //it->first.second->render(it->second,i/100.0);
       int floats = i*it->first->getFloats();
       if (floats + it->first->getFloats() >= data.size()) //not enough room, gotta resize
       {
           data.resize(data.size()*2); //double size every time, hopefully will limit resize calls.
       }
       SpriteParameter param = it->second;
       it->first->loadData(&data[0],param,floats);
       ++i;
       if (it == end || std::next(it)->first != it->first) //render all current sprite parameters in one go
       {
            it->first->draw(program,&data[0],i);
            i = 0;
       }
    }
    params.clear();
}*/

