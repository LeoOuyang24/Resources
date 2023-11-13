#include "physics.h"
#include "vanilla.h"

void ForcePart::setForce(const ForceVector& force_)
{
    force = force_;
    float length = glm::length(force);
    if (length > maxMag) //make sure "force" can not be too high
    {
        force = maxMag/length*force; //basically run glm::normalize but without recalculating the length (square roots expensive apparently)
    }
    force.x = abs(force.x) <= .00001 ? 0 : force.x; //if either force component is too small, round down to 0
    force.y = abs(force.y) <= .00001 ? 0 : force.y;
}

void ForcePart::addForce(const ForceVector& force_)
{
    setForce(force + force_);
}

void ForcePart::applyFriction(unsigned int instances)
{
    setForce((float)pow(friction,instances)*force);

}

ForcesComponent::ForcesComponent(Entity& entity, float friction_, float maxForce_) : Component(entity), ComponentContainer<ForcesComponent>(&entity),
                                                                                        finalForce({maxForce_,friction_})
{

}

bool ForcesComponent::getBeingPushed()
{
    return finalForce.force.x != 0 || finalForce.force.y != 0;
}

void ForcesComponent::addForce(ForceVector force)
{
    finalForce.addForce(force);
}

void ForcesComponent::update()
{
    BasicMoveComponent* move = entity->getComponent<BasicMoveComponent>();
    if (getBeingPushed() && move)
    {
        float frictionFactor = DeltaTime::deltaTime; //fraction of the friction since last frame
        if (finalForce.friction != 1)
        {
            frictionFactor = (float)(1.0f - pow(finalForce.friction,DeltaTime::deltaTime))/(1.0f - finalForce.friction); //if friction is 1, we set frictionFactor to DeltaTime::deltaTime as that is the amount of friction since last frame.
                                                                                                    //otherwise, use formula of finite geometric series to calculate
        }
        move->addMoveVec(frictionFactor*finalForce.force); //apply finalForce as if it was applied once a millisecond, with friction applied once a millisecond
        //printRect(glm::vec4(move->getPos(),finalForce));
        finalForce.applyFriction(DeltaTime::deltaTime);//lower friction based on time passed
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
    if (BasicMoveComponent* move = entity->getComponent<BasicMoveComponent>())
    {
        return move->getCenter();
    }
}

void ChainLinkComponent::update()
{
    if (BasicMoveComponent* move = entity->getComponent<BasicMoveComponent>())
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
                    move->setPos(downPoint - disp);
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

            if (BasicMoveComponent* move = link->getEntity().getComponent<BasicMoveComponent>())
                {
                    move->setPos(pos);
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
