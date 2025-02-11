#define _USE_MATH_DEFINES
#include <math.h>

#include "positionals.h"


Positional::Positional(const glm::vec2& point) : pos(point)
{

}

bool Positional::collides(const glm::vec4& box)
{
    return pointInVec(box,pos.x,pos.y,0);
}

bool Positional::collidesLine(const glm::vec4& line)
{
    return pointLineDistance(line,pos)==0;
}

glm::vec4 Positional::getBoundingRect() const
{
    return {pos.x,pos.y,0,0};
}

glm::vec2 Positional::getPos() const
{
    return pos;
}

glm::vec2 Positional::getCenter() const
{
    return pos;
}

float Positional::getTilt() const
{
    return tilt;
}

void Positional::setPos(const glm::vec2& pos)
{
    this->pos = pos;
}

void Positional::setTilt(float newTilt) //clamp to -M_PI - +M_PI, just like C++ trig functions
{
    tilt = fmod(newTilt,2*M_PI) + 2*M_PI*(newTilt < 0);
    if (tilt >M_PI)
    {
        tilt -= 2*M_PI;
    }
}
RectPositional::RectPositional(const glm::vec4& box, float newTilt) : Positional({box.x,box.y}), rect(box)
{
    setTilt(newTilt);
}

glm::vec4 RectPositional::getBoundingRect() const
{
    if (tilt == 0)
    {
        return rect;
    }
    else
    {
        glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
        glm::vec2 topLeft =rotatePoint({rect.x,rect.y},center,tilt),
                botLeft = rotatePoint({rect.x,rect.y + rect.a},center,tilt),
                topRight = rotatePoint({rect.x + rect.z,rect.y},center,tilt),
                botRight = rotatePoint({rect.x + rect.z, rect.y + rect.a},center,tilt);
        glm::vec2 xy = glm::vec2(std::min(botRight.x,std::min(topRight.x,std::min(topLeft.x,botLeft.x))),
                                 std::min(botRight.y,std::min(topRight.y,std::min(topLeft.y,botLeft.y))));
        return glm::vec4(xy.x,xy.y,
                         std::max(botRight.x,std::max(topRight.x,std::max(topLeft.x,botLeft.x))) - xy.x, //yes I know that I can calculate the dimensions based on xy.x and xy.y without finding the largest x or y but do I care???
                         std::max(botRight.y,std::max(topRight.y,std::max(topLeft.y,botLeft.y))) - xy.y);
    }
}

glm::vec2 RectPositional::getPos() const
{
    return {rect.x,rect.y};
}

const glm::vec4& RectPositional::getRect() const
{
    return rect;
}



void RectPositional::setPos(const glm::vec2& pos)
{
    rect.x = pos.x;
    rect.y = pos.y;
}

void RectPositional::setCenter(const glm::vec2& pos)
{
    rect.x = pos.x - rect.z/2;
    rect.y = pos.y - rect.a/2;
}

void RectPositional::setRect(const glm::vec4& rect_)
{
    rect = rect_;
}

bool RectPositional::collidesLine(const glm::vec4& line)
{
    return lineInVec({line.x,line.y},{line.z,line.a},rect,tilt);
}

bool RectPositional::collides(const glm::vec4& box)
{
    return vecIntersect(box,rect,tilt,0);
}

glm::vec2 RectPositional::getCenter() const
{
    return {rect.x + rect.z/2, rect.y + rect.a/2};
}

CirclePositional::CirclePositional(const glm::vec2& pos, float radius_) : Positional(pos), radius(radius_)
{

}

bool CirclePositional::collides(const glm::vec4& box)
{
    return pointVecDistance(box,pos.x,pos.y) <= radius;
}

bool CirclePositional::collidesLine(const glm::vec4& line)
{
    return pointLineDistance(line,pos) <= radius;
}

float CirclePositional::distance(const glm::vec2& point)
{
    float distance = glm::length(pos - point);
    return std::max(distance - radius, distance);
}

glm::vec4 CirclePositional::getBoundingRect() const
{
    return {pos.x - radius, pos.y, radius*2, radius*2};
}
