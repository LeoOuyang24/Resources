#ifndef SDLHELPER_H_INCLUDED
#define SDLHELPER_H_INCLUDED

#include <list>

#include "SDL.h"

typedef unsigned long long gameFrame;

class DeltaTime //represents a timer that times how much time/frames has passed. deltaTime member variable milliseconds since last frame
{
    static int lastTime; //used to calculate deltaTime;
    int setTime= -1; //-1 means not set
    gameFrame setFrame = 0;
    static gameFrame currentFrame; //the current frame. Incremented every time update is called
public:
    static int deltaTime;
    static void update();
    static gameFrame getCurrentFrame();
    void set(); //sets the time to SDL_GetTicks and setFrame to the currentFrame
    virtual void reset(); //sets time to -1
    bool isSet(); //returns whether or not the alarm is set
    bool timePassed(double passed); //returns whether or not the passed time is greater than or equal to passed in milliseconds
    bool framesPassed(gameFrame passed); //returns whether or not the passed frames have passed
    int getTime(); //gets setTime
    int getTimePassed(); //gets the time since setTime. -1 if not set
    int getFramesPassed(); //gets the frames since setFrame. -1 if not set
};

class KeyManager //the keymanager stores numbers pressed in order based on the most recent key pressed.
{
    static char lastLetter; //last letter that was typed
    static char lastChar; //the last character that was typed. 0 if the key was held
    static SDL_Keycode justPressed; //last key to be pressed. -1 if the key was held
    static std::list<SDL_Keycode> numbers;
    static Uint32 lastEvent;
    static SDL_Keycode lastKey; //the last key to be pressed/released
public:
    static double getLatest();
    static SDL_Keycode getLater(double m, double n); //of m and n, finds which key which pressed most recently.
    static void addNumber(double key);
    static int findNumber(double n); //finds the index of n or -1 if n isn't found
    static void getKeys(SDL_Event& e); //update function;
    static SDL_Keycode getJustPressed();
    static void removeNumber(double number);
    static char getLastChar(); //gets the last character that was typed






};

class MouseManager
{
    static int justClicked;
    static int justReleased;
    static bool left;
    static bool right;
    static bool middle;
    static Uint32 lastEvent;
    static std::pair<int,int> mouseWheel;
    static bool* getButton(int key); //helper function that returns the boolean corresponding to the key pressed
public:
   static void update(SDL_Event& e);
    static int getJustClicked();
    static int getJustReleased();
    static const bool isPressed(int key);
    static std::pair<int,int> getMousePos();
    static const std::pair<int,int>& getMouseWheel(); //returns mouseWheel scrolling info as well as whether or not the mouse is scrolling


};


#endif // SDLHELPER_H_INCLUDED
