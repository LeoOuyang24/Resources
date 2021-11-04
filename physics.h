#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include "components.h"
#include "glGame.h"

typedef glm::vec2 ForceVector;
class ForcesComponent : public Component, public ComponentContainer<ForcesComponent> //component that pushes MoveComponent based on what forces are currently being applied
{
protected:
    ForceVector finalForce = glm::vec2(0); //after applying all forces, this is the final x and y displacement to move
    MoveComponent* move = nullptr; //forcesComponent will only work if there is a moveComponent on the entity
    float friction= 0; //how much to decrease finalForce
    void applyForce(ForceVector force);
public:
    constexpr static float baseFriction = .999f;
    ForcesComponent(Entity& entity, float friction_ = baseFriction);
    void setMoveComponent(MoveComponent* move_);
    MoveComponent* getMove()
    {
        return move;
    }
    bool getBeingPushed();
    virtual void addForce(ForceVector force);
    void update();
    glm::vec2 getFinalForce()
    {
        return finalForce;
    }
    virtual ~ForcesComponent()
    {

    }

};

typedef std::shared_ptr<Entity> LinkPtr; //technically, this could just be a pointer to a positional, but we do chainLinkComponent because it makes it easier to find the next ChainLinks of the chain
class ChainLinkComponent : public ForcesComponent, public ComponentContainer<ChainLinkComponent>
{
    LinkPtr down;
    float linkDist = 0; //distance between points
    float tension = 0; //proportion of the force between up and down that should affect this component
public:
    ChainLinkComponent(Entity& entity, const LinkPtr& down_, float linkDist_, float tension_, float friction_ = ForcesComponent::baseFriction);
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
    ChainRenderComponent(const glm::vec4 color_,ChainLinkRender renderFunc_, RenderCamera* camera, Entity& entity);
    void update();
};

struct Chain
{
    LinkPtr top,bottom;
    Chain(ChainLinkRender renderFunc_, const glm::vec4& color1, const glm::vec4& color2, float linkDist_, float tension_, int length, glm::vec2 start, float angle = 0, float friction = ForcesComponent::baseFriction); //color1 and color2: forms a color gradient along the chain, can be left to glm::vec4(0) if you don't care about color
    void setBottomPos(const glm::vec2& pos);
    void update();
};

#endif // PHYSICS_H_INCLUDED
