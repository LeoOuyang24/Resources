#include <SDL.h>

#include "SDLhelper.h"

#include "glInterface.h"


glm::vec4 Panel::scale(const glm::vec4& scaleRect)
{
    glm::vec4 renderRect = rect*glm::vec4(scaleRect.z,scaleRect.a,scaleRect.z,scaleRect.a);
    renderRect.x += scaleRect.x; //scale our button down first and then move it
    renderRect.y += scaleRect.y;
    return renderRect;

}
Panel::Panel(const glm::vec4& rect_, const glm::vec4& bColor, SpriteWrapper* spr, double z_ ) : rect(rect_), backgroundColor(bColor), sprite(spr), baseZ(z_)
{

}
const glm::vec4& Panel::getRect()
{
    return rect;
}
void Panel::changeRect(const glm::vec4& rect)
{
    this->rect = rect;
}
void Panel::update(float mouseX,float mouseY, float z, const glm::vec4& scale)
{

}
void Panel::update(float mouseX, float mouseY, float z)
{
    update(mouseX,mouseY,z + baseZ,{0,0,1,1});
}
void Panel::updateBlit(float z, const glm::vec4& blit)
{
    glm::vec4 scale = {blit.x - rect.x*blit.z/rect.z,blit.y - rect.y*blit.a/rect.a,blit.z/rect.z,blit.a/rect.a};
    glm::vec2 mousePos = RenderProgram::toAbsolute(pairtoVec(MouseManager::getMousePos()));
    update(mousePos.x,mousePos.y,z,scale);
}

Message::Message(const glm::vec4& box, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc)(), double z_ ) :
                Panel(box,color,spr,z_), font(font), paper(param), dynamicString(strFunc)
{

}

void Message::update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect)
{
    glm::vec4 renderRect = scale(scaleRect);
   // std::cout << rect.x << " " << rect.y << std::endl;
    if (!sprite && backgroundColor.a > 0) //if the sprite would over lap the background color or the backgroundColor is transparent, don't render it
    {
        PolyRender::requestRect(renderRect,backgroundColor,true,0,baseZ + z);
    }
    else if (sprite)
    {
        sprite->request({renderRect,0,NONE,{1,1,1,1},&RenderProgram::basicProgram,baseZ + z});
    }
    if (font)
    {
        std::string print = paper.text;
        if (dynamicString)
        {
            print = dynamicString();
        }
        font->requestWrite({print,renderRect,paper.angle,paper.color,baseZ + z + .1});
    }
}


Button::Button(const glm::vec4& box,  void (*func)(), SpriteWrapper* spr,
               const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc) (), double z_ ) :
               Message( box,spr,param,font,color,strFunc,z_), toDo(func), baseColor(color), original(param)
{

}
void Button::press()
{
    if (toDo != nullptr)
    {
        toDo();
    }
}

void Button::hover()
{
    paper.color *= .5;
    backgroundColor *= .5;
}

void Button::render(bool hover, float x, float y, float z, float xScale, float yScale)
{
   // std::cout << rect.x << " " << rect.y << std::endl;
    glm::vec4 renderRect = scale({x,y,xScale,yScale});
    if (!sprite && backgroundColor.a > 0) //if the sprite would over lap the background color or the backgroundColor is transparent, don't render it
    {
        glm::vec4 color = backgroundColor;
        if (hover)
        {
            color *= .5;
        }
        PolyRender::requestRect(renderRect,backgroundColor,true,0,z);
    }
    else if (sprite)
    {
        sprite->request({renderRect,0,NONE,{1,1,1,1},&RenderProgram::basicProgram,z});

    }
    if (font)
    {
        font->requestWrite({paper.text,renderRect,paper.angle,paper.color,z + .1});
    }
}

void Button::update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect)
{
    glm::vec4 renderRect = scale(scaleRect);
  //  printRect(renderRect);
    //printRect(scaleRect);
    if (pointInVec(renderRect,mouseX,mouseY,0))
    {
        hover();
        if (MouseManager::getJustClicked() == SDL_BUTTON_LEFT)
        {
            //printRect(rect);
            press();
        }
    }
    Message::update(mouseX,mouseY,z, scaleRect);
    paper = original;
    backgroundColor = baseColor;
}

