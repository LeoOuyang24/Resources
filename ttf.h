#ifndef SDL_ttf_H_INCLUDED
#define SDL_ttf_H_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <functional>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

static SDL_Renderer* render = nullptr;

typedef void (*functionPtr)();
typedef std::function<void()>* functionWrapper;
typedef std::function<void()> actualFunction;
const SDL_Color BLACK = {0,0,0};

struct Paper;

bool initRender(SDL_Window& window, int index = -1, Uint32 flags = SDL_RENDERER_ACCELERATED) // sets render to a window
{
    render = SDL_CreateRenderer(&window, index, flags );
    if (render == nullptr)
    {
        std::cout << "Error initiating render: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

SDL_Texture* write(std::string how_write, std::string type, SDL_Color* backColor,const SDL_Color& fontColor,int how_big,SDL_Renderer* renderer );
void write(std::string how_write, std::string type, SDL_Color* backColor, const SDL_Color& fontColor,Paper* sheet, int how_big, int x , int y , int w , int h , SDL_Renderer* renderer); //how_write is a font. type is the string to be written.
bool changeTexture(SDL_Texture*& text, std::string sprite_location, std::string delimiter = "absolutely nothing", SDL_Renderer* renderer = render) // delimiter allows one to pass a blank string without an error message
{

if (sprite_location == delimiter)
{
    text = NULL;
    return true;
}
    SDL_Surface* surf = IMG_Load(sprite_location.c_str());
    text = SDL_CreateTextureFromSurface(renderer, surf );
    SDL_FreeSurface(surf);
    if (!(text))
    {
        std::cout << "Warning: error changing texture to: " << sprite_location << " " << IMG_GetError() << std::endl;
        return false;
    }
    return text;
}
SDL_Texture* changeTexture(std::string sprite_location,  std::string delimiter = "absolutely nothing", SDL_Renderer* renderer = render)
{
    SDL_Texture* t = nullptr;
   if (changeTexture(t, sprite_location, delimiter, renderer))
   {
       return t;
   }
   return nullptr;
}
bool renderTexture(SDL_Texture* text,SDL_Rect* rect,const SDL_Rect* drect,  double angleInDegrees = 0,SDL_Point* center = nullptr, const SDL_RendererFlip flip = SDL_FLIP_NONE, SDL_Renderer* renderer = render)
{ //rect is the portion of the sprite
    if (!text)
    {
       // std::cout <<  << std::endl;
        std::cout << ("Error: error rendering texture: texture is NULL.") << std::endl;
        throw std::logic_error("");
        return false;
    }
    else if (drect&&  drect->w*drect->h <= 0)
    {
       // std::cout << "Error: Passed drect has area of 0." << std::endl;
        std::cout << "Error in RenderTexture function: Width is: " << drect->w << " and Height is: " << drect->h << std::endl;
        return false;
    }
   SDL_RenderCopyEx(renderer, text, rect, drect, angleInDegrees, center, flip  );

}


bool renderTexture(SDL_Texture* text,SDL_Rect* rect,const SDL_Rect& drect,  double angleInDegrees = 0,SDL_Point* center = nullptr, const SDL_RendererFlip flip = SDL_FLIP_NONE, SDL_Renderer* renderer = render)
{ //rect is the portion of the sprite that needs to be rendered

renderTexture(text,rect,&drect,angleInDegrees,center,flip,renderer);

}

void printRect(SDL_Rect r)
{
    std::cout << convert(r.x) << " " << convert(r.y) << " " << r.w << " " << r.h << std::endl;
}

class KeyManager : public NumberManager //the keymanager stores numbers pressed in order based on the most recent key pressed.
{
public:
    KeyManager()
    {

    }
    double getLatest()
    {
        if (numbers.size() > 0)
        return numbers[numbers.size() - 1];
        else
        {
            return -1;
        }
    }
    void addNumber(double key)
    {
        numbers.push_back(key);
    }
    int findNumber(double n)
    {
        for (int i = 0; i < numbers.size(); i ++)
        {
            if (numbers[i] == n )
            {
                return i;
            }
        }
        return -1;
    }
    void getKeys(SDL_Event& e) //adds and removes keys based on e.type
    {
        if (e.type == SDL_KEYDOWN)
        {
            int sym = e.key.keysym.sym;
            if (findNumber(sym) == -1)
            {
                addNumber(sym);
            }
        }
        else if (e.type == SDL_KEYUP)
        {
            int index = findNumber(e.key.keysym.sym);
            numbers.erase(numbers.begin() + index);
        }

    }

};

enum Direction
{
    RIGHT,
    UP,
    LEFT,
    DOWN,
    INDETERMINATE
};



bool PointInRect(const SDL_Point& point, const SDL_Rect& rect) //returns true if the point is in the rect or on the borders
{
    return ((point.x >= rect.x && point.x <= rect.x + rect.w) && (point.y >= rect.y && point.y <= rect.y + rect.h));
}

bool LineAndLineIntersect(const SDL_Point& a, const SDL_Point& b, const SDL_Point& c, const SDL_Point& d)
{
    double intersect = 0;
    bool value = false;
    if (a.x == b.x)
    {
        intersect = a.x;
    }
    else if (c.x == d.x)
    {
        intersect = c.x;
    }
    else
    {
        double slope1 = ((double)a.y - b.y)/(a.x - b.x);
        double slope2 = ((double)c.y - d.y)/(c.x - d.x);
        double cept1 = a.y - slope1*a.x;
        double cept2 = c.y - slope2*c.x;
      //  std::cout << cept1 << " " << cept2 << std::endl;
        if (slope1 == slope2 && cept1 == cept2)
        {
            intersect = std::max(std::min(a.x,b.x),std::min(c.x,d.x)); //finds the second smallest point out of the four points
        }
        else
        {
            intersect = (cept1 - cept2)/(slope2 - slope1);
        }
    }

        value = ( (intersect >= std::min(a.x,b.x) && intersect <= std::max(a.x,b.x ))&& (intersect >= std::min(c.x,d.x) && intersect <= std::max(c.x,d.x)) );
       //     std::cout << intersect << std::endl;
          //  std::cout << "value: " <<  value << std::endl;


    return value;
}

bool LineAndRectIntersect(const SDL_Rect& r, int x1, int y1, int x2, int y2 )
{
    SDL_Point a = {x1,y1};
    SDL_Point b = {x2,y2};
    SDL_Point topLeft = {r.x,r.y};
    SDL_Point topRight = {r.x + r.w, r.y};
    SDL_Point bottomLeft = {r.x, r.y + r.h};
    SDL_Point bottomRight = {r.x + r.w, r.y + r.h};
    bool value = false;
    if (PointInRect(a, r)) // we only have to check one endpoint
    {
        value = true;
    }
    else if (PointInRect(b,r))
    {
        value = true;
    }
   else if ( LineAndLineIntersect(topLeft,topRight,a, b) ||
             LineAndLineIntersect(topRight, bottomRight, a, b) ||
             LineAndLineIntersect(bottomRight, bottomLeft, a, b) ||
             LineAndLineIntersect(bottomLeft, topLeft, a,b) )
             {
                 value = true;
             }
    else
    {
        value = false;
    }
    return value;

}


bool RectIntersect(const SDL_Rect& r1, const SDL_Rect& r2)
{
    if (r1.x <= r2.x + r2.w && r1.x + r1.w >= r2.x && r1.y <= r2.y + r2.h && r1.y + r1.h >= r2.y)
    {
        return true;
    }
    return false;

}

class Paper
{

    SDL_Texture* text = NULL;
    SDL_Rect* portion = NULL;
    protected:
    std::string spriteLocation = "";
    std::string message = ""; // if this paper holds a message, this string records it
              bool display = true;
    public:
           int spriteW = 0, spriteH = 0;
        Paper(std::string sprite_location, const SDL_Rect& p = {0,0,0,0})
    {
        load(sprite_location,p);
      //  SDL_QueryTexture(text, NULL, NULL, &spriteW, &spriteH);
    }
    Paper(std::string font_location, std::string toBePrint,  SDL_Color* backColor = NULL, const SDL_Color& fontColor = BLACK,int howBig = 64)
    {
        load(font_location,toBePrint,backColor,fontColor,howBig);
        //  SDL_QueryTexture(text, NULL, NULL, &spriteW, &spriteH);
    }
    Paper()
    {
         portion = new SDL_Rect{0,0,0,0};
         text = SDL_CreateTexture(render, SDL_PIXELFORMAT_UNKNOWN,SDL_TEXTUREACCESS_TARGET, 0,0);
    }
    Point getDimen() const
    {
        return Point{spriteW,spriteH};
    }
    virtual bool load(std::string sprite_location, const SDL_Rect& p = {0,0,0,0})
    {
        portion = new SDL_Rect(p);
        return changeText(sprite_location);
    }
   virtual bool load(std::string font_location, std::string toBePrint,  SDL_Color* backColor = NULL, const SDL_Color& fontColor = BLACK,int howBig = 64)
    {
        return changeText(font_location, toBePrint, backColor, fontColor,howBig);
    }
    void clear()//erases all contents of the paper
    {
         SDL_DestroyTexture(text);
    }
    SDL_Texture* getText()
    {
        return text;
    }
    void changeDisplay()
    {
        if (display)
            display = false;
        else
            display = true;
    }
    void changeDisplay(bool newDisplay)
    {
        display = newDisplay;
    }
    bool getDisplay()
    {
        return display;
    }
    virtual void show(SDL_Renderer* r = render)
    {
        if (display)
        {
            show(r,NULL,NULL);
        }
    }
    virtual void show(const SDL_Rect& r,SDL_Rect* portion = nullptr, SDL_Renderer* re = render)
    {
         show(re, portion, &r);
    }
    virtual void show(SDL_Renderer* renderer,SDL_Rect* part ,SDL_Rect const * blit ,double angleInDegrees =0 ,SDL_Point* center = nullptr, const SDL_RendererFlip flip = SDL_FLIP_NONE)
    {
       if (!text)
       {
           throw std::logic_error("Text is null");
       }
       if (part && (part->w == 0 || part->h == 0))
       {
           part = NULL;
       }
        if (display)
             renderTexture(text, part, blit,angleInDegrees, center, flip,renderer);
    }
    bool changeText(std::string font_location, std::string toBePrint, SDL_Color* backColor = NULL, const SDL_Color& fontColor = BLACK,int howBig = 64, SDL_Renderer* renderer = render)
    {        SDL_Texture* t  = write(font_location, toBePrint, backColor, fontColor,howBig, renderer);
        message = toBePrint;
          SDL_QueryTexture(text, NULL, NULL, &spriteW, &spriteH);
        return changeText(*t);

    }
    bool changeText(std::string sprite_location, SDL_Renderer* renderer = render)
    {
        if(!changeTexture(text, sprite_location, "absolutely nothing", renderer))
        {
            std::cout << "Error changing paper texture to: " << sprite_location << std::endl;
            return false;
        }
        else
        spriteLocation = sprite_location;
        SDL_QueryTexture(text, NULL, NULL, &spriteW, &spriteH);
        return true;
    }
    bool changeText( SDL_Texture& newText )
    {
        if (text)
            SDL_DestroyTexture(text);
        text = &newText;
          SDL_QueryTexture(text, NULL, NULL, &spriteW, &spriteH);
          return text;
    }
    void changeTextColor(double r, double g, double b)
    {
         SDL_SetTextureColorMod(text, r,g,b);
    }
    bool isTextValid()
    {
        return text;
    }
        SDL_Rect* getPortion () const
    {
        return portion;
    }
    void changePortion(const SDL_Rect& newRect)
    {
        if (portion)
        {
            portion->x = newRect.x;
            portion->y = newRect.y;
            portion->w = newRect.w;
            portion->h = newRect.h;
        }
    }
    void changePortion(int x, int y)
    {
        changePortion({x,y,portion->w,portion->h});
    }
    std::string getMessage()
    {
        return message;
    }
    std::string getSpriteLocation()
    {
        return spriteLocation;
    }
 virtual  ~Paper()
    {

        SDL_DestroyTexture(text);
        if (portion!= NULL)
            delete portion;
    }


};

class Animation : public Paper // a class that allows for an animated paper. The text and rect variables in this case represent the current frame of animation and where to blit that current frame
{
    protected:
    int frameTime = 0; //the amount of time of each frame in milliseconds.
    int start = 0; // the time at which the animation began
    bool stopped = false;
    public:
    Animation(std::string spriteSheetLocation, const SDL_Rect& portionOfSpriteSheet, int frameT ) : Paper( spriteSheetLocation, portionOfSpriteSheet)
    {
     //   std::cout << portion->w << std::endl;
        frameTime = frameT;
    }
    Animation()
    {
        Paper();
    }
    virtual bool load(std::string spriteSheetLocation, const SDL_Rect& portionOfSpriteSheet,int frameT )
    {
        changePortion(portionOfSpriteSheet);
        frameTime = frameT;
        return changeText(spriteSheetLocation);

    }
    using Paper::show;
    virtual void show(SDL_Renderer* renderer,SDL_Rect* part ,SDL_Rect const * blit ,double angleInDegrees =0 ,SDL_Point* center = nullptr, const SDL_RendererFlip flip = SDL_FLIP_NONE)
    { //SDL_Rect* part does nothing

           //std::cout << spriteW/portion.get()->w<< std::endl;
           SDL_Rect* thisPortion = getPortion();
           if (thisPortion->w != 0 && thisPortion->h != 0 )
           {
                if (getDisplay() )
                {
//                std::cout << portion->w << std::endl;
              if (!stopped)
              {
                    if (start == 0) //if the animation hasn't started yet, start now
                    {
                        start = SDL_GetTicks();
                    }
                  int ticks = SDL_GetTicks()-start;
                  //std::cout << ticks << std::endl;
                //   std::cout << portion->w << std::endl;

             changePortion(    {   (ticks/frameTime)%(spriteW/thisPortion->w)*thisPortion->w,
            (ticks/(spriteW/thisPortion->w*frameTime))%((spriteH/thisPortion->h))*thisPortion->h , thisPortion->w, thisPortion->h}  );
              }
                   // std::cout << thisPortion->y << " " << thisPortion->x  << std::endl;
                    Paper::show(renderer,thisPortion,blit,angleInDegrees, center, flip);
                 //   if (portion.get()->x == 0)
                 //   std::cout << portion.get()->x << std::endl;
                }
           }
    }
    void restart()
    {
        changePortion(0,0);
        start = 0;
        stopped = false;
    }
    void changeStopped(bool newStop)
    {
        stopped = newStop;
    }
       bool hasStopped()
   {
       return stopped;
   }


};

class TempAnimation : public Animation // a class that represents an animation that plays then disappears
{
    protected:
    int duration = 0;
public:
    TempAnimation(std::string spriteSheetLocation, const SDL_Rect& portionOfSpriteSheet, int frameT, int howLong ) : Animation(spriteSheetLocation, portionOfSpriteSheet, frameT)
    {
        duration = howLong;
    }
    TempAnimation(std::string spriteSheetLocation, const SDL_Rect& portionOfSpriteSheed,  int howLong) : Animation() //use this constructor if the effect should play once
    {
        load(spriteSheetLocation,portionOfSpriteSheed,0);
        duration = howLong;
        SDL_Rect* portion = getPortion();
        frameTime = howLong/(spriteW/portion->w*spriteH/portion->h);
    }
    TempAnimation() : Animation()
    {

    }
    void changeDuration(int newDuration)
    {
        duration = newDuration;
    }
    using Paper::show;
   virtual void show(SDL_Renderer* renderer,SDL_Rect* part ,SDL_Rect* blit ,double angleInDegrees =0 ,SDL_Point* center = nullptr, const SDL_RendererFlip flip = SDL_FLIP_NONE)
   {
       if (getDisplay())
       {
           if (start == 0)
           {
               start = SDL_GetTicks();
           }
           if (SDL_GetTicks() - start < duration)
           {
               part = getPortion();
               if (!(part->x + part->w == spriteW  && part->y + part->h == spriteH))
               Animation::show(renderer, part, blit, angleInDegrees, center, flip);
           }
           else
           {
            //   std::cout << "Done" << std::endl;
     done();
       //     start = SDL_GetTicks();
           }
       }

   }
   void done()
   {
       stopped = true;
   }
};

class Button
{
    protected:
        bool selected = false;
std::function<void()>* use = {};
public:
   Paper* sheet = nullptr;
    Paper* text = nullptr;
   SDL_Rect rect ={};

    Button(int x, int y, int w, int h,Paper& sprite, std::function<void()>* func,SDL_Renderer* see = render)
    {
        rect = {x,y,w,h};
        sheet = &sprite;
        use = func;
    }
    Button(int x, int y, int w, int h, std::string font, std::string words, SDL_Color* c,const SDL_Color& fontColor,std::function<void()>* func,SDL_Renderer* see = render)
    {
        rect = {x,y,w,h};
        text->changeText( font, words, c, fontColor, 64);
       if (func != NULL)
       {
       use= func;
       }
    }
       Button(int x, int y, int w, int h, std::string font, std::string words, int r, int g, int b,const SDL_Color& fontColor,functionWrapper func,SDL_Renderer* see = render)
    {
        rect = {x,y,w,h};
        SDL_Color c;
        if (r >= 0 && g>= 0 && b>=0)
        {
            c = {r,g,b};
        }
        text->changeText(font, words, &c, fontColor,64, see);
       if (func != NULL)
       {
       use= func;
       }
    }
    Button()
    {

    }
    bool checkMouse(int x, int y)
    {
        SDL_Point p = {x,y};
        return SDL_PointInRect(&p,&rect);
    }
    bool getSelected()
    {
        return selected;
    }
    void changeSelected(bool newChosen)
    {

        selected = newChosen;
    }
    virtual void show()
    {
        sheet->show(rect);
        text->show(rect);
    }
    virtual void function()
    {
       // std::cout << "A" << std::endl;
        if (use != NULL)
        (*use)();
    }
    virtual void update(int x, int y)
    {
        if (checkMouse(x,y))
        {
            function();
        }
    }
    virtual void during()
    {

    }
    virtual void release()
    {

    }
};



struct Screen
{
std::vector<Button*> buttons ;
  Paper* bg = nullptr;
   std::vector< Paper* > sheets;
    std::function<void()>* first = NULL;
    std::function<void()>* last = NULL;
    std::function<void()>* use = NULL; //function that runs in the background
    Screen(Paper& background,  functionWrapper func = NULL, functionWrapper f = NULL, functionWrapper la = NULL, std::vector<Paper*>* message = NULL,std::vector<Button*>* l = NULL,SDL_Renderer* renderer = render)
    {
        if (l)
        {
            buttons = *l;
        }
         bg = &background;
        if (func != NULL)
        {
            use =  func;
        }
        if (f)
        {
           first= f;
        }
        if (la)
        {
            last = la;
        }
        if (message)
        {
            sheets = *message;
        }

    }
    virtual void show()
    {
        bg->show();
            int  size = sheets.size();
            for (unsigned int i = 0; i < size; i ++)
            {
                sheets[i]->show();
            }
            size = buttons.size();
            for (unsigned int i = 0; i < size;i ++)
            {
                buttons[i]->show();
            }

    }
    virtual void update(SDL_Event& e)
    {
        function();
        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            if (e.button.button == SDL_BUTTON_LEFT)
            {
                int x,y;
                SDL_GetMouseState(&x, &y);
                int size = buttons.size();
                for (int i = 0; i < size; i ++)
                {
                    if (buttons[i]->checkMouse(x,y))
                    {
                        buttons[i]->function();
                        break;
                    }
                }
            }
        }
    }
    virtual void function()
    {
        if (use != NULL)
        (*use)();
    }
    virtual void useFirst()
    {
        if (first)
        {
            (*first)();
        }
    }
    virtual void useLast()
    {
        if (last)
        {
            (*last)();
        }
    }
    ~Screen()
    {
    buttons.clear();

    }
};





SDL_Texture* write(std::string how_write, std::string type, SDL_Color* backColor,int r, int g, int b, int how_big,SDL_Renderer* renderer = render) //returns an SDL_Texture with the text written
{
    if ((how_write.length() >= 4 && how_write.substr(how_write.length()-4, 4) != ".ttf") || how_write.length() <= 4)
    {
        how_write += ".ttf";
    }
    TTF_Font* font = TTF_OpenFont(how_write.c_str(), how_big);
if (font == NULL)
{
   std:: cout << "Error in Write function: " << TTF_GetError() << std::endl;
   return NULL;
}
   SDL_Color foregroundColor = { r,g,b };
   //SDL_Color backgroundColor = backColor;
char *text = new char[type.length() +1 ];
strcpy(text, type.c_str());
//cout << "y: " << y << endl;
SDL_Surface* textSurface = NULL;
        if (backColor == NULL )
           textSurface = TTF_RenderText_Blended(font, text, foregroundColor);
        else
            textSurface = TTF_RenderText_Shaded(font, text, foregroundColor, *backColor);

   if ( textSurface == NULL)
   {
       std::cout << SDL_GetError() << std::endl;
      std:: cout << "Error in Write function: " << TTF_GetError() << std::endl;
  //    return nullptr;
   }
   delete[] text;
return SDL_CreateTextureFromSurface(renderer, textSurface);
//SDL_SetTextureBlendMode(sheet->text, SDL_BLENDMODE_BLEND);
//SDL_SetTextureAlphaMod(sheet->text, 32);

//SDL_RenderCopy(render, gh, NULL, text);

}

SDL_Texture* write(std::string how_write, std::string type, SDL_Color* backColor,const SDL_Color& fontColor,  int how_big,SDL_Renderer* renderer = render) //how_write is a font. type is the string to be written.
{
  return  write(how_write, type, backColor, fontColor.r,fontColor.g, fontColor.b,  how_big, renderer);
}

void write(std::string how_write, std::string type, SDL_Color* backColor,const SDL_Color& fontColor, Paper* sheet, int how_big,SDL_Renderer* renderer = render) //how_write is a font. type is the string to be written.
{
    SDL_Texture* texture = write(how_write, type, backColor, fontColor.r,fontColor.g, fontColor.b, how_big, renderer  );
    if (texture)
        sheet->changeText(*texture);

}






#endif // SDL_ttf_H_INCLUDED
