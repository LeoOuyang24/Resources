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
    virtual glm::vec2 getCenter() const;
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
        return pointVecDistance(rect, point.x, point.y,tilt);
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

    #include "FreeTypeHelper.h"

#include <unordered_set>
class BiTree
{
    class BiTreeNode;
    class BiTreeScore;
    class BiTreeElement;
    typedef std::map<BiTreeScore,BiTreeElement> BiTreeStorage; //stores all elements. Maps score to element
    typedef std::unordered_map<Positional*,BiTreeStorage::iterator> addressBook;
    struct BiTreeElement //represents the element plus the rect that represents it
    {
        Positional* positional = 0;
        glm::vec4 rect = glm::vec4(0);
        BiTreeNode* node = 0;
    };

    struct BiTreeScore
    {
        constexpr static int roundNth = 4; //nth place to round to
        float nodeY = 0, elementX = 0; //the 2 elements of our score
        Positional* ptr = 0;
        bool operator < (const BiTreeScore& other) const
        {
            return round(nodeY,roundNth) == round(other.nodeY,roundNth) ?  //if both elements belong in the same node
                    round(elementX,roundNth) == round(other.elementX,roundNth) ? //if both elements have the same x (after rounding)
                        ptr < other.ptr :  //return the pointer comparison
                        round(elementX,roundNth) < round(other.elementX,roundNth) :  //otherwise return who has the smaller x
                    round(nodeY,roundNth) < round(other.nodeY,roundNth);  //otherwise return who belongs in the smaller node
        }
        bool operator == (const BiTreeScore& other) const
        {
          return (round(nodeY,roundNth) == round(other.nodeY,roundNth) && round(elementX,roundNth) == round(other.elementX,roundNth)) && ptr == other.ptr;
        }
        bool operator <= (const BiTreeScore& other) const
        {
            return *this < other || *this == other;
        }
        std::string toString() const
        {
            std::ostringstream stream;
            stream << nodeY << " " << elementX << " " << ptr;
            return stream.str();
        }
    };

    static constexpr int maxNodeSize = 250; //maximum number of elements in a node
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
        int bigSize = 0;
    };

    static constexpr auto pairHasher = [](const std::pair<Positional*,Positional*>& p){
                                                                return std::hash<Positional*>{}(p.first) ^ std::hash<Positional*>{}(p.second);
                                                                };
    //std::unordered_map<Positional*,>
    typedef std::unordered_set<std::pair<Positional*,Positional*>, decltype(pairHasher)> FoundStorage;

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
        takes in a const BiTreeElement&. If func has a return value, we return the value for the function call of the
        lowest scoring subdivision.*/
        if (!node.nodes[0]) //found a leaf, which must be the node the element belongs in
        {
            return func({&wrap,rect,&node});
        }
        else //otherwise keep searching
        {
            float split = node.vertDimen.x + node.vertDimen.y/2; //midpoint of the node where the split between the children happened
            if (rect.y < split && rect.y + rect.a > split) //we have an element that crosses the border, so we have to split it
            {
                float botHeight = rect.y + rect.a - split; //height of bottomt chunk
                if constexpr(std::is_void<decltype(processElement(func,wrap,{},*node.nodes[0]))>::value)
                    {
                        processElement(func,wrap,{rect.x,rect.y,rect.z,split - rect.y},*node.nodes[0]);
                        processElement(func,wrap,{rect.x,split,rect.z,botHeight},*node.nodes[1]);
                        return;
                    }
                else
                {
                    auto val = processElement(func,wrap,{rect.x,rect.y,rect.z,split - rect.y},*node.nodes[0]); //process top chunk. Return this value because it must be precede the bottom chunk
                    processElement(func,wrap,{rect.x,split,rect.z,botHeight},*node.nodes[1]); //process bottom chunk
                    return val;
                }

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
                //return processElement(func,wrap,rect,*node.nodes[(rect.y >= split)]);
            }
        }

    }
    template<typename Callable>
    void processNode(Callable func, BiTreeNode& node, bool tail) //calls func on each node, func should take in BiTreeNode& as its only parameter
    {
        //tail represents whether or not this function should be called before or after recursive calls
        if (tail)
        {
            func(node);
        }
        if (node.nodes[0])
        {
            processNode(func,*node.nodes[0],tail);
            processNode(func,*node.nodes[1],tail);
        }
        if (!tail)
        {
            func(node);
        }
    }

    bool stopProcessing(const BiTreeElement& p1, const BiTreeElement& p2) //returns whether or not we should keep processing
    {
        return (p1.positional == p2.positional ||
            p2.rect.x > p1.rect.x + p1.rect.z ||
            p2.rect.x + p2.rect.z < p1.rect.x) ;
    }
    template<typename Callable, typename Collides>
    bool doCollision(Callable func, Collides collided, const BiTreeElement& p1, const BiTreeElement& p2, FoundStorage& storage)
    {
        ///if p1 and p2 collide, call func on them. Returns whether or not we should process more elements
        ///func should take in 2 RectPositionals (reference or not)
        ///collides should take in 2 BiTreeElements (reference or not) and return whether or not we should call func
        if (stopProcessing(p1,p2))
        {
            return false;
        }
        else
        {
            RectPositional* r1 = static_cast<RectPositional*>(p1.positional);
            RectPositional* r2 = static_cast<RectPositional*>(p2.positional);
            if (collided(p1,p2) && storage.find({r1,r2}) == storage.end())
            {
                func(*r1,*r2);
                storage.insert({r1,r2});
               // PolyRender::requestRect(candidate->second.rect,{1,0,0,1},false,0,2);
            }
            return true;
        }
    }
    BiTreeScore calculateScore(const BiTreeElement& element); //given wrapper and the node it belongs in, calculates the score used in scoring
    BiTreeStorage::iterator insert(const BiTreeElement& element, BiTreeNode& node); //core insertion function. inserts element into node. Does not split node. Updates element's node field
    BiTreeStorage::iterator remove(const BiTreeStorage::iterator& it); //core removal function
    void split(BiTreeNode& node); //calls split with updateIt = elements.end()
    void updateNode(BiTreeStorage::iterator& it, BiTreeNode& node); //given it, updates a node assuming it is in that node. Doesn't actually insert it
    void resetNode(BiTreeNode& node);
