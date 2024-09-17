#include <iostream>

#include "FreeTypeHelper.h"
#include "geometry.h"
#include "resourcesMaster.h"

std::unique_ptr<BasicRenderPipeline> FontGlobals::wordProgram;
Font FontGlobals::tnr;


Character::Character(char c) : Sprite()
{
    letter = c;

    //characters are ALWAYS transluscent. They are always a mix of red pixels and black pixels, the latter of which the fragment shader then deletes
    //by that definition, they are technically always opaque but because the black pixels are basically transparent, they are actually always transluscent
    transluscent = true;

}

Character::Character(char c, FT_Face& face) : Character(c)
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
    size =     glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
    bearing =  glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
    advance =    face->glyph->advance.x;

}

const glm::ivec2& Character::getBearing()
{
    return bearing;
}
const glm::ivec2& Character::getSize()
{
    return size;
}
GLuint Character::getAdvance()
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

    void FontGlobals::init(int screenWidth, int screenHeight)
    {
        std::string source = templateShader(stripComments(readFile(ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/vertex/betterShader.h").first), true,
                                            {"vec4 color"},
                                            {"vec4 shade"},
                                            {"shade = color"});
        wordProgram = std::unique_ptr<BasicRenderPipeline>(new BasicRenderPipeline({LoadShaderInfo{source,GL_VERTEX_SHADER,false},
                                                                                   LoadShaderInfo{ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/shaders/fragment/wordFragment.h",GL_FRAGMENT_SHADER,true}}));

        tnr.init(ResourcesConfig::config[ResourcesConfig::RESOURCES_DIR] + "/tnr.ttf");
    }

    Font::Font(std::string source)
    {
        init(source);
    }

glm::vec2 Font::getDimen(std::string text, GLfloat hScale, GLfloat vScale)
{
    std::string::const_iterator c;
    double totalWidth = 0, maxHeight = maxVert.x + maxVert.y, maxWidth = 0;;
    auto end = text.end();
   // std::cout << hScale << " " << vScale << std::endl;
    for (c = text.begin(); c != end; ++c)
    {
        if (*c != '\n')
        {
            Character* ch = (characters[*c].get());
            GLfloat h = ch->getSize().y * vScale;
            totalWidth += (ch->getAdvance())*hScale/64;
            //maxHeight += (h > maxHeight)*(h-maxHeight);
        }
        else
        {
            maxHeight += maxVert.x + maxVert.y;
            maxWidth = totalWidth > maxWidth ? totalWidth : maxWidth;
            totalWidth = 0;
        }


    }
    maxWidth = totalWidth > maxWidth ? totalWidth : maxWidth;
    return {maxWidth,maxHeight*vScale};
}

void Font::requestWrite(const FontParameter& param, BasicRenderPipeline& pipeline)
{
    glm::vec4 absRect = absoluteValueRect(param.rect);


    float scale = 1.0f;

    if (param.scale > 0) //if the scale is positive, just mindless use it
    {
        scale = param.scale;
    }
    else
    {

        glm::vec2 dimen = getDimen(param.text,1,1);

        scale = dimen.x*abs(param.scale) > absRect.z || dimen.y*abs(param.scale) > absRect.a ? //if the scale is negative, we may want have to clamp to
                        std::min(absRect.z/dimen.x,absRect.a/dimen.y) ://the size of the rect instead
                        abs(param.scale);
    }


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
       // PolyRender::requestRect(glm::vec4(x,y,getDimen(param.text,scale,scale)),glm::vec4(1,0,0,1),false,0,1);
        break;
    case DOWN:
        y += param.rect.a - getDimen(param.text,1,scale).y;
        break;
    }

    int startX = x;
    int size = param.text.size();
    //PolyRender::requestRect(param.rect,{1,0,0,1},false,0,1);
    //PolyRender::requestRect(absRect,{1,0,1,1},false,0,1);

    for (int i = 0; i < size; ++i)
    {
        char c = param.text[i];
        if (c == '\n')
        {
            y += (maxVert.x + maxVert.y)*scale;
            x = startX;
            continue;
        }
        Character* ch = (characters[c].get());
        glm::ivec2 bearing = ch->getBearing();
        glm::ivec2 chSize = ch->getSize();
        GLfloat xpos = x + bearing.x*scale;
        GLfloat ypos = y+ (maxVert.x - bearing.y)*scale;
        //std::cout << y << " " << absRect.y << " " << (maxVert.x - bearing.y)  << "\n";

        //glm::vec2 pos = rotatePoint({xpos,ypos},center,param.angle);
        glm::vec2 pos = {xpos,ypos};
        GLfloat w = chSize.x*scale;
        GLfloat h = (chSize.y)*scale;

        //PolyRender::requestRect(glm::vec4(xpos,ypos,w,h),glm::vec4(1,0,0,1),false,0,1);


        glm::vec4 finalRect = glm::vec4(rotateRect({xpos,ypos,w,h},{param.rect.x + param.rect.z/2,param.rect.y + param.rect.a/2},param.angle));

        SpriteManager::requestSprite({pipeline,characters[c].get()},finalRect,param.z,param.angle,0,param.color);

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

