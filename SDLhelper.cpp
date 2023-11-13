#include "SDLhelper.h"
#include <iostream>

int DeltaTime::deltaTime;
int DeltaTime::lastTime = 0;
gameFrame DeltaTime::currentFrame = 0;

void DeltaTime::update()
{
    deltaTime = (SDL_GetTicks() - lastTime);
    lastTime = SDL_GetTicks();
    currentFrame ++;
}

gameFrame DeltaTime::getCurrentFrame()
{
    return currentFrame;
}

void DeltaTime::set()
{
    setTime = SDL_GetTicks();
    setFrame = currentFrame;
}

void DeltaTime::reset()
{
    setTime = -1;
    setFrame = -1;
}
bool DeltaTime::isSet()
{
    return setTime != -1;
}

bool DeltaTime::framesPassed(gameFrame passed)
{
    //std::cout << currentFrame - setFrame << std::endl;
    return isSet() && currentFrame - setFrame >= passed;
}

bool DeltaTime::timePassed(double passed)
{
    return isSet() && std::max((int)(SDL_GetTicks() - setTime),1) >= passed; //convenient to assume that 1 millesecond has always passed
}
int DeltaTime::getTime()
{
    return setTime;
}
int DeltaTime::getTimePassed()
{
    return setTime == -1 ? -1 : SDL_GetTicks() - setTime;
}

int DeltaTime::getFramesPassed()
{
    return setFrame == -1 ? -1 : currentFrame - setFrame;
}

SDL_Keycode KeyManager::justPressed = -1;
std::unordered_set<SDL_Keycode> KeyManager::keys;
Uint32 KeyManager::lastEvent = 0;
SDL_Keycode KeyManager::lastKey = -1;
char KeyManager::lastChar = 0;

bool KeyManager::isPressed(SDL_Keycode key) //finds the index of n or -1 if n isn't found
{
    return keys.find(key) != keys.end();
}

void KeyManager::update(SDL_Event& e)
{
    justPressed = -1;
    lastChar = 0;
    SDL_Keycode sym = e.key.keysym.sym;
    switch(e.type)
    {
        case SDL_KEYDOWN:
            if (!isPressed(sym))
            {
                keys.insert(sym);
                justPressed = sym;
            }
            break;
        case SDL_KEYUP:
            keys.erase(sym);
            break;
        case SDL_TEXTINPUT:
            lastChar = e.text.text[0];
            break;
    }
    lastKey = sym;
}

SDL_Keycode KeyManager::getJustPressed()
{
    return justPressed;
}

char KeyManager::getLastChar()
{
    return lastChar;
}

bool MouseManager::left = false;
bool MouseManager::right = false;
bool MouseManager::middle = false;
int MouseManager::justClicked = -1;
int MouseManager::justReleased = -1;
Uint32 MouseManager::lastEvent = 0;
std::pair<int,int> MouseManager::mouseWheel;
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

void MouseManager::reset()
{
    justReleased = -1;
    justClicked = -1;
    mouseWheel = {0,0};
}

void MouseManager::update(SDL_Event& e)
{
    reset();
    if (e.type == SDL_MOUSEWHEEL)
    {
        mouseWheel = {e.wheel.x,e.wheel.y};
    }
    else
    {
        if (lastEvent != e.type)
        {
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                bool* ptr = getButton(e.button.button);
                if (*ptr == false)
                {
                    justClicked = e.button.button;
                    *ptr = true;
                }
            }

            else
            {
                if (e.type == SDL_MOUSEBUTTONUP)
                {
                    *getButton(e.button.button) = false;
                    justReleased = e.button.button;
                }
            }
        }
    }

    lastEvent = e.type;
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

int MouseManager::getJustReleased()
{
    return justReleased;
}

const std::pair<int,int>& MouseManager::getMouseWheel()
{
    return mouseWheel;
}
