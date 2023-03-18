#include <limits.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

int convertTo1(double number) // a method that converts a number to 1 or -1 depending on its sign. If entry is 0, return 0;
{
    if (number == 0)
    {
        return 0;
    }
    return number/(fabs(number));
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

void fastPrint(std::string str)
{
    int size = str.size();
    for (int i = 0; i < size; i ++)
    {
        putchar(str[i]);
    }
}



void fillBytesVecWork( std::vector<char>& bytesVec,size_t bytes,int finalBytes)//base case after all attributes have been processeed
{
    if (bytes < finalBytes) //if we underprovided data, replace the rest of the data with 0s
    {
        bytesVec.resize(bytesVec.size()+ finalBytes - bytes,'\0');
    }
}


