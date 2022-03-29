#ifndef PATHFIND_H_INCLUDED
#define PATHFIND_H_INCLUDED

#include <unordered_map>
#include <deque>

#include "glGame.h"
#include "components.h"

struct PathPoint //includes a point and the line node-node border it resides on
{
    glm::vec2 point, a, b;
};

typedef std::deque<PathPoint> Path;

void renderPath(Path& path, RenderCamera* cam = RenderCamera::currentCamera);

struct HashPoint
{
    std::size_t operator() (const glm::vec2& p1) const; //hash function for glm::vec2. As of now, glm::vec2s of negative points are mapped to the same location as glm::vec2s of positive points (-1,2) = (1,2)
};


class NavMesh //a navigation mesh of rectangles
{
    class NavMeshNode;
    typedef std::unordered_map<NavMeshNode*,glm::vec4> Neighbors; //pointer to the node and the line of intersection. The XY of the line should be the of the leftmost point

   class NavMeshNode : public RectPositional
    {
        //typedef std::vector<Neighbors> Adjacents;
         Neighbors nextTo;
    public:

        NavMeshNode(const glm::vec4& rect);
        void addNode(NavMeshNode& n); //adds a node as a neighbor. If the node is already a neighbor, update the intersection line
        bool isNextTo(NavMeshNode& n); //returns true if n is already in our neighbors list
        void removeNode(NavMeshNode& n);
        void setDimen(const glm::vec2& dimens);
        void setRect(const glm::vec4& rect);
        Neighbors& getNextTo();
        void render() const; //draws lines between this node and its neighbors
        ~NavMeshNode();
    };

    glm::vec4 bounds; //rect of the entire mesh
    QuadTree nodeTree; // a quadtree of all the nodes. Makes insertion and finding nodes faster
    QuadTree negativeTree ; //tree of RectPositionals that are not nodes. Essentially, tree of areas where objects can't move to.
  //  void addHelper(const glm::vec4& rect, NavMeshNode* current);
    NavMeshNode* getNode(const glm::vec2& point); //returns the node at the given position. Null if none
    NavMeshNode* getNearestNode(const glm::vec2& point); //returns the nearest node at the given position. The same as getNode if the point is in a node. Returns null if there are no nearby nodes

    void addNode(const glm::vec4& rect); //adds a node to the mesh assuming it doesn't collide with a preexisting node
    void addNode(NavMeshNode& node);

    void removeNode(NavMeshNode& node); //deletes node from both the vector and the nodeTree
    void splitNode(NavMeshNode& node, const glm::vec4& overlap); //creates new nodes and adds them to neighbors based on how overlap splits node. Does NOT delete node
    glm::vec2 displacePoint(const glm::vec2& point,const glm::vec4& line, const glm::vec4& nodeRect, double width); //Helper Function for getPath(). given a node's Rect and its boundary, this function predicts where walls are and moves the point accordingly.

    void addWall(const glm::vec4& wall); //adds a wall but DOES NOT add a rectpositional to the negative tree. That should be handled in other functions, such as smartAddWall

public:
    NavMesh(const glm::vec4& bounds_);
    void smartAddNode(const glm::vec4& rect); //adds a negative space and a RectPositional
    std::shared_ptr<RectPositional> smartAddWall(RectPositional& rect); //adds the RectPositional and returns a smart pointer to it
    void reset(); //clears nodeTree and negativeTree but keeps bounds
    glm::vec4 validMove(const glm::vec4& start, const glm::vec2& displacement); //returns where start should end up without colliding into any walls;
    glm::vec4 getWallRect(const glm::vec4& rect); //returns the wall that rect intersects with. glm::vec4(0) if there is no wall
    positionalVec getWallsNearby(const glm::vec4& rect);
    bool notInWall(const glm::vec4& rect); //returns true if rect is in a wall.
    glm::vec4 getNearestNodeRect(const glm::vec2& point); //returns the rect of the node that the point belongs to. glm::vec4(0) if there is no nearby node
    glm::vec2 closestPoint(const glm::vec2& point); //returns the closest valid point to point. Throws if there are no nearby nodes
    Path getPath(const glm::vec2& start, const glm::vec2& end, int width = 0); //returns a path from start to end using A*, returning the empty path if there is no path. Not guaranteed to be the shortest path. Width is the shortest distance we can be from any given negative area. This allows us to find paths for large objects without colliding with walls. This function is so convoluted there's a whole documentation in the documents folder!
    bool straightLine(const glm::vec4& line); //returns true if the line doesn't overlap with any negative space.
    glm::vec4 getRandomArea(const glm::vec2& origin, double minDist, double maxDist); //returns the area of a randomly chosen navmesh node that is between min and max distacne from origin. Guaranteed won't crash by generating a point out of bounds
    //note that this function just randomly generates a point and then chooses the nearest node. This means that larger nodes have larger chances of of being selected. This makes sense, but it should be noted that not all nodes have an equal chance of being selected.
    void removeWall(RectPositional& rect); //removes the wall at the given position and replaces with empty space
    glm::vec4 getBounds();
    int getNumWalls(); //returns negativeTree.count()
    int getNumNodes();
    void render();
    void renderNode(const glm::vec2& point);
    ~NavMesh();
};

struct Terrain : public RectPositional
{
    typedef void (*RenderFunc)(Terrain&);
    RenderFunc renderFunc; //render function, makes it easy to create terrain with different rendering without having to make a whole child class
    Terrain( const glm::vec4& box, RenderFunc r = nullptr) : RectPositional(box), renderFunc(r)
    {

    }
    virtual void render()
    {
        if (!renderFunc)
        {
            PolyRender::requestRect(RenderCamera::currentCamera->toScreen(rect),{1,0,0,1},true,0,1);
        }
        else
        {
            renderFunc(*this);
        }
    }
};

class EntityTerrainManager : public EntityPosManager
{
    std::list<std::shared_ptr<Terrain>> terrain;
protected:
    std::shared_ptr<NavMesh> mesh;
    virtual void updateTerrain();
public:
    virtual void init(const glm::vec4& rect);
    void addTerrain(Terrain& box);
    using EntityPosManager::addEntity;
    void addEntity(Entity& entity, float x,float y, bool centered = true);
    std::shared_ptr<NavMesh>& getMeshPtr();
    void update();
};

class PathFindComponent : public MoveComponent, public ComponentContainer<PathFindComponent>
{
protected:
    std::weak_ptr<NavMesh> mesh;
    Path path;
    bool adjustTilt = false; //whether to call setTiltToTarget with each update
public:
    PathFindComponent(bool setAngle, std::shared_ptr<NavMesh>& mesh_,float speed, const glm::vec4& rect, Entity& entity);
    virtual void setTarget(const glm::vec2& point);
    virtual bool atTarget(); //returns whether or not we have reached our final target
    void update();
};

#endif // PATHFIND_H_INCLUDED
