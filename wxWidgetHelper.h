#ifndef WXWIDGETHELPER_H_INCLUDED
#define WXWIDGETHELPER_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <unordered_set>

class wxKeyManager //the keymanager stores numbers pressed in order based on the most recent key pressed.
{
    static int justPressed; //the last key to be pressed/released. ASCII for characters, wxKeyCode for other values, -1 if no key was pressed
    static std::unordered_set<int> keys;
public:
    static void getKeys(wxKeyEvent& e, bool keyDown); //update function;
    static int getJustPressed();
    static bool isDown(int key);
    static void reset(); //sets justPressed to -1. Should be called when there is no key pressed
};

class wxMouseManager
{
    static int justClicked;
    static int justReleased;
    static bool left;
    static bool right;
    static bool middle;
    static wxPoint mousePos;
    static bool* getButton(int key); //helper function that returns the boolean corresponding to the key pressed
public:
    static void update(wxMouseEvent& e);
    static int getJustClicked();
    static int getJustReleased();
    static const bool isPressed(int key);
    static const wxPoint& getMousePos();


};

class MyGameFrame : public wxFrame
{
    void onKeyDown(wxKeyEvent& event);
    void onKeyUp(wxKeyEvent& event);
    void mouseEvent(wxMouseEvent& event);
public:
    MyGameFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

};

#endif // WXWIDGETHELPER_H_INCLUDED
