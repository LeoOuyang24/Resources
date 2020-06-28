#ifndef GLGAME_H_INCLUDED
#define GLGAME_H_INCLUDED

#include <glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <vector>
#include <memory>

#include "render.h"

bool rectPathIntersect(const glm::vec4& oldRect, const glm::vec4& newRect, const glm::vec4& collide); //returns true if oldRect collided with collide on its path to newRect

class Positional //anything that has a position
{
protected:
    glm::vec2 pos;
public:
    Positional(const glm::vec2& point);
    virtual bool collides(const glm::vec4& box); //how to determine whether or not this thing collides with a rect (used in quadtree)
    virtual double distance(const glm::vec2& point) //finds how far this positional is from a point (used in to find all objects within a distance
     {
         return pointDistance(point, pos);
     }
    virtual glm::vec2 getPos() const;
    virtual ~Positional()
    {
    }
};
class RectPositional : public Positional //anything whose position is represented by a rectangle
{
protected:
    glm::vec4 rect;
public:
    RectPositional(const glm::vec4& box);
    virtual bool collides(const glm::vec4& box);
    virtual double distance(const glm::vec2& point)
    {
        return pointVecDistance(rect, point.x, point.y);
    }
    virtual glm::vec2 getPos() const;
    const glm::vec4& getRect() const;
    glm::vec2 getCenter() const;
};

typedef std::vector<std::shared_ptr<Positional>> pointerVec;
typedef std::vector<Positional*> positionalVec;

class QuadTree //this is a quadtree of shared_ptr, meaning this quadtree actually owns its objects
{
    static constexpr int maxCapacity = 100; //maximum capacity
    pointerVec vec;
    glm::vec4 region;
    void split();
    bool contains(Positional& positional); //returns whether or not the quadtree actually contains the positional, not whether or not it should contain the positional
    QuadTree* nodes[4] = {nullptr,nullptr,nullptr,nullptr};
    void move(QuadTree& t1, QuadTree& t2, Positional& obj); //given that obj is in t1, this removes obj and adds it into t2
    void getNearestHelper(positionalVec& vec, const glm::vec2& center, double radius);
    void getNearestHelper(positionalVec& vec, const glm::vec4& area );
    void getNearestHelper(positionalVec& vec, Positional& obj);
public:
    QuadTree(const glm::vec4& rect);
    void render(const glm::vec2& displacement);
    ~QuadTree();
    int count (); //total number of things in this quadtree and its children
    int size(); //number of things in this quadtree
    void remove(Positional& obj); //removes the obj. This only removes the pointer; doesn't actually delete the object
    QuadTree* find(Positional& obj); //finds the quadtree obj belongs in. Returns null if the obj doesn't belong. Can return this QuadTree.
    positionalVec getNearest( Positional& obj); //get all Positionals that are in the same quadtree as obj
    positionalVec getNearest( const glm::vec4& area); //get all Positionals that intersect with area
    positionalVec getNearest(const glm::vec2& center, double radius); //finds all objects within a certain radius
    void add(Positional& obj);
    void add(const std::shared_ptr<Positional>& obj);
    void clear();
    void map(void (*fun)(const Positional& pos)); //applies pos to all objects in tree as well as children
    QuadTree* update(Positional& obj, QuadTree& expected); //given an obj and its expected quadtree, checks to see if the obj is where its expected. If it has moved, change and return its new location
    inline const glm::vec4& getRect()
    {
        return region;
    }
};


class RawQuadTree
{
    static const int maxCapacity; //maximum capacity
    positionalVec vec;
    glm::vec4 region;
    void split();
    bool contains(Positional& positional);
    RawQuadTree* nodes[4] = {nullptr,nullptr,nullptr,nullptr};
    void move(RawQuadTree& t1, RawQuadTree& t2, Positional& obj); //given that obj is in t1, this removes obj and adds it into t2
    void getNearestHelper(positionalVec& vec, const glm::vec2& center, double radius);
    void getNearestHelper(positionalVec& vec, const glm::vec4& area );
    void getNearestHelper(positionalVec& vec, Positional& obj);

public:
    RawQuadTree(const glm::vec4& rect);
    void render(const glm::vec2& displacement);
    ~RawQuadTree();
    int count (); //total number of things in this quadtree and its children
    int size(); //number of things in this quadtree
    void remove(Positional& obj); //removes the obj. This only removes the pointer; doesn't actually delete the object
    RawQuadTree* find(Positional& obj); //finds the quadtree obj belongs in. Returns null if the obj doesn't belong. Can return this QuadTree.
    positionalVec getNearest( Positional& obj); //get all Positionals that are in the same quadtree as obj
    positionalVec getNearest( const glm::vec4& area); //get all Positionals that intersect with area
    positionalVec getNearest(const glm::vec2& center, double radius); //finds all objects within a certain radius
    void add(Positional& obj);
    void clear();
    RawQuadTree* update(Positional& obj, RawQuadTree& expected); //given an obj and its expected quadtree, checks to see if the obj is where its expected. If it has moved, change and return its new location
    inline const glm::vec4& getRect()
    {
        return region;
    }
};

#endif // GLGAME_H_INCLUDED
