#ifndef GLGAME_H_INCLUDED
#define GLGAME_H_INCLUDED

#include <glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <vector>
#include <memory>

#include "geometry.h"
#include "render.h"
#include "SDLhelper.h"

bool rectPathIntersect(const glm::vec4& oldRect, const glm::vec4& newRect, const glm::vec4& collide); //returns true if oldRect collided with collide on its path to newRect

class Positional //anything that has a position
{
protected:
    glm::vec2 pos;
public:
    Positional(const glm::vec2& point);
    virtual bool collides(const glm::vec4& box); //how to determine whether or not this thing collides with a rect (used in quadtree)
    virtual bool collidesLine(const glm::vec4& line); //returns if this collides with a line
    virtual double distance(const glm::vec2& point) //finds how far this positional is from a point (used in to find all objects within a distance
     {
         return pointDistance(point, pos);
     }
    virtual glm::vec4 getBoundingRect() const;
    virtual glm::vec2 getPos() const;
    virtual ~Positional()
    {
    }
};
class RectPositional : public Positional //anything whose position is represented by a rectangle
{
protected:
    glm::vec4 rect;
    float tilt = 0; //angle we tilted at. Used for collision detection
public:
    RectPositional(const glm::vec4& box, float tilt = 0);
    virtual bool collides(const glm::vec4& box);
    virtual bool collidesLine(const glm::vec4& line);
    virtual double distance(const glm::vec2& point)
    {
        return pointVecDistance(rect, point.x, point.y);
    }
    glm::vec4 getBoundingRect() const;
    virtual glm::vec2 getPos() const;
    const glm::vec4& getRect() const;
    float getTilt() const;
    void setTilt(float tilt_);
    glm::vec2 getCenter() const;
};

class RectPosCamera : public RenderCamera  //a camera that follows a RectPositional
{
    std::weak_ptr<RectPositional> followee;
public:
    void init(const std::shared_ptr<RectPositional>& followee_, int w, int h);
    void setFollowee(const std::shared_ptr<RectPositional>& followee_);
    void update();
};



typedef  bool (*PositionalCompare) (const Positional& pos1, const Positional& pos2);

class PosWrapper
{
    //once upon a time, Quadtree was a quadtree that only held shared pointers. As a result, RawQuadTree was created to hold raw pointers. The problem is,
    //their code is nearly identical, just one handles raw pointers instead of smart ones. A generic way of handling both was needed. PosWrapper is that
    //solution: it is a very simple parent class with a single function that returns a pointer to a Positional. This is all we need to generalize raw and
    //smart pointers
public:
    virtual Positional* get() const
    {
        std::cout << "ERROR\n";
        return nullptr;
    }
    virtual ~PosWrapper()
    {

    }
};

struct SharedWrapper : public PosWrapper
{
    std::shared_ptr<Positional> ptr;
    SharedWrapper(Positional* ptr_) : ptr(ptr_)
    {

    }
    Positional* get() const
    {
        return ptr.get();
    }
};

struct WeakWrapper : public PosWrapper
{
    std::weak_ptr<Positional> ptr;
    WeakWrapper(const std::shared_ptr<Positional>& ptr_) : ptr(ptr_)
    {

    }
    Positional* get() const
    {
        return ptr.lock().get();
    }
};

struct RawWrapper : public PosWrapper
{
    Positional* ptr;
    RawWrapper(Positional* ptr_) : ptr(ptr_)
    {

    }
    Positional* get() const
    {
        return ptr;
    }
};

typedef std::vector<std::unique_ptr<PosWrapper>> pointerVec;
typedef std::vector<Positional*> positionalVec;

