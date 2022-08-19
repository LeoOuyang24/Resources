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
std::list<SDL_Keycode> KeyManager::numbers;
Uint32 KeyManager::lastEvent = 0;
SDL_Keycode KeyManager::lastKey = -1;
char KeyManager::lastChar = 0;
char KeyManager::lastLetter = 0;
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
    justPressed = -1;
    lastChar = 0;
    int sym = e.key.keysym.sym;
    if (e.type != lastEvent || sym != lastKey || lastLetter != e.text.text[0])
    {
        if (e.type == SDL_KEYDOWN)
        {
            if (findNumber(sym) == -1)
            {
                addNumber(sym);
                justPressed = sym;
            }
        }
        else if (e.type == SDL_KEYUP)
            {
                removeNumber(sym);
            }
        else if (e.type == SDL_TEXTINPUT)
        {
            lastChar = e.text.text[0];
        }
    }
    lastLetter = e.text.text[0];
    lastKey = sym;
    lastEvent = e.type;

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
void MouseManager::update(SDL_Event& e)
{
    justReleased = -1;
    justClicked = -1;
    if (e.type == SDL_MOUSEWHEEL)
    {
        mouseWheel = {e.wheel.x,e.wheel.y};
    }
    else
    {
        mouseWheel = {0,0};
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
