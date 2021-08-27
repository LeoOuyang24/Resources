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


void Panel::setDoUpdate(bool val)
{
    doUpdate = val;
}

bool Panel::getDoUpdate()
{
    return doUpdate;
}

const glm::vec4& Panel::getRect()
{
    return rect;
}
bool Panel::getDead()
{
    return dead;
}

void Panel::changeRect(const glm::vec4& rect)
{
    this->rect = rect;
}

void Panel::setSprite(SpriteWrapper* sprite)
{
    this->sprite = sprite;
}

void Panel::update(float mouseX,float mouseY, float z, const glm::vec4& scale)
{
    param = originalSprite;
}
void Panel::update(float mouseX, float mouseY, float z)
{
    update(mouseX,mouseY,z,{0,0,1,1});
}
void Panel::updateBlit(float z, const glm::vec4& blit)
{
    glm::vec4 scale = {blit.x - rect.x*blit.z/rect.z,blit.y - rect.y*blit.a/rect.a,blit.z/rect.z,blit.a/rect.a};
    auto mousePos = MouseManager::getMousePos();
    update(mousePos.first,mousePos.second,z,scale);
}

void Panel::updateBlit(float z, RenderCamera& camera, bool absolute)
{
    updateBlit(z,camera,absolute,rect);
}

void Panel::updateBlit(float z, RenderCamera& camera, bool absolute, const glm::vec4& blit)
{
    glm::vec4 renderRect, scale;
    glm::vec2 mousePos;
    if (absolute)
    {
       mousePos = camera.toAbsolute(pairtoVec(MouseManager::getMousePos()));
        renderRect =     camera.toAbsolute(blit);
    }
    else
    {
       mousePos = camera.toScreen(camera.toWorld(pairtoVec(MouseManager::getMousePos()))); //we have to convert to toScreen because buttons make checks based on their rect in the screenSpace after scaling
        renderRect =     camera.toScreen(blit);
    }
       scale = {renderRect.x - rect.x*renderRect.z/rect.z,renderRect.y - rect.y*renderRect.a/rect.a,renderRect.z/rect.z,renderRect.a/rect.a};
     //  PolyRender::requestNGon(10,mousePos,10,{1,0,0,1},0,true,1);
    update(mousePos.x,mousePos.y,z,scale);
}

Message::Message(const glm::vec4& box, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc)(), double z_ ) :
                Panel(box,color,spr,z_), font(font), paper(param), originalPaper(param),dynamicString(strFunc)
{

}

void Message::update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect)
{
    glm::vec4 renderRect = scale(scaleRect);
   // std::cout << rect.x << " " << rect.y << std::endl;
    if (!sprite && backgroundColor.a > 0) //if the sprite would over lap the background color or the backgroundColor is transparent, don't render it
    {
        PolyRender::requestRect(renderRect,backgroundColor,true,0,baseZ + param.z + z);
    }
    else if (sprite)
    {
        sprite->request({renderRect,param.radians,param.effect,param.tint,param.program,baseZ + param.z+ z,param.portion});
    }
    if (font)
    {
        std::string print = paper.text;
        if (dynamicString)
        {
            print = dynamicString();
        }
        font->requestWrite({print,renderRect,paper.angle,paper.color,baseZ + param.z + z + .1});
    }
    paper = originalPaper;

    Panel::update(mouseX,mouseY,z,scaleRect);
}

Ticker::Ticker(int duration, const glm::vec4& box, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc)(), double z_ ) :
    Message(box,spr,param,font,color,strFunc,z_), milliseconds(duration)
{
   time.set();
   baseY = box.y;
}

void Ticker::update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect)
{
    if (time.timePassed(milliseconds))
    {
        dead = true;
    }
    float seconds = time.getTimePassed()/1000.0;
    rect.y = baseY - log(time.getTimePassed())*10;

    paper.color.a = 1.0/(seconds);
    param.tint.a = 1.0/(seconds);
    Message::update(mouseX,mouseY,z,scaleRect);
}

Button::Button(const glm::vec4& box,  void (*func)(), SpriteWrapper* spr,
               const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc) (), double z_ ) :
               Message( box,spr,param,font,color,strFunc,z_), toDo(func), baseColor(color)
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

