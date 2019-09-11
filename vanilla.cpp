   #include "vanilla.h"

    Named::Named(std::string nameTag)
    {
        name = nameTag;
    }

std::string Named::getName() const
    {
        return name;
    }

    IDed::IDed(int ID)
    {
        id = ID;
    }

    int IDed::getID()
    {
        return id;
    }

    NumberManager::NumberManager()
    {

    }
    void NumberManager::addNumber(double number)
    {
        int size = numbers.size();
        for(int i = 0; i < size; i ++)
        {
            if (number <= numbers[i])
            {
                numbers.insert(numbers.begin() + i,number);
                return;
            }
        }
        numbers.push_back(number);
    }
    int NumberManager::findNumber(double code)
    {
        int lower = 0;
        int higher = numbers.size();
        int index = -1;
        if (higher != 0)
        {
            index = (lower+higher)/2;
            while(numbers[index] != code) // binary search to find the key code
            {
                int current = numbers[index];
                if (current < code)
                {
                    lower = index;
                }
                else
                {
                    higher = index;
                }
                index = (lower+higher)/2;
                if (current == numbers[index])
                {
                    return -1;
                }
            }
        }
        return index;

    }
    void NumberManager::removeNumber(double number)
    {
        int index = findNumber(number);
        if (index != -1)
        {
            numbers.erase(numbers.begin() + index);
        }
    }
    NumberManager::~NumberManager()
    {
        numbers.clear();
    }
 Rect Polygon::getBoundingRect() const
{
   Rect r = {minX,minY, maxX - minX, maxY - minY};
  // printRect(r);
   return r;
}
bool rectIntersectWithinRange(const Rect& r1, double dist, const Rect& r2) //checks if a rectange 2dist larger on all dimension than r1 would intersect with r2
{
    return RectIntersect({r1.x - dist, r1.y - dist, r1.w + 2*dist, r1.h + 2*dist}, r2);
}
bool rectIntersectWithinRange(float x1, float y1, float w1, float h1, double dist, float x2, float y2, float w2, float h2)
{
    return rectIntersectWithinRange({x1,y1,w1,h1},dist,{x2,y2,w2,h2});
}
    void printPoint(const Point& p)
{
    std::cout << p.x << " " << p.y << std::endl;
}

void printRect(const Rect& r)
{
    std::cout << r.x << " " << r.y << " " << r.w << " " << r.h << std::endl;
}
bool PointInRect(const Point& point, const Rect& rect) //returns true if the point is in the rect or on the borders
{
    return ((point.x >= rect.x && point.x <= rect.x + rect.w) && (point.y >= rect.y && point.y <= rect.y + rect.h));
}
bool LineAndLineIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
{
    Point intersect (0,0);
    bool value = false;
    if (a.x == b.x)
    {
        intersect.x = a.x;
        double slope = (c.y - d.y)/(c.x - d.x);
        intersect.y = slope*a.x + c.y - slope*c.x;
    }
    else if (c.x == d.x)
    {
        intersect.x = c.x;
        double slope = (a.y - b.y)/(a.x - b.x);
        intersect.y = slope*c.x + a.y - slope*a.x;
    }
    else
    {
        double slope1 = (a.y - b.y)/(a.x - b.x);
        double slope2 = (c.y - d.y)/(c.x - d.x);
        double cept1 = a.y - slope1*a.x;
        double cept2 = c.y - slope2*c.x;
      //  std::cout << cept1 << " " << cept2 << std::endl;
        if (slope1 == slope2 )
        {
            if (cept1 == cept2)
            {
                intersect.x = std::max(std::min(a.x,b.x),std::min(c.x,d.x)); //finds the second smallest point out of the four points
                intersect.y = std::max(std::min(a.y,b.y),std::min(c.y,d.y));
            }
        }
        else
        {
            intersect.x = (cept1 - cept2)/(slope2 - slope1);
            intersect.y = slope1*intersect.x + cept1;
        }
    }

        value = ( (intersect.x >= std::min(a.x,b.x) && intersect.x <= std::max(a.x,b.x)) && (intersect.x >= std::min(c.x,d.x) && intersect.x <= std::max(c.x,d.x)) &&
                  (intersect.y >= std::min(a.y,b.y) && intersect.y <= std::max(a.y,b.y)) && (intersect.y >= std::min(c.y,d.y) && intersect.y <= std::max(c.y,d.y)));
           // std::cout << intersect.x << " " << intersect.y << std::endl;
          //  std::cout << "value: " <<  value << std::endl;


    return value;
}

bool LineAndLineIntersect(const Line& l1, const Line& l2)
{
    return LineAndLineIntersect(l1.a,l1.b, l2.a,l2.b);
}

bool LineAndRectIntersect(const Rect& r, double x1, double y1, double x2, double y2 )
{
    Point a = {x1,y1};
    Point b = {x2,y2};
    Point topLeft = {r.x,r.y};
    Point topRight = {r.x + r.w, r.y};
    Point bottomLeft = {r.x, r.y + r.h};
    Point bottomRight = {r.x + r.w, r.y + r.h};
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
bool RectIntersect(const Rect& r1, const Rect& r2)
{
    if (r1.x <= r2.x + r2.w && r1.x + r1.w >= r2.x && r1.y <= r2.y + r2.h && r1.y + r1.h >= r2.y)
    {
        return true;
    }
    return false;

}

int convertTo1(double number) // a method that converts a number to 1 or -1 depending on its sign. If entry is 0, return 0;
{
    if (number == 0)
    {
        return 0;
    }
 //   std::cout << number/fabs(number);
    return number/(fabs(number));
}

double PointDistance(const Point& p1, const Point& p2)
{
    return sqrt(pow((p1.x - p2.x),2) + pow(p1.y - p2.y,2));
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

double randomDecimal(int places)
{
    int power = pow(10, places);
    return rand()%(power)/((double)power);
}

double convert(std::string input)
{
  for (int i = 0; i < input.length(); i ++) // checks to see if the input has non-digits in it.
    {
        int ascii = int(input[i]);
        if ((ascii < 48 || ascii > 57) && ascii != 46 && ascii != 45) // The ascii for digits 0-9 is 48 - 57 inclusive. 46 is the ascii for the decimal point. 45 is the negative sign.
        {
            std::cout << "Vanilla Warning: conversion of " + input << "will return 0 because input contains non-numbers." << std::endl; // if there are non-digits, warn the user and then return 0.
            return 0;
        }
    }
    double j;
     std::istringstream os(input);
    os >> j;
    return j;
}

int charCount(std::string s, char c) //returns how many times c shows up in s
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
double absMin(double x, double y)
{
    if (abs(x) <= abs(y))
    {
        return x;
    }
    return y;
}
double absMax(double x, double y)
{
    if (abs(x) >= abs(y))
    {
        return x;
    }
    return y;
}

int randWithGaps(const std::vector<std::pair<int,int>>& ranges)
{
    int range = rand()%ranges.size();
    const std::pair<int,int>* pair = &ranges[range];
    return rand()%(pair->second-pair->first) + pair->first;
}

     std::string* divideString(std::string input) //divides a string into parts and puts them all into an array
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
