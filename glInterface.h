#ifndef GLINTERFACE_H_INCLUDED
#define GLINTERFACE_H_INCLUDED

#include <memory>

#include "FreeTypeHelper.h"

#include "render.h"

class Button
{
protected:
    glm::vec4 rect;
    Font* font = nullptr;
    FontParameter paper;
    SpriteWrapper* sprite = nullptr;
    void (*toDo) () = nullptr;
public:
    Button(const glm::vec4& box, void (*func)(), SpriteWrapper* spr, const FontParameter& param, Font* font);
    Button(Button&& button);
    virtual void press();
    virtual void render(int x = 0, int y = 0); //x and y are the offset if we want to render the button relative to something
    const glm::vec4& getRect();
    void changeRect(const glm::vec4& rect);

};

class Interface;
class Window;
class WindowSwitchButton : public Button
{
    Interface* interface = nullptr;
    Window* switchTo = nullptr;
public:
    WindowSwitchButton(const glm::vec4& box,SpriteWrapper* spr, Interface& face, Window& to, const FontParameter& param, Font* font);
    void press();
};

class Window
{
protected:
    std::vector<std::unique_ptr<Button>> buttons;
    glm::vec4 rect;
    SpriteWrapper* sprite = nullptr;
    virtual void init(const glm::vec2& dimen, SpriteWrapper* spr);
    virtual void init(const glm::vec4& box, SpriteWrapper* spr);
public:
    Window(const glm::vec2& dimen, SpriteWrapper* spr); //if window is supposed to be centered
    Window(const glm::vec4& box, SpriteWrapper* spr); //if window doesn't need to be centered
    void addButton(Button* button); //treats this window's top right corner as the origin when adding this button
    virtual void update(int x, int y, bool clicked);//this function also renders the window. x,y, and clicked are where the mouse is and whether or not it clicked
};

class Interface //class that controls what windows are open;
{
    Window* current = nullptr;
public:
    void update();
    void switchCurrent(Window* window);
};

#endif // GLINTERFACE_H_INCLUDED
