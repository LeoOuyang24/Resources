#include <SDL.h>

#include "SDLhelper.h"

#include "glInterface.h"


Button::Button(const glm::vec4& box,  void (*func)(), SpriteWrapper* spr, const FontParameter& param, Font* font, const glm::vec4& color)
{
    this->font = font;
    changeRect(box);
    toDo = func;
    this->paper = param;
    sprite = spr;
    backgroundColor = color;

}

Button::Button(Button&& button)
{
    //std::cout << "moving!" << std::endl;
    changeRect(button.rect);
    toDo = button.toDo;
    sprite = button.sprite;
    this->font = button.font;
    this->paper = button.paper;
    button.font = nullptr;
    button.sprite = nullptr;
    button.toDo = nullptr;
    backgroundColor = button.backgroundColor;
}

void Button::changeRect(const glm::vec4& rect)
{
    this->rect = rect;
    paper.rect = rect;
}

void Button::press()
{
    if (toDo != nullptr)
    {
        toDo();
    }
}

void Button::render(int x, int y)
{
    glm::vec4 renderRect = paper.rect;
    renderRect.x += x;
    renderRect.y += y;
    if (!sprite && backgroundColor.a > 0) //if the sprite would over lap the background color or the backgroundColor is transparent, don't render it
    {
        PolyRender::requestRect({renderRect.x, renderRect.y,rect.z, rect.a},backgroundColor,true,0,0);
    }
    else if (sprite)
    {
        sprite->request({{renderRect.x, renderRect.y,rect.z, rect.a}});
    }
    if (font)
    {
        FontParameter copyPaper = paper;
        copyPaper.rect = renderRect;
        font->requestWrite({paper.text,renderRect,paper.angle,paper.color,paper.z});
    }
}

const glm::vec4& Button::getRect()
{
    return rect;
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

Window::Window(const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg)
{
    init(dimen,spr,bg);
}

Window::Window(const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg)
{
    init(box, spr,bg);
}

void Window::init(const glm::vec2& dimen, SpriteWrapper* spr, const glm::vec4& bg) //if window is supposed to be centered
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    rect = {screenDimen.x/2 - dimen.x/2, screenDimen.y/2 - dimen.y/2, dimen.x, dimen.y };
    sprite = spr;
    backgroundColor = bg;
}

void Window::init(const glm::vec4& box, SpriteWrapper* spr, const glm::vec4& bg)
{
    rect = box;
    sprite = spr;
    backgroundColor = bg;
}

void Window::addButton(Button& button)
{
    buttons.emplace_back(&button);
    const glm::vec4* ptr = &(button.getRect());
    button.changeRect({ptr->x + rect.x, ptr->y + rect.y, ptr->z, ptr->a});
}

void Window::update(int x, int y, bool clicked)
{
    int size = buttons.size();
    for (int i = 0; i < size; ++i)
    {
        buttons[i].get()->render();
        if (clicked && pointInVec(buttons[i].get()->getRect(),x,y,0))
        {
            buttons[i].get()->press();
        }
    }
    if (sprite)
    {
        sprite->request({{rect}});
    }
    else if (backgroundColor.a > 0)
    {
        PolyRender::requestRect(rect,backgroundColor,true,0,0);
    }
}

void Interface::update()
{
    if (current)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        current->update(x,y,MouseManager::getJustClicked() == SDL_BUTTON_LEFT);
    }
}

void Interface::switchCurrent(Window* window)
{
    current = window;
}
