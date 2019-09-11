#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

#include <SDL_image.h>
#include<SDL_net.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <math.h>
#include <vector>
#include <list>
#include <functional>
#include <dirent.h>
#include <memory>

extern SDL_Renderer* render;
SDL_Texture* write(std::string how_write, std::string type, SDL_Color* backColor,int how_big,SDL_Renderer* r );
struct paper
{
  bool display = true;
    SDL_Rect* rect = NULL;
    SDL_Texture* text = NULL;
        paper(SDL_Rect* r , SDL_Texture* t)
    {
        rect = r;
        text = t;
    }
    paper(SDL_Rect* r, std::string font_location, std::string message, SDL_Color* backColor = NULL, int howBig = 64)
    {
        rect = r;
        text = write(font_location, message, backColor, howBig, render);
    }
    paper()
    {
         rect = new SDL_Rect{0, 0, 0, 0};
    }
    void clear()//erases all contents of the paper
    {
         SDL_DestroyTexture(text);
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
    void show(SDL_Renderer* r = render)
    {
        if (display)
        {
            SDL_RenderCopy(r, text, NULL, rect);
        }
    }
    ~paper()
    {

        SDL_DestroyTexture(text);
        if (rect != NULL)
        delete rect;
    }


};

SDL_Texture* write(std::string how_write, std::string type, SDL_Color* backColor,int how_big,SDL_Renderer* r = render) //returns an SDL_Texture with the text written
{
    if (how_write.substr(how_write.length()-4, 4) != ".ttf")
    {
        how_write += ".ttf";
    }
    TTF_Font* font = TTF_OpenFont(how_write.c_str(), how_big);
if (font == NULL)
{
   std:: cout << "Error in Write function: " << TTF_GetError() << std::endl;
   return NULL;
}
   SDL_Color foregroundColor = { 0, 0, 0 };
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
   }
return SDL_CreateTextureFromSurface(render, textSurface);
//SDL_SetTextureBlendMode(sheet->text, SDL_BLENDMODE_BLEND);
//SDL_SetTextureAlphaMod(sheet->text, 32);
delete[] text;
//SDL_RenderCopy(render, gh, NULL, text);

}
void write(std::string how_write, std::string type, SDL_Color* backColor, paper* sheet, int how_big,SDL_Renderer* r = render) //how_write is a font. type is the string to be written.
{
    if (sheet == NULL)
    {
        std::cout << "WARNING: Paper passed to write function is null. Nothing was done to modify it." << std::endl;
        return;
    }
    if (how_write.substr(how_write.length()-4, 4) != ".ttf")
    {
        how_write += ".ttf";
    }
    TTF_Font* font = TTF_OpenFont(how_write.c_str(), how_big);
if (font == NULL)
{
   std:: cout << "Error in Write function: " << TTF_GetError() << std::endl;
   return;
}
   SDL_Color foregroundColor = { 0, 0, 0 };
   //SDL_Color backgroundColor = backColor;
char *text = new char[type.length() +1 ];
strcpy(text, type.c_str());
//cout << "y: " << y << endl;
SDL_Surface* textSurface;
        if (backColor == NULL )
           textSurface = TTF_RenderText_Blended(font, text, foregroundColor);
        else
            textSurface = TTF_RenderText_Shaded(font, text, foregroundColor, *backColor);

   if ( textSurface == NULL)
   {
       std::cout << "Errpr loading texture in write function: " << SDL_GetError() << std::endl;
      std:: cout << "Error in Write function: " << TTF_GetError() << std::endl;
   }

if (sheet->text != NULL)
{
   SDL_DestroyTexture(sheet->text);
}


//std::cout << " LOL" << std::endl;
sheet->text = SDL_CreateTextureFromSurface(render, textSurface);
//SDL_SetTextureBlendMode(sheet->text, SDL_BLENDMODE_BLEND);
//SDL_SetTextureAlphaMod(sheet->text, 32);
delete[] text;
//SDL_RenderCopy(render, gh, NULL, text);

}
int findIntLength(int x)
{
    if (x / 10 == 0)
    {
        return 1;
    }
    else
    {
        return 1 + findIntLength(x/10);
    }
}

