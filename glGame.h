#ifndef GLGAME_H_INCLUDED
#define GLGAME_H_INCLUDED

#include <glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <vector>
#include <memory>

#include "components.h"
#include "geometry.h"
#include "render.h"
#include "SDLhelper.h"

bool rectPathIntersect(const glm::vec4& oldRect, const glm::vec4& newRect, const glm::vec4& collide); //returns true if oldRect collided with collide on its path to newRect

class SpatialGrid //Spatial Partitioning data structure (SPDS) that partitions space into a bunch of X by X squares
{
 protected:
    static int normalizeCoord(float coord, int nodeDimen) //rounds coord to the nearest multiple of nodeDimen
    {
        return (floor(coord/nodeDimen))*nodeDimen;
    }

    //Unlike other SPDS, Spatial Grid can work over an infinite space
    struct NodeStorageHasher //hashes points based on which node the point belongs in
    {
        int nodeDimen = 0; //dimensions of the nodes
        NodeStorageHasher(int x) : nodeDimen(x == 0 ? 1 : abs(x)) //set nodeDimen to the absolute value of x, or 1 if the node dimensions is 0 for some reason
        {

        }
        size_t operator()(const glm::vec2& point) const
        {
            glm::vec2 nodePoint = {SpatialGrid::normalizeCoord(point.x,nodeDimen),SpatialGrid::normalizeCoord(point.y,nodeDimen)}; //normalize coordinates to the top left corners of our nodes
            float sum = (nodePoint.x + nodePoint.y);
            return .5*(sum*sum + sum) + nodePoint.y; //Maps our coordinates into a single unique number, https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
        }
    };
    typedef std::list<PositionalComponent*> Positionals; //allows us to store all positionals in our grid in one data structure

    struct Node{
        Positionals::iterator end; //the LAST positional in our node. positionals.end() if empty.    {
        int size = 0; //number of positionals in our node
        void reset(Positionals::iterator& null) //resets node. Pass in the new iterator you want end to point to after reset
        {
            size = 0;
            end = null;
        }
    };
    typedef std::unordered_map<glm::vec2,Node,NodeStorageHasher> NodeStorage;

    typedef std::pair<Node&,PositionalComponent&> LookupEntry;

    struct LookUpEntryHash
    {
        size_t operator()(const LookupEntry& entry) const
        {
            return hashCombine(std::hash<Node*>{}(&entry.first),std::hash<PositionalComponent*>{}(&entry.second));
        }
    };

    struct LookUpEntryEqualTo
    {
        bool operator()(const LookupEntry& e1, const LookupEntry& e2) const
        {
            return &e1.first == &e2.first && &e1.second == &e2.second;
        }
    };

    typedef std::unordered_map<LookupEntry,Positionals::iterator,LookUpEntryHash, LookUpEntryEqualTo> Lookup; //used to find the precise position of a Positional in a node
    NodeStorage nodes;
    Positionals positionals;
    Lookup lookup;
    int nodeDimen = 0; //dimensions of the nodes
    TASK_ID taskID = 0;
    int normalizeCoord(float coord) //non-static version of the same function that uses our own nodeDimen
    {
        return (floor(coord/nodeDimen))*nodeDimen;
    }
    void addToNode(PositionalComponent& p, Node& node)
    {
        Positionals::iterator it;
        if (node.size == 0)
        {
            positionals.push_front(&p);
            node.end = positionals.begin(); //if node has nothing, set the end iterator to the only element
            it = node.end;
        }
        else
        {
            it = positionals.insert(node.end,&p);
        }
        lookup.insert({{node,p},it}); //update lookup table for looking up iterators
        node.size ++;
    }
    void removeFromNode(PositionalComponent& p, Node& node)
    {
        auto it = lookup.find({node,p});
        if (it != lookup.end()) //positional actually exists in node
            {
                if (it->second == node.end)
                {
                    node.end = node.size > 1 ? //update node iterator, doesnt actually remove the element
                                std::prev(node.end) : //if there are more elements in the node, the next one becomes the new end iterator
                                positionals.end(); //otherwise, node is empty (after removal)
                }
                node.size --;
                positionals.erase(it->second); //officially remove element
                lookup.erase({node,p});
            }
    }

