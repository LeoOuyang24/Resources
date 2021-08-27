#ifndef GLINTERFACE_H_INCLUDED
#define GLINTERFACE_H_INCLUDED

#include <memory>

#include "FreeTypeHelper.h"
#include "SDLhelper.h"

#include "render.h"

class Panel //parent class of anything that goes on a window, including other windows and buttons. DOES NOT RENDER ANYTHING; create children for functionality
{
protected:
    float baseZ = 0; //z of the panel. Added to param.z
    glm::vec4 scale(const glm::vec4& scaleRect);
    glm::vec4 rect, backgroundColor;
    SpriteWrapper* sprite = nullptr;
    SpriteParameter param; //used to modify the spriteParameter. rect is effected by the Panel.Rect, rather than the param.rect
    const SpriteParameter originalSprite;
    bool dead = false;
    bool doUpdate = true; //whether to update this window or not

public:
    Panel(const glm::vec4& rect_, const glm::vec4& bColor, SpriteWrapper* spr, double z_ = 0);
    void setDoUpdate(bool val);
    bool getDoUpdate();
    const glm::vec4& getRect();
    bool getDead();
    void changeRect(const glm::vec4& rect);
    void setSprite(SpriteWrapper* sprite);
    virtual void update(float mouseX,float mouseY, float z, const glm::vec4& scale); //mouseX and mouseY are the coordinates of the mouse and should
                                            //already be scaled and not have to be modified, z is the z coordinate to render to,
                                            //scale is NOT the rect that we want to project to; the x,y is the x-y increment to render to, and the z and a are the x and yscale respectively
    void update(float mouseX, float mouseY, float z); //same as other update function except we don't worry about scaling
    void updateBlit(float z, const glm::vec4& blit); //blits to a rect and updates
    void updateBlit(float z, RenderCamera& camera, bool absolute);
    void updateBlit(float z, RenderCamera& camera, bool absolute, const glm::vec4& blit); //renders at blit location
};

class Message : public Panel //a very simple rect that displays messages and sprites
{
protected:
    Font* font = nullptr;
    const FontParameter originalPaper; // original is the original font settings. We set paper back to original every iteration
    FontParameter paper;
    std::string (*dynamicString) () = nullptr; //sometimes, we want to print a variable that changes. This function allows us to do that
public:
    Message(const glm::vec4& box, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc)(), double z_ = 0);
    virtual void update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect);
};

class Ticker : public Message
{
    int milliseconds;
    DeltaTime time;
    float baseY = 0;
public:
    Ticker(int duration,const glm::vec4& box, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc)(), double z_ = 0);
    virtual void update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect);
};

class Button : public Message
{
protected:
    const glm::vec4 baseColor;
    void (*toDo) () = nullptr;
public:
    using Panel::update;

    Button(const glm::vec4& box, void (*func)(), SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, std::string (*strFunc)() = nullptr, double z_ = 0);
    virtual void press();
    virtual void hover(); //what to do if the mouse hovers over this button
    virtual void render(float x, float y, float z, const glm::vec4& scaleRect); // just calls Message::update() by default. Can be modified for more flexibility
    virtual void update(float mouseX, float mouseY, float z, const glm::vec4& scaleRect);
};

class Interface;
class Window;
class WindowSwitchButton : public Button
{
    Interface* interface = nullptr;
    Window* switchTo = nullptr;
public:
    WindowSwitchButton(const glm::vec4& box,SpriteWrapper* spr, Interface& face, Window& to, const FontParameter& param, Font* font, const glm::vec4& color);
    void press();
};

class CondSwitchButton : public WindowSwitchButton // a Window Switch Button that i sonly active under certain conditions
{
    bool (*cond) () = nullptr;
public:
    CondSwitchButton(const glm::vec4& box,SpriteWrapper* spr, Interface& face, Window& to, const FontParameter& param, Font* font, const glm::vec4& color,
                     bool (*condFunc)());
    void render(float x, float y, float z, const glm::vec4& scaleRect);
    virtual bool doSwitch(); //whether to switch or not
    void press();
};


class Window : public Panel
{
    typedef std::pair<std::unique_ptr<Panel>,bool> PanelPair; //panel and whether to render it absoltue or not. Unique ptr because panels should only be owned by the Window, nothing else
protected:
    std::list<PanelPair> panels;
    RenderCamera* camera = nullptr;
public:
    Window( const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg, double z_ = 0); //if window is supposed to be centered
    Window( const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg, double z_ = 0); //if window doesn't need to be centered
    int countPanels();
    void removePanel(Panel& button);
    void addPanel(Panel& button, bool absolute = false); //Adds button relative to top right corner
    void setCamera(RenderCamera* cam);
    virtual void update(float x, float y, float z, const glm::vec4& scale);//this function also renders the window. x,y, and clicked are where the mouse is and whether or not it clicked
    virtual void updateTop(float z, const glm::vec4& blit); //given a rect, calculates the scaling rect. Useful for windows that have no parent windows
    virtual void updateTop(float z); //same as other updateTop except blit is our own rect. Basically no scaling
    virtual void switchTo(Window& swapTo); //called when swapping away from this window. swapTo is the new window
    virtual void onSwitch(Window& previous); //called when swapping to this window. previous is the previous window
    virtual void requestNGon(int n,const glm::vec2& center,double side,const glm::vec4& color,double angle,bool filled,float z);
    virtual void requestRect(const glm::vec4& rect, const glm::vec4& color, bool filled, double angle, float z);
    virtual void requestLine(const glm::vec4& line, const glm::vec4& color, float z);
};

enum class OnOffMode
{
    DYNAMIC, //set doUpdate to !doUpdate
    OFF, //set doUpdate to false
    ON //set doUpdate to true
};

class OnOffButton : public Button //alternates between two windows
{
public:
    OnOffButton(Window& window1,Window& window2, const glm::vec4& rect, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color);
    void press();
private:
    Window* target1 = nullptr;
    Window* target2 = nullptr;
};

class Interface //class that controls what windows are open;
{
    Window* current = nullptr;
public:
    void update();
    void switchCurrent(Window* window);
};

#endif // GLINTERFACE_H_INCLUDED