void write(std::string how_write, std::string type, SDL_Color* backColor, paper* sheet, int how_big, int x , int y , int w , int h , SDL_Renderer* r = render) //how_write is a font. type is the string to be written.
{
     if (sheet == NULL)
    {
        std::cout << "WARNING: Paper passed to write function is null. Nothing was done to modify it." << std::endl;
        return;
    }
    write(how_write, type, backColor, sheet, how_big, r);
    if (sheet->rect == NULL)
    {
        sheet->rect = new SDL_Rect;
    }
  sheet->rect->x = x;
  sheet->rect->y = y;
  sheet->rect->h = h;
  sheet->rect->w = w;


}

int findBigInt(int i, int a)
{
    if (i > a)
        return i;
    else
        return a;
}


std::string convert(double input)
{
     std::ostringstream os;
    os << input;
    return os.str();
}

std::string convert(int input) //takes an int and returns the int in string form
{
    std::ostringstream os;
    os << input;
    return os.str();
}

double convert(std::string input)
{
    double j;
     std::istringstream os(input);
    os >> j;
    return j;
}

typedef void (*functionPtr)();
typedef std::function<void()>* functionWrapper;
typedef std::function<void()> actualFunction;

template<typename T>
bool fillContainer(std::vector<T> vec, std::string directory, std::function<void(std::string)>& func)
{
    DIR* dir = opendir(directory.c_str());
    dirent* d = NULL;
    if (!dir)
    {
        std::cout << "Error opening directory: " << directory << "(" << errno << ")" << std::endl;
        return false;
    }
    while (d = readdir(dir))
    {
      std::string  name = (std::string)d->d_name;
        if (name != "." && name != ".." )
        {
       // cout << name << endl;
       func(name);
        }
    }
    return true;
}

template<class R, class O, class ...I>
struct wrapperFunction
{
    using functionPtr = R (*)(I...);
    O* owner = NULL;
    functionPtr actualFunction = NULL;
    wrapperFunction(functionPtr function, O* newOwner = NULL)
    {
        owner = newOwner;
        if (function != NULL)
        actualFunction = function;
    }
    void use()
    {
        if (actualFunction != NULL)
        {
           // std::cout << "yes" << std::endl;
        actualFunction();
        }
    }
};


struct button
{

   SDL_Rect* rect = NULL;
    SDL_Texture* sprite = NULL;
   std::string spriteLocation = ""; // a variable that holds where the sprite is located
std::function<void()>* use = {};
    button(int x, int y, int w, int h, std::string sprite_location, std::function<void()>* func,SDL_Renderer* see = render)
    {
        rect = new SDL_Rect{x, y, w, h};
        if (sprite_location!= "")
        {
        sprite = SDL_CreateTextureFromSurface(see,IMG_Load(sprite_location.c_str()));
        if (sprite == NULL)
        {
            std::cout << "Error loading button " << sprite_location<< "  : " << IMG_GetError() << std::endl;
        }

       spriteLocation = sprite_location;
        }
        use = func;
    }
    button(int x, int y, int w, int h, std::string font, std::string text, SDL_Color* c,functionWrapper func,SDL_Renderer* see = render)
    {
        rect = new SDL_Rect{x, y, w, h};
        sprite = write(font, text, c, 64, render);
       if (func != NULL)
       {
       use= func;
       }
    }
       button(int x, int y, int w, int h, std::string font, std::string text, int r, int g, int b,functionWrapper func,SDL_Renderer* see = render)
    {
        SDL_Color c;
        if (r >= 0 && g>= 0 && b>=0)
        {
            c = {r,g,b};
        }
        rect = new SDL_Rect{x, y, w, h};
        sprite = write(font, text, &c, 64, render);
       if (func != NULL)
       {
       use= func;
       }
    }
    button()
    {

    }
    virtual void function()
    {
       // std::cout << "A" << std::endl;
        if (use != NULL)
        (*use)();
    }
};