class QuadTree //quadtree for 2d collision detection
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
    void render(RenderCamera& camera); //calls camera.toScreen() on the rect;
    ~QuadTree();
    int count (); //total number of things in this quadtree and its children
    int size(); //number of things in this quadtree
    bool remove(Positional& obj, PositionalCompare func = nullptr); //removes and deletes the obj. Optionally, the user can pass in a function that tells us how we know we've found the right positional
    QuadTree* find(Positional& obj); //finds the quadtree obj belongs in. Returns null if the obj doesn't belong. Can return this QuadTree.
    positionalVec getNearest( Positional& obj); //get all Positionals that are in the same quadtree as obj
    positionalVec getNearest( const glm::vec4& area); //get all Positionals that intersect with area
    positionalVec getNearest(const glm::vec2& center, double radius); //finds all objects within a certain radius
    void add(PosWrapper& obj);
    std::shared_ptr<Positional>& add(Positional& pos); //adds positional in as a SharedWrapper and returns the shared pointer
    void clear();
    void map(void (*fun)(const Positional& pos)); //applies pos to all objects in tree as well as children
    QuadTree* update(Positional& obj, QuadTree& expected); //given an obj and its expected quadtree, checks to see if the obj is where its expected. If it has moved, change and return its new location
    inline const glm::vec4& getRect()
    {
        return region;
    }
};

#include <set>
class BiTree
{
    struct BiTreeElement //represents the element plus the rect that represents it
    {
        Positional* positional;
        glm::vec4 rect;
    };

    struct BiTreeScore
    {
        constexpr static int roundNth = 4; //nth place to round to
        float nodeY, elementX; //the 2 elements of our score
        bool operator < (const BiTreeScore& other) const
        {
            return round(nodeY,roundNth) == round(other.nodeY,roundNth) ? round(elementX,roundNth) < round(other.elementX,roundNth) : round(nodeY,roundNth) < round(other.nodeY,roundNth);
        }
        bool operator == (const BiTreeScore& other) const
        {
          return (round(nodeY,roundNth) == round(other.nodeY,roundNth) && round(elementX,roundNth) == round(other.elementX,roundNth));
        }
        bool operator <= (const BiTreeScore& other) const
        {
            return *this < other || *this == other;
        }
        std::string toString() const
        {
            return convert(nodeY) + " " + convert(elementX);
        }
    };

    typedef std::multimap<BiTreeScore,BiTreeElement> BiTreeStorage; //stores all elements. Maps score to element
    static constexpr int maxNodeSize = 256; //maximum number of elements in a node
    struct BiTreeNode
    {
        /*BiTreeNodes have 2 modes: Whole and Split
            Whole: BiTreeNodes have no non-null children, but start and size work as intended. Size represents the number of elements in the node
            Split: BiTreeNode has 2 non-null children and start and size is neglected
        */
        glm::vec2 vertDimen; //y coordinate and height of the node
        int size = 0; //represents size if whole
        BiTreeStorage::iterator start; //the iterator to where in the storage the node begins
        BiTreeNode* nodes[2] = {0,0}; //pointer to children
    };

    BiTreeStorage elements;
    BiTreeNode head;
    glm::vec4 region;

    template<typename Callable>
    auto processElement(Callable func, Positional& wrap, const glm::vec4& rect)
    {
        return processElement(func,wrap,rect,head);
    }

