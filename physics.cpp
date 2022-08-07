#include "physics.h"
#include "vanilla.h"

void ForcesComponent::applyForce(ForceVector force)
{
    finalForce += force;
}

ForcesComponent::ForcesComponent(Entity& entity, float friction_) : Component(entity), ComponentContainer<ForcesComponent>(&entity), move(entity.getComponent<MoveComponent>()),friction(friction_)
{

}

void ForcesComponent::setMoveComponent(MoveComponent* move_)
{
    move = move_;
}

bool ForcesComponent::getBeingPushed()
{
    return finalForce.x != 0 || finalForce.y != 0;
}

void ForcesComponent::addForce(ForceVector force)
{
    applyForce(force); //we can save some time by automatically applying one-time forces
    if (glm::length(force) > 10)
    {
        force = glm::normalize(force)*10.0f;
    }
}

void ForcesComponent::update()
{
    if (getBeingPushed() && move)
    {
        move->setPos(move->getPos() + finalForce);
        //printRect(glm::vec4(move->getPos(),finalForce));
        finalForce = friction*finalForce;

        finalForce.x = abs(finalForce.x) <= .00001 ? 0 : finalForce.x;
        finalForce.y = abs(finalForce.y) <= .00001 ? 0 : finalForce.y;
       // forces.clear();
    }

}

ChainLinkComponent::ChainLinkComponent(Entity& entity, const LinkPtr& down_, float linkDist_, float tension_,float friction_) :
                                        ForcesComponent(entity,friction_), ComponentContainer<ChainLinkComponent>(entity), down(down_), linkDist(linkDist_), tension(tension_)
{

}

void ChainLinkComponent::setLink(const LinkPtr& link)
{
    down = link;
}

ChainLinkComponent* ChainLinkComponent::getNextLink()
{
    if (down.get())
    {
        return down->getComponent<ChainLinkComponent>();
    }
    return nullptr;
}

glm::vec2 ChainLinkComponent::getLinkPos()
{
    if (move)
    {
        return move->getCenter();
    }
}

void ChainLinkComponent::update()
{
    if (move)
    {
        ForcesComponent::update();
        auto downPtr = down.get();
        bool print = false;
        if (downPtr)
        {
            if (ChainLinkComponent* downLink = downPtr->getComponent<ChainLinkComponent>())
            {
                glm::vec2 ourPoint = getLinkPos();
                glm::vec2 downPoint = downLink->getLinkPos();
                glm::vec2 chainForce = tension*(downPoint - ourPoint);
                addForce(chainForce);
                downLink->addForce(-1.0f*chainForce);

                if (pointDistance(downPoint,ourPoint) > linkDist)
                {
                    glm::vec2 disp = linkDist*glm::normalize(downPoint - ourPoint);
                    move->teleport(downPoint - disp);
                }
            }
        }
        //if (print)
          //  printRect(glm::vec4(oldForce,finalForce));
    }
}

ChainRenderComponent::ChainRenderComponent(const glm::vec4 color_,ChainLinkRender renderFunc_, Entity& entity) :
                                                                                RenderComponent(entity),
                                                                                ComponentContainer<ChainRenderComponent>(entity),
                                                                                renderFunc(renderFunc_),
                                                                                color(color_)
{

}

void ChainRenderComponent::update()
{
    if (renderFunc)
    {
        if (ChainLinkComponent* link = entity->getComponent<ChainLinkComponent>())
        {
            renderFunc(*this,*link);
        }
    }
}

Chain::Chain(ChainLinkRender renderFunc_, const glm::vec4& color1, const glm::vec4& color2,  float linkDist_,  int length, glm::vec2 start, float tension_, float angle, float friction_)
{
    //color1 to color2 is a gradient along the chain, linkDist is the max distance between links, tension is the tension of the chain,
    glm::vec2 tilt = glm::vec2(cos(angle),-sin(angle))*linkDist_;
    LinkPtr prev;
    glm::vec4 gradient = color2 - color1;
    for (int i = 0; i < length; ++i)
    {
        LinkPtr link = std::shared_ptr<Entity>(new Entity());
        glm::vec2 spawn = start + (float)i*tilt;
        link->addComponent(*(new MoveComponent(0,glm::vec4(spawn,1,1),*link.get())));
        link->addComponent(*(new ChainLinkComponent(*link.get(),prev,linkDist_,tension_,friction_)));
        link->addComponent(*(new ChainRenderComponent(color1 + (float)i/length*gradient,renderFunc_,*link.get())));
        prev = link;
        if (i == 0)
        {
            bottom= link;
        }
        else if (i == length - 1)
        {
            top = link;
        }
        //links.push_back(link);
    }
}

void Chain::setBottomPos(const glm::vec2& pos)
{
    if (ChainLinkComponent* link = bottom->getComponent<ChainLinkComponent>())
        {

            if (MoveComponent* move = link->getMove())
                {
                    move->teleport(pos);
                }
        }
}

void Chain::update()
{
    apply([](ChainLinkComponent& link){
        link.getEntity().update();
        });
}

void Chain::collide(Entity& entity)
{
    apply([entity](ChainLinkComponent& link) mutable {
            link.collide(entity);
        });
}
