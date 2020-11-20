#ifndef GLINTERFACE_H_INCLUDED
#define GLINTERFACE_H_INCLUDED

#include <memory>

#include "FreeTypeHelper.h"

#include "render.h"

class Panel //parent class of anything that goes on a window, including other windows and buttons
{
protected:
    glm::vec4 scale(const glm::vec4& scaleRect);
    glm::vec4 rect, backgroundColor;
    SpriteWrapper* sprite = nullptr;
    double baseZ = 0; //will be added to the z passed in the update functions. In Windows, also added to child panels

public:
    Panel(const glm::vec4& rect_, const glm::vec4& bColor, SpriteWrapper* spr, double z_ = 0);
    const glm::vec4& getRect();
    void changeRect(const glm::vec4& rect);
    virtual void update(float mouseX,float mouseY, float z, const glm::vec4& scale); //mouseX and mouseY are the coordinates of the mouse and should
                                            //already be scaled and not have to be modified, z is the z coordinate to render to,
                                            //scale is NOT the rect that we want to project to; the x,y is the x-y increment to render to, and the z and a are the x and yscale respectively
    void update(float mouseX, float mouseY, float z); //same as other update function except we don't worry about scaling
    void updateBlit(float z, const glm::vec4& blit); //blits to a rect and updates
};

class Button : public Panel
{
protected:
    const glm::vec4 baseColor; //base background color. Allows us to use backgroundColor to temporarily set the button color; good for changing color when the mouse is hovering.
    Font* font = nullptr;
    FontParameter paper;
    const FontParameter original; // original is the original font settings. We set paper back to original every iteration
    void (*toDo) () = nullptr;
public:
    using Panel::update;

    Button(const glm::vec4& box, void (*func)(), SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color, double z_ = 0);
    virtual void press();
    virtual void hover(); //what to do if the mouse hovers over this button
    virtual void render(bool mouseHover,float x = 0.0f, float y = 0.0f, float z = 0.0f, float xScale = 1, float yScale= 1); //x and y are the offset if we want to render the button relative to something
    virtual void update(float mouseX, float mouseY, float z, const glm::vec4& blit);
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

class Window : public Panel
{
protected:
    std::vector<std::unique_ptr<Panel>> panels;
    bool doUpdate = true; //whether to update this window or not
public:
    Window( const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg, double z_ = 0); //if window is supposed to be centered
    Window( const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg, double z_ = 0); //if window doesn't need to be centered
    void setDoUpdate(bool val);
    bool getDoUpdate();
    int countPanels();
    void addPanel(Panel& button); //Adds button relative to top right corner
    virtual void update(float x, float y, float z, const glm::vec4& scale);//this function also renders the window. x,y, and clicked are where the mouse is and whether or not it clicked
    virtual void updateTop(float z, const glm::vec4& blit); //given a rect, calculates the scaling rect. Useful for windows that have no parent windows
    virtual void updateTop(float z); //same as other updateTop except blit is our own rect. Basically no scaling
    virtual void switchTo(Window& swapTo); //called when swapping away from this window. swapTo is the new window
    virtual void onSwitch(Window& previous); //called when swapping to this window. previous is the previous window
};

enum class OnOffMode
{
    DYNAMIC, //set doUpdate to !doUpdate
    OFF, //set doUpdate to false
    ON //set doUpdate to true
};

class OnOffButton : public Button //sets a window's DoUpdate to a certain value.
{
public:

    OnOffButton(OnOffMode mode, Window& window, const glm::vec4& rect, SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color);
    void press();
private:
    OnOffMode mode = OnOffMode::DYNAMIC;
    Window* target = nullptr;
};

class Interface //class that controls what windows are open;
{
    Window* current = nullptr;
public:
    void update();
    void switchCurrent(Window* window);
};

#endif // GLINTERFACE_H_INCLUDED
