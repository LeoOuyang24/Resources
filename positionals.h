#ifndef POSITIONALS_H_INCLUDED
#define POSITIONALS_H_INCLUDED

#include "geometry.h"

class Positional //anything that has a position
{
protected:
    glm::vec2 pos;
    float tilt = 0; //angle we tilted at. Used for collision detection

public:
    Positional(const glm::vec2& point);
    virtual bool collides(const glm::vec4& box); //how to determine whether or not this thing collides with a rect (used in quadtree)
    virtual bool collidesLine(const glm::vec4& line); //returns if this collides with a line
    virtual float distance(const glm::vec2& point) //finds how far this positional is from a point (used in to find all objects within a distance
     {
         return pointDistance(point, pos);
     }
    virtual glm::vec4 getBoundingRect() const;
    virtual glm::vec2 getPos() const;
    virtual glm::vec2 getCenter() const;
    virtual float getTilt() const;


    virtual void setPos(const glm::vec2& pos);
    void setTilt(float tilt_);

};
class RectPositional : public Positional //anything whose position is represented by a rectangle
{
protected:
    glm::vec4 rect;
public:
    RectPositional(const glm::vec4& box, float tilt = 0);
    virtual bool collides(const glm::vec4& box);
    virtual bool collidesLine(const glm::vec4& line);
    virtual float distance(const glm::vec2& point)
    {
        return pointVecDistance(rect, point.x, point.y,tilt);
    }
    glm::vec4 getBoundingRect() const;
    virtual glm::vec2 getPos() const;
    const glm::vec4& getRect() const;
    glm::vec2 getCenter() const;

    void setPos(const glm::vec2& pos);
    void setCenter( const glm::vec2& pos);
    void setRect(const glm::vec4& rect);
};

class CirclePositional : public Positional
{
protected:
    float radius = 0;
public:
    CirclePositional(const glm::vec2& pos, float radius);
    virtual bool collides(const glm::vec4& box);
    virtual bool collidesLine(const glm::vec4& line);
    float distance(const glm::vec2& point);

    glm::vec4 getBoundingRect() const;
};

#endif // POSITIONALS_H_INCLUDED
