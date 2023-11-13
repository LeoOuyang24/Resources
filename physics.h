#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include "components.h"
#include "glGame.h"

typedef glm::vec2 ForceVector;

constexpr float MAX_FORCE = 10.0f;
constexpr float BASE_FRICTION = .999f;

struct ForcePart
{
    //used to store a single force.
    //the idea here is you may have multiple forces from different sources (for example, a sailboat is affected by wind, gravity, and buoyancy).
    //you'd then have one "ForcePart" representing each source
    //later down the road, maybe you want to turn off the gravity, you'd be able to turn the gravity component and just the gravity component
    float maxMag = MAX_FORCE; //maximum magnitude of the force
    float friction = BASE_FRICTION; //amount to multiply force by with applyFriction
    ForceVector force = {0,0}; //the actual force we are representing

    void setForce(const ForceVector& force);
    void addForce(const ForceVector& force);
    void applyFriction(unsigned int instances = 1); //apply friction "instances" times
};

class ForcesComponent : public Component, public ComponentContainer<ForcesComponent> //component that pushes MoveComponent based on what forces are currently being applied
{
protected:
    ForcePart finalForce;
public:
    ForcesComponent(Entity& entity, float friction_ = BASE_FRICTION, float maxForce = MAX_FORCE);
    bool getBeingPushed();
    virtual void addForce(ForceVector force);
    void update();
    glm::vec2 getFinalForce()
    {
        return finalForce.force;
    }
    virtual ~ForcesComponent()
    {

    }

};

typedef std::shared_ptr<Entity> LinkPtr; //technically, this could just be a pointer to a positional, but we do chainLinkComponent because it makes it easier to find the next ChainLinks of the chain
class ChainLinkComponent : public ForcesComponent, public ComponentContainer<ChainLinkComponent>
{
    LinkPtr down;

public:
    constexpr static float baseTension = .0001f;
    const float linkDist = 0; //distance between points
    const float tension = 0; //proportion of the force between up and down that should affect this component
    ChainLinkComponent(Entity& entity, const LinkPtr& down_, float linkDist_, float tension_ = BASE_FRICTION, float friction_ = BASE_FRICTION);
    void setLink(const LinkPtr& link);
    ChainLinkComponent* getNextLink();
    virtual glm::vec2 getLinkPos(); //returns the point of the link, (0,0) if moveComponent is null otherwise the center. Virtual so different links can return different points (such as different points on a rectangle)
    void update();
};

class ChainRenderComponent;
typedef void (*ChainLinkRender)(ChainRenderComponent&,ChainLinkComponent&);
class ChainRenderComponent : public RenderComponent, public ComponentContainer<ChainRenderComponent>
{
    ChainLinkRender renderFunc;
public:
    glm::vec4 color;
    ChainRenderComponent(const glm::vec4 color_,ChainLinkRender renderFunc_,  Entity& entity);
    void update();
};

struct Chain
{
    LinkPtr top,//top-most link, primarily used to propagate functions through all the links
    bottom; //bottom-most link, less used than top since it can't refer to the link above it, but it's sometimes useful to set its position to move the rest of the chain
    Chain(ChainLinkRender renderFunc_, const glm::vec4& color1, const glm::vec4& color2, float linkDist_,  int length, glm::vec2 start,float tension_ = .00001f, float angle = 0, float friction = BASE_FRICTION); //color1 and color2: forms a color gradient along the chain, can be left to glm::vec4(0) if you don't care about color
    template<typename Callable>
    Chain(Callable func, int length) //customizable constructor; given a callable, (can be a lambda with captures!). Calls func length times.
    {
        //func is expected to be a function that takes in the previous (top) LinkPtr and an int (ordinal number of the link) and returns a LinkPtr
        LinkPtr prev = top;
        for (int i = 0; i < length; ++i)
        {
            prev = func(prev, i);
            if (i == 0)
            {
                bottom = prev;
            }
            else if (i == length - 1)
            {
                top = prev;
            }
        }
    }
    void setBottomPos(const glm::vec2& pos);
    void update();
    virtual void collide(Entity& entity);
    template<typename Callable>
    void apply(Callable func) // runs a callable on each ChainLinkComponent, starting from top. func is expected to take in a ChainLinkComponent& as parameter, return value is irrelevant
    {
        if (top.get())
        {
            ChainLinkComponent* link = top->getComponent<ChainLinkComponent>();
            while (link)
            {
                func(*link);
                link = link->getNextLink();
            }
        }
    }
    template<typename RetType, typename Callable>
    RetType fold(RetType start, Callable fun) //Given a starting value, computes a final value based on each ChainLink. Think Fold_left in OCAML.
    {
       //fun = callable(RetType, ChainLinkComponent&) -> RetType.
       //start will be copied, so pass in something that is light to copy
       RetType val = start;
        apply([&val, &fun](ChainLinkComponent& link)
              {
                  val = fun(val,link);
              });
        return val;
    }
    template<typename Callable>
    bool checkAll(Callable fun) //Checks if all chainLinks meet a certain critera, can terminate early if a false is found. Fun has to return something that can be interpreted as a bool (such as a pointer)
    {
        //fun = callable(ChainLinkComponent&) -> bool
        if (top.get())
        {
            ChainLinkComponent* link = top->getComponent<ChainLinkComponent>();
            while (link)
            {
                if (!fun(*link))
                {
                    return false;
                }
                link = link->getNextLink();
            }
            return true;
        }
        return false;
    }
};

#endif // PHYSICS_H_INCLUDED
