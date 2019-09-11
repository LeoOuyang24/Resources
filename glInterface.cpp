#include <SDL.h>

#include "SDLhelper.h"

#include "glInterface.h"


Button::Button(const glm::vec4& box,  void (*func)(), SpriteWrapper& spr, const FontParameter& param, Font* font)
{
    this->font = font;
    changeRect(box);
    toDo = func;
    this->paper = param;
    sprite = &spr;
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
}

void Button::changeRect(const glm::vec4& rect)
{
    this->rect = rect;
}

void Button::press()
{
    if (toDo != nullptr)
    {
        toDo();
    }
}

void Button::render()
{
    sprite->request({rect});
    if (font)
    {
        font->write(Font::wordProgram,paper);
    }
}

const glm::vec4& Button::getRect()
{
    return rect;
}

WindowSwitchButton::WindowSwitchButton(const glm::vec4& box, SpriteWrapper& spr, Interface& face, Window& to, const FontParameter& param, Font* font) : Button(box,nullptr,spr, param, font)
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

Window::Window(const glm::vec2& dimen, SpriteWrapper* spr)
{
    init(dimen,spr);
}

Window::Window(const glm::vec4& box, SpriteWrapper* spr)
{
    init(box, spr);
}

void Window::init(const glm::vec2& dimen, SpriteWrapper* spr) //if window is supposed to be centered
{
    glm::vec2 screenDimen = RenderProgram::getScreenDimen();
    rect = {screenDimen.x/2 - dimen.x/2, screenDimen.y/2 - dimen.y/2, dimen.x, dimen.y };
    sprite = spr;
}

void Window::init(const glm::vec4& box, SpriteWrapper* spr)
{
    rect = box;
    sprite = spr;
}

void Window::addButton(Button* button)
{
    if (button)
    {
        buttons.emplace_back(button);
        const glm::vec4* ptr = &(button->getRect());
        button->changeRect({ptr->x + rect.x, ptr->y + rect.y, ptr->z, ptr->a});
    }
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