    template<typename Callable>
    auto processElement(Callable func, Positional& wrap, const glm::vec4& rect,  BiTreeNode& node)
    {
        /*given a positionwrapper, calls func on it and each of its subdivisons. func should be a function that
        takes in a const BiTreeElement& and a BiTreeNode. If func has a return value, we return the value for the function call of the
        lowest scoring subdivision.*/
        if (!node.nodes[0]) //found a leaf, which must be the node the element belongs in
        {
            return func({&wrap,rect},node);
        }
        else //otherwise keep searching
        {
            float split = node.vertDimen.x + node.vertDimen.y/2; //midpoint of the node where the split between the children happened
            if (rect.y < split && rect.y + rect.a > split) //we have an element that crosses the border, so we have to split it
            {
                float botHeight = rect.y + rect.a - split; //height of bottomt chunk
                processElement(func,wrap,{rect.x,split,rect.z,botHeight},*node.nodes[1]); //process bottom chunk
                return processElement(func,wrap,{rect.x,rect.y,rect.z,rect.a - botHeight},*node.nodes[0]); //process top chunk. Return this value because it must be precede the bottom chunk
            }
            else //element doesn't cross border, so just process it in the node it is in
            {
                if (rect.y < split) //top chunk
                {
                    return processElement(func,wrap,rect,*node.nodes[0]);
                }
                else //bottom chunk
                {
                    return processElement(func,wrap,rect,*node.nodes[1]);
                }
            }
        }

    }
    BiTreeScore calculateScore(const BiTreeElement& element, BiTreeNode& node); //given wrapper and the node it belongs in, calculates the score used in scoring
    BiTreeStorage::iterator insert(const BiTreeElement& element, BiTreeNode& node); //inserts element into node. Does not split node
    void updateNode(BiTreeStorage::iterator& it, BiTreeNode& node); //given it, updates a node assuming it is in that node. Doesn't actually insert it
public:
    BiTree(const glm::vec4& region_);
    unsigned int size()
    {
        return elements.size();
    }
    void insert(Positional& wrap); //calculates the node wrap belongs in and inserts it, splitting nodes if necesesary
    BiTreeStorage::iterator remove(Positional& wrap);
    template<typename Callable>
    void processCollisions(Callable func) //func should take in 2 RectPositional&
    { //REFACTOR: currently only works with rect positionals
        auto it = elements.begin();
        auto end = elements.end();
        while (it != end)
        {
            float xBorder = it->second.rect.x + it->second.rect.z; //furthest x coordinate an entity can have before we stop checking
            RectPositional* r1 = static_cast<RectPositional*>(it->second.positional);
            //PolyRender::requestRect(it->second.rect,{1,1,0,1},false,0,2);
            auto candidate = std::next(it);
            int count = 0;
           // printRect(r1->getRect());
            while (candidate != end)
            {
               // std::cout <<"\t" << candidate->second.positional <<" ";
               // printRect(candidate->second.rect);
                if (candidate->second.positional == it->second.positional ||
                    candidate->second.rect.x > xBorder ||
                    candidate->second.rect.x < it->second.rect.x)
                {
                    break;
                }
                else
                {
                    RectPositional* r2 = static_cast<RectPositional*>(candidate->second.positional);
                    if (vecIntersect(r1->getRect(),r2->getRect(),r1->getTilt(),r2->getTilt()))
                    {
                        func(*r1,*r2);
                       // PolyRender::requestRect(candidate->second.rect,{1,0,0,1},false,0,2);
                    }
                }
                ++candidate;
                count++;
            }
            ++it;
        }
    }
    template<typename Callable>
    void map(Callable func) //applies func to each element subdivision
    {
        //func should take in a Positional& as its only parameter
        /*auto end = elements.end();
        for (auto it = elements.begin(); it != end;++it)
        {
            func(*it->second.positional);
        }*/
        map(func,head);
    }
    void showCollisions()
    {
        glm::vec2 mousePos = pairtoVec(MouseManager::getMousePos());
        Positional p(mousePos);
        auto it = elements.begin();
        processElement([&it,this](const BiTreeElement& element, BiTreeNode& node) mutable {
                       it = insert(element,node);
                       },p,glm::vec4(mousePos,10,10));
        auto candidate = std::next(it);
        int i = 0;
        while (candidate != elements.end())
        {
            if (    candidate->second.positional == it->second.positional ||
                    candidate->second.rect.x > it->second.rect.x + it->second.rect.z ||
                    candidate->second.rect.x < it->second.rect.x)
                    {
                        break;
                    }
                PolyRender::requestRect(candidate->second.positional->getBoundingRect(),{1,1,0,1},true,0,1);
                PolyRender::requestRect(candidate->second.rect,{1,0,0,1},true,0,1);

            ++candidate;
            ++i;
        }
        remove(p);
        std::cout << i << "\n";
    }
private:
    template<typename Callable>
    void map(Callable func, BiTreeNode& node)
    {
        if (node.nodes[0])
        {
            map(func,*node.nodes[0]);
            map(func,*node.nodes[1]);
        }
        else
        {
            //std::cout << "NEW NODE " << node.vertDimen.x << " " << node.vertDimen.y<< "\n";
            PolyRender::requestLine({region.x,node.vertDimen.x + node.vertDimen.y/2,region.x + region.z,node.vertDimen.x + node.vertDimen.y/2},
                                    {1,1,0,1},1,1,0);
            int count = 0;
            auto it = node.start;
            while (count < node.size )
            {
                func(*it->second.positional);
                ++it;
                count++;
            }
        }
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
    void remove(Positional& obj, PositionalCompare func = nullptr); //removes the obj. This only removes the pointer; doesn't actually delete the object
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