WindowSwitchButton::WindowSwitchButton(const glm::vec4& box, SpriteWrapper* spr, Interface& face, Window& to, const FontParameter& param, Font* font, const glm::vec4& color) : Button(box,nullptr,spr, param, font, color)
{
    interface = &face;
    switchTo = &to;
}

void WindowSwitchButton::press()
{
    if (interface && switchTo)
    {
        interface->switchCurrent(switchTo);
    }
}

Window::Window(const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg, double z_) : Panel({ dimen.x/2, dimen.y/2,dimen.x,dimen.y},bg,spr,z_)
{
    rect.z += (rect.z == 0) * RenderProgram::getScreenDimen().x; //set dimensions that are 0 to that of the full screen size
    rect.a += (rect.a == 0) * RenderProgram::getScreenDimen().y;
}

Window::Window( const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg, double z_) : Panel( box,bg,spr, z_)
{
    rect.z += (rect.z == 0) * RenderProgram::getScreenDimen().x; //set dimensions that are 0 to that of the full screen size
    rect.a += (rect.a == 0) * RenderProgram::getScreenDimen().y;
}

void Window::setDoUpdate(bool val)
{
    doUpdate = val;
}

bool Window::getDoUpdate()
{
    return doUpdate;
}

int Window::countPanels()
{
    return panels.size();
}

void Window::addPanel(Panel& button)
{
    panels.emplace_back(&button);
    button.changeRect(button.getRect() + glm::vec4({rect.x,rect.y,0,0}));
}

void Window::update(float x, float y, float z, const glm::vec4& blit)
{
    if (doUpdate)
    {
        glm::vec4 renderRect = scale(blit);
        int size = panels.size();
        for (int i = 0; i < size; ++i)
        {
            bool hover = pointInVec(panels[i].get()->getRect(),x,y,0);
            panels[i].get()->update(x,y,baseZ + z + 0.1,blit);
            /*if (clicked && hover)
            {
                panels[i].get()->press();
            }*/
        }
        if (sprite)
        {
            sprite->request({renderRect,0,NONE,{1,1,1,1},&RenderProgram::basicProgram,z + baseZ});
        }
        else if (backgroundColor.a > 0)
        {
            PolyRender::requestRect(renderRect,backgroundColor,true,0,z + baseZ);
        }
    }
}

void Window::updateTop(float z, const glm::vec4& blit)
{
   /* glm::vec4 scale = {blit.x - rect.x*blit.z/rect.z,blit.y - rect.y*blit.a/rect.a,blit.z/rect.z,blit.a/rect.a};
    double x = MouseManager::getMousePos().first*scale.z - scale.x; //scale the mouse Position
    double y = MouseManager::getMousePos().second*scale.a - scale.y;*/
    updateBlit(z,blit);
}

void Window::updateTop(float z)
{
    updateTop(z,rect);
}

void Window::switchTo(Window& other)
{

}

void Window::onSwitch(Window& previous)
{

}

OnOffButton::OnOffButton(OnOffMode mode, Window& window, const glm::vec4& rect, SpriteWrapper* spr, const FontParameter& param, Font* font,
                         const glm::vec4& color) : Button(rect,nullptr,spr,param,font,color), mode(mode), target(&window)
{

}

void OnOffButton::press()
{
    if (target)
    {
        if (mode == OnOffMode::DYNAMIC)
        {
            target->setDoUpdate(!target->getDoUpdate());
        }
        else
        {
            target->setDoUpdate((int)(mode)-1);
        }
    }
}

void Interface::update()
{
    if (current)
    {
        current->updateTop(0);
    }
}

void Interface::switchCurrent(Window* window)
{
    if (current)
    {
        current->switchTo(*window);
    }
    if (window)
    {
        window->onSwitch(*current);
    }
    RenderProgram::resetRange();
    current = window;

}
