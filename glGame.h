#ifndef GLGAME_H_INCLUDED
#define GLGAME_H_INCLUDED

#include <glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <vector>
#include <memory>

bool rectPathIntersect(const glm::vec4& oldRect, const glm::vec4& newRect, const glm::vec4& collide); //returns true if oldRect collided with collide on its path to newRect

class Positional //anything that has a position
{
protected:
    glm::vec2 pos;
public:
    Positional(const glm::vec2& point);
    virtual bool collides(const glm::vec4& box); //how to determine whether or not this thing collides with a rect (used in quadtree)
    virtual glm::vec2 getPos() const;
    ~Positional()
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
    virtual glm::vec2 getPos() const;
    const glm::vec4& getRect() const;
};

class QuadTree
{
    static const int maxCapacity; //maximum capacity
    std::vector<std::shared_ptr<Positional>> vec;
    glm::vec4 region;
    void split();
    bool contains(Positional& positional);
    QuadTree* nodes[4] = {nullptr,nullptr,nullptr,nullptr};
    void move(QuadTree& t1, QuadTree& t2, Positional& obj); //given that obj is in t1, this removes obj and adds it into t2
public:
    QuadTree(const glm::vec4& rect);
    void render(const glm::vec2& displacement);
    ~QuadTree();
    int count ();
    std::shared_ptr<Positional>* remove(Positional& obj); //removes the shared_ptr of obj. If the ptr is the only ptr, the obj will also be deleted. Returns true if obj is found and removed but not necessarily deleted
    QuadTree* find(Positional& obj); //finds the quadtree obj belongs in. Returns null if the obj doesn't belong. Can return this QuadTree.
    void getNearest(std::vector<Positional*>& vec, Positional& obj); //get all Positionals that are in the same quadtree as obj
    void getNearest(std::vector<Positional*>& vec, const glm::vec4& area); //get all Positionals that intersect with area
    void add(Positional& obj);
    void add(std::shared_ptr<Positional>& ptr);
    void add(std::shared_ptr<Positional>&& ptr); //adds ptr to vec. This should only be called if it is known that this quadtree should contain ptr
    void clear();
    QuadTree* update(Positional& obj, QuadTree& expected); //given an obj and its expected quadtree, checks to see if the obj is where its expected. If it has moved, change and return its new location
};

class RawQuadTree
{
    static const int maxCapacity; //maximum capacity
    std::vector<Positional*> vec;
    glm::vec4 region;
    void split();
    bool contains(Positional& positional);
    RawQuadTree* nodes[4] = {nullptr,nullptr,nullptr,nullptr};
    void move(RawQuadTree& t1, RawQuadTree& t2, Positional& obj); //given that obj is in t1, this removes obj and adds it into t2
public:
    RawQuadTree(const glm::vec4& rect);
    void render(const glm::vec2& displacement);
    ~RawQuadTree();
    int count (); //total number of things in this quadtree and its children
    int size(); //number of things in this quadtree
    Positional* remove(Positional& obj); //removes the shared_ptr of obj. If the ptr is the only ptr, the obj will also be deleted. Returns true if obj is found and removed but not necessarily deleted
    RawQuadTree* find(Positional& obj); //finds the quadtree obj belongs in. Returns null if the obj doesn't belong. Can return this QuadTree.
    void getNearest(std::vector<Positional*>& vec, Positional& obj); //get all Positionals that are in the same quadtree as obj
    void getNearest(std::vector<Positional*>& vec, const glm::vec4& area); //get all Positionals that intersect with area
    void add(Positional& obj);
    void clear();
    RawQuadTree* update(Positional& obj, RawQuadTree& expected); //given an obj and its expected quadtree, checks to see if the obj is where its expected. If it has moved, change and return its new location
    inline const glm::vec4& getRect()
    {
        return region;
    }
};

#endif // GLGAME_H_INCLUDED