public:
    static constexpr auto defaultCollides = [](const BiTreeElement& e1, const BiTreeElement& e2){
                        RectPositional* r1 = static_cast<RectPositional*>(e1.positional);
                        RectPositional* r2 = static_cast<RectPositional*>(e2.positional);
                        return vecIntersect(r1->getRect(),r2->getRect(),r1->getTilt(),r2->getTilt());
                       }; //default collisions function
    BiTree(const glm::vec4& region_);
    unsigned int size();
    unsigned int countNodes();
    glm::vec4 getRegion();
    void insert(Positional& wrap); //calculates the node wrap belongs in and inserts it, splitting nodes if necesesary
    BiTreeStorage::iterator remove(Positional& wrap);  //removes wrap and returns an iterator to the next element, or elements.end() if wrap is not found
    void showNodes(RenderCamera* camera = RenderCamera::currentCamera);

    template<typename UpdateFunc>
    BiTreeStorage::iterator update(Positional& pos, UpdateFunc func,  BiTreeStorage::iterator it)
    { //CURRENTLY BROKEN. Update may return invalid iteratorss
        //given a positional and an update function, update its spot in the bitree. Returns either an iterator pointing to the positional's current position, or the next element
        //func should take in a Positional&
        //'it' is expected to be either a iterator that points to one subelement of pos
        //if 'it' is not elements.end, it will be passed into remove and the return value will be either 'it' or the iterator after 'it' if it is removed
       //if 'it' is elements.end, it is ignored
        BiTreeStorage::iterator removed;
        if (it == this->elements.end())
        {
            removed = remove(pos);
        }
        else
        {
            removed = remove(it);
            remove(pos);
        }
        func(pos);
        insert(pos);

        return removed;
    }
    template<typename UpdateFunc>
    BiTreeStorage::iterator update(Positional& pos, UpdateFunc func)
    {
        return update(pos,func,elements.end());
    }

    void clear(); //removes all elements and nodes
    template<typename Callable>
    void findNearest(const glm::vec2& center, float radius, Callable func) //finds all objects within radius distance of center
    {
        //func should take in a single Positional&
        RectPositional rect({center.x - radius,center.y - radius, radius*2,radius*2});

       auto collides = [center,radius](const BiTreeElement& e1, const BiTreeElement& e2){
                                return (e2.positional->distance(center) <= radius);};
        auto realFunc = [&func](Positional& p1, Positional& p2) //wrap func in a function that takes 2 parameters since that's what doCollision expects
        {
            func(p2);
        };
       findCollision(rect,realFunc,collides);
    }
    template<typename Callable>
    void findNearest(Positional& p, float radius, Callable func) //find all objects within radius distance of p
    {
        //func should take in a single Positional&
        glm::vec2 center = p.getCenter();
        RectPositional rect({center.x - radius,center.y - radius, radius*2,radius*2});

        auto collides = [center,radius,&p](const BiTreeElement& e1, const BiTreeElement& e2){
                                return e2.positional != &p && (e2.positional->distance(center) <= radius);};
        auto realFunc = [&func](Positional& p1, Positional& p2) //wrap func in a function that takes 2 parameters since that's what doCollision expects
        {
            func(p2);
        };
        findCollision(rect,realFunc,collides);
    }
    template<typename Callable, typename Collides>
    void findCollision(Positional& wrap, Callable func, Collides collides = defaultCollides) //given a positional, finds elements it collides with and calls func
    {
        //CURRENTLY DOESN"T WORK WHEN FINDING ELEMENTS BEFORE wrap
        ///func should take in 2 Positional&. First parameter will always be wrap. Func should NOT remove wrap, as that would cause undefined behavior
        FoundStorage storage(maxNodeSize,pairHasher);
        processElement([this,&func,&collides,&storage](const BiTreeElement& element) mutable {
                          BiTreeScore score = calculateScore(element);
                          auto found = elements.lower_bound(score);
                          if (found != elements.begin())
                          {
                              auto it = (std::make_reverse_iterator(found));
                              auto rend = elements.rend();
                              while (it != rend)
                              {
                                   if (!doCollision(func,collides,element,it->second,storage))
                                    {
                                        break;
                                    }
                                  ++it;
                              }
                          }
                          auto it = found;
                          auto end = elements.end();
                          while (it != end)
                          {
                              if (!doCollision(func,collides,element,it->second,storage))
                              {
                                  break;
                              }
                              ++it;
                          }
                          },wrap,wrap.getBoundingRect());
    }
    template<typename Callable>
    void processCollisions(Callable func) //func should take in 2 RectPositional&
    { //REFACTOR: currently only works with rect positionals
        auto it = elements.begin();
        auto end = elements.end();
        FoundStorage storage(elements.size(),pairHasher);

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
                if (!doCollision(func,defaultCollides,it->second,candidate->second,storage))
                {
                    break;
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
        auto end = elements.end();
        for (auto it = elements.begin(); it != end;++it)
        {
            func(*it->second.positional);
        }
        //map(func,head);
    }
    template<typename UpdateFunc>
    void selectUpdate(const glm::vec4& selector, UpdateFunc func)
    {
        //func should take in a Positional&
        std::unordered_set<Positional*> visited;
        RectPositional rect(selector);
        processElement([this,&func,&visited](const BiTreeElement& element) mutable {
                       int count = 0;
                          BiTreeScore score = calculateScore(element);
                          auto found = elements.lower_bound(score);
                          auto storageEnd = visited.end();
                          if (found != elements.begin() && found != elements.end())
                          {
                              auto it = (std::prev(found));
                              while (it != elements.begin())
                              {
                                    if (stopProcessing(element,it->second))
                                    {
                                        break;
                                    }
                                    if (defaultCollides(element,it->second) && visited.find(it->second.positional) == storageEnd)
                                    {
                                        visited.insert(it->second.positional);
                                        it = update(*it->second.positional,func,it);
                                    }
                                    --it;//decrement no matter what because it either stayed the same or moved up
                                  //REFACTOR: If an element is reinserted to a spot such that it is now equal to it on the next iteration
                                  //, that element will be checked again, which is slightly inefficient (but thankfully not updated). Applies to both while loops
                              }
                              if (it == elements.begin())
                              {

                                if (defaultCollides(element,it->second) && visited.find(it->second.positional) == storageEnd)
                                {
                                    visited.insert(it->second.positional);
                                    update(*it->second.positional,func);
                                }
                              }
                          }
                          auto it = found;
                          auto end = elements.end();
                          while (it != end)
                          {
                                if (stopProcessing(element,it->second))
                                {
                                    break;
                                }
                                auto old = it->second.positional;
                                if (defaultCollides(element,it->second) && visited.find(it->second.positional) == storageEnd)
                                {
                                    visited.insert(it->second.positional);
                                    it = update(*it->second.positional,func,it);

                                }
                              /*  if (it != end && it->second.positional == old) //if we either didn't update or the update function didn't delete the element, increment
                                {
                                    ++it;
                                }*/
                          }
                          },rect,selector);
    }
    template<typename UpdateFunc>
    void updateAll(UpdateFunc func) //like selectUpdate but it updates everything func should take in a Positional&
    {
        auto end = elements.end();
        std::unordered_set<Positional*> visited;
        auto storageEnd = visited.end();
        int i = 0;
        for (auto it = this->elements.begin(); it != end;++it)
        {
            auto old = it->second.positional;
            if (visited.find(it->second.positional) == storageEnd)
            {
                visited.insert(it->second.positional);
                func(*it->second.positional);
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
