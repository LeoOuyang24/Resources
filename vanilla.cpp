#include <limits.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

#include "vanilla.h"

int convertTo1(double number) // a method that converts a number to 1 or -1 depending on its sign. If entry is 0, return 0;
{
    if (number == 0)
    {
        return 0;
    }
    return number/(fabs(number));
}

std::pair<std::string,bool> readFile(std::string file)
{
    std::ifstream input;
    input.open(file);
    if (input.is_open())
    {
        std::stringstream stream;
        stream << input.rdbuf();
        input.close();

        return {stream.str(),true};
    }
    return {"",false};
}

bool floatEquals(float a, float b, int precision)
{
    return (int)(pow(10,precision)*a) == (int)(pow(10,precision)*b);
}

size_t hashCombine( size_t lhs, size_t rhs ) {
  lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
  return lhs;
}

float round(float decimal, int n)
{
    int ten = pow(10,n);
    return ((float)floor(decimal*ten))/ten;
}

bool angleRange(float rad1, float rad2, float range)
{
    float clamped =remainder(rad1,2*M_PI);
    float clampedMax =remainder(rad2 + range,2*M_PI);
    float clampedMin = remainder(rad2 - range,2*M_PI);
    if (clampedMin > clampedMax)
    {
        clampedMax += 2*M_PI;
        clamped += 2*M_PI*(clamped < 0);
    }
    return clamped <=  clampedMax && clamped >= clampedMin;
}

double randomDecimal(int places)
{
    int power = pow(10, places);
    return rand()%(power)/((double)power);
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

void fastPrint(std::string str)
{
    int size = str.size();
    for (int i = 0; i < size; i ++)
    {
        putchar(str[i]);
    }
}


