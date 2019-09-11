#include "SDLhelper.h"
#include <iostream>

double DeltaTime::deltaTime;
double DeltaTime::lastTime = 0;

void DeltaTime::update()
{
    deltaTime = (SDL_GetTicks() - lastTime);
    lastTime = SDL_GetTicks();
}

void DeltaTime::set()
{
    setTime = SDL_GetTicks();
}
double DeltaTime::getTime()
{
    return setTime;
}
void DeltaTime::reset()
{
    setTime = -1;
}
bool DeltaTime::isSet()
{
    return setTime != -1;
}
bool DeltaTime::timePassed(double passed)
{
    return isSet() && SDL_GetTicks() - setTime >= passed;
}

SDL_Keycode KeyManager::justPressed = -1;
std::list<SDL_Keycode> KeyManager::numbers;
Uint32 KeyManager::lastEvent = 0;
SDL_Keycode KeyManager::lastKey = -1;
double KeyManager::getLatest()
{
    if (numbers.size() > 0)
    {

        return *(std::prev(numbers.end()));
    }
    else
    {
        return -1;
    }
}
SDL_Keycode KeyManager::getLater(double m, double n) //of m and n, finds which key which pressed later/recently.
{
    int size = numbers.size();
    if (size > 0)
    {
        for(std::list<SDL_Keycode>::iterator i = std::prev(numbers.end()); i != numbers.begin(); i --)
        {
            if ((*i) == m || (*i) == n)
                {
                    return (*i);
                }
        }
    }
    return -1;
}
void KeyManager::addNumber(double key)
{
    numbers.push_back(key);
}
int KeyManager::findNumber(double n) //finds the index of n or -1 if n isn't found
{
    int num = 0;
    for (std::list<SDL_Keycode>::iterator i = numbers.begin(); i != numbers.end(); i ++)
    {
        if ((*i) == n )
        {
            return num;
        }
        num ++;
    }
    return -1;
}

void KeyManager::getKeys(SDL_Event& e)
{
    int sym = e.key.keysym.sym;
    if (e.type != lastEvent || sym != lastKey)
    {
        if (e.type == SDL_KEYDOWN)
        {
            if (findNumber(sym) == -1)
            {
                addNumber(sym);
                justPressed = sym;
            }
        }
        else
        {
            if (e.type == SDL_KEYUP)
            {
                removeNumber(sym);
            }
        }
    }
    lastKey = sym;
    lastEvent = e.type;
    if (sym == justPressed)
    {
        justPressed = -1;
    }
    else
    {
        justPressed = sym;
    }

}
void KeyManager::removeNumber(double number)
{
    int index = findNumber(number);
    if (index != -1)
    {
        numbers.erase(std::next(numbers.begin(), index));
    }
}

SDL_Keycode KeyManager::getJustPressed()
{
    return justPressed;
}
bool MouseManager::left = false;
bool MouseManager::right = false;
bool MouseManager::middle = false;
int MouseManager::justClicked = -1;
bool* MouseManager::getButton(int key)
{
    switch (key)
    {
    case SDL_BUTTON_LEFT:
        return &left;
        break;
    case SDL_BUTTON_RIGHT:
        return &right;
        break;
    case SDL_BUTTON_MIDDLE:
        return &middle;
        break;
    default:
        throw "Wanted mouse button doesn't exist!";
    }
}
void MouseManager::update(SDL_Event& e)
{
    if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        bool* ptr = getButton(e.button.button);
        if (*ptr == false)
        {
            justClicked = e.button.button;
            *ptr = true;
        }
        else
        {
            justClicked = -1;
        }
    }
    else
    {
        justClicked = -1;
        if (e.type == SDL_MOUSEBUTTONUP)
        {
            *getButton(e.button.button) = false;
        }

    }
}

std::pair<int,int> MouseManager::getMousePos()
{
    int x, y;
    SDL_GetMouseState(&x,&y);

    return {x,y};
}

const bool MouseManager::isPressed(int key)
{
    return *getButton(key);
}

int MouseManager::getJustClicked()
{
    return justClicked;
}