    template<typename T>
    void processAllNodes(const glm::vec4& rect,T func) //given a function, runs the function on each node the positional belongs in.
    {
        //func = (const glm::vec2&, Node&) => void
        int horizLimit = normalizeCoord(rect.r + rect.z);
        int vertLimit = normalizeCoord(rect.y + rect.a);
        for (int i = normalizeCoord(rect.r); i <= horizLimit; i += nodeDimen)
        {
            for (int j = normalizeCoord(rect.y); j <= vertLimit; j += nodeDimen)
            {
                        func({i,j},nodes[{i,j}]);
            }
        }
    }
    template<typename T>
    void processAllExistingNodes(const glm::vec4& rect,T func) //given a function, runs the function on each EXISTING node the positional belongs in.
    {
        //func = (const glm::vec2&, Node&) => void
        int horizLimit = normalizeCoord(rect.r + rect.z);
        int vertLimit = normalizeCoord(rect.y + rect.a);
        for (int i = normalizeCoord(rect.r); i <= horizLimit; i += nodeDimen)
        {
            for (int j = normalizeCoord(rect.y); j <= vertLimit; j += nodeDimen)
            {
                if(nodes.find({i,j}) != nodes.end())
                    {
                        func({i,j},nodes[{i,j}]);
                    }
            }
        }
    }
public:
    SpatialGrid(int nodeSize_) : nodeDimen(nodeSize_), nodes(0,NodeStorageHasher(nodeSize_))
    {

    }
    ~SpatialGrid()
    {
        clear();
    }
    void insert(PositionalComponent& positional)
    {
        processAllNodes(positional.getBoundingRect(),[this,&positional](const glm::vec2& point, Node& node){

                        addToNode(positional,node);
                        });
       // std::cout << nodes.size() << "\n";

    }
    void remove(PositionalComponent& positional) //does nothing if positional is not found
    {
        processAllExistingNodes(positional.getBoundingRect(),[this,&positional](const glm::vec2& point, Node& node){
                        removeFromNode(positional,node);
                        });
    }
    void update(PositionalComponent& p, const glm::vec4& oldPos) //given a positional in our data structure and its old bounding rect, we update p's position
    {
        glm::vec4 newPos = p.getBoundingRect();
        auto hashFunc = nodes.hash_function();
        if (oldPos == newPos || //see if we don't have to move "p" at all. This happens if either it hasn't moved or if "p" would remain in the same node anyway
           (hashFunc({oldPos.x,oldPos.y}) == hashFunc({newPos.x, newPos.y}) && //if the topLeft and botRight corners of "p"'s new position are in the same node as the old position, we don't have to move "p"
            hashFunc({oldPos.x + oldPos.z, oldPos.y + oldPos.a}) == hashFunc({newPos.x + newPos.z, newPos.y + newPos.a})))
                {
                    return;
                }
        else//"p" does have to move. Consider old nodes and remove from nodes we no longer intersect from while adding to new nodes we do intersect with
        {
            processAllExistingNodes(oldPos,[this,&p,&newPos](const glm::vec2& point, Node& node){
                            if (!vecIntersect(newPos,glm::vec4(point,nodeDimen,nodeDimen))) //if our new position would intersect with this node, don't remove
                            {
                                removeFromNode(p,node);
                            }
                            }); //remove "p" based on where it was previously
           processAllNodes(newPos,[this, &p](const glm::vec2& point, Node& node){
                            if (lookup.find({node,p} ) == lookup.end()) //if not in node, add it
                                {
                                    addToNode(p,node);
                                }
                            }); //add positional into new nodes its not already in

        }

    }
    template<typename T>
    void findNearest(PositionalComponent& p, T func) //run "func" on every positional "p" shares a node with, "p" doesn't have to be actually in the grid
    {
        //func = (PositionalComponent&) => void
        processAllExistingNodes(p.getBoundingRect(),[this,func, &p](const glm::vec2& point, Node& node) mutable {
                        int i =0;
                        for ( auto it = node.end; i < node.size; i++, --it)
                        {
                            if (&p != (*it) && p.collidesRect((*it)->getBoundingRect())) //REFACTOR: this may not be accurate if the positional "it" is pointing to is not a rectangle
                            {
                                func(**it);
                            }
                        }
                        });
    }
    template<typename T>
    void findNearest(const glm::vec2& pos, int dist, T func) //run "func" on all Positionals within "dist" distance from "pos"
    {
        //func = (PositionalComponent&) => void
        glm::vec4 rect = glm::vec4(pos - glm::vec2(dist,dist),dist*2,dist*2); //find all positionals that collide with this rect
        processAllExistingNodes(rect,[this,dist,func,pos](const glm::vec2& point, Node& node) mutable {
                        int i =0;
                        for ( auto it = node.end; i < node.size; i++, --it)                        {
                            if ((*it)->distance(pos) <= dist)
                            {
                                func(**it);
                            }
                        }
                        });
    }
    void updateEntities(const glm::vec4& region); //update all entities in region, calling their updateOnce function and update their positions
    bool inGrid(PositionalComponent& p) //true if p is in the spatial grid in the appropriate nodes
    {
        bool answer = true;
        PolyRender::requestRect(ViewPort::toScreen(p.getBoundingRect()),glm::vec4(0,1,0,1),0,0,1);
        processAllExistingNodes(p.getBoundingRect(),[&answer,this,&p](const glm::vec2& point, Node& node) mutable {
                        if (lookup.find({node,p}) == lookup.end())
                            {
                                PolyRender::requestRect(glm::vec4(ViewPort::toScreen(point),nodeDimen,nodeDimen),glm::vec4(1,0,0,1),0,0,1);
                            }
                            else
                            {
                                PolyRender::requestRect(glm::vec4(ViewPort::toScreen(point),nodeDimen,nodeDimen),glm::vec4(1,1,0,1),0,0,1);
                            }
                        answer = answer && (lookup.find({node,p}) != lookup.end()); //REFACTOR: should short circuit if answer is ever false; don't have to process every node if answer is ever false
                        });
        return answer;
    }
    void clear() //removes all entities from datastructure
    {
        nodes.clear();
        positionals.clear();
        lookup.clear();
    }
    void render()
    {
        for (auto it = nodes.begin(); it!= nodes.end(); ++it)
        {
            glm::vec4 nodeRect = glm::vec4(it->first + glm::vec2(1,1),nodeDimen - 2,nodeDimen - 2); //render nodes slightly smaller to prevent overlap

            glm::vec4 color = it->second.size != 0 ? glm::vec4(1,1,0,1) : glm::vec4(1,0,0,1); //if node has stuff in it, render it yellow, otherwise red
            PolyRender::requestRect(nodeRect,color,false,0,1);
        }
    }
    int size() //not necessarily the number of unique entities in the grid, just the number of positionals are stored.
    {
        return positionals.size();
    }
};


#endif // GLGAME_H_INCLUDED