struct screen
{
std::vector<button*>* buttons  = NULL;
    std::unique_ptr<SDL_Texture,std::function<void(SDL_Texture*)>> bg ;
   std::vector< std::shared_ptr<paper> >sheets;
    std::function<void()>* first = NULL;
    std::function<void()>* last = NULL;
    std::function<void()>* use = NULL;
    screen(const char* background,  functionWrapper func = NULL, functionWrapper f = NULL, functionWrapper la = NULL, SDL_Renderer* renderer = render,paper* message = NULL,std::vector<button*>* l = NULL)
    {
            buttons = l;
         bg =std::unique_ptr<SDL_Texture,std::function<void(SDL_Texture*)>> (nullptr, &SDL_DestroyTexture);
         if ((std::string)background != "" )
         {
            SDL_Surface* surf = IMG_Load(background);
            if (surf == NULL)
            {
                std::cout << "Error loading background: " + (std::string)background + " " + IMG_GetError() << std::endl;
             }
            bg.reset(SDL_CreateTextureFromSurface(renderer, surf));
            if ( bg == NULL )
            {
                std::cout << "Error loading background: " + (std::string)background + " " + IMG_GetError() << std::endl;
            }
         }
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
            sheets.push_back(std::unique_ptr<paper>(message));
            message->changeDisplay(true);
        }

    }
        screen(std::string background, functionWrapper func = NULL, functionWrapper f = NULL, functionWrapper la = NULL, SDL_Renderer* renderer = render,std::vector<paper*>* message = NULL,std::vector<button*>* l = NULL)
    {

        screen(background.c_str(), func, f, la, render, NULL, l);
        if (message)
        {
            int size = message->size();
            for (int i = 0; i < size; i++)
            {
                     sheets.push_back(std::unique_ptr<paper>(message->at(i)));
                     message->at(i)->changeDisplay(true);
            }
            delete message;
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
    ~screen()
    {
        if (buttons)
    buttons->clear();

    }
};




struct contact // a struct that holds client contact info. a seomwhat serialized form of client
{
    int port;//port is this contact's port. serv is the port the server uses to connect to the client
   std:: string name;
    paper sheet, moves; // two paper objects to handle two sprites
    contact(std::string title, int p )
    {
        port = p;
        name = title;
        //address = a;
    }
    std::string packet()
    {
        std::ostringstream os;
        os << port;
        return os.str() + "|" + name;
    }

};

 bool send(UDPpacket* pack,UDPsocket sock, int otherPort,  const char* address, std::string message)
    {
       // UDPpacket* pack = SDLNet_AllocPacket(512);
        pack->len = message.length();
        memcpy(pack->data, message.c_str(), message.length());
        IPaddress ip;
        SDLNet_ResolveHost(&ip, address, otherPort );
        pack->address.host = ip.host;
        pack->address.port = ip.port;
      //  std::cout << "Sent: " << pack->data << std::endl;
 //     std::cout << &sock << std::endl;
   // std::cout << otherPort << std::endl;
   //std::cout << &pack << std::endl;
       if ( SDLNet_UDP_Send(sock, -1, pack) == 0)
       {
          std:: cout << "Error sending data: " << SDLNet_GetError( ) << std::endl;
           return false;
       }
       else
       {
          return true;
       }

    }

    int charCount(std::string s, char c)
    {
       unsigned int fin = 0;
        for (unsigned int x = 0; x < s.length(); x++)
        {
            if (s[x] == c)
            {
                fin++;
            }
        }
        return fin;
    }

    std::string* divideString(std::string input) //divides a string into ints
    {
        input +=" ";
        int howBig = charCount(input, ' ') + charCount(input, '@');
        std::string *arr = new std::string[howBig];
        int l = 0;
        std::string seg = "";
        for (unsigned int v = 0; v< input.length(); v++)
        {
            if ((input[v] == ' ' || input[v] == '@' )&& (input[v-1] != ' ' || input[v-1] != '@'))
            {
                arr[l] = (seg);
                l++;
                seg = "";
            }
            else if (input[v] != ' ')
                seg+= input[v];

         //       std::cout << seg << std::endl;
        }
      /*  for (unsigned int u = 0; u < sizeof(arr)/sizeof(int); u ++)
        {
//std::            cout << arr[u] << std::endl;
        }*/
        return arr;
    }

    int gcf(int x, int y)
    {
        x = abs(x);
        y = abs(y);
        if (x== y)
        {
            return x;
        }
        else if (x == 1 || y == 1)
        {
            return 1;
        }
        if (y > x)
        {
            return gcf(x, y-x);
        }
        else
            return gcf(x-y, y);

    }


template<typename T>
void addAll(std::vector<T>& a, std::vector<T>& b)
{
    int size = b.size();
    for (int x = 0; x < size; x++)
    {
        a.push_back(b[x]);
    }
}


#endif // RESOURCES_H_INCLUDED
