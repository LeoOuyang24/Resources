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
    glm::vec4 backgroundColor;
    void (*toDo) () = nullptr;
public:
    Button(const glm::vec4& box, void (*func)(), SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color);
    Button(Button&& button);
    virtual void press();
    virtual void render(float x = 0.0f, float y = 0.0f, float z = 0.0f, float xScale = 1, float yScale= 1); //x and y are the offset if we want to render the button relative to something
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
    WindowSwitchButton(const glm::vec4& box,SpriteWrapper* spr, Interface& face, Window& to, const FontParameter& param, Font* font, const glm::vec4& color);
    void press();
};

class Window
{
protected:
    std::vector<std::unique_ptr<Button>> buttons;
    glm::vec4 rect;
    glm::vec4 backgroundColor;
    SpriteWrapper* sprite = nullptr;
    virtual void init(const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg);
    virtual void init(const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg);
public:
    Window(const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg); //if window is supposed to be centered
    Window(const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg); //if window doesn't need to be centered
    const glm::vec4& getRect();
    int countButtons();
    void addButton(Button& button); //Adds button relative to top right corner
    virtual void update(int x, int y, int z, bool clicked, const glm::vec4& rect);//this function also renders the window. x,y, and clicked are where the mouse is and whether or not it clicked
    virtual void update(int x, int y, int z, bool clicked); //renders with the class's rect
};

class Interface //class that controls what windows are open;
{
    Window* current = nullptr;
public:
    void update();
    void switchCurrent(Window* window);
};

#endif // GLINTERFACE_H_INCLUDED