void Button::render(float x, float y, float z, const glm::vec4& scaleRect)
{
    Message::update(x,y,z, scaleRect);
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
    render(mouseX,mouseY,z, scaleRect);
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

CondSwitchButton::CondSwitchButton(const glm::vec4& box,SpriteWrapper* spr, Interface& face, Window& to, const FontParameter& param, Font* font, const glm::vec4& color,
                     bool (*condFunc)()) : WindowSwitchButton(box,spr,face,to,param,font,color), cond(condFunc)
{

}

void CondSwitchButton::render(float x, float y, float z, const glm::vec4& scaleRect)
{
    if (!doSwitch())
    {
        backgroundColor *= glm::vec4(glm::vec3(.5),1);
    }
    Button::render(x,y,z,scaleRect);
}

bool CondSwitchButton::doSwitch()
{
    return cond && cond();
}

void CondSwitchButton::press()
{
    if (doSwitch())
    {
        WindowSwitchButton::press();
    }
}

Window::Window(const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg, double z_) : Panel({ 0,0,dimen.x,dimen.y},bg,spr,z_)
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    rect.z += (rect.z == 0) * screenDimen.x; //set dimensions that are 0 to that of the full screen size
    rect.a += (rect.a == 0) * screenDimen.y;
    rect.x = screenDimen.x/2 - rect.z/2;
    rect.y = screenDimen.y/2 - rect.a/2;
}

Window::Window( const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg, double z_) : Panel( box,bg,spr, z_)
{
    rect.z += (rect.z == 0) * RenderProgram::getScreenDimen().x; //set dimensions that are 0 to that of the full screen size
    rect.a += (rect.a == 0) * RenderProgram::getScreenDimen().y;
}

int Window::countPanels()
{
    return panels.size();
}

void Window::removePanel(Panel& button)
{
    auto end = panels.end();
    for (auto it = panels.begin(); it != end; ++it)
    {
        if ((*it).first.get() == &button)
        {
            panels.erase(it);
            break;
        }
    }
}

void Window::addPanel(Panel& button, bool absolute)
{
    panels.push_back({std::unique_ptr<Panel>(&button),absolute});
    button.changeRect(button.getRect() + glm::vec4({rect.x,rect.y,0,0}));
}

void Window::setCamera(RenderCamera* cam)
{
    camera = cam;
}

void Window::update(float x, float y, float z, const glm::vec4& blit)
{
    if (doUpdate)
    {
        glm::vec4 renderRect = scale(blit);
        if (sprite)
        {
            sprite->request({renderRect,0,NONE,{1,1,1,1},&RenderProgram::basicProgram,z + param.z});
        }
        else if (backgroundColor.a > 0)
        {
            PolyRender::requestRect(renderRect,backgroundColor,true,0,z + param.z);
        }
        auto end = panels.end();
        for (auto i = panels.begin(); i != end;)
        {
            if (i->first.get()->getDoUpdate())
            {
                bool hover = pointInVec(i->first.get()->getRect(),x,y,0);
                if (camera)
                {
                    i->first.get()->updateBlit(baseZ + param.z + z + .1,*camera, i->second);
                }
                else
                {
                    i->first.get()->update(x,y,baseZ + param.z+ z + .1,blit);
                }
                if (i->first->getDead())
                {
                    i = panels.erase(i);
                }
                else
                {
                    ++i;
                }
            }
            else
            {
                ++i;
            }

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

void Window::requestNGon(int n,const glm::vec2& center,double side,const glm::vec4& color,double angle,bool filled,float z)
{
    glm::vec2 newCenter = camera ? camera->toScreen(center) : center;
    PolyRender::requestNGon(n,newCenter,side,color,angle,filled,z);
}

void Window::requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z)
{
    glm::vec4 newRect = camera ? camera->toScreen(rect) : rect;
    PolyRender::requestRect(newRect, color, filled, angle, z);
}

void Window::requestLine(const glm::vec4& line, const glm::vec4& color, float z)
{

    glm::vec4 newLine = camera ? glm::vec4(camera->toScreen({line.x,line.y}),camera->toScreen({line.z,line.a})) : line;
    PolyRender::requestLine(newLine,color,z);
}

OnOffButton::OnOffButton(Window& window1, Window& window2, const glm::vec4& rect, SpriteWrapper* spr, const FontParameter& param, Font* font,
                         const glm::vec4& color) : Button(rect,nullptr,spr,param,font,color), target1(&window1), target2(&window2)
{

}

void OnOffButton::press()
{
    if (target1 && target2)
    {
        bool onOne = target1->getDoUpdate();
        target2->setDoUpdate(onOne); //turn one off/on and turn 2 on/off
        target1->setDoUpdate(!onOne);
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
