#include "wxWidgetHelper.h"

int wxKeyManager::justPressed = -1;
std::unordered_set<int> wxKeyManager::keys;


void wxKeyManager::getKeys(wxKeyEvent& e, bool keyDown)
{
    justPressed = -1;
    int sym = e.GetKeyCode();
    if (sym != justPressed)
    {
        if (keyDown)
        {
            if (!isDown(sym))
            {
                keys.insert(sym);
                justPressed = sym;
            }
        }
        else
        {
            keys.erase(sym);
        }
    }
}

int wxKeyManager::getJustPressed()
{
    return justPressed;
}

bool wxKeyManager::isDown(int key)
{
    return keys.find(key) != keys.end();
}

void wxKeyManager::reset()
{
    justPressed = -1;
}

bool wxMouseManager::left = false;
bool wxMouseManager::right = false;
bool wxMouseManager::middle = false;
int wxMouseManager::justClicked = -1;
int wxMouseManager::justReleased = -1;
wxPoint wxMouseManager::mousePos;
bool* wxMouseManager::getButton(int key)
{
    switch (key)
    {
    case wxMOUSE_BTN_LEFT:
        return &left;
        break;
    case wxMOUSE_BTN_RIGHT:
        return &right;
        break;
    case wxMOUSE_BTN_MIDDLE:
        return &middle;
        break;
    default:
        throw "Wanted mouse button doesn't exist!";
    }
}
void wxMouseManager::update(wxMouseEvent& e)
{
    if (e.ButtonDown())
    {
        justClicked = e.GetButton();
        *getButton(justClicked) = true;
    }
    else if (e.ButtonUp())
    {
        justReleased = e.GetButton();
        *getButton(justReleased) = false;
    }

    mousePos = wxPoint(e.GetX(),e.GetY());
}

int wxMouseManager::getJustClicked()
{
    return justClicked;
}

int wxMouseManager::getJustReleased()
{
    return justReleased;
}

const bool wxMouseManager::isPressed(int key)
{
    return *getButton(key);
}

const wxPoint& wxMouseManager::getMousePos()
{
    return mousePos;
}

void MyGameFrame::onKeyDown(wxKeyEvent& event)
{
    wxKeyManager::getKeys(event,true);
}

void MyGameFrame::onKeyUp(wxKeyEvent& event)
{
    wxKeyManager::getKeys(event,false);
}

void MyGameFrame::mouseEvent(wxMouseEvent& event)
{
    wxMouseManager::update(event);
}

MyGameFrame::MyGameFrame(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY,title, pos, size)
{
    Bind(wxEVT_KEY_DOWN,&MyGameFrame::onKeyDown,this);
    Bind(wxEVT_KEY_UP,&MyGameFrame::onKeyUp,this);
    Bind(wxEVT_MOTION, &MyGameFrame::mouseEvent,this);
    Bind(wxEVT_LEFT_DOWN,&MyGameFrame::mouseEvent,this);
    Bind(wxEVT_LEFT_UP,&MyGameFrame::mouseEvent,this);
    Bind(wxEVT_RIGHT_DOWN,&MyGameFrame::mouseEvent,this);
    Bind(wxEVT_RIGHT_UP,&MyGameFrame::mouseEvent,this);
    Bind(wxEVT_MIDDLE_UP,&MyGameFrame::mouseEvent,this);
    Bind(wxEVT_MIDDLE_DOWN,&MyGameFrame::mouseEvent,this);



}
